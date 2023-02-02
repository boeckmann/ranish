#include "part.h"

#define FsX 12
#define FsY 6
#define FsW 66
#define FsH 17

#define BmX 21
#define BmY 5
#define BmW 58
#define BmH 20

#define BoX  29
#define BoY  14
#define BoW  48
#define BoH  10
#define BoX2 (BoX + 22)

#define SiX 40
#define SiY 18
#define SiW 32
#define SiH 6

#define FmX 4
#define FmY 10
#define FmW 74
#define FmH 5

void select_system_type(struct part_long *p)
{
    struct event ev;
    int i, n, top, act, N, barpos;

    border_window(Yellow + BakWhite, FsX, FsY, FsW, FsH, Border22f);
    write_string(Black + BakWhite, FsX + 2, FsY + 1, HEADER_SYSTYPE);
    for (i = 0; i < FsW - 4; i++)
        tmp[i] = 'Ä';
    tmp[i] = 0;
    write_string(Yellow + BakWhite, FsX + 2, FsY + 2, tmp);
    write_string(Yellow + BakWhite, FsX + 28, FsY + FsH - 1, " F1 - Help ");
    write_string(
        Yellow + BakWhite, FsX + 42, FsY + FsH - 1, " INS - Custom code ");

    for (N = 0; os_desc[N].os_id != OS_ADV /* valid ids */; N++)
        ;

    /* N - number of records in table  */
    /* n - number of records on screen */

    top = 0;
    act = (os_desc[p->os_num].os_id == OS_UNKN) ? 0 : p->os_num;
    n   = (N < FsH - 4) ? N : FsH - 4;

#define BARX   (FsX + FsW - 3)
#define BARY   (FsY + 3)
#define BARLEN (n) /* (n-2) */

    while (1) {
        if (act < top)
            top = act;
        if (act >= top + n)
            top = act - n + 1;

        for (i = 0; i < n; i++) {
            sprintf_systype(tmp, top + i);
            write_string(((os_desc[top + i].flags & 1) == 1)
                             ? ((top + i == act) ? (BrWhite + BakBlack)
                                                 : (Black + BakWhite))
                             : ((top + i == act) ? (White + BakBlack)
                                                 : (BrBlue + BakWhite)),
                         FsX + 2,
                         FsY + 3 + i,
                         tmp);
        }
        /*
             if( top>0 ) write_char( Yellow+BakWhite, BARX, BARY, '');
                    else write_char( Yellow+BakWhite, BARX, BARY, ' ');

             if( top+n<N ) write_char( Yellow+BakWhite, BARX, BARY+BARLEN,
           ''); else write_char( Yellow+BakWhite, BARX, BARY+BARLEN, ' ');
        */
        if (N != n) {
            barpos = (top * (BARLEN - 1)) / (N - n);

            for (i = 0; i < BARLEN; i++)
                write_char(Yellow + BakWhite,
                           BARX,
                           BARY + i,
                           (i != barpos) ? '°' : '²');
        }

        move_cursor(1, 26);

        get_event(&ev, EV_KEY);

        if (ev.key == 27) /* ESC */
        {
            break;
        }

        if (ev.key == 13) /* Enter */
        {
            p->os_num = act;
            p->os_id  = os_desc[act].os_id;
            break;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#0x%04X", os_desc[act].os_id);
            html_view(tmp);
        } else if (ev.scan == 0x52E0 || ev.scan == 0x5200) /* Insert */
        {
            if (enter_string(4, 16, PROMPT_FSTYPE, FsW, tmp, 0)) {
                i = 0;
                sscanf(tmp, "%X", &i);
                if (i != 0 && i < 256) {
                    p->os_id = i << 8;
                    for (i = 0; os_desc[i].os_id != p->os_id &&
                                os_desc[i].os_id != OS_UNKN;
                         i++)
                        ;
                    p->os_num = i;
                    break;
                }
            }
        } else if (ev.scan == 0x47E0 || ev.scan == 0x4700) /* Home */
        {
            act = 0;
        } else if (ev.scan == 0x4FE0 || ev.scan == 0x4F00) /* End */
        {
            act = N - 1;
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < N - 1)
                act++;
        } else if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (act > 0)
                act--;
        } else if (ev.scan == 0x49E0 || ev.scan == 0x4900) /* PgUp */
        {
            act -= n;
            if (act < 0)
                act = 0;
        } else if (ev.scan == 0x51E0 || ev.scan == 0x5100) /* PgDn */
        {
            act += n;
            if (act >= N)
                act = N - 1;
        } else if (ev.key >= 'A' && ev.key <= 'Z' ||
                   ev.key >= 'a' && ev.key <= 'z') {
            char ch = (ev.key >= 'A' && ev.key <= 'Z') ? ev.key : ev.key - 32;

            for (i = act + 1; i < N; i++)
                if ((os_desc[i].flags & 1) &&
                    (os_desc[i].name[0] == ch ||
                     os_desc[i].name[0] == ch + 32)) {
                    act = i;
                    break;
                }

            if (i == N)
                for (i = act + 1; i < N; i++)
                    if (os_desc[i].name[0] == ch ||
                        os_desc[i].name[0] == ch + 32) {
                        act = i;
                        break;
                    }
        }
    } /* while(1) */

} /* select_system_type */

