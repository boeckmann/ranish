#define _PART_C

#include "part.h"

int num_disks;
int hd, select_target, mode, quiet;

void main(int argc, char **argv)
{
    int i;
    unsigned cyls,heads,sects;
    hd = 0x80;

    set_messages();

    get_base_dir(argv[0]);

    argc--;
    argv++;

    while (argc != 0 && (argv[0][0] == '-' || argv[0][0] == '/')) {
        if (argv[0][1] == 'D' || argv[0][1] == 'd' && argc > 1) {
            i = atoi(argv[1]);
            if (i < 1 || i > 16)
                usage();
            hd += i - 1;
            argc -= 2;
            argv += 2;
        }
        else if (argv[0][1] == 'G' || argv[0][1] == 'g' && argc > 1) {
            if (sscanf(argv[1], "%u,%u,%u", &cyls, &heads, &sects) != 3 ||
                heads > 255 || sects > 63) {
                fprintf(stderr,
                    "Invalid CHS geometry given.\n");
                exit(1);
            }
            force_num_sects = sects;
            force_num_heads = heads;
            force_num_cyls = cyls;
            argc -= 2;
            argv += 2;
        }
        else if (argv[0][1] == 'Q' || argv[0][1] == 'q') {
            quiet = 1;
            argc--;
            argv++;
        } 
        else if (argv[0][1] == 'X' || argv[0][1] == 'x') {
            lba_enabled = 0;
            argc--;
            argv++;
        }
        else
            break;
    }

    screen_init();
    diskio_init();

    if (detected_os == SYS_WIN_NT) {
        fprintf(stderr,
                "This program will not run under Windows NT.\n"
                "Run it under DOS or Windows 95/98\n"
                "For more help visit " WEBSITE_URL "\n");
        exit(1);
    }

    if (argc == 0) {
        interactive = 1;
        start_gui();
    } else {
        interactive = 0;
        command_line(argc, argv);
    }

    diskio_exit();
    screen_exit();
    exit(0);
} /* main */

void start_gui(void)
{
    static struct part_long pseudo_part;
    struct part_long *p = &pseudo_part;

    show_mouse();
    move_mouse(80, 25);

    while (1) {
        border_window(BORDER_COLOR, 1, 1, 80, 25, MAIN_BORDER);
        write_string(TITLE_COLOR, 4, 2, PROGRAM_TITLE);
        border_window(BORDER_COLOR, 2, 3, 78, 2, HRZN_BORDER);

        write_string(HEADER_COLOR, 4, 4, HEADER_GET);
        write_int(TABLE_COLOR, 14, 4, 2, hd - 0x80 + 1);

        if ((num_disks = get_disk_info(hd, &dinfo, buf)) == -1) {
            show_error(ERROR_DISK_INFO);
            if (hd != 0x80) {
                hd = 0x80;
                continue;
            }
            break;
        }

        if (dinfo.sect_size != SECT_SIZE) {
            show_error("This drive has no 512 bytes per sector! Quitting...");
            break;
        }

        p->level           = 0;
        p->empty           = 0;
        p->valid           = 1;
        p->inh_changed     = 0;
        p->inh_invalid     = 0;
        p->inconsist_err   = 0;
        p->boot_record_err = 0;

        p->container      = 0;
        p->container_base = 0;

        p->start_cyl  = 0;
        p->end_cyl    = dinfo.num_cyls - 1;
        p->start_head = 0;
        p->end_head   = dinfo.num_heads - 1;
        p->start_sect = 1;
        p->end_sect   = dinfo.num_sects;

        p->rel_sect = 0;
        p->num_sect = dinfo.total_sects;

        select_target = 0;

        setup_mbr(p);

        if (select_target == 0)
            break;

        hd = (hd - 0x80 + 1) % num_disks + 0x80;
    }

    clear_window(White + BakBlack, 1, 1, 80, 25);
    move_cursor(1, 1);
    hide_mouse();
} /* start_gui */


