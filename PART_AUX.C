#include "part.h"

void part_to_disk_addr(struct part_long *p, unsigned long rel_sect,
                       struct disk_addr *daddr)
{
    unsigned long abs_sect;

    abs_sect = QUICK_BASE(p) + rel_sect;

    daddr->disk = dinfo.disk;
    daddr->sect = abs_sect;
} /* part_to_disk_addr */

int disk_read_rel(struct part_long *p, unsigned long rel_sect, void *buf,
                  int num_sect)
{
    struct disk_addr daddr;

    part_to_disk_addr(p, rel_sect, &daddr);

    return disk_read(&daddr, buf, num_sect);
} /* disk_read_rel */

int disk_write_rel(struct part_long *p, unsigned long rel_sect, void *buf,
                   int num_sect)
{
    struct disk_addr daddr;

    part_to_disk_addr(p, rel_sect, &daddr);

    return disk_write(&daddr, buf, num_sect);
} /* disk_write_rel */

void pack_part_tab(struct part_long *part, struct part_rec *part_rec, int n)
{
    int i;
    unsigned long rel_sect;
    unsigned long start_sect;
    unsigned long num_sect;
    unsigned long end_sect;

    for (i = 0; i < n; i++) {
        if (part[i].active == 0) {
            if (part_rec[i].boot_flag != 0)
                part_rec[i].boot_flag = 0;
        } else {
            if (part_rec[i].boot_flag == 0)
                part_rec[i].boot_flag = 0x80;
        }

        rel_sect = part[i].rel_sect;
        start_sect = rel_sect + part[i].container_base;
        num_sect = part[i].num_sect;
        end_sect = start_sect + num_sect - 1;

        part_rec[i].rel_sect = part[i].rel_sect;
        part_rec[i].num_sect = part[i].num_sect;

        part_rec[i].os_id = part[i].os_id >> 8;


        /* check if partition is out of 1024/255/63 bounds */
        /* if that is the case adjust CHS values */
        if ((num_sect > 0) && (part[i].start_cyl > 1023)) {
            part_rec[i].start_cylL = 0xFF;
            part_rec[i].start_sectCylH = 0xFF;
            part_rec[i].start_head = 0xFE;
        }
        else {
            part_rec[i].start_cylL = part[i].start_cyl & 0xFF;
            part_rec[i].start_sectCylH =
                (part[i].start_sect & 0x3f) | ((part[i].start_cyl >> 2) & 0xc0);
            part_rec[i].start_head = part[i].start_head;            
        }

        if ((num_sect > 0) && (part[i].end_cyl > 1023)) {
            part_rec[i].end_cylL = 0xFF;
            part_rec[i].end_sectCylH = 0xFF;
            part_rec[i].end_head = 0xFE;
        }
        else {
            part_rec[i].end_cylL = part[i].end_cyl & 0xFF;
            part_rec[i].end_sectCylH =
                (part[i].end_sect & 0x3f) | ((part[i].end_cyl >> 2) & 0xc0);
            part_rec[i].end_head = part[i].end_head;            
        }
    }
} /* pack_part_tab */


void unpack_part_tab(struct part_rec *part_rec, struct part_long *part, int n,
                     struct part_long *container)
{
    int i;
    unsigned long rel_sect;
    unsigned long start_sect;
    unsigned long num_sect;
    unsigned long end_sect;