void edit_str_field(struct event *ev, int ev_mask, int attr, int x, int y,
                    int maxlen, char *str, int *pos)
{
    int i;

    while (1) {
        i = strlen(str);
        if (*pos > i)
            *pos = i;
        if (*pos == maxlen - 1)
            (*pos)--;
        clear_window(attr, x + i, y, maxlen - i - 1, 1);
        write_string(attr, x, y, str);
        move_cursor(x + *pos, y);

        get_event(ev, ev_mask | EV_KEY);

        if (!(ev->ev_type & EV_KEY))
            break;

        if (ev->scan == 0x47E0 || ev->scan == 0x4700) /* Home */
        {
            *pos = 0;
        } else if (ev->scan == 0x4FE0 || ev->scan == 0x4F00) /* End */
        {
            *pos = strlen(str);
        } else if (*pos > 0 &&
                   (ev->scan == 0x4BE0 || ev->scan == 0x4B00)) /* left */
        {
            (*pos)--;
        } else if (*pos < strlen(str) &&
                   (ev->scan == 0x4DE0 || ev->scan == 0x4D00)) /* right */
        {
            (*pos)++;
        } else if (ev->key == 8 && *pos > 0) /* BkSp */
        {
            for (i = *pos; i < maxlen && str[i] != 0; i++)
                str[i - 1] = str[i];
            str[i - 1] = 0;
            (*pos)--;
        } else if (ev->scan == 0x5300 || ev->scan == 0x53E0) /* Del */
        {
            for (i = *pos; i < maxlen && str[i] != 0; i++)
                str[i] = str[i + 1];
            str[i] = 0;
        } else if (ev->key >= ' ') {
            if (strlen(str) < maxlen - 1) {
                for (i = maxlen - 1; i > *pos; i--)
                    str[i] = str[i - 1];
                str[i] = ev->key;
                (*pos)++;
            }
        } else
            break;

    } /* while */

} /* edit_str_field */

void edit_int_field(struct event *ev, int ev_mask, int attr, int x, int y,
                    int field_len, unsigned long *n, unsigned long limit)
{
    unsigned i;

    while (1) {
        if (field_len > 0) {
            sprintf(tmp, "%*lu", field_len, *n);
            move_cursor(x + field_len - 1, y);
        } else {
            sprintf(tmp, "%-*lu", -field_len, *n);
            for (i = 0; tmp[i] != ' ' && tmp[i] != 0; i++)
                ;
            move_cursor(x + i, y);
        }

        write_string(attr, x, y, tmp);

        get_event(ev, ev_mask | EV_KEY);

        if (!(ev->ev_type & EV_KEY))
            break;

        if (ev->key == 8 || ev->scan == 0x5300 ||
            ev->scan == 0x53E0) /* BkSp || Del */
        {
            (*n) /= 10;
        } else if (ev->key == '+') {
            if ((*n) < limit)
                (*n)++;
        } else if (ev->key == '-') {
            if ((*n) > 0)
                (*n)--;
        } else if (ev->key >= '0' && ev->key <= '9') {
            i = ev->key - '0';

            if (limit >= i && (*n) <= (limit - i) / 10u)
                (*n) = (*n) * 10u + i;
        } else
            break;

    } /* while */

} /* edit_int_field */

int get_part_num(struct adv_part_rec *part, int tag)
{
    int i;
    if (tag == 0)
        return 0;
    for (i = 0; i < MAX_PART_ROWS; i++)
        if (part[i].tag == tag)
            return i + 1;
    return 0;
}

int get_free_tag(struct adv_part_rec *part)
{
    int i, tag;

    for (tag = 1; tag <= MAX_PART_ROWS; tag++) {
        for (i = 0; i < MAX_PART_ROWS; i++)
            if (part[i].tag == tag)
                break;
        if (i == MAX_PART_ROWS)
            break;
    }

    return tag;
}

int get_keys(unsigned short *keys, int num_keys)
{
    struct event ev;
    int i, x, y, w, h, q;

    x = 1;
    w = 80;
    y = 23;
    h = 3;

    save_window(x, y, w, h, buf);
    border_window(Yellow + BakWhite, x, y, w, h, Border22f);
    write_string(
        Yellow + BakWhite, x + 53, y + h - 1, " Esc, BkSp, \\ - quote ");

    q = 0;
    while (1) {
        clear_window(BrWhite + BakBlack, x + 2, y + 1, w - 4, 1);
        for (i = 0; i < num_keys; i++) {
            sprintf(tmp, "%04X", keys[i]);
            write_string(BrWhite + BakBlack, x + 2 + 5 * i, y + 1, tmp);
        }
        move_cursor(x + 2 + 5 * i, y + 1);

        get_event(&ev, EV_KEY);

        if (q == 0 && ev.key == 27)
            break;
        else if (q == 0 && ev.key == 8) {
            if (num_keys > 0)
                num_keys--;
        } else if (q == 0 && ev.key == '\\') {
            if (num_keys < 15)
                q = 1;
        } else if (num_keys < 15) {
            keys[num_keys++] = ev.scan;
            q                = 0;
        }
    }

    for (i = num_keys; i < 15; i++)
        keys[i] = 0;

    load_window(x, y, w, h, buf);
    return num_keys;
} /* get_keys */