int setup_mbr(struct part_long *p)
{
    int i, j, n, top, row, field, scr_rows, num_rows;
    int old_top, old_row, old_field, preview_mode, mx, my, pressed;
    int valid, hint_y, view, ext_fix;
    int force_redraw_header, force_redraw_menu, force_redraw_table;
    int force_initialize, force_recalculate, force_adv_adjust;
    int force_edit_menu, act_menu_sav = 0, force_special_copy = 1;
    unsigned long *edit_target, edit_limit, empty_start;
    char *hint = 0, *mesg = 0, *warn = 0, *help = 0, *data, *data_orig;
    unsigned short ipl_chksum;

    int data_size;
    struct mbr *mbr;
    struct adv *adv;
    struct part_long *part;

    static struct field_desc
    {
        unsigned long edit_limit;
        int pos;
        int len;
    } *fd,
        fd_chs[8] = {{0, 4, 8},
                     {0, 13, 23},
                     {8555167, 34, 7},
                     {255, 42, 3},
                     {63, 46, 3},
                     {8555167, 50, 7},
                     {255, 58, 3},
                     {63, 62, 3}},
        fd_lba[4] = {
            {0, 4, 8}, {0, 13, 23}, {4294967295, 34, 10}, {4294967295, 45, 10}};

    view      = (p->level == 0) ? VIEW_MBR : VIEW_EMBR;
    num_rows  = (p->level == 0) ? MAX_PART_ROWS : 4;
    data_size = (p->level == 0) ? SECT_SIZE + ADV_DATA_SIZE : SECT_SIZE;

    if (dinfo.lba) {
        mode = MODE_LBA;
    }

    if ((data = malloc(2 * data_size)) == 0) {
        show_error(ERROR_MALLOC);
        return FAILED;
    }
    if ((part = malloc(sizeof(struct part_long) * num_rows)) == 0) {
        show_error(ERROR_MALLOC);
        free(data);
        return FAILED;
    }

    memset(data, 0, 2 * data_size);

    data_orig = data + data_size;

    mbr = (struct mbr *)(data);
    adv = (struct adv *)(data + SECT_SIZE);

    if (disk_read_rel(p, 0, mbr, 1) == -1) {
        show_error(ERROR_READ_MBR);
        free(data);
        free(part);
        return FAILED;
    }

    if (p->level == 0 &&
        (mbr->x.adv.adv_mbr_magic == ADV_MBR_MAGIC ||
         strcmp(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE) == 0)) {
        if (disk_read_rel(p, mbr->x.adv.rel_sect, adv, ADV_DATA_SECT) == -1) {
            show_error(ERROR_READ_ADV);
        }
        if (strcmp(adv->signature, ADV_DATA_SIGNATURE) == 0) {
            view         = VIEW_ADV;
            act_menu_sav = mbr->x.adv.act_menu;
        } else
            show_error(ERROR_ADV_BAD);
    }

    if (mbr->x.new.bm_sign == BM_238_SIGN) {
        show_error("This version is incompatible with non-public "
            "domain versions 2.38-2.4X.");
    }

    memmove(data_orig, data, data_size);

    top   = 0;
    row   = 0;
    field = 0;

    pressed           = 0;
    preview_mode      = 0;
    force_edit_menu   = 0;
    force_initialize  = 1;
    force_adv_adjust  = 0;
    force_recalculate = 0;

    if (p->level == 0 && dinfo.num_cyls == dinfo.bios_num_cyls + 1) {
        sprintf(tmp3, MESG_BIOS_CYL, dinfo.bios_num_cyls);
        mesg = tmp3;
    }

    if (view == VIEW_ADV) {
        ipl_chksum = calc_chksum((unsigned short*)mbr->x.adv.code, sizeof(mbr->x.adv.code) / 2);

        if (ipl_chksum != ADV_IPL_CHKSUM_MAGIC) {
            sprintf(tmp3, "ADV IPL code version mismatch or corrupted. "
                "Press F2 to reinstall!");
            show_error(tmp3);
        }
    }

    if (p->inh_invalid || p->inh_changed) {
        mesg = TEXT("One or more parent MBR not saved or invalid! Changes can not be saved.");
    }

    while (1) {
        help = 0;

        if (top != old_top || row != old_row || field != old_field) {
            old_top            = top;
            old_row            = row;
            old_field          = field;
            force_redraw_table = 1;
        }

        if (force_initialize == 1) {
            if (view == VIEW_ADV) {
                if (adv->version > ADV_DATA_VERSION) {
                    show_error(ERROR_ADV_VER);
                }
                if (adv->version < ADV_DATA_VERSION) {
                    convert_adv_data(adv);
                    force_special_copy = 1;
                }

                hint_y    = 18;
                scr_rows  = 8;
                num_rows  = MAX_PART_ROWS;
                data_size = SECT_SIZE + ADV_DATA_SIZE;
                unpack_adv_part_tab(adv->part, part, num_rows, p);
            } else {
                top      = 0;
                hint_y   = 14;
                num_rows = 4;
                scr_rows = 4;
                if (row >= 4)
                    row = 0;
                data_size = SECT_SIZE;
                warn = unpack_part_tab(mbr->part_rec, part, num_rows, p);
            }

            valid = validate_table(part, num_rows, p);

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            force_adv_adjust    = 1;
            force_initialize    = 0;
        }

        if (force_redraw_menu == 1) {
            redraw_menu(view);
            force_redraw_menu = 0;
        }

        if (force_redraw_header == 1) {
            redraw_header(view, mode, p);
            force_redraw_header = 0;
            force_redraw_table  = 1;
        }

        if (force_recalculate == 1) {
            recalculate_part(&part[row], mode);
            valid = validate_table(part, num_rows, p);

            force_redraw_table = 1;
            force_adv_adjust   = 1;
            force_recalculate  = 0;
        }

        if (view == VIEW_ADV && force_adv_adjust == 1) {
            struct part_long tmp;
            int cur_row_was_unused;

            cur_row_was_unused = (part[row].os_id == 0) ? 1 : 0;

            for (i = 0; i < num_rows; i++)
                if (part[i].os_id == OS_ADV ||
                    part[i].os_id == 0 && i == num_rows - 1) {
                    if (part[i].os_id == OS_ADV &&
                        part[i].num_sect >= ADV_NUM_SECTS)
                        break;
                    part[i].os_id    = OS_ADV;
                    part[i].num_sect = ADV_NUM_SECTS;
                    determine_os_num(&part[i]);
                    recalculate_part(&part[i], MODE_LBA);
                    break;
                }

            while (1) /* Bubble sort all USED rows */
            {
                n = 0;
                for (i = 0; i < num_rows - 1; i++)
                    if (part[i + 1].os_id != 0)
                        if (part[i].rel_sect > part[i + 1].rel_sect) {
                            tmp         = part[i];
                            part[i]     = part[i + 1];
                            part[i + 1] = tmp;
                            if (row == i)
                                row++;
                            else if (row == i + 1)
                                row--;
                            n = 1;
                        } else if (part[i].os_id == 0) {
                            tmp         = part[i];
                            part[i]     = part[i + 1];
                            part[i + 1] = tmp;
                            if (row == i + 1)
                                row--;
                            n = 1;
                        }
                if (n == 0)
                    break;
            }

            while (row > 0 && part[row - 1].os_id == 0)
                row--;

            empty_start = 1; /* fill unused space */
            for (i = 0; i < num_rows && part[i].os_id != 0; i++)
                if (part[i].rel_sect > empty_start) /* add empty partition */
                {
                    tmp = part[num_rows - 1];
                    if (tmp.os_id != 0) {
                        warn = ERROR_NO_UNUSED;
                        break;
                    }
                    for (j = num_rows - 1; j > i; j--)
                        part[j] = part[j - 1];
                    part[i]          = tmp;
                    part[i].rel_sect = empty_start;
                    part[i].num_sect = part[i + 1].rel_sect - empty_start;
                    recalculate_part(&part[i], MODE_LBA);
                    empty_start = part[i + 1].rel_sect;
                    if (row > i || row == i && !cur_row_was_unused)
                        row++;
                } else
                    empty_start =
                        max(empty_start, part[i].rel_sect + part[i].num_sect);

            if (empty_start < p->num_sect && i != num_rows) {
                part[i].rel_sect = empty_start;
                part[i].num_sect = p->num_sect - empty_start;
                recalculate_part(&part[i], MODE_LBA);
                i++;
            }

            for (; i < num_rows; i++)
                if (part[i].os_id == 0) {
                    part[i].rel_sect = 0;
                    part[i].num_sect = 0;
                    recalculate_part(&part[i], MODE_LBA);
                }

            valid = validate_table(part, num_rows, p);

            for (i = 0; i < num_rows; i++)
                if (part[i].os_id == OS_ADV) {
                    if (part[i].num_sect < ADV_NUM_SECTS) {
                        part[i].mbr_err = 1;
                        part[i].valid   = 0;
                        valid           = 0;
                    }
                    break;
                }

            if (row < top || row >= top + scr_rows)
                top = row;
            if (top + scr_rows > num_rows)
                top = num_rows - scr_rows;

            old_top = top;
            old_row = row;

            force_adv_adjust = 0;
        }

        if (force_redraw_table == 1) {
            for (i = 0; i < scr_rows; i++) {
                sprintf_partrec(tmp, &part[top + i], top + i + 1, view);
                tmp[76] = 0;
                if (part[top + i].empty || part[top + i].valid) {
                    if (part[top + i].os_id != 0)
                        write_string(TABLE_COLOR, 3, i + 9, tmp);
                    else
                        write_string(HEADER_COLOR, 3, i + 9, tmp);
                } else
                    write_string(INVAL_COLOR, 3, i + 9, tmp);
            }

            if (top > 0) {
                int color = HINT_COLOR;
                for (i = 0; i < top; i++)
                    if (!part[i].empty && !part[i].valid)
                        color = INVAL_COLOR;
                write_string(color, 73, 8, "more ");
            } else
                write_string(HINT_COLOR, 73, 8, "      ");

            if (top + scr_rows < num_rows) {
                int color = HINT_COLOR;
                for (i = top + scr_rows; i < num_rows; i++)
                    if (!part[i].empty && !part[i].valid)
                        color = INVAL_COLOR;
                write_string(color, 73, hint_y - 1, "more ");
            } else
                write_string(HINT_COLOR, 73, hint_y - 1, "      ");

            write_string(INVAL_COLOR, 64, hint_y + 1, "              ");
            write_string(INVAL_COLOR, 67, hint_y + 2, "           ");
            write_string(INVAL_COLOR, 70, hint_y + 3, "        ");
            write_string(INVAL_COLOR, 73, hint_y + 4, "     ");

            if (part[row].empty == 0) {
                if (part[row].inconsist_err)
                    write_string(INVAL_COLOR, 64, hint_y + 1, ERR_INCONS);
                else if (part[row].boot_record_err)
                    write_string(INVAL_COLOR, 64, hint_y + 1, ERR_BOOTREC);
                if (part[row].overlap_err)
                    write_string(INVAL_COLOR, 67, hint_y + 2, ERR_OVERLAP);
                if (part[row].range_err)
                    write_string(INVAL_COLOR, 70, hint_y + 3, ERR_RANGE);
                if (part[row].mbr_err)
                    write_string(INVAL_COLOR, 73, hint_y + 4, ERR_MBR);
            }

            force_redraw_table = 0;
        }

        if (view == VIEW_ADV)
            pack_adv_part_tab(part, adv->part, num_rows);
        else
            pack_part_tab(part, mbr->part_rec, num_rows);

        if (force_special_copy == 1) {
            memmove(data_orig, data, data_size);
            force_special_copy = 0;
        }

        p->changed = (memcmp(data, data_orig, data_size) == 0) ? 0 : 1;

        if (p->changed == 0)
            write_string(HINT_COLOR, 15, 24, "F2");
        if (p->changed == 1)
            write_string(Blink + HINT_COLOR, 15, 24, "F2");

        if (field == 0) /* Active: Yes/No */
        {
            if (view == VIEW_ADV) {
                write_string(ACTIVE_COLOR,
                             5,
                             row - top + 9,
                             (part[row].active) ? " Menu " : "  No  ");
                move_cursor((part[row].active) ? 6 : 7, row - top + 9);
                hint = HINT_ADV;
            } else {
                write_string(ACTIVE_COLOR,
                             5,
                             row - top + 9,
                             (part[row].active) ? "  Yes " : "  No  ");
                move_cursor(7, row - top + 9);
                hint = HINT_INS;
            }
        } else if (field == 1) /* Partition Type */
        {
            write_string(ACTIVE_COLOR,
                         11,
                         row - top + 9,
                         sprintf_os_name(tmp, &part[row]));
            move_cursor(11, row - top + 9);
            hint = (view == VIEW_ADV) ? HINT_ADV : HINT_INS;
        } else if (mode == MODE_CHS && field == 2)
            edit_target = &part[row].start_cyl;
        else if (mode == MODE_CHS && field == 3)
            edit_target = &part[row].start_head;
        else if (mode == MODE_CHS && field == 4)
            edit_target = &part[row].start_sect;
        else if (mode == MODE_CHS && field == 5)
            edit_target = &part[row].end_cyl;
        else if (mode == MODE_CHS && field == 6)
            edit_target = &part[row].end_head;
        else if (mode == MODE_CHS && field == 7)
            edit_target = &part[row].end_sect;
        else if (mode == MODE_LBA && field == 2)
            edit_target = &part[row].rel_sect;
        else if (mode == MODE_LBA && field == 3)
            edit_target = &part[row].num_sect;

        ext_fix = (p->level > 1 && part[row].os_id == OS_EXT);

        if (field > 1) {
            fd         = (mode == MODE_CHS) ? &fd_chs[field] : &fd_lba[field];
            edit_limit = fd->edit_limit;

            write_int(
                ACTIVE_COLOR, fd->pos, row - top + 9, fd->len, *edit_target);
            move_cursor(fd->pos + fd->len - 1, row - top + 9);

            if (mode == MODE_CHS)
                sprintf(tmp,
                        HINT_CHS,
                        ext_fix ? p->end_cyl + 1
                                : part[row].container->start_cyl,
                        part[row].container->end_cyl,
                        dinfo.num_heads - 1,
                        dinfo.num_sects);
            else
                sprintf(tmp,
                        HINT_LBA,
                        ext_fix ? p->rel_sect + p->num_sect : 1,
                        part[row].container->num_sect - 1,
                        part[row].container->num_sect - 1 -
                            (ext_fix ? p->rel_sect + p->num_sect - 1 : 0));

            hint = tmp;
        }

        clear_window(BORDER_COLOR, 2, hint_y, 78, 1);

        if (warn != 0)
            write_string(WARN_COLOR, 7, hint_y, warn);
        else if (mesg != 0)
            write_string(MESG_COLOR, 7, hint_y, mesg);
        else if (hint != 0)
            write_string(HINT_COLOR, 4, hint_y, hint);

        if (view == VIEW_EMBR)
            help = "#extended";
        if (!valid)
            help = "#conflict";

        for (i = 0; i < num_rows; i++) {
            part[i].inh_changed = (p->changed || p->inh_changed) ? 1 : 0;
            part[i].inh_invalid = (!part[i].valid || p->inh_invalid) ? 1 : 0;
        }

        if (force_edit_menu == 1) {
            edit_boot_menu(adv, 0, 1);
            force_initialize = 1;
            force_edit_menu  = 0;
            continue;
        }

        mx = 0;
        my = 0;

        while (1) {
            get_event(&ev, EV_KEY | EV_MOUSE);

            if (ev.ev_type & EV_KEY)
                break;
            if (ev.ev_type & EV_MOUSE) {
                /*
                            sprintf(tmp,"y=%2d  x=%2d",ev.y, ev.x);
                            write_string(BrCyan+BakBlue,68,23,tmp);
                */
                if (pressed == 0 && ev.left == 1) {
                    pressed = 1;
                    mx      = ev.x;
                    my      = ev.y;
                    break;
                }
                if (pressed == 1 && ev.left == 0) {
                    pressed = 0;
                }
            }
        } /* while - get event */

#define CLICK(y, x1, x2) ((mx) >= (x1) && (mx) <= (x2) && (my) == (y))

        if (hint == HINT_INS || hint == HINT_ADV) {
            if (CLICK(hint_y, 7, 30))
                ev.key = 0x20; /* Space */
            if (CLICK(hint_y, 32, 57))
                ev.scan = 0x5200; /* Ins */
            if (CLICK(hint_y, 59, 74))
                ev.key = 0x0D; /* Enter */
        }

        hint = mesg = warn = 0;

        if (ev.scan == 0x6700) /* Ctrl-F10 - Unconditional Exit */
        {
            exit(1);
        }

        if (preview_mode == 1 && (ev.key == 27 || CLICK(24, 68, 77))) {
            view             = VIEW_ADV;
            preview_mode     = 0;
            force_initialize = 1;
            continue;
        }

        if (ev.key == 27 || CLICK(24, 68, 77)) /* ESC - Quit */
        {
            if (p->changed == 0)
                break;
            mesg = MESG_NOT_SAVED;
            continue;
        }

        if (CLICK(2, 3, 78)) {
            html_view("#contact");
            continue;
        }

        if ((ev.ev_type & (EV_MOUSE | EV_TIMER)) && my >= 9 &&
            my < 9 + scr_rows) {
            int n = (mode == MODE_CHS) ? 8 : 4;

            fd = (mode == MODE_CHS) ? fd_chs : fd_lba;

            for (i = 0; i < n; i++)
                if (mx >= fd[i].pos && mx < fd[i].pos + fd[i].len) {
                    row   = my - 9 + top;
                    field = i;
                    break;
                }
            continue;
        }

        if (!valid &&
            (CLICK(hint_y + 1, 64, 77) || CLICK(hint_y + 2, 66, 77) ||
             CLICK(hint_y + 3, 68, 77) || CLICK(hint_y + 4, 70, 77))) {
            html_view("#conflict");
            continue;
        }

        if (view == VIEW_EMBR &&
            (ev.scan == 0x49E0 || ev.scan == 0x4900)) /* PgUp */
        {
            if (p->changed == 0)
                break;
            continue;
        }

        if (view != VIEW_ADV &&
            (ev.scan == 0x51E0 || ev.scan == 0x5100)) /* PgDn */
        {
            for (i = 0; i < num_rows; i++)
                if (part[i].os_id == OS_EXT)
                    break;

            if (i != num_rows) {
                os_desc[part[i].os_num].setup(&part[i]);
                force_redraw_header = 1;
                force_redraw_table  = 1;
                force_redraw_menu   = 1;
            }
            continue;
        }

        if ((field == 0 || field == 1) && ev.key == 0x0D) /* Enter - Setup */
        {
            if (part[row].empty)
                continue;
            if (part[row].os_id == 0)
                continue;

            if (part[row].os_id == OS_ADV) {
                edit_boot_menu(adv, row, 1);
            } else {
                if (os_desc[part[row].os_num].setup == 0) {
                    show_error(ERROR_NO_SETUP);
                    continue;
                }

                os_desc[part[row].os_num].setup(&part[row]);
            }

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            continue;
        }

        if (ev.scan == 0x0F09) /* Tab */
        {
            if (field == 0 || field == 1)
                field = 2;
            else if (mode == MODE_CHS && field < 5)
                field = 5;
            else if (mode == MODE_LBA && field < 3)
                field = 3;
            else
                field = 0;
            continue;
        }

        if (field > 1 && ev.key == 0x0D) /* Enter - MoveOn */
        {
            if (mode == MODE_CHS && field < 5)
                field = 5;
            else if (mode == MODE_LBA && field < 3)
                field = 3;
            else
                field = 0;
            continue;
        }

        if (ev.scan == 0x4BE0 || ev.scan == 0x4B00) /* left */
        {
            if (field > 0)
                field--;
            continue;
        }

        if (ev.scan == 0x4DE0 || ev.scan == 0x4D00) /* right */
        {
            if (mode == MODE_CHS && field < 7)
                field++;
            if (mode == MODE_LBA && field < 3)
                field++;
            continue;
        }

        if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (row < num_rows - 1)
                row++;
            if (top + scr_rows <= row)
                top++;
            continue;
        }

        if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (row > 0)
                row--;
            if (top > row)
                top--;
            continue;
        }

        if (ev.scan == 0x4FE0 || ev.scan == 0x4F00) /* End */
        {
            row = num_rows - 1;
            top = row + 1 - scr_rows;
            continue;
        }

        if (ev.scan == 0x47E0 || ev.scan == 0x4700) /* Home */
        {
            row = 0;
            top = 0;
            continue;
        }

        if (view == VIEW_ADV && (ev.scan == 0x49E0 || ev.scan == 0x4900 ||
                                 CLICK(8, 73, 78))) /* PgUp */
        {
            row = (row >= scr_rows) ? row - scr_rows : 0;
            top = (top >= scr_rows) ? top - scr_rows : 0;
            continue;
        }

        if (view == VIEW_ADV && (ev.scan == 0x51E0 || ev.scan == 0x5100 ||
                                 CLICK(hint_y - 1, 73, 78))) /* PgDn */
        {
            row = (row < num_rows - scr_rows) ? row + scr_rows : num_rows - 1;
            top = (top < num_rows - 2 * scr_rows) ? top + scr_rows
                                                  : num_rows - scr_rows;
            continue;
        }

        if (ev.scan == 0x3B00 || CLICK(24, 4, 12)) /* F1 - Help */
        {
            html_view(help);
            continue;
        }

        if (ev.scan == 0x3E00 || CLICK(24, 45, 53)) /* F4 - Mode */
        {
            if (mode == MODE_CHS) {
                mode = MODE_LBA;
                if (field == 2 || field == 3 || field == 4)
                    field = 2;
                if (field == 5 || field == 6 || field == 7)
                    field = 3;
            } else {
                mode = MODE_CHS;
                if (field == 2)
                    field = 2;
                if (field == 3)
                    field = 5;
            }
            force_redraw_header = 1;
            continue;
        }

        if (ev.key == 'F' || ev.key == 'f' || CLICK(20, 11, 29)) /* Format */
        {
            int fstatus;

#define NUM_ARGS 10

            char *argv[NUM_ARGS];

            if (part[row].empty)
                continue;

            if (os_desc[part[row].os_num].format == 0) {
                show_error(ERROR_NO_FORMAT);
                continue;
            }

            if (part[row].inh_invalid) {
                show_error(ERROR_FIX_FIRST);
                continue;
            }

            if (part[row].inh_changed) {
                mesg = MESG_NOT_SAVED;
                continue;
            }

            sprintf(tmp2, "#0x%04X", part[row].os_id);

            if (enter_string(
                    4, hint_y, PROMPT_FORMAT, sizeof(tmp), tmp, tmp2) == 0)
                continue;

            parse_arg(tmp, argv, NUM_ARGS);

            fstatus = os_desc[part[row].os_num].format(&part[row], argv);

            if (fstatus == OK)
                mesg = MESG_FORMAT_OK;
            else if (fstatus == CANCEL)
                mesg = WARN_FORMAT_CANCEL;
            else if (fstatus == FAILED)
                show_error(ERROR_FORMAT_FAILED);

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            continue;
        }

        if (ev.key == 'V' || ev.key == 'v' || CLICK(21, 11, 32)) /* Verify */
        {
            int fstatus;

            if (part[row].empty)
                continue;

            if (part[row].valid == 0) {
                show_error(ERROR_FIX_FIRST);
                continue;
            }

            fstatus = generic_verify(&part[row], 0, 0);

            if (fstatus == OK)
                mesg = MESG_VERIFY_OK;
            else if (fstatus == CANCEL)
                mesg = WARN_VERIFY_CANCEL;
            else if (fstatus == FAILED) {
                show_error(ERROR_VERIFY_FAILED);
                sprintf(tmp, "BIOS error code %02X", (unsigned int)diskio_errno);
                show_error(tmp);
            }

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            continue;
        }

        if (ev.key == 'e' || ev.key == 'E' || CLICK(22, 11, 33))
        {
            int fstatus;

            if (part[row].empty)
                continue;

            if (part[row].valid == 0) {
                show_error(ERROR_FIX_FIRST);
                continue;
            }

            if (part[row].inh_changed) {
                mesg = MESG_NOT_SAVED;
                continue;
            }            

            if (enter_string(
                    4, hint_y, "Type \"yes\" to erase data", sizeof(tmp), tmp, NULL) == 0)
                continue;

            if (strcmp(tmp, "yes") != 0) {
                mesg = TEXT("Erase canceled by user.");
                continue;
            }
            
            fstatus = generic_clean(&part[row]);

            if (fstatus == OK)
                mesg = TEXT("Erase completed.");
            else if (fstatus == CANCEL)
                mesg = TEXT("Erase canceled by user.");
            else if (fstatus == FAILED)
                show_error(TEXT("Erase failed."));

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            continue;
        }
        if (preview_mode == 1) {
            mesg = HINT_RETURN;
            continue;
        }

        if (view == VIEW_ADV &&
            (ev.key == 'P' || ev.key == 'p' || CLICK(22, 11, 38))) {
            preview_mode     = 1;
            view             = VIEW_MBR;
            force_initialize = 1;
            mesg             = HINT_RETURN;
            continue;
        }

        if (view != VIEW_ADV && (ev.key == 'Y' || ev.key == 'y' ||
                                 ev.key == ' ' && part[row].active == 0)) {
            for (i = 0; i < num_rows; i++)
                part[i].active = 0;
            part[row].active   = 1;
            force_redraw_table = 1;
            continue;
        }

        if (view != VIEW_ADV && (ev.key == 'N' || ev.key == 'n' ||
                                 ev.key == ' ' && part[row].active == 1)) {
            part[row].active   = 0;
            force_redraw_table = 1;
            continue;
        }

        if (view == VIEW_ADV && ev.key == ' ') /* Edit Boot Menu */
        {
            edit_boot_menu(adv, row, 0);
            force_initialize = 1;
            continue;
        }

        if (ev.scan == 0x52E0 || ev.scan == 0x5200 ||
            ev.key == 'T' || ev.key == 't')                 /* Insert */
        {
            int prev_os_id = part[row].os_id;

            if (view == VIEW_ADV && part[row].os_id == OS_ADV)
                continue;

            if (view == VIEW_EMBR && row == 1 && part[row].os_id == 0) {
                part[row].os_id = OS_EXT;
                determine_os_num(&part[row]);
            }

            select_system_type(&part[row]);

            if (part[row].level > 2 && part[row].os_id == OS_EXT)
                part[row].container = p->container;
            else
                part[row].container = p;
            part[row].container_base = QUICK_BASE(part[row].container);

            if (view == VIEW_MBR && row == 0 && part[0].empty &&
                part[0].os_id != 0 && part[1].empty && part[2].empty &&
                part[3].empty)
                part[0].active = 1;

            if (part[row].empty && part[row].os_num != 0) {
                if (mode == MODE_CHS) {
                    unsigned long start_head, start_cyl, end_cyl;

                    start_cyl = p->start_cyl;
                    end_cyl   = part[row].container->end_cyl;

                    start_head = (row == 0) ? 1 : 0;

                    if (row > 0 && !part[row - 1].empty) {
                        start_cyl = part[row - 1].end_cyl + 1;
                    }

                    if (part[row].level > 2 && part[row].os_id == OS_EXT) {
                        start_cyl = p->end_cyl + 1;
                    } else if (row < 3 && !part[row + 1].empty) {
                        end_cyl = part[row + 1].start_cyl - 1;
                    }

                    part[row].start_cyl  = start_cyl;
                    part[row].start_head = start_head;
                    part[row].start_sect = 1;
                    part[row].end_cyl    = end_cyl;
                    part[row].end_head   = dinfo.num_heads - 1;
                    part[row].end_sect   = dinfo.num_sects;
                } else {
                    if (row > 0 && !part[row - 1].empty)
                        part[row].rel_sect =
                            part[row - 1].rel_sect + part[row - 1].num_sect;
                    else
                        part[row].rel_sect = dinfo.num_sects;

                    if (row < 3 && !part[row + 1].empty)
                        part[row].num_sect =
                            part[row + 1].rel_sect - part[row].rel_sect;
                    else
                        part[row].num_sect =
                            part[row].container->num_sect - part[row].rel_sect;

                    if (part[row].level > 2 && part[row].os_id == OS_EXT) {
                        part[row].rel_sect = p->rel_sect + p->num_sect;
                        part[row].num_sect =
                            part[row].container->num_sect - part[row].rel_sect;
                    }
                }
                field             = 2;
                force_recalculate = 1;
            }

            if (prev_os_id == 0 && part[row].os_num != 0) {
                field = 2;
            }

            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
            continue;
        } /* insert */

        if (field < 2) {
            /* do nothing -> threfore else will be only for field>=2 (long
             * ints) */
        } else if (ev.key == 8 || ev.scan == 0x53E0 ||
                   ev.scan == 0x5300) /* BkSp/Del */
        {
            *edit_target /= 10;
            force_recalculate = 1;
            continue;
        } else if (ev.key >= '0' && ev.key <= '9') {
            int x = ev.key - '0';

            if (edit_limit > x && (edit_limit - x) / 10 >= *edit_target) {
                *edit_target      = *edit_target * 10 + x;
                force_recalculate = 1;
            }

            continue;
        } else if (mode == MODE_CHS && ev.key == '+') {
            if (*edit_target < edit_limit) {
                (*edit_target)++;
                force_recalculate = 1;
            }
            continue;
        } else if (mode == MODE_CHS && ev.key == '-') {
            if (*edit_target > 0) {
                (*edit_target)--;
                force_recalculate = 1;
            }
            continue;
        } else if (mode == MODE_LBA && ev.key == '+') {
            if (*edit_target < edit_limit) {
                (*edit_target)++;
                force_recalculate = 1;
            }
            continue;
        } else if (mode == MODE_LBA && ev.key == '-') {
            if (*edit_target > 0) {
                (*edit_target)--;
                force_recalculate = 1;
            }
            continue;
        }

        if (ev.scan == 0x53E0 || ev.scan == 0x5300) /* Delete */
        {
            if (view == VIEW_ADV && part[row].os_id == OS_ADV)
                continue;
            
            if (part[row].os_id == 0x0500 || part[row].os_id == 0x0F00 || 
                part[row].os_id == 0x1F00) {
               if (enter_string(
                       4, hint_y, "This will delete all nested partitions. Type \"yes\"", sizeof(tmp), tmp, NULL) == 0)
                   continue;

               if (strcmp(tmp, "yes") != 0) {
                   mesg = TEXT("Deletion canceled by user.");
                   continue;
               }
            }

            part[row].empty      = 1;
            part[row].valid      = 0;
            part[row].os_id      = 0;
            part[row].os_num     = 0;
            part[row].active     = 0;
            part[row].orig_row   = 0;
            part[row].start_cyl  = 0;
            part[row].end_cyl    = 0;
            part[row].start_head = 0;
            part[row].end_head   = 0;
            part[row].start_sect = 0;
            part[row].end_sect   = 0;
            part[row].rel_sect   = 0;
            part[row].num_sect   = 0;
            force_recalculate    = 1;
            force_redraw_table   = 1;
            continue;
        }

        if (ev.scan == 0x3C00 || CLICK(24, 15, 31)) /* F2 - Save */
        {
            if (view == VIEW_ADV) {
                if (!valid) {
                    show_error(ERROR_FIX_FIRST);
                    continue;
                }

                if (!prepare_adv_mbr_for_save(part, mbr, adv)) {
                    show_error(ERROR_NO_ADV);
                    continue;
                }

                mbr->x.adv.act_menu = act_menu_sav;
            } else if (p->inh_invalid || p->inh_changed) {
                show_error(ERROR_INH_INVAL);
                continue;
            }

            disk_lock(dinfo.disk);

            mbr->magic_num = MBR_MAGIC_NUM;

            if (view == VIEW_ADV &&
                (disk_write_rel(p, mbr->x.adv.rel_sect, adv, ADV_DATA_SECT) ==
                     -1 ||
                 disk_write_rel(p,
                                mbr->x.adv.rel_sect + ADV_DATA_SECT,
                                ADV_MANAGER,
                                ADV_CODE_SECT) == -1)) {
                warn = ERROR_SAVE_ADV;
            } else if (disk_write_rel(p, 0, mbr, 1) == -1) {
                warn = ERROR_SAVE_MBR;
            } else {
                mesg = (view == VIEW_ADV) ? MESG_ADV_SAVED : MESG_MBR_SAVED;
                memmove(data_orig, data, data_size);
            }

            disk_unlock(dinfo.disk);
            force_recalculate  = 1;
            force_adv_adjust   = 1;
            force_special_copy = 1;
            continue;
        }

        if (ev.scan == 0x3D00 || CLICK(24, 34, 42)) /* F3 - Undo */
        {
            memmove(data, data_orig, data_size);
            if (view == VIEW_MBR &&
                (mbr->x.adv.adv_mbr_magic == ADV_MBR_MAGIC ||
                 strcmp(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE) == 0)) {
                memmove(data, data_orig, SECT_SIZE + ADV_DATA_SIZE);
                view = VIEW_ADV;
            } else if (view == VIEW_ADV &&
                       mbr->x.adv.adv_mbr_magic != ADV_MBR_MAGIC &&
                       strcmp(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE) !=
                           0) {
                view = VIEW_MBR;
            }

            if (view == VIEW_ADV &&
                strcmp(adv->signature, ADV_DATA_SIGNATURE) != 0) {
                view = VIEW_MBR;
            }

            force_initialize = 1;
            continue;
        }

        if (p->level == 0 &&
            (ev.scan == 0x3F00 || CLICK(24, 56, 64))) /* F5 - Disk */
        {
            if (p->changed) {
                mesg = MESG_NOT_SAVED;
                continue;
            }
            select_target = 1;
            break;
        }

        if (view != VIEW_ADV &&
            (ev.key == 'H' || ev.key == 'h' || CLICK(19, 11, 38))) { /* Hide */
            if (part[row].os_id == 0x0100 || part[row].os_id == 0x0400 ||
                part[row].os_id == 0x0600 || part[row].os_id == 0x0700 ||
                part[row].os_id == 0x0B00 || part[row].os_id == 0x0C00 ||
                part[row].os_id == 0x0E00 || part[row].os_id == 0x0F00)
                part[row].os_id += 0x1000;
            else if (part[row].os_id == 0x1100 || part[row].os_id == 0x1400 ||
                     part[row].os_id == 0x1600 || part[row].os_id == 0x1700 ||
                     part[row].os_id == 0x1B00 || part[row].os_id == 0x1C00 ||
                     part[row].os_id == 0x1E00 || part[row].os_id == 0x1F00)
                part[row].os_id -= 0x1000;
            else
                part[row].os_id = OS_HIDDEN;
            determine_os_num(&part[row]);
            force_redraw_table = 1;
            continue;
        }

        if (view == VIEW_MBR && (ev.key == 'U' || ev.key == 'u' ||
                                 CLICK(17, 11, 61))) { /* Uninstall */
            memmove(mbr, STD_IPL, sizeof(mbr->x.std.code));
            mesg = MESG_UNINSTALL;
            continue;
        }

        if (view == VIEW_ADV && (ev.key == 'U' || ev.key == 'u' ||
                                 CLICK(22, 42, 68))) { /* Uninstall Advanced */
            int n, x;

            unpack_adv_part_tab(adv->part, part, MAX_PART_ROWS, p);
            force_initialize = 1;

            for (i = 0, n = 0; i < MAX_PART_ROWS; i++)
                if (part[i].os_id != 0 && part[i].os_id != OS_ADV)
                    n++;

            if (n > 4) {
                show_error(ERROR_GR_FOUR);
                continue;
            }

            for (i = 0, n = 0, x = dinfo.disk; i < MAX_PART_ROWS; i++)
                if (part[i].os_id != 0 && part[i].os_id != OS_ADV) {
                    if (i == n) {
                        n++;
                        continue;
                    }
                    if (part[i].active) {
                        part[i].active = x;
                        x              = 0;
                    }
                    part[n++] = part[i];
                }

            if (x != 0 && n != 0 && part[0].os_id != 0)
                part[0].active = x;

            if (n < 4)
                memset(part + n, 0, (4 - n) * sizeof(struct part_long));

            for (x = 0; x < n; x++)
                for (i = 0; i < 4; i++)
                    if (part[i].os_id != 0 && part[i].orig_row != 0 &&
                        part[i].orig_row != i) {
                        int r   = part[i].orig_row - 1;
                        part[4] = part[r];
                        part[r] = part[i];
                        part[i] = part[4];
                    }

            pack_part_tab(part, mbr->part_rec, 4);

            if (mbr->x.adv.adv_mbr_magic == ADV_MBR_MAGIC ||
                strcmp(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE) == 0) {
                memset(mbr, 0, 0x1BE);
                memmove(mbr, STD_IPL, sizeof(mbr->x.std.code));
            }

            mesg             = MESG_UNINSTALL;
            force_initialize = 1;
            view             = VIEW_MBR;
            continue;
        }

        if (view == VIEW_EMBR &&
            (ev.key == 'I' || ev.key == 'i' ||
             CLICK(16, 11, 60))) { /* Install dummy boot rec into EMBR */
            memmove(mbr->x.std.code, EMP_IPL, EMP_SIZE);
            strncpy(mbr->x.std.code + EMP_SIZE,
                    MESG_EXT_NONBOOT,
                    sizeof(mbr->x.std.code) - EMP_SIZE);
            mesg = MESG_INSTALL;
        } else if (view == VIEW_EMBR &&
                   (ev.key == 'R' || ev.key == 'r' ||
                    CLICK(17, 11, 60))) { /* Read how to make ext bootable */
            html_view("#make_primary");
        } else if (view == VIEW_MBR &&
                   (ev.key == 'A' || ev.key == 'a' ||
                    CLICK(16, 49, 61))) { /* Install Advanced */
            int m;
            struct adv_part_rec tmp_part_rec;

            if (!valid) {
                show_error(ERROR_FIX_FIRST);
                continue;
            }

            memset(adv, 0, ADV_DATA_SIZE);

            strcpy(adv->signature, ADV_DATA_SIGNATURE);
            adv->version = ADV_DATA_VERSION;
            strncpy(adv->adv_title, MANAGER_TITLE, sizeof(adv->adv_title));

            m = 0;

            if (dinfo.disk == 0x80) {
                adv->menu[m].boot_type = MENU_BOOT_FLOPPY;
                sprintf(adv->menu[m].name, "Boot from floppy");
                m++;
            }

            for (i = 0, n = 0; i < 4;
                 i++) /* copying all non-empty partitions */
                if (part[i].os_id != 0) {
                    if (part[i].os_id == 0x1100 || /* FAT-12     */
                        part[i].os_id == 0x1400 || /* FAT-16     */
                        part[i].os_id == 0x1600 || /* FAT-16 BIG */
                        part[i].os_id == 0x1700 || /* NTFS       */
                        part[i].os_id == 0x1B00 || /* FAT-32     */
                        part[i].os_id == 0x1C00 || /* FAT-32 LBA */
                        part[i].os_id == 0x1E00)   /* FAT-16 LBA */
                    {
                        part[i].os_id -=
                            0x1000; /* Unhiding hidden partitions */
                        determine_os_num(&part[i]);
                    }

                    adv->part[n].os_id    = part[i].os_id;
                    adv->part[n].orig_row = i + 1;
                    adv->part[n].rel_sect = part[i].rel_sect;
                    adv->part[n].num_sect = part[i].num_sect;
                    adv->part[n].tag      = n + 1;
                    if (adv->menu[0].x.part.tag == 0 &&
                        (part[i].os_id == 0x0100 || /* FAT-12     */
                         part[i].os_id == 0x0400 || /* FAT-16     */
                         part[i].os_id == 0x0600 || /* FAT-16 BIG */
                         part[i].os_id == 0x0700 || /* NTFS       */
                         part[i].os_id == 0x0B00 || /* FAT-32     */
                         part[i].os_id == 0x0C00 || /* FAT-32 LBA */
                         part[i].os_id == 0x0E00))  /* FAT-16 LBA */
                    {
                        adv->menu[0].x.part.tag = n + 1;
                    }
                    if (i > 0 &&
                        (part[i - 1].os_id == 0x8100 || /* Linix */
                         part[i - 1].os_id == 0x8300)   /* Linix ext2fs */
                        && (part[i].os_id >> 8) == 0x82 /* Linix swap */) {
                        adv->part[n].os_id           = 0x8201; /* Linux Swap */
                        adv->part[n].tag             = 0;
                        adv->menu[m - 1].x.part.show = SHOW_NEXT;
                    } else if (part[i].os_id != OS_EXT && /* Extended DOS */
                               part[i].os_id != 0x0A00) /* OS/2 Boot Manager */
                    {
                        adv->menu[m].boot_type  = MENU_BOOT_PART;
                        adv->menu[m].x.part.tag = n + 1;
                        sprintf_os_name(adv->menu[m].name, &part[i]);
                        m++;
                        if (!part[i].empty && part[i].valid &&
                            (part[i].os_id == 0x0600 /* FAT-16 */ ||
                             part[i].os_id == 0x0B00 /* FAT-32 */ ||
                             part[i].os_id == 0x0C00 /* FAT-32 LBA */)) {
                            if (disk_read_rel(&part[i], 0, tmp, 1) != -1) {
                                if (memcmp(tmp + 3, "MSWIN", 5) == 0) {
                                    sprintf(adv->menu[m - 1].name,
                                            "Windows 95");
                                    sprintf(adv->menu[m].name,
                                            "Windows 95 / Menu      (F8)");
                                    sprintf(adv->menu[m + 1].name,
                                            "Windows 95 / Safe mode (F5)");
                                    sprintf(adv->menu[m + 2].name,
                                            "Windows 95 / Command Prompt");
                                    adv->menu[m].boot_type  = MENU_BOOT_PART;
                                    adv->menu[m].x.part.tag = n + 1;
                                    adv->menu[m].x.part.num_keys = 1;
                                    adv->menu[m].x.part.keys[0] =
                                        0x4200; /* F8    */
                                    m++;
                                    adv->menu[m].boot_type  = MENU_BOOT_PART;
                                    adv->menu[m].x.part.tag = n + 1;
                                    adv->menu[m].x.part.num_keys = 1;
                                    adv->menu[m].x.part.keys[0] =
                                        0x3F00; /* F5    */
                                    m++;
                                    adv->menu[m].boot_type  = MENU_BOOT_PART;
                                    adv->menu[m].x.part.tag = n + 1;
                                    adv->menu[m].x.part.num_keys = 3;
                                    adv->menu[m].x.part.keys[0] =
                                        0x4200; /* F8    */
                                    adv->menu[m].x.part.keys[1] =
                                        0x0736; /* 6     */
                                    adv->menu[m].x.part.keys[2] =
                                        0x1C0D; /* Enter */
                                    m++;
                                }
                            }
                        }
                    }
                    n++;
                }

            for (i = 0; i < n - 1; i++) /* sorting them by rel_sect */
                for (j = i + 1; j < n; j++)
                    if (adv->part[j].rel_sect < adv->part[i].rel_sect) {
                        tmp_part_rec = adv->part[i];
                        adv->part[i] = adv->part[j];
                        adv->part[j] = tmp_part_rec;
                    }

            if (num_disks > dinfo.disk - 0x80 + 1) {
                adv->menu[m].boot_type  = MENU_BOOT_NEXT_HD;
                adv->menu[m].x.part.tag = 0;
                sprintf(adv->menu[m].name, "Boot from the next Hard Disk");
                m++;
            }

            if (n > 1 && adv->part[n - 1].os_id == OS_EXT)
                for (i = 0; i < m; i++)
                    if (adv->menu[i].x.part.show == SHOW_NONE) {
                        if (adv->menu[i].boot_type == MENU_BOOT_PART)
                            adv->menu[i].x.part.show = SHOW_LAST;
                        else
                            adv->menu[i].x.part.tag = n - 1 + 1;
                    }

            empty_start = 1;
            for (i = 0; i < n; i++)
                if (empty_start + ADV_NUM_SECTS > adv->part[i].rel_sect) {
                    empty_start =
                        adv->part[i].rel_sect + adv->part[i].num_sect;
                } else
                    break;

            if (empty_start + ADV_NUM_SECTS > dinfo.total_sects) {
                show_error(ERROR_ADV_NOSPACE);
                continue;
            }

            for (j = n - 1; j >= i; j--)
                adv->part[j + 1] = adv->part[j];

            adv->part[i].os_id = OS_ADV;
            adv->part[i].tag   = 0;

            if (empty_start == 1 &&
                (n == 0 || (adv->part[1].rel_sect >= dinfo.num_sects &&
                            ADV_NUM_SECTS <= dinfo.num_sects - 7))) {
                adv->part[i].rel_sect = 7;
                adv->part[i].num_sect = dinfo.num_sects - 7;
            } else {
                adv->part[i].rel_sect = empty_start;
                adv->part[i].num_sect = ADV_NUM_SECTS;
            }

            adv->options |= ADV_OPT_VIR_CHECK;

            if (n != 0)
                force_edit_menu = 1;
            force_initialize = 1;
            view             = VIEW_ADV;
        } else if (view == VIEW_MBR && (ev.key == 'I' || ev.key == 'i' ||
                                        CLICK(16, 11, 47))) { /* Install IPL */
            if (memcmp(mbr, IPL, 0x180) == 0)
                memmove(mbr, IPL, sizeof(mbr->x.ipl.code));
            else {
                memmove(mbr, IPL, sizeof(mbr->x.ipl));
                mesg = MESG_INSTALL;
            }
            setup_ipl(mbr);
            force_redraw_header = 1;
            force_redraw_table  = 1;
            force_redraw_menu   = 1;
        } else if (ev.key == 'S' || ev.key == 's' ||
                   CLICK(20, 42, 64)) /* Save */
        {
            if (enter_string(
                    20, 10, PROMPT_FILE, sizeof(file_name), file_name, 0) == 0)
                continue;
            if (view != VIEW_ADV &&
                    save_to_file(file_name, mbr, SECT_SIZE) == -1 ||
                view == VIEW_ADV &&
                    save_to_file(file_name, adv, ADV_DATA_SIZE) == -1)
                warn = ERROR_SAVE_FILE;
            else
                mesg = (view == VIEW_ADV) ? MESG_ADVFILE_SAVED : MESG_FILE_SAVED;
        } else if (ev.key == 'L' || ev.key == 'l' ||
                   CLICK(21, 42, 66)) /* Load */
        {
            char *zz;
            if (enter_string(
                    20, 10, PROMPT_FILE, sizeof(file_name), file_name, 0) == 0)
                continue;
            if ((zz = malloc(ADV_DATA_SIZE)) == 0) {
                show_error(ERROR_MALLOC);
                continue;
            }
            if (view != VIEW_ADV &&
                    load_from_file(file_name, zz, SECT_SIZE) == -1 ||
                view == VIEW_ADV &&
                    load_from_file(file_name, zz, ADV_DATA_SIZE) == -1) {
                warn = ERROR_LOAD_FILE;
                if (view == VIEW_ADV &&
                    load_from_file(file_name, zz, SECT_SIZE) != -1)
                    warn = "Warning! Specified file is not Advanced Boot "
                           "Manager partition table";
            } else {
                if (view == VIEW_ADV && strcmp(zz, ADV_DATA_SIGNATURE) != 0) {
                    show_error(ERROR_ADV_BAD);
                } else {
                    mesg = (view == VIEW_ADV) ? MESG_ADVFILE_LOADD : MESG_FILE_LOADD;
                    if (view == VIEW_MBR &&
                        strcmp(zz, ADV_DATA_SIGNATURE) == 0) {
                        load_from_file(file_name, zz, ADV_DATA_SIZE);
                        view = VIEW_ADV;
                    }
                    if (view != VIEW_ADV)
                        memmove(mbr, zz, SECT_SIZE);
                    else
                        memmove(adv, zz, ADV_DATA_SIZE);
                    force_initialize = 1;
                }
            }
            free(zz);
        } else if (view != VIEW_ADV && /* Load IPL Code */
                   (ev.key == 'C' || ev.key == 'c' || CLICK(19, 42, 61))) {
            char *zz;
            if (enter_string(
                    20, 10, PROMPT_FILE, sizeof(file_name), file_name, 0) == 0)
                continue;
            if ((zz = malloc(SECT_SIZE)) == 0) {
                show_error(ERROR_MALLOC);
                continue;
            }
            if (load_from_file(file_name, zz, SECT_SIZE) == -1)
                warn = ERROR_LOAD_FILE;
            else {
                memmove(mbr, zz, sizeof(mbr->x.std.code));
                mesg = MESG_INSTALL;
            }
            free(zz);
        }

    } /* while(1) */

    free(part);
    free(data);

    return 0;
} /* setup_mbr */