    for (i = 0; i < n; i++) {
        part[i].level = container->level + 1;

        if (part[i].level > 2 && part[i].os_id == OS_EXT)
            part[i].container = container->container;
        else
            part[i].container = container;

        part[i].container_base = QUICK_BASE(part[i].container);

        rel_sect = part_rec[i].rel_sect;
        start_sect = rel_sect + part[i].container_base;
        num_sect = part_rec[i].num_sect;
        end_sect = start_sect + num_sect - 1;

        part[i].rel_sect = rel_sect;
        part[i].num_sect = part_rec[i].num_sect;

        part[i].active = (part_rec[i].boot_flag == 0) ? 0 : 1;
        part[i].os_id = part_rec[i].os_id << 8;

        part[i].start_cyl = part_rec[i].start_cylL |
                            ((part_rec[i].start_sectCylH & 0xc0) << 2);
        part[i].start_head = part_rec[i].start_head;
        part[i].start_sect = part_rec[i].start_sectCylH & 0x3f; 

        part[i].end_cyl =
            part_rec[i].end_cylL | ((part_rec[i].end_sectCylH & 0xc0) << 2);
        part[i].end_head = part_rec[i].end_head;
        part[i].end_sect = part_rec[i].end_sectCylH & 0x3f;

        if (num_sect > 0) {
            /* start is out of CHS bounds */
            if (start_sect != ABS_REL_SECT(&part[i]) &&
                    (part[i].start_cyl == 1023) &&
                    (part[i].start_sect == 63) &&
                    (part[i].start_head >= 254)) {
                part[i].start_cyl = CYL(start_sect);
                part[i].start_head = HEAD(start_sect);
                part[i].start_sect = SECT(start_sect);

            } 

            if (end_sect != (ABS_END_SECT(&part[i])) &&
                    (part[i].end_cyl == 1023) &&
                    (part[i].end_sect == 63) &&
                    (part[i].end_head >= 254)) {
                part[i].end_cyl = CYL(end_sect);
                part[i].end_head = HEAD(end_sect);
                part[i].end_sect = SECT(end_sect);
            }
        }
        else {
            part[i].start_cyl = 0;
            part[i].start_head = 0;
            part[i].start_sect = 0;
            part[i].end_cyl = 0;
            part[i].end_head = 0;
            part[i].end_sect = 0;
        }

        if (i > 0 &&
            (part[i - 1].os_id == 0x8100 || /* Linix */
             part[i - 1].os_id == 0x8300)   /* Linix ext2fs */
            && (part[i].os_id >> 8) == 0x82 /* Linix swap */) {
            part[i].os_id = 0x8201; /* Linux Swap */
        }

        determine_os_num(&part[i]);

    }
} /* unpack_part_tab */

void pack_adv_part_tab(struct part_long *part, struct adv_part_rec *part_rec,
                       int n)
{
    int i;

    for (i = 0; i < n; i++) {
        part_rec[i].os_id    = part[i].os_id;
        part_rec[i].tag      = part[i].active;
        part_rec[i].orig_row = part[i].orig_row;

        part_rec[i].rel_sect = part[i].rel_sect;
        part_rec[i].num_sect = part[i].num_sect;
    }

} /* pack_adv_part_tab */

void unpack_adv_part_tab(struct adv_part_rec *part_rec, struct part_long *part,
                         int n, struct part_long *container)
{
    int i, mode = MODE_LBA;

    for (i = 0; i < n; i++) {
        part[i].os_id    = part_rec[i].os_id;
        part[i].active   = part_rec[i].tag;
        part[i].orig_row = part_rec[i].orig_row;

        part[i].rel_sect = part_rec[i].rel_sect;
        part[i].num_sect = part_rec[i].num_sect;

        part[i].level          = container->level + 1;
        part[i].container      = container;
        part[i].container_base = QUICK_BASE(container);

        recalculate_part(&part[i], mode);

        determine_os_num(&part[i]);
    }

} /* unpack_adv_part_tab */

void determine_os_num(struct part_long *p)
{
    int i = 0;

    while (os_desc[i].os_id != p->os_id && os_desc[i].os_id != OS_UNKN)
        i++;

    if (p->os_id == 0xFF00) {
        while (os_desc[i].os_id != OS_HIDDEN)
            i++;
    }

    p->os_num = i;
} /* determine_os_num */