int two_btn_dialog(char *msg, char *btn1, int res1, char *btn2, int res2)
{
    int x, y = FmY, w, h = 4, pressed;
    struct event ev;
    static char buf[4 * 80 * 2];
    int result = res1;
    int b1l, b2l;

    b1l = strlen(btn1);
    b2l = strlen(btn2);
    w = strlen(msg) + 6;
    if (w % 2 == 1)
        w++;
    if (w > 78)
        w = 78;
    x = (80 - w) / 2 + 1;

    save_window(x, y, w, h, buf);
    border_window(Yellow + BakWhite, x, y, w, h, Border22f);

    write_string(Black + BakWhite, x + 3, y + 1, msg);
    pressed = 0;

    while (1) {
        if (result == res1) {
            move_cursor(38-b1l, y+2);
        }
        else if (result == res2) {
            move_cursor(42, y+2);
        }
        write_string((result == res1) ? BrWhite + BakBlack : Black + BakWhite, 38 - b1l - 1, y + 2, " ");
        write_string((result == res1) ? BrWhite + BakBlack : Black + BakWhite, 38 - b1l, y + 2, btn1);
        write_string((result == res1) ? BrWhite + BakBlack : Black + BakWhite, 38, y + 2, " ");

        write_string((result == res1) ? Black + BakWhite : BrWhite + BakBlack, 41, y + 2, " ");
        write_string((result == res1) ? Black + BakWhite : BrWhite + BakBlack, 42, y + 2, btn2);
        write_string((result == res1) ? Black + BakWhite : BrWhite + BakBlack, 42 + b2l, y + 2, " ");

        get_event(&ev, EV_KEY | EV_MOUSE);

        if (ev.key == 13) {
            break;
        }
        else if (ev.key == 27) {
            result = 0;
            break;
        }
        else if (ev.key == 'y' || ev.key == 'Y') {
            result = res1;
            break;
        }
        else if (ev.key == 'n' || ev.key == 'N') {
            result = res2;
            break;
        }        
        if (ev.scan == 0x4BE0 || ev.scan == 0x4B00 /* left */
            || ev.scan == 0x4DE0 || ev.scan == 0x4D00 /* right */
            || ev.scan == 0x0F09) /* TAB */ {
            result = (result == res1) ? res2 : res1;
        }

        else if (ev.ev_type & EV_MOUSE)
            if (ev.x >= 41 && ev.x <= 42 + b2l &&
                ev.y == y + 2) {
                if (ev.left == 1)
                    pressed = 1;
                if (ev.left == 0 && pressed == 1) {
                    result = res2;
                    break;
                }
            } else if (ev.x >= 37 - b1l && ev.x <= 38 &&
                ev.y == y + 2) {
                if (ev.left == 1)
                    pressed = 1;
                if (ev.left == 0 && pressed == 1) {
                    result = res1;
                    break;
                }
            } else
                pressed = 0;
    }

    load_window(x, y, w, h, buf);

    return result;
}



void boot_menu_options(struct adv *adv)
{
    struct event ev;
    int act, pos;
    char password[9];
    unsigned long timeout, def_menu;

    border_window(Yellow + BakWhite, BoX, BoY, BoW, BoH, Border22f);

    timeout  = adv->timeout;
    def_menu = adv->def_menu + 1;
    if (!(adv->options & ADV_OPT_DEF_MENU))
        def_menu = 0;

    write_string(Black + BakWhite, BoX + 2, BoY + 1, MENU_ADV_OPT_TITLE);
    write_string(Black + BakWhite, BoX + 2, BoY + 3, MENU_ADV_OPT_VIRCHECK);
    write_string(Black + BakWhite, BoX + 2, BoY + 4, MENU_ADV_OPT_CLEARSCR);
    write_string(Black + BakWhite, BoX + 2, BoY + 5, MENU_ADV_OPT_TIMEOUT);
    write_string(Black + BakWhite, BoX + 2, BoY + 6, MENU_ADV_OPT_DEFAULT);
    write_string(Black + BakWhite, BoX + 2, BoY + 7, MENU_ADV_OPT_PASSWORD);
    write_string(Black + BakWhite, BoX + 2, BoY + 8, MENU_ADV_OPT_IGN_UNUSED);

    act = 0;
    pos = 0;

    while (1) {
        sprintf(tmp, "%-31s", adv->adv_title);
        write_string(act == 0 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     BoX + 14,
                     BoY + 1,
                     tmp);

        sprintf(tmp, "%s", (adv->options & ADV_OPT_VIR_CHECK) ? "Yes" : "No ");
        write_string(act == 1 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     BoX2,
                     BoY + 3,
                     tmp);
        if (act == 1)
            move_cursor(BoX2, BoY + 3);

        sprintf(tmp, "%s", (adv->options & ADV_OPT_CLEAR_SCR) ? "Yes" : "No ");
        write_string(act == 2 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     BoX2,
                     BoY + 4,
                     tmp);
        if (act == 2)
            move_cursor(BoX2, BoY + 4);

        if (act != 3) {
            sprintf(tmp, "%-3ld", timeout);
            write_string(Black + BakWhite, BoX2, BoY + 5, tmp);
        }

        if (act != 4) {
            if (adv->options & ADV_OPT_DEF_MENU)
                sprintf(tmp, "%-3ld     ", def_menu);
            else
                sprintf(tmp, "Not set");
            write_string(Black + BakWhite, BoX2, BoY + 6, tmp);
        }

        sprintf(tmp, "%s", (adv->password == 0) ? "Not set " : "******* ");
        write_string(act == 5 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     BoX2,
                     BoY + 7,
                     tmp);
        if (act == 5)
            move_cursor(BoX2, BoY + 7);

        sprintf(
            tmp, "%s", (adv->options & ADV_OPT_IGN_UNUSED) ? "Yes" : "No ");
        write_string(act == 6 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     BoX2,
                     BoY + 8,
                     tmp);
        if (act == 6)
            move_cursor(BoX2, BoY + 8);

        if (act == 0)
            edit_str_field(&ev,
                           0,
                           BrWhite + BakBlack,
                           BoX + 14,
                           BoY + 1,
                           sizeof(adv->adv_title),
                           adv->adv_title,
                           &pos);
        else if (act == 3)
            edit_int_field(
                &ev, 0, BrWhite + BakBlack, BoX2, BoY + 5, -3, &timeout, 99);
        else if (act == 4) {
            write_string(Black + BakWhite, BoX2, BoY + 6, "       ");
            edit_int_field(&ev,
                           0,
                           BrWhite + BakBlack,
                           BoX2,
                           BoY + 6,
                           -3,
                           &def_menu,
                           MAX_MENU_ROWS);
            if (def_menu != 0)
                adv->options |= ADV_OPT_DEF_MENU;
            else
                adv->options &= ~ADV_OPT_DEF_MENU;
        } else if (act == 5) {
            int ppos    = 0;
            password[0] = 0;
            edit_str_field(
                &ev, 0, BrWhite + BakBlack, BoX2, BoY + 7, 9, password, &ppos);
            if (ev.key == 13) {
                adv->password = encrypt_password(password);
                act           = 0;
                continue;
            }
            memset(password, 0, 9);
        } else
            get_event(&ev, EV_KEY);

        if (ev.key == 27) /* ESC */
        {
            break;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#setup_adv_opt");
            html_view(tmp);
        } else if (ev.key == 13) /* Enter */
        {
            if (act < 6)
                act++;
            else
                act = 0;
        } else if (act == 1 && (ev.key == 'Y' || ev.key == 'y')) {
            adv->options |= ADV_OPT_VIR_CHECK;
        } else if (act == 1 && (ev.key == 'N' || ev.key == 'n')) {
            adv->options &= ~ADV_OPT_VIR_CHECK;
        } else if (act == 1 && ev.key == ' ') {
            adv->options ^= ADV_OPT_VIR_CHECK;
        } else if (act == 2 && (ev.key == 'Y' || ev.key == 'y')) {
            adv->options |= ADV_OPT_CLEAR_SCR;
        } else if (act == 2 && (ev.key == 'N' || ev.key == 'n')) {
            adv->options &= ~ADV_OPT_CLEAR_SCR;
        } else if (act == 2 && ev.key == ' ') {
            adv->options ^= ADV_OPT_CLEAR_SCR;
        } else if (act == 6 && (ev.key == 'Y' || ev.key == 'y')) {
            adv->options |= ADV_OPT_IGN_UNUSED;
        } else if (act == 6 && (ev.key == 'N' || ev.key == 'n')) {
            adv->options &= ~ADV_OPT_IGN_UNUSED;
        } else if (act == 6 && ev.key == ' ') {
            adv->options ^= ADV_OPT_IGN_UNUSED;
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < 6)
                act++;
        } else if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (act > 0)
                act--;
        }

    } /* while(1) */

    adv->timeout = timeout;
    if (adv->options & ADV_OPT_DEF_MENU) {
        adv->def_menu = def_menu - 1;
    }
} /* boot_menu_options */