void command_line(int argc, char **argv)
{
    char ch, *data;
    struct mbr *mbr;
    struct adv *adv;
    struct part_long *p, *part;
    int i, x, view, valid, num_rows;

    if (argv[0][0] == '-' || argv[0][0] == '/')
        argv[0]++;
    else
        usage();

    if (_stricmp(argv[0], "Reboot") == 0)
        reboot();

    for (i = 0; (ch = "IiPpCcAaVvFfSsLlHh"[i]) != 0; i++)
        if (argv[0][0] == ch)
            break;

    if (ch == 0) {
        usage();
        return;
    }

    if (argv[0][0] == 'I' || argv[0][0] == 'i') /* Print IDE information */
    {
        flush_caches();
        print_ide_info();
        return;
    }

    if (get_disk_info(hd, &dinfo, buf) == -1)
        cmd_error(ERROR_DISK_INFO);

    if (dinfo.sect_size != SECT_SIZE) {
        printf("This drive has no 512 bytes per sector! Quitting...\n");
        return;
    }

    if ((data = malloc(SECT_SIZE + ADV_DATA_SIZE)) == 0) {
        cmd_error(ERROR_MALLOC);
        return;
    }
    if ((p = malloc(sizeof(struct part_long) * (MAX_PART_ROWS + 1))) == 0) {
        cmd_error(ERROR_MALLOC);
        free(data);
        return;
    }

    mbr  = (struct mbr *)(data);
    adv  = (struct adv *)(data + SECT_SIZE);
    part = p + 1;

    p->level           = 0;
    p->empty           = 0;
    p->valid           = 1;
    p->inh_changed     = 0;
    p->inh_invalid     = 0;
    p->inconsist_err   = 0;
    p->boot_record_err = 0;

    p->container      = 0;
    p->container_base = 0;

    p->start_cyl  = 0;
    p->end_cyl    = dinfo.num_cyls - 1;
    p->start_head = 0;
    p->end_head   = dinfo.num_heads - 1;
    p->start_sect = 1;
    p->end_sect   = dinfo.num_sects;

    p->rel_sect = 0;
    p->num_sect = dinfo.total_sects;

    if (argv[0][0] == 'L' || argv[0][0] == 'l') /* Load */
    {
        if (argc < 2)
            usage();

        if (load_from_file(argv[1], mbr, SECT_SIZE) == -1)
            cmd_error(ERROR_LOAD_FILE);
        if (strcmp((char *)mbr, ADV_DATA_SIGNATURE) != 0) {
            num_rows = 4;
            unpack_part_tab(mbr->part_rec, part, num_rows, p);
        } else {
            if (load_from_file(argv[1], adv, ADV_DATA_SIZE) == -1)
                cmd_error(ERROR_LOAD_FILE);
            if (adv->version > ADV_DATA_VERSION)
                cmd_error(ERROR_ADV_VER);
            if (adv->version < ADV_DATA_VERSION)
                convert_adv_data(adv);
            num_rows = MAX_PART_ROWS;
            unpack_adv_part_tab(adv->part, part, num_rows, p);
            view = VIEW_ADV;
        }

        valid = validate_table(part, num_rows, p);

        if (!valid)
            cmd_error(ERROR_FIX_FIRST);

        if (view == VIEW_ADV) {
            if (!prepare_adv_mbr_for_save(part, mbr, adv))
                cmd_error(ERROR_NO_ADV);
        }

        disk_lock(dinfo.disk);

        mbr->magic_num = MBR_MAGIC_NUM;

        if (view == VIEW_ADV &&
            (disk_write_rel(p, mbr->x.adv.rel_sect, adv, ADV_DATA_SECT) ==
                 -1 ||
             disk_write_rel(p,
                            mbr->x.adv.rel_sect + ADV_DATA_SECT,
                            ADV_MANAGER,
                            ADV_CODE_SECT) == -1)) {
            disk_unlock(dinfo.disk);
            cmd_error(ERROR_SAVE_ADV);
        }

        if (disk_write_rel(p, 0, mbr, 1) == -1) {
            disk_unlock(dinfo.disk);
            cmd_error(ERROR_SAVE_MBR);
        }

        disk_unlock(dinfo.disk);
        printf("%s\n", (view == VIEW_ADV) ? MESG_ADV_SAVED2 : MESG_MBR_SAVED2);
        return;
    } /* load */

    if (disk_read_rel(p, 0, mbr, 1) == -1)
        cmd_error(ERROR_READ_MBR);

    view = VIEW_MBR;

    if (mbr->x.adv.adv_mbr_magic == ADV_MBR_MAGIC ||
        strcmp(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE) == 0) {
        if (disk_read_rel(p, mbr->x.adv.rel_sect, adv, ADV_DATA_SECT) == -1) {
            fprintf(stderr, "Warning: %s\n\n", ERROR_READ_ADV);
        } else if (strcmp(adv->signature, ADV_DATA_SIGNATURE) != 0) {
            fprintf(stderr, "Warning: %s\n\n", ERROR_ADV_BAD);
        } else {
            view = VIEW_ADV;
        }
    }

    if (view == VIEW_ADV) {
        if (adv->version > ADV_DATA_VERSION) {
            fprintf(stderr, "Warning: %s\n\n", ERROR_ADV_VER);
        }
        if (adv->version < ADV_DATA_VERSION)
            convert_adv_data(adv);
        num_rows = MAX_PART_ROWS;
        unpack_adv_part_tab(adv->part, part, num_rows, p);
    } else {
        num_rows = 4;
        unpack_part_tab(mbr->part_rec, part, num_rows, p);
    }

    valid = validate_table(part, num_rows, p);

    if (argv[0][0] == 'P' || argv[0][0] == 'p') /* Print partition table */
    {
        int rec_opt = (argc > 1 && (argv[1][0] == '-' || argv[1][0] == '/') &&
                       (argv[1][1] == 'R' || argv[1][1] == 'r'))
                          ? 1
                          : 0;
        if (rec_opt) {
            printf("  %s\n\n", PROGRAM_TITLE);
            if (view == VIEW_ADV) {
                print_adv_menu(adv);
                printf("\n----------------------------------------------------"
                       "---------------------------\n\n");
            }
        }

        printf(HEADER_CMD,
               dinfo.disk - 127,
               dinfo.total_sects / 2048,
               dinfo.num_cyls,
               dinfo.num_heads,
               dinfo.num_sects,
               sprintf_long(tmp, dinfo.total_sects));

        printf("Valid%s\n |   %s\n\n", HEADER_CHS2, HEADER_CHS3);

        for (i = 0; i < num_rows; i++) {
            if (num_rows > 4 && part[i].empty)
                continue;
            sprintf_partrec(tmp, &part[i], i + 1, view);
            tmp[75] = 0;
            printf("%s %s\n",
                   part[i].empty ? "   " : (part[i].valid ? "OK " : " X "),
                   tmp);
        }

        printf("\n");

        if (rec_opt) {
            print_part_details(part, num_rows);

            for (i = 0; i < num_rows; i++)
                if (!part[i].empty && part[i].valid && part[i].os_id != 0 &&
                    part[i].os_id != OS_ADV) {
                    printf("\n\n----------------------------------------------"
                           "---------------------------------\n\n"
                           "%s  [  %luM = %s sectors  at  CHS=(%lu,%lu,%lu)  "
                           "]\n\n",
                           sprintf_os_name(tmp, &part[i]),
                           part[i].num_sect / 2048,
                           sprintf_long(tmp2, part[i].num_sect),
                           part[i].start_cyl,
                           part[i].start_head,
                           part[i].start_sect);

                    if (os_desc[part[i].os_num].print != 0)
                        os_desc[part[i].os_num].print(&part[i]);
                    else
                        print_unknown(&part[i]);
                }
        } else /* Print logical drives */
        {
            int level = 0;
            int num   = num_rows + 1;
            while (1) {
                if (level > 32)
                    cmd_error("Recursion is too deep.");

                for (i = 0; i < num_rows; i++)
                    if (part[i].os_id == OS_EXT)
                        break;

                if (i == num_rows)
                    break;

                if (level == 0)
                    printf(MESG_LOGICAL);

                if (disk_read_rel(&part[i], 0, mbr, 1) == -1)
                    cmd_error(ERROR_READ_MBR);
                p        = &part[MAX_PART_ROWS - 1];
                *p       = part[i];
                num_rows = 4;
                unpack_part_tab(mbr->part_rec, part, num_rows, p);
                valid = validate_table(part, num_rows, p);
                for (i = 0; i < num_rows; i++)
                    if (part[i].os_id != 0 && part[i].os_id != OS_EXT) {
                        sprintf_partrec(tmp, &part[i], num++, view);
                        tmp[75] = 0;
                        printf("%s %s\n",
                               part[i].empty ? "   "
                                             : (part[i].valid ? "OK " : " X "),
                               tmp);
                    }
                level++;
            }
        } /* print logical */

        return;
    } /* print */

    if (argv[0][0] == 'S' || argv[0][0] == 's') /* Save */
    {
        if (argc < 2)
            usage();
        if (view != VIEW_ADV) {
            if (save_to_file(argv[1], mbr, SECT_SIZE) == -1)
                cmd_error(ERROR_SAVE_FILE);
        } else {
            if (save_to_file(argv[1], adv, ADV_DATA_SIZE) == -1)
                cmd_error(ERROR_SAVE_FILE);
        }
        printf("%s\n", (view == VIEW_ADV) ? MESG_ADVFILE_SAVED : MESG_FILE_SAVED);
        if (!valid)
            fprintf(stderr, "Warning: %s\n", WARN_INVALID);
        return;
    } /* save */

    if (argv[0][0] == 'C' || argv[0][0] == 'c') /* Compare */
    {
        if (argc < 2)
            usage();
        if (view != VIEW_ADV) {
            if (load_from_file(argv[1], tmp, SECT_SIZE) == -1 ||
                memcmp(mbr, tmp, SECT_SIZE) != 0)
                cmd_error(MESG_DIFFERENT);
        } else {
            if (load_from_file(argv[1], tmp, ADV_DATA_SIZE) == -1 ||
                memcmp(adv, tmp, ADV_DATA_SIZE) != 0)
                cmd_error(MESG_DIFFERENT);
        }
        if (!valid)
            fprintf(stderr, "Warning: %s\n", WARN_INVALID);
        printf("%s\n", MESG_NO_DIFFERENCES);
        return;
    } /* save */

    if (!valid)
        cmd_error(ERROR_FIX_FIRST);

    if (argv[0][0] == 'A' || argv[0][0] == 'a') /* Activate */
    {
        int x;

        if (view == VIEW_ADV) {
            printf("Command has no effect, because Advanced Boot Manager is installed.\n");
            return;
        }
        if (argc < 2)
            usage();
        i = atoi(argv[1]) - 1;
        if (i < 0 || i > 3)
            usage();

        for (x = 0; x < 4; x++)
            part[x].active = 0;
        part[i].active = 1;

        pack_part_tab(part, mbr->part_rec, 4);

        disk_lock(hd);

        if (disk_write_rel(p, 0, mbr, 1) == -1) {
            disk_unlock(hd);
            cmd_error(ERROR_SAVE_MBR);
        }

        printf("%s\n", MESG_MBR_SAVED);

        disk_unlock(hd);
        return;
    } /* activate */

    if (argv[0][0] == 'H' || argv[0][0] == 'h') /* Hide */
    {
        if (view == VIEW_ADV) {
            printf("Command has no effect, because Advanced Boot Manager is installed.\n");
            return;
        }
        if (argc < 2)
            usage();
        i = atoi(argv[1]) - 1;
        if (i < 0 || i > 3)
            usage();

        if (part[i].os_id == 0x0100 || part[i].os_id == 0x0400 ||
            part[i].os_id == 0x0600 || part[i].os_id == 0x0700 ||
            part[i].os_id == 0x0B00 || part[i].os_id == 0x0C00 ||
            part[i].os_id == 0x0E00 || part[i].os_id == 0x0F00)
            part[i].os_id += 0x1000;
        else if (part[i].os_id == 0x1100 || part[i].os_id == 0x1400 ||
                 part[i].os_id == 0x1600 || part[i].os_id == 0x1700 ||
                 part[i].os_id == 0x1B00 || part[i].os_id == 0x1C00 ||
                 part[i].os_id == 0x1E00 || part[i].os_id == 0x1F00)
            part[i].os_id -= 0x1000;
        else {
            fprintf(
                stderr,
                "I will not hide partition which I couldn't unhide later\n");
            return;
        }

        pack_part_tab(part, mbr->part_rec, 4);

        disk_lock(hd);

        if (disk_write_rel(p, 0, mbr, 1) == -1) {
            disk_unlock(hd);
            cmd_error(ERROR_SAVE_MBR);
        }

        printf("%s\n", MESG_MBR_SAVED);

        disk_unlock(hd);
        return;
    } /* hide */

    if (argv[0][0] == 'F' || argv[0][0] == 'f') /* Format */
    {
        if (argc < 2)
            usage();
        i = atoi(argv[1]) - 1;
        if (i < 0 || i >= num_rows)
            usage();

        if (part[i].empty)
            cmd_error(ERROR_FORMAT_EMPTY);

        if (os_desc[part[i].os_num].format == 0)
            cmd_error(ERROR_NO_FORMAT);

        x = os_desc[part[i].os_num].format(&part[i], argv + 2);
        if (x == FAILED)
            cmd_error(ERROR_FORMAT_FAILED);
        else if (x == CANCEL)
            cmd_error(WARN_FORMAT_CANCEL);
        else
            printf("\n%s\n", MESG_FORMAT_OK);

        return;
    }

    if (argv[0][0] == 'V' || argv[0][0] == 'v') /* Verify */
    {
        if (argc < 2)
            usage();
        i = atoi(argv[1]) - 1;
        if (i < 0 || i >= num_rows)
            usage();

        if (part[i].empty)
            cmd_error(ERROR_FORMAT_EMPTY);

        x = generic_verify(&part[i], 0, 0);
        if (x == FAILED)
            cmd_error(ERROR_VERIFY_FAILED);
        else if (x == CANCEL)
            cmd_error(WARN_VERIFY_CANCEL);
        else
            printf("\n%s\n", MESG_VERIFY_OK);

        return;
    }

} /* command_line */