void recalculate_part(struct part_long *p, int mode)
{
    unsigned long rel_sect, end_sect;

    if (mode == MODE_CHS) {
        p->rel_sect = 0;
        p->num_sect = 0;

        if (p->start_sect != 0) {
            rel_sect    = ABS_REL_SECT(p);
            p->rel_sect = rel_sect - p->container_base;

            if (p->end_sect != 0) {
                end_sect = ABS_END_SECT(p);
                if (end_sect >= rel_sect)
                    p->num_sect = end_sect - rel_sect + 1;
            }
        }
    } else /* mode==MODE_LBA */
    {
        p->start_cyl  = 0;
        p->end_cyl    = 0;
        p->start_head = 0;
        p->end_head   = 0;
        p->start_sect = 0;
        p->end_sect   = 0;

        if (p->rel_sect != 0 || p->num_sect != 0) {
            rel_sect = p->container_base + p->rel_sect;
            end_sect = p->container_base + p->rel_sect + p->num_sect - 1;

            p->start_cyl  = CYL(rel_sect);
            p->start_head = HEAD(rel_sect);
            p->start_sect = SECT(rel_sect);

            if (p->num_sect != 0) {
                p->end_cyl  = CYL(end_sect);
                p->end_head = HEAD(end_sect);
                p->end_sect = SECT(end_sect);
            }
        }
    } /* LBA */

} /* recalculate_part */

int validate_table(struct part_long *part, int n, struct part_long *container2)
{
    int i, j;
    struct part_long *p, *q;
    struct part_long *container;
    unsigned long rel_sect, end_sect, num_sect;

    for (p = part, i = 0; i < n; i++, p++) {
        p->empty = 1;

        if (p->start_cyl != 0)
            p->empty = 0;
        else if (p->start_head != 0)
            p->empty = 0;
        else if (p->start_sect != 0)
            p->empty = 0;

        else if (p->end_cyl != 0)
            p->empty = 0;
        else if (p->end_head != 0)
            p->empty = 0;
        else if (p->end_sect != 0)
            p->empty = 0;

        else if (p->rel_sect != 0)
            p->empty = 0;
        else if (p->num_sect != 0)
            p->empty = 0;
    }

    for (p = part, i = 0; i < n; i++, p++) {
        if (p->empty)
            continue;

        p->valid           = 1;
        p->mbr_err         = 0;
        p->range_err       = 0;
        p->overlap_err     = 0;
        p->inconsist_err   = 0;
        p->boot_record_err = 0;

        container = p->container;

        rel_sect = ABS_REL_SECT(p);

        end_sect = ABS_END_SECT(p);

        num_sect = end_sect - rel_sect + 1;

        if (rel_sect != p->rel_sect + p->container_base)
            p->inconsist_err = 1;
        if (num_sect != p->num_sect)
            p->inconsist_err = 1;

        if (mode == MODE_CHS) {
            if (p->end_cyl < p->start_cyl)
                p->range_err = 1;
            else if (p->end_cyl == p->start_cyl && p->end_head < p->start_head)
                p->range_err = 1;
            else if (p->end_cyl == p->start_cyl &&
                     p->end_head == p->start_head &&
                     p->end_sect < p->start_sect)
                p->range_err = 1;

            if (p->end_cyl > container->end_cyl)
                p->range_err = 1;
            else if (p->end_cyl == container->end_cyl &&
                     p->end_head > container->end_head)
                p->range_err = 1;
            else if (p->end_cyl == container->end_cyl &&
                     p->end_head == container->end_head &&
                     p->end_sect > container->end_sect)
                p->range_err = 1;

            if (p->start_cyl < container->start_cyl)
                p->range_err = 1;
            else if (p->start_cyl == container->start_cyl &&
                     p->start_head < container->start_head)
                p->range_err = 1;
            else if (p->start_cyl == container->start_cyl &&
                     p->start_head == container->start_head &&
                     p->start_sect < container->start_sect)
                p->range_err = 1;

            if (p->start_cyl >= dinfo.num_cyls)
                p->range_err = 1;
            if (p->end_cyl >= dinfo.num_cyls)
                p->range_err = 1;

            if (p->start_head >= dinfo.num_heads)
                p->range_err = 1;
            if (p->end_head >= dinfo.num_heads)
                p->range_err = 1;

            if (p->start_sect > dinfo.num_sects)
                p->range_err = 1;
            if (p->end_sect > dinfo.num_sects)
                p->range_err = 1;

            if (p->start_sect == 0)
                p->range_err = 1;
            if (p->end_sect == 0)
                p->range_err = 1;

            if (p->start_cyl == container->start_cyl &&
                p->start_head == container->start_head &&
                p->start_sect == container->start_sect)
                p->mbr_err = 1;
        } else /* mode==MODE_LBA */
        {
            if (p->rel_sect == 0)
                p->mbr_err = 1;
            if (p->rel_sect >= container->num_sect)
                p->range_err = 1;
            if (p->num_sect - 1 > container->num_sect)
                p->range_err = 1;
            if (p->rel_sect + p->num_sect - 1 >= container->num_sect)
                p->range_err = 1;

            rel_sect = p->rel_sect + p->container_base;
            end_sect = rel_sect + p->num_sect - 1;
        }

        for (q = part, j = 0; j < n; j++, q++) {
            unsigned long q_rel_sect, q_end_sect;

            if (i == j || q->empty)
                continue;

            q_rel_sect = (mode == MODE_CHS)
                             ? ABS_REL_SECT(q)
                             : (q->rel_sect + q->container_base);
            q_end_sect = (mode == MODE_CHS) ? ABS_END_SECT(q)
                                            : (q_rel_sect + q->num_sect - 1);

            if (rel_sect >= q_rel_sect && rel_sect <= q_end_sect)
                p->overlap_err = 1;
            if (q_rel_sect >= rel_sect && q_rel_sect <= end_sect)
                p->overlap_err = 1;
        } /* q */

        if (p->mbr_err || p->range_err || p->overlap_err || p->inconsist_err)
            p->valid = 0;
    } /* p */

    for (i = 0; i < n; i++)
        if (container2->level > 1 && part[i].os_id == OS_EXT &&
            part[i].start_cyl < container2->end_cyl + 1) {
            part[i].range_err = 1;
            part[i].valid     = 0;
        }

    for (i = 0; i < n; i++)
        if (part[i].empty == 0 && part[i].valid == 0)
            return 0;

    return 1;
} /* validate_table */