void edit_boot_menu(struct adv *adv, int p, int flag)
{
    struct event ev;
    int old_col, old_act, redraw;
    int i, k, act, col, pos, n = MAX_MENU_ROWS;

    static char *show[5] = {" ---- ", " Last ", " Next ", " Prev ", " Last3"};
    static char *part_row[5]  = {"/*", "/1", "/2", "/3", "/4"};
    static char *boot_type[5] = {"-", "C", "D", "A", "?"};

    border_window(Yellow + BakWhite, BmX, BmY, BmW, BmH, Border22f);

    write_string(Black + BakWhite, BmX + 2, BmY + 1, HEADER_BOOT_MENU);
    for (i = 0; i < BmW - 4; i++)
        tmp[i] = 'Ä';
    tmp[i] = 0;
    write_string(Yellow + BakWhite, BmX + 2, BmY + 2, tmp);
    write_string(Yellow + BakWhite, BmX + 2, BmY + BmH - 1, " F1 - Help ");
    write_string(Yellow + BakWhite,
                 BmX + 18,
                 BmY + BmH - 1,
                 " Space, Ins, Del, Alt-O, Alt-K ");

    act    = 0;
    col    = 1;
    pos    = 0;
    redraw = 1;

    if (adv->part[p].tag != 0) {
        for (i = 0; i < n; i++)
            if (adv->menu[i].boot_type != MENU_BOOT_EMPTY &&
                adv->menu[i].x.part.tag == adv->part[p].tag) {
                act = i;
                break;
            }
    } else if (adv->part[p].os_id != 0 && adv->part[p].os_id != OS_ADV) {
        for (i = 0; i < n; i++)
            if (adv->menu[i].boot_type == MENU_BOOT_EMPTY) {
                adv->menu[i].boot_type  = MENU_BOOT_PART;
                adv->part[p].tag        = get_free_tag(adv->part);
                adv->menu[i].x.part.tag = adv->part[p].tag;
                act                     = i;
                break;
            }
    }

    while (1) {
        if (old_col != col || old_act != act || redraw) {
            for (i = 0; i < n; i++) {
                k = get_part_num(adv->part, adv->menu[i].x.part.tag);
                sprintf(tmp,
                        " %2d  %s %2d%s %s %s  %-29s %s ",
                        i + 1,
                        boot_type[adv->menu[i].boot_type],
                        k,
                        k == 0 ? "  " : part_row[adv->part[k - 1].orig_row],
                        show[adv->menu[i].x.part.show],
                        adv->menu[i].options & MENU_OPT_PASSW ? "Y" : "-",
                        adv->menu[i].name,
                        adv->menu[i].x.part.num_keys != 0 ? "Y" : "-");
                write_string(Black + BakWhite, BmX + 2, BmY + 3 + i, tmp);
            }
            old_col = col;
            old_act = act;
            redraw  = 0;
        }

        if (col == 0) {
            write_string(BrWhite + BakBlack,
                         BmX + 7,
                         BmY + 3 + act,
                         boot_type[adv->menu[act].boot_type]);
            move_cursor(BmX + 7, BmY + 3 + act);
        } else if (col == 1) {
            i = get_part_num(adv->part, adv->menu[act].x.part.tag);
            write_int(BrWhite + BakBlack, BmX + 9, BmY + 3 + act, 2, i);
            move_cursor(BmX + 10, BmY + 3 + act);
        } else if (col == 2) {
            k = get_part_num(adv->part, adv->menu[act].x.part.tag);
            if (k == 0)
                tmp[1] = 0;
            else if (adv->part[k - 1].orig_row == 0)
                tmp[0] = '*';
            else
                tmp[0] = adv->part[k - 1].orig_row + '0';
            tmp[1] = 0;
            write_string(BrWhite + BakBlack, BmX + 12, BmY + 3 + act, tmp);
            move_cursor(BmX + 12, BmY + 3 + act);
        } else if (col == 3) {
            write_string(BrWhite + BakBlack,
                         BmX + 14,
                         BmY + 3 + act,
                         show[adv->menu[act].x.part.show]);
            move_cursor(BmX + 15, BmY + 3 + act);
        } else if (col == 4) {
            write_string(BrWhite + BakBlack,
                         BmX + 21,
                         BmY + 3 + act,
                         adv->menu[act].options & MENU_OPT_PASSW ? "Y" : "-");
            move_cursor(BmX + 21, BmY + 3 + act);
        }

        if (flag == 1) {
            boot_menu_options(adv);
            flag   = 0;
            redraw = 1;
            continue;
        }

        if (col == 5) {
            edit_str_field(&ev,
                           0,
                           BrWhite + BakBlack,
                           BmX + 24,
                           BmY + 3 + act,
                           sizeof(adv->menu[act].name),
                           adv->menu[act].name,
                           &pos);
        } else
            get_event(&ev, EV_KEY);

        if (ev.key == 27) /* ESC */
        {
            break;
        }
        if (ev.key == 13) /* Enter */
        {
            if (col == 1)
                break;
            col    = 1;
            redraw = 1;
        } else if (ev.key == 9) /* Tab */
        {
            col = (col + 1) % 6;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#setup_adv");
            html_view(tmp);
        } else if (ev.scan == 0x1800) /* Alt-O = Options */
        {
            boot_menu_options(adv);
            redraw = 1;
        } else if (ev.scan == 0x2500) /* Alt-K = Keys */
        {
            adv->menu[act].x.part.num_keys = get_keys(
                adv->menu[act].x.part.keys, adv->menu[act].x.part.num_keys);
            redraw = 1;
        } else if (col == 0 && (ev.key == '+' || ev.key == ' ')) {
            adv->menu[act].boot_type++;
            if (adv->menu[act].boot_type > 3)
                adv->menu[act].boot_type = 1;
        } else if (col == 0 && ev.key == '-') {
            if (adv->menu[act].boot_type == 1)
                adv->menu[act].boot_type = 3;
            else
                adv->menu[act].boot_type--;
        } else if (col == 1 && (ev.key == '+' || ev.key == ' ')) {
            int m = MAX_PART_ROWS;
            k     = get_part_num(adv->part, adv->menu[act].x.part.tag);
            for (i = k; i < m; i++) {
                if (adv->part[i].os_id != 0 && adv->part[i].os_id != OS_ADV) {
                    if (adv->part[i].tag == 0)
                        adv->part[i].tag = get_free_tag(adv->part);
                    adv->menu[act].x.part.tag = adv->part[i].tag;
                    break;
                }
                if (i == m - 1 && m == MAX_PART_ROWS) {
                    m = k - 1;
                    i = -1;
                }
            }
            if (adv->menu[act].boot_type == 0 &&
                adv->menu[act].x.part.tag != 0)
                adv->menu[act].boot_type = MENU_BOOT_PART;
            redraw = 1;
        } else if (col == 1 && ev.key == '-') {
            int m = 0;
            k     = get_part_num(adv->part, adv->menu[act].x.part.tag);

            for (i = k - 2; i >= m; i--) {
                if (adv->part[i].os_id != 0 && adv->part[i].os_id != OS_ADV) {
                    if (adv->part[i].tag == 0)
                        adv->part[i].tag = get_free_tag(adv->part);
                    adv->menu[act].x.part.tag = adv->part[i].tag;
                    break;
                }
                if (i == m && m == 0) {
                    m = k;
                    i = MAX_PART_ROWS;
                }
            }
            if (adv->menu[act].boot_type == 0 &&
                adv->menu[act].x.part.tag != 0)
                adv->menu[act].boot_type = MENU_BOOT_PART;
            redraw = 1;
        } else if (col == 2 && (ev.key == '+' || ev.key == ' ')) {
            k = get_part_num(adv->part, adv->menu[act].x.part.tag);
            if (k == 0)
                continue;
            else
                k--;
            adv->part[k].orig_row = (adv->part[k].orig_row + 1) % 5;
            redraw                = 1;
        } else if (col == 1 && ev.key == '-') {
            k = get_part_num(adv->part, adv->menu[act].x.part.tag);
            if (k == 0)
                continue;
            else
                k--;
            adv->part[k].orig_row = (adv->part[k].orig_row + 4) % 5;
            redraw                = 1;
        } else if (col == 3 && (ev.key == '+' || ev.key == ' ')) {
            adv->menu[act].x.part.show = (adv->menu[act].x.part.show + 1) % 5;
        } else if (col == 3 && ev.key == '-') {
            adv->menu[act].x.part.show = (adv->menu[act].x.part.show + 4) % 5;
        } else if (col == 4 && ev.key == ' ') {
            adv->menu[act].options ^= MENU_OPT_PASSW;
            redraw = 1;
        } else if (ev.scan == 0x5200 || ev.scan == 0x52E0) /* Insert */
        {
            for (i = MAX_MENU_ROWS - 1; i > act; i--)
                adv->menu[i] = adv->menu[i - 1];
            memset(&adv->menu[i], 0, sizeof(struct adv_menu_rec));
            redraw = 1;
        } else if (ev.scan == 0x5300 || ev.scan == 0x53E0) /* Del */
        {
            for (i = act; i < MAX_MENU_ROWS - 1; i++)
                adv->menu[i] = adv->menu[i + 1];
            memset(&adv->menu[i], 0, sizeof(struct adv_menu_rec));
            redraw = 1;
        } else if (ev.scan == 0x47E0 || ev.scan == 0x4700) /* Home */
        {
            act = 0;
        } else if (ev.scan == 0x4FE0 || ev.scan == 0x4F00) /* End */
        {
            act = n - 1;
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < n - 1)
                act++;
        } else if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (act > 0)
                act--;
        } else if (ev.scan == 0x49E0 || ev.scan == 0x4900) /* PgUp */
        {
            act = 0;
        } else if (ev.scan == 0x51E0 || ev.scan == 0x5100) /* PgDn */
        {
            act = n - 1;
        } else if (ev.scan == 0x4BE0 || ev.scan == 0x4B00) /* left */
        {
            if (col > 0)
                col--;
        } else if (ev.scan == 0x4DE0 || ev.scan == 0x4D00) /* right */
        {
            if (col < 5)
                col++;
        }

    } /* while(1) */

    for (i = 0; i < MAX_PART_ROWS; i++)
        if (adv->part[i].tag != 0) {
            for (k = 0; k < MAX_MENU_ROWS; k++)
                if (adv->part[i].tag == adv->menu[k].x.part.tag)
                    break;
            if (k == MAX_MENU_ROWS)
                adv->part[i].tag = 0;
        }

} /* edit_boot_menu */