int write_int(int attr, int x, int y, int w, unsigned long xx)
{
    char tmp[30];

    sprintf(tmp, "%*lu", w, xx);
    write_string(attr, x, y, tmp);

    return 0;
} /* write_int */

char *sprintf_long(char *tmp, unsigned long num)
{
    int i    = 13; /* 1,111,111,111 */
    tmp[i--] = 0;  /* 0 234 678 012 */

    if (num == 0) {
        tmp[i--] = '0';
    }

    while (num != 0) {
        if (i == 9 || i == 5 || i == 1)
            tmp[i--] = ',';
        tmp[i--] = num % 10 + '0';
        num /= 10;
    }

    return tmp + i + 1;
} /* sprintf_long */

char *sprintf_systype(char *tmp, int i)
{
    sprintf(tmp,
            " 0x%02X   %-23s     %s     %s      %s ",
            os_desc[i].os_id >> 8,
            os_desc[i].name,
            (os_desc[i].setup != 0) ? "Yes" : " - ",
            (os_desc[i].format != 0) ? "Yes" : " - ",
            (os_desc[i].print != 0) ? "Yes" : " - ");
    return tmp;
} /* sprintf_systype */

char *sprintf_os_name(char *tmp, struct part_long *p)
{
    if (os_desc[p->os_num].os_id != OS_UNKN)
        sprintf(tmp, "%-23s", os_desc[p->os_num].name);
    else
        sprintf(tmp, NAME_OS_UNKN, p->os_id >> 8);

    return tmp;
} /* sprintf_os_name */