void setup_ipl(struct mbr *mbr)
{
    struct event ev;
    unsigned long n;
    int b, v, d, act;

    border_window(Yellow + BakWhite, SiX, SiY, SiW, SiH, Border22f);

    n = mbr->x.ipl.dot_len;
    v = mbr->x.ipl.vir_chk;
    d = mbr->x.ipl.def_part;
    b = mbr->x.ipl.fd_params[0];

    write_string(Black + BakWhite, SiX + 2, SiY + 1, IPL_VIRUS);
    write_string(Black + BakWhite, SiX + 2, SiY + 2, IPL_DOTS);
    write_string(Black + BakWhite, SiX + 2, SiY + 3, IPL_DEFAULT);
    write_string(Black + BakWhite, SiX + 2, SiY + 4, IPL_NOACTV);

    act = 0;

    while (1) {
        sprintf(tmp, "%s", v != 0 ? "Yes" : "No ");
        write_string(act == 0 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     SiX + 27,
                     SiY + 1,
                     tmp);
        if (act == 0)
            move_cursor(SiX + 27, SiY + 1);

        sprintf(tmp, "%-3ld", n);
        write_string(Black + BakWhite, SiX + 27, SiY + 2, tmp);

        if (d == ' ')
            sprintf(tmp, "Sp ");
        else if (d == '\t')
            sprintf(tmp, "Tab");
        else
            sprintf(tmp, "%c  ", d == 0 ? ' ' : d);
        write_string(act == 2 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     SiX + 27,
                     SiY + 3,
                     tmp);
        if (act == 2)
            move_cursor(SiX + 27, SiY + 3);

        sprintf(tmp, "%s", b == 0 ? "A" : "D");
        write_string(act == 3 ? (BrWhite + BakBlack) : (Black + BakWhite),
                     SiX + 27,
                     SiY + 4,
                     tmp);
        if (act == 3)
            move_cursor(SiX + 27, SiY + 4);

        if (act == 1)
            edit_int_field(
                &ev, 0, BrWhite + BakBlack, SiX + 27, SiY + 2, -3, &n, 64);
        else
            get_event(&ev, EV_KEY);

        if (n == 0) {
            n = 1;
            continue;
        }

        if (ev.key == 27) /* ESC */
        {
            break;
        }
        if (ev.key == 13) /* Enter */
        {
            if (act < 3)
                act++;
            else
                break;
        } else if (ev.scan == 0x3B00) /* F1 - Help */
        {
            sprintf(tmp, "#setup_ipl");
            html_view(tmp);
        } else if (act == 0 && (ev.key == 'Y' || ev.key == 'y')) {
            v = 1;
        } else if (act == 0 && (ev.key == 'N' || ev.key == 'n')) {
            v = 0;
        } else if (act == 0 && ev.key == ' ') {
            if (v == 0)
                v = 1;
            else
                v = 0;
        } else if (act == 2 && ev.key == ' ') {
            if (d == 0)
                d = '1';
            else if (d >= '1' && d < '4')
                d++;
            else if (d == '4')
                d = 'A';
            else if (d == 'A')
                d = '\t';
            else if (d == '\t')
                d = ' ';
            else
                d = 0;
        } else if (act == 2 &&
                   (ev.key == '\t' || ev.key >= '1' && ev.key <= '4' ||
                    ev.key == 'A' || ev.key == 'a')) {
            d = ev.key;
            if (d == 'a')
                d = 'A';
        } else if (act == 2 &&
                   (ev.scan == 0x5300 || ev.scan == 0x53E0)) /* Del */
        {
            d = 0;
        } else if (act == 3 && (ev.key == 'A' || ev.key == 'a')) {
            b = 0;
        } else if (act == 3 && (ev.key == 'D' || ev.key == 'd')) {
            b = dinfo.disk + 1;
        } else if (act == 3 && ev.key == ' ') {
            if (b == 0)
                b = dinfo.disk + 1;
            else
                b = 0;
        } else if (ev.scan == 0x50E0 || ev.scan == 0x5000) /* down */
        {
            if (act < 3)
                act++;
        } else if (ev.scan == 0x48E0 || ev.scan == 0x4800) /* up */
        {
            if (act > 0)
                act--;
        }

    } /* while(1) */

    mbr->x.ipl.dot_len      = n;
    mbr->x.ipl.vir_chk      = v;
    mbr->x.ipl.def_part     = d;
    mbr->x.ipl.fd_params[0] = b;

} /* setup_ipl */

int enter_string(int x, int y, char *prompt, int maxlen, char *str, char *help)
{
    struct event ev;
    int i, w, x2, w2;

    w2 = 73 - x - strlen(prompt);
    if (w2 > maxlen)
        w2 = maxlen;
    w  = strlen(prompt) + 5 + w2;
    x2 = x + strlen(prompt) + 3;

    save_window(x, y, w, 3, buf);
    border_window(Yellow + BakWhite, x, y, w, 3, Border22f);
    write_string(Black + BakWhite, x + 2, y + 1, prompt);

    if (help != 0)
        write_string(Yellow + BakWhite, x + w - 14, y + 2, " F1 - Help ");

    if (str == file_name) {
        i = strlen(str);
    } else {
        i      = 0;
        str[0] = 0;
    }

    while (1) {
        edit_str_field(&ev, 0, BrWhite + BakBlack, x2, y + 1, w2, str, &i);

        if (ev.key == 27) {
            i = 0;
            break;
        }
        if (ev.key == 13) {
            i = 1;
            break;
        }

        if (ev.scan == 0x3B00 && help != 0) /* F1 - Help */
        {
            html_view(help);
        }
    } /* while(1) */

    load_window(x, y, w, 3, buf);
    return i;
} /* enter_string */

void show_error(char *msg)
{
    int x, y = 10, w, h = 4, pressed;
    struct event ev;
    static char buf[4 * 80 * 2];

    w = strlen(msg) + 6;
    if (w % 2 == 1)
        w++;
    if (w > 78)
        w = 78;
    x = (80 - w) / 2 + 1;

    save_window(x, y, w, h, buf);
    border_window(Yellow + BakRed, x, y, w, h, Border22f);

    write_string(BrWhite + BakRed, x + 3, y + 1, msg);
    write_string(Black + BakWhite, x + w / 2 - 2, y + 2, " OK ");

    pressed = 0;

    while (1) {
        get_event(&ev, EV_KEY | EV_MOUSE);

        if ((ev.ev_type & EV_KEY) && (ev.key == 27 || ev.key == 13))
            break;

        if (ev.ev_type & EV_MOUSE)
            if (ev.x >= x + w / 2 - 2 && ev.x < x + w / 2 + 2 &&
                ev.y == y + 2) {
                if (ev.left == 1)
                    pressed = 1;
                if (ev.left == 0 && pressed == 1)
                    break;
            } else
                pressed = 0;
    }

    load_window(x, y, w, h, buf);
} /* show_error */