char *format_size(unsigned long num_sect, char *tmp2)
{
    char *tmp3;

    if (num_sect >=  20480000ul) {
        tmp3 = sprintf_long(tmp2, num_sect / (2048ul*1024ul));
        tmp2[14]='G';
    }
    else if (num_sect >= 20000ul) {
        tmp3 = sprintf_long(tmp2, num_sect / (2048ul));
        tmp2[14]='M';            
    }
    else if (num_sect > 0) {
        tmp3 = sprintf_long(tmp2, num_sect / (2ul));
        tmp2[14]='K';                        
    }
    else {
        memset(tmp2, ' ', 16);
        tmp2[16]='\0';
        return tmp2;
    }
    tmp2[13] = ' ';
    tmp2[15] = 'i';
    tmp2[16] = '\0';

    return tmp3;
}

char *sprintf_partrec(char *tmp, struct part_long *p, int num, int view)
{
    char tmp1[30], tmp2[30], *tmp3;


    if (mode == MODE_CHS) {
        tmp3 = format_size(p->num_sect, tmp2);
        sprintf(tmp,
                "%2d %-4s %s%7lu %3lu %3lu %7lu %3lu %3lu %12s ",
                num,
                p->active ? (view == VIEW_ADV ? "Menu" : " Yes") : " No",
                sprintf_os_name(tmp1, p),
                p->start_cyl,
                p->start_head,
                p->start_sect,
                p->end_cyl,
                p->end_head,
                p->end_sect,
                tmp3);
    } else /* MODE_LBA */
    {
        tmp3 = format_size(p->num_sect, tmp2);

        sprintf(tmp,
                "%2d %-4s %s%10lu %10lu %10lu%12s ",
                num,
                p->active ? (view == VIEW_ADV ? "Menu" : " Yes") : " No",
                sprintf_os_name(tmp1, p),
                p->rel_sect,
                p->num_sect,
                (p->num_sect == 0) ? (0) : (p->rel_sect + p->num_sect - 1),
                tmp3);
    }

    return tmp;
} /* sprintf_part */

void usage(void)
{
    printf(HELP_CMD_LINE);
    exit(1);
}

void cmd_error(char *msg)
{
    if (!quiet)
        fprintf(stderr, "\nError: %s\n", msg);
    exit(1);
}

unsigned short calc_chksum(const unsigned short *p, const int len)
{
    unsigned short sum = 0;
    const unsigned short *const e = p+len;
    
    while (p < e) {
        sum = sum + *(p++);
    }

    return sum;
}

int prepare_adv_mbr_for_save(struct part_long *part, struct mbr *mbr,
                             struct adv *adv)
{
    int i, j;
    struct part_long h;
    unsigned short chksum;

    for (i = 0; i < MAX_PART_ROWS; i++)
        if (part[i].os_id == OS_ADV)
            break;

    if (i == MAX_PART_ROWS || part[i].num_sect < ADV_NUM_SECTS)
        return 0;

    memcpy(mbr->x.adv.code, ADV_IPL, sizeof(mbr->x.adv.code));
    mbr->x.adv.rel_sect      = part[i].rel_sect;
    mbr->x.adv.adv_chk_sum   = 0;
    mbr->x.adv.reserved      = 0;
    mbr->x.adv.act_menu      = 0;
    mbr->x.adv.boptions      = 0;
    mbr->x.adv.adv_mbr_magic = ADV_MBR_MAGIC;
    
    /* we calculate the checksum of the IPL code and store it
       into the code. Then if part.exe computes the checksum it must be equal to
       ADV_IPL_CHECKSUM_MAGIC.
       Otherwise IPL got corrupted or it is the wrong version. */
    chksum = calc_chksum((unsigned short*)mbr->x.adv.code, sizeof(mbr->x.adv.code) / 2);
    chksum = ADV_IPL_CHKSUM_MAGIC - chksum;

    mbr->x.adv.code[ADV_IPL_CHKSUM_OFFSET+0] = (chksum) & 0xff;
    mbr->x.adv.code[ADV_IPL_CHKSUM_OFFSET+1] = (chksum >> 8) & 0xff;

    memset(mbr->part_rec, 0, sizeof(mbr->part_rec));
    h.active         = 1;
    h.rel_sect       = 1;
    h.num_sect       = dinfo.total_sects - 1;
    h.container_base = 0;
    h.container      = part[i].container;
    h.os_id          = OS_HIDDEN;
    recalculate_part(&h, MODE_LBA);
    pack_part_tab(&h, mbr->part_rec, 1);

    if (adv->options & ADV_OPT_IGN_UNUSED) {
        for (i = 0; i < MAX_PART_ROWS; i++)
            if (adv->part[i].os_id == 0) {
                for (j = i; j < MAX_PART_ROWS - 1; j++)
                    adv->part[j] = adv->part[j + 1];
                memset(&adv->part[j], 0, sizeof(struct adv_part_rec));
            }
    }

    /* BUG: this overwrites part of ADV_IPL!!! */
    /*strcpy(mbr->x.adv_old.signature, ADV_DATA_SIGNATURE);
    mbr->x.adv_old.version = ADV_DATA_VERSION;*/

    strcpy(adv->signature, ADV_DATA_SIGNATURE);
    adv->version = ADV_DATA_VERSION;

    /* add boot menu title only of not customized by user */
    if (strncmp(adv->adv_title, MANAGER_TITLE, sizeof(adv->adv_title) - 8) ==
        0)
        strncpy(adv->adv_title, MANAGER_TITLE, sizeof(adv->adv_title));

    return 1;
} /* prepare_adv_mbr_for_save */

int save_to_file(char *filename, void *buf, int len)
{
    FILE *f;

    if ((f = fopen(filename, "wb")) == 0)
        return -1;

    if (fwrite(buf, 1, len, f) != len)
        return -1;

    fclose(f);
    return 0;
} /* save_to_file */

int load_from_file(char *filename, void *buf, int len)
{
    FILE *f;

    if ((f = fopen(filename, "rb")) == 0)
        return -1;

    if (fread(buf, 1, len, f) != len)
        return -1;

    fclose(f);
    return 0;
} /* load_from_file */

void get_base_dir(char *path)
{
    char *p;

    p = path + strlen(path) - 1;

    while (p != path && *p != '\\' && *p != '/')
        p--;

    if (*p == '\\' || *p == '/')
        strncpy(base_dir, path, p - path + 1);
    else
        base_dir[0] = 0;

} /* get_base_dir */

void parse_arg(char *str, char **argv, int argv_size)
{
    int i   = 0;
    char *p = str;

    while (1) {
        while (*p == ' ' || *p == '\t')
            p++;
        if (*p == 0)
            break;
        if (i >= argv_size - 1)
            break;
        argv[i++] = p;
        while (*p != ' ' && *p != '\t' && *p != 0)
            p++;
        if (*p == 0)
            break;
        *p = 0;
        p++;
    }

    argv[i] = 0;
} /* parse_arg */

void print_part_details(struct part_long *part, int num_rows)
{
    int i;
    /*
       HardDisk 1   1236 MB  [ 1023 cyl x 255 heads x 63 sects = 11,111,111
    sects ]

    Valid            Starting       Ending    Starting  Numb of   Ending
    Partition |   # HD  FS  Cyl Head Sct  Cyl Head Sct   sector  sectors sector
    Size [KB] OK   1 80  FF 1023  255  63 1023  255  63 12345678 12345678
    12345678  1,111,111 X   2  0  FF 1023  255  63 1023  255  63 12345678
    12345678 12345678  1,111,111 3  0  00    0    0   0    0    0   0        0
    0 4  0  00    0    0   0    0    0   0        0        0
    */
    printf("Valid            Starting       Ending    Starting  Numb of   "
           "Ending  Partition\n"
           " |   # %3s FS  Cyl Head Sct  Cyl Head Sct   sector  sectors   "
           "sector  Size [KB]\n\n",
           num_rows == 4 ? "HD " : "Row");

    for (i = 0; i < num_rows; i++) {
        if (part[i].empty)
            continue;

        printf("%s %2d %2X  %02X %4lu  %3lu  %2lu %4lu  %3lu  %2lu %8lu %8lu "
               "%8lu %10s\n",
               (part[i].valid) ? "OK " : " X ",
               i + 1,
               num_rows == 4 ? 0x80 : part[i].orig_row,
               part[i].os_id >> 8,
               part[i].start_cyl,
               part[i].start_head,
               part[i].start_sect,
               part[i].end_cyl,
               part[i].end_head,
               part[i].end_sect,
               part[i].rel_sect,
               part[i].num_sect,
               (part[i].num_sect == 0)
                   ? (0)
                   : (part[i].rel_sect + part[i].num_sect - 1),
               sprintf_long(tmp, part[i].num_sect / 2));
    }

    printf("\n");

    for (i = 0; i < num_rows; i++)
        if (!part[i].empty && !part[i].valid) {
            printf("Errors in record %d:", i + 1);
            if (part[i].mbr_err)
                printf("  mbr");
            if (part[i].range_err)
                printf("  range");
            if (part[i].overlap_err)
                printf("  overlapped");
            if (part[i].inconsist_err)
                printf("  inconsistent");
            if (part[i].boot_record_err)
                printf("  boot_record_invalid");
            printf("\n");
        }

} /* print_part_details */