void redraw_menu(int view)
{
    if (view == VIEW_ADV)
        border_window(BORDER_COLOR, 1, 13, 80, 12, VERT_BORDER);
    if (view != VIEW_ADV)
        border_window(BORDER_COLOR, 1, 13, 80, 12, VERT_BORDER);

    if (view == VIEW_MBR) {
        write_string(HINT_COLOR, 11, 16, "I)");
        write_string(MENU_COLOR, 13, 16, MENU_INSTALL);
        write_string(HINT_COLOR, 47, 16, "A)");
        write_string(HINT_COLOR, 11, 17, "U)");
        write_string(MENU_COLOR, 13, 17, MENU_UNINSTALL);

        write_string(HINT_COLOR, 11, 19, "H)");
        write_string(MENU_COLOR, 13, 19, MENU_HIDE);
        write_string(HINT_COLOR, 42, 19, "C)");
        write_string(MENU_COLOR, 44, 19, MENU_INST_IPL);
    } else if (view == VIEW_ADV) {
        write_string(HINT_COLOR, 11, 22, "P)");
        write_string(MENU_COLOR, 13, 22, MENU_PREVIEW);

        write_string(HINT_COLOR, 42, 22, "U)");
        write_string(MENU_COLOR, 44, 22, MENU_ADV_UNINST);

    } else if (view == VIEW_EMBR) {
        write_string(HINT_COLOR, 11, 16, "I)");
        write_string(MENU_COLOR, 13, 16, MENU_INST_EXT);
        write_string(HINT_COLOR, 11, 17, "R)");
        write_string(MENU_COLOR, 13, 17, MENU_MAKE_PRI);

        write_string(HINT_COLOR, 11, 19, "H)");
        write_string(MENU_COLOR, 13, 19, MENU_HIDE);
        write_string(HINT_COLOR, 42, 19, "C)");
        write_string(MENU_COLOR, 44, 19, MENU_INST_IPL);
    }

    write_string(HINT_COLOR, 11, 20, "F)");
    write_string(MENU_COLOR, 13, 20, MENU_FORMAT);
    write_string(HINT_COLOR, 11, 21, "V)");
    write_string(MENU_COLOR, 13, 21, MENU_VERIFY);
    write_string(HINT_COLOR, 11, 22, "E)");
    write_string(MENU_COLOR, 13, 22, TEXT(" Erase partition content"));

    write_string(HINT_COLOR, 42, 20, "S)");
    write_string(MENU_COLOR, 44, 20, (view == VIEW_ADV) ? MENU_SAVE_ADV : MENU_SAVE_MBR);
    write_string(HINT_COLOR, 42, 21, "L)");
    write_string(MENU_COLOR, 44, 21, (view == VIEW_ADV) ? MENU_LOAD_ADV : MENU_LOAD_MBR);

    write_string(HINT_COLOR, 4, 24, "F1");
    write_string(MENU_COLOR, 6, 24, " - Help ");

    write_string(HINT_COLOR, 15, 24, "F2");
    write_string(MENU_COLOR, 17, 24, " - Save Changes ");

    write_string(HINT_COLOR, 34, 24, "F3");
    write_string(MENU_COLOR, 36, 24, " - Undo ");

    write_string(HINT_COLOR, 45, 24, "F4");
    write_string(MENU_COLOR, 47, 24, " - Mode ");

    write_string(
        HINT_COLOR & ((view == VIEW_EMBR) ? 0x77 : 0xFF), 56, 24, "F5");
    write_string(
        MENU_COLOR & ((view == VIEW_EMBR) ? 0x77 : 0xFF), 58, 24, " - Disk ");

    write_string(HINT_COLOR, 68, 24, "ESC");
    write_string(MENU_COLOR, 71, 24, " - Quit ");

} /* redraw_menu */