int print_embr(struct part_long *p)
{
    int i;
    struct mbr *mbr        = (struct mbr *)buf;
    struct part_long *part = malloc(4 * sizeof(struct part_long));

    if (part == 0)
        cmd_error(ERROR_MALLOC);

    if (disk_read_rel(p, 0, mbr, 1) == -1) {
        fprintf(stderr, "Warning: %s\n\n", ERROR_READ_MBR);
        return 0;
    }

    printf("EMBR Level %2d      Magic number: 0x%04X\n\n",
           p->level,
           mbr->magic_num);

    if (p->level > 30)
        cmd_error("This is too deep.");

    unpack_part_tab(mbr->part_rec, part, 4, p);

    validate_table(part, 4, p);

    printf("Valid%s\n |   %s\n\n", HEADER_CHS2, HEADER_CHS3);

    for (i = 0; i < 4; i++) {
        sprintf_partrec(tmp, &part[i], i + 1, VIEW_EMBR);
        tmp[75] = 0;
        printf("%s %s\n",
               part[i].empty ? "   " : (part[i].valid ? "OK " : " X "),
               tmp);
    }

    printf("\n");

    print_part_details(part, 4);

    for (i = 0; i < 4; i++)
        if (!part[i].empty && part[i].valid && part[i].os_id != 0 &&
            part[i].os_id != OS_ADV) {
            printf("\n\n------------------------------------------------------"
                   "-------------------------\n\n"
                   "%s  [  %luM = %s sectors  at  CHS=(%lu,%lu,%lu)  ]\n\n",
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

    free(part);
    return 0;
} /* print_embr */

void convert_adv_data(struct adv *adv)
{
    struct adv_v2
    {
        unsigned char signature[15]; /* AdvBootManager\0 */
        unsigned char version;       /* Data structure version */
        unsigned long rel_sect;
        unsigned char act_menu;
        unsigned char reserved[9];
        unsigned char def_menu;
        unsigned char timeout;
        unsigned char options;
        unsigned char options2;
        unsigned long password; /* code to validate password */
        unsigned char reserved2[10];
        unsigned char adv_title[32];
        struct adv_menu_rec menu[MAX_MENU_ROWS]; /* 1280 */
        struct adv_part_rec part[MAX_PART_ROWS]; /*  512 */
    } *adv_v2 = (struct adv_v2 *)adv;

    adv->version  = ADV_DATA_VERSION;
    adv->def_menu = adv_v2->def_menu;
    adv->timeout  = adv_v2->timeout;
    adv->options  = adv_v2->options;
    adv->options2 = adv_v2->options2;
    adv->password = 0;
    memset(adv->reserved2, 0, sizeof(adv->reserved2));

    if (interactive) {
        show_error("Advanced MBR had an old version. It was converted to the "
                   "new one.");
        /*
            show_error("If you don't want to save it press Ctrl-F10 for
           unconditional exit.");
        */
    } else {
        printf("Advanced MBR had an old version. It was converted to the new "
               "one.\n");
    }
} /* convert_adv_data */