void redraw_header(int view, int mode, struct part_long *p)
{
    border_window(BORDER_COLOR, 1, 4, 80, 5, VERT_BORDER);

    if (view == VIEW_EMBR) {
        write_string(HEADER_COLOR, 4, 5, HEADER_EMBR);
        write_int(TABLE_COLOR, 14, 5, 2, p->level);
        write_int(TABLE_COLOR, 18, 5, 8, QUICK_SIZE(p) / 2048);
    }

    if (mode == MODE_CHS) {
        write_string(HEADER_COLOR, 4, 4, HEADER_CHS);
        write_int(TABLE_COLOR, 9, 4, 2, dinfo.disk - 0x80 + 1);
        write_int(TABLE_COLOR, 13, 4, 8, dinfo.total_sects / 2048);
        write_int(TABLE_COLOR, 31, 4, 7, dinfo.num_cyls);
        write_int(TABLE_COLOR, 52, 4, 3, dinfo.num_heads);
        write_int(TABLE_COLOR, 65, 4, 3, dinfo.num_sects);
        write_string(HEADER_COLOR, 4, 6, HEADER_CHS2);
        write_string(HEADER_COLOR, 4, 7, HEADER_CHS3);
    } else /* mode==MODE_LBA */
    {
        write_string(HEADER_COLOR, 4, 4, HEADER_LBA);
        write_int(TABLE_COLOR, 9, 4, 2, dinfo.disk - 0x80 + 1);
        write_int(TABLE_COLOR, 13, 4, 8, dinfo.total_sects / 2048ul);
        sprintf(tmp, "%13s", sprintf_long(tmp + 100, QUICK_BASE(p)));
        write_string(TABLE_COLOR, 34, 4, tmp);
        sprintf(tmp, "%13s", sprintf_long(tmp + 100, QUICK_SIZE(p)));
        write_string(TABLE_COLOR, 55, 4, tmp);
        write_string(HEADER_COLOR, 4, 6, HEADER_LBA2);
        write_string(HEADER_COLOR, 4, 7, HEADER_LBA3);
    }
    if (dinfo.lba) {
        write_string(TABLE_COLOR, 3, 25, "LBA");
    }
} /* redraw_header */

void progress_bar(char *bar, int percent)
{
    int i;

    if (percent < 0)
        percent = 0;
    if (percent > 100)
        percent = 100;

    for (i = 0; i < percent / 2; i++)
        bar[i] = 0xB2;
    while (i != 50)
        bar[i++] = 0xB0;
    bar[i] = 0;
}

int progress(char *msg)
{
    int t = 0;
    struct event ev;
    static int prev_bar;
    static char prev_msg[21];

    if (msg != 0 && interactive == 1) {
        move_cursor(1, 26);
        if (*msg == '^') {
            border_window(Yellow + BakWhite, FmX, FmY, FmW, FmH, Border22f);
            write_string(Black + BakWhite, FmX + 2, FmY + 1, msg + 1);
            prev_bar = 0;
        } else if (*msg == '%') {
            if (prev_bar == 0 || strcmp(prev_msg, msg + 1) != 0) {
                progress_bar(tmp, atoi(msg + 1));
                write_string(Black + BakWhite, FmX + 2, FmY + 3, tmp);
                clear_window(Black + BakWhite, FmX + 2 + 50, FmY + 3, 20, 1);
                write_string(Black + BakWhite, FmX + 2 + 50, FmY + 3, msg + 1);
                prev_bar = 1;
                strncpy(prev_msg, msg + 1, 20);
            }
        } else {
            while (*msg == '~') {
                msg++;
                t += 20;
            }
            clear_window(Black + BakWhite, FmX + 2, FmY + 3, 70, 1);
            write_string(Black + BakWhite, FmX + 2, FmY + 3, msg);
            prev_bar = 0;
        }
    } /* interactive */
    else if (*msg != 0 && interactive == 0) {
        if (*msg == '^') {
            if (prev_bar == 1)
                printf("\n");
            printf("%s\n", msg + 1);
            prev_bar = 0;
        } else if (*msg == '%') {
            if (prev_bar == 0 || strcmp(prev_msg, msg + 1) != 0) {
                progress_bar(tmp, atoi(msg + 1));
                printf("\r%s%-20s", tmp, msg + 1);
                prev_bar = 1;
                strncpy(prev_msg, msg + 1, 20);
            }
        } else {
            while (*msg == '~') {
                msg++;
                t += 12;
            }
            if (prev_bar == 1)
                printf("\n");
            printf("%s\n", msg);
            prev_bar = 0;
        }
    }

    if (t == 0) {
        get_event(&ev, EV_KEY | EV_NONBLOCK);
    } else {
        ev.timer = t;
        get_event(&ev, EV_KEY | EV_TIMER);
    }

    if (ev.ev_type == EV_KEY && ev.key == 27) /* ESC */
    {
        return CANCEL;
    }

    return OK;
} /* progress */

void print_adv_menu(struct adv *adv)
{
    int i, k;

    static char *show[5] = {" ---- ", " Last ", " Next ", " Prev ", " Last3"};
    static char *part_row[5]  = {"/*", "/1", "/2", "/3", "/4"};
    static char *boot_type[5] = {"-", "C", "D", "A", "?"};

    printf("Advanced Boot Manager    Signature: %s    Advanced Data Version: "
           "%d\n\n",
           adv->signature,
           adv->version);

    printf("Title: %s\n\n", adv->adv_title);

    printf("Check boot viruses:  %s\n",
           adv->options & ADV_OPT_VIR_CHECK ? "Yes" : "No");
    printf("Clear screen:        %s\n",
           adv->options & ADV_OPT_CLEAR_SCR ? "Yes" : "No");
    printf("Boot menu timeout:   %d\n", adv->timeout);
    printf("Default boot menu:   ");
    if (adv->options & ADV_OPT_DEF_MENU)
        printf("%d\n");
    else
        printf("Not set\n");
    printf("Boot menu password:  %s\n",
           adv->password == 0 ? "Not set" : "*******");
    printf("Ignore unused part:  %s\n",
           adv->options & ADV_OPT_IGN_UNUSED ? "Yes" : "No");

    printf("\n"
           "  #  Device  Part/Row  Show  Passw   Name                        "
           "Keys\n"
           "------------------------------------------------------------------"
           "----\n");

    for (i = 0; i < MAX_MENU_ROWS; i++) {
        k = get_part_num(adv->part, adv->menu[i].x.part.tag);
        printf(" %2d    %s       %2d%s   %s   %s     %-29s %s\n",
               i + 1,
               boot_type[adv->menu[i].boot_type],
               k,
               k == 0 ? "  " : part_row[adv->part[k - 1].orig_row],
               show[adv->menu[i].x.part.show],
               adv->menu[i].options & MENU_OPT_PASSW ? "Y" : "-",
               adv->menu[i].name,
               adv->menu[i].x.part.num_keys != 0 ? "Y" : "-");
    }

} /* print_adv_menu */
