/*
 * $Id: mdns_utils.c,v 1.3 2015/01/13 06:59:01 kevinwu Exp $
 *
 * $Copyright: (c) 2013 Broadcom Corp.
 * All Rights Reserved.$
 *
 */


#include "system.h"

#ifdef CFG_ZEROCONF_MDNS_INCLUDED

#include "appl/mdns.h"


/* Local Domains */
/* local */
CODE const uint8 dns_local_domain[MDNS_LOCAL_DOMAIN_LEN] = 
    {0x05, 0x6c, 0x6f, 0x63, 0x61, 0x6c};

/* 254.169.in-addr.arpa */
CODE const uint8 ipv4_rev_domain[MDNS_IPV4_REV_DOMAIN_LEN] = 
    {0x03, 0x32, 0x35, 0x34, 0x03, 0x31, 0x36, 0x39, 0x07, 0x69, 
     0x6e, 0x2d, 0x61, 0x64, 0x64, 0x72, 0x04, 0x61, 0x72, 0x70, 
     0x61};

/* {8,9,a,b}.e.f.ip6.arpa */
CODE const uint8 ipv6_rev_domain1[MDNS_IPV6_REV_DOMAIN_LEN] = 
    {0x01, 0x38, 0x01, 0x65, 0x01, 0x66, 0x03, 0x69, 0x70, 0x36, 
     0x04, 0x61, 0x72, 0x70, 0x61};
CODE const uint8 ipv6_rev_domain2[MDNS_IPV6_REV_DOMAIN_LEN] = 
    {0x01, 0x39, 0x01, 0x65, 0x01, 0x66, 0x03, 0x69, 0x70, 0x36, 
     0x04, 0x61, 0x72, 0x70, 0x61};
CODE const uint8 ipv6_rev_domain3[MDNS_IPV6_REV_DOMAIN_LEN] = 
    {0x01, 0x61, 0x01, 0x65, 0x01, 0x66, 0x03, 0x69, 0x70, 0x36, 
     0x04, 0x61, 0x72, 0x70, 0x61};
CODE const uint8 ipv6_rev_domain4[MDNS_IPV6_REV_DOMAIN_LEN] = 
    {0x01, 0x62, 0x01, 0x65, 0x01, 0x66, 0x03, 0x69, 0x70, 0x36, 
     0x04, 0x61, 0x72, 0x70, 0x61};



/* 
 * Function:
 *   mdns_convert_string2label
 * Description:
 *   Convert the domain string to domain label format.
 * Parameters:
 *   name : domain name string
 *   domain_label : domain name label format 
 * Returns:
 *   SYS_OK
 */

sys_error_t
mdns_convert_string2label(const     char *name, uint8 *domain_label)
{
    uint8   *c, str_len, label_len, i;

    str_len = sal_strlen(name);
    c = domain_label;
    label_len = 0;
    for (i = 0; i < str_len; i++) {
        if (name[i] == '.') {
            *c = label_len;
            c += (i + 1);
            label_len = 0;
            
        } else {
            domain_label[i+1] = name[i];
            label_len++;
        }
    }

    return SYS_OK;
    
}


/* 
 * Function:
 *   mdns_string_shift_down
 * Description:
 *   
 * Parameters:
 *   str : the whole string
 *   name_start : the start position of the name
 *   name_len : the length of the domain name
 *   movement : the distance to move
 * Returns:
 *   SYS_OK
 */

sys_error_t
mdns_string_shift_down(uint8 *str, uint8 name_start, 
        uint8 name_len, uint8 movement)
{
    uint8   i;

    for (i = (name_start + name_len); i >= name_start; i--) {
        str[i + movement] = str[i];
    }

    return SYS_OK;
}

/* 
 * Function:
 *   mdns_string_shift_up
 * Description:
 *   
 * Parameters:
 *   str : the whole string
 *   name_start : the start position of the name
 *   name_len : the length of the domain name
 *   movement : the distance to move
 * Returns:
 *   SYS_OK
 */

sys_error_t
mdns_string_shift_up(uint8 *str, uint8 name_start, 
        uint8 name_len, uint8 movement)
{
    uint8   i;

    for (i = name_start; i <= (name_start + name_len); i++) {
        str[i - movement] = str[i];
    }
    return SYS_OK;
}



/* 
 * Function:
 *   mdns_rdata_cmp
 * Description:
 *   Compare two resource data with 
 * Parameters:
 *   rd1_len : the length of rdata1
 *   rdata1 : the data of record 1
 *   rd2_len : the length of rdata2
 *   rdata2 : the data of record 2
 * Returns:
 *   negative value if record 1 is smaller than record 2.
 *   positive value if record 2 is bigger than record 2.
 *   zero is record 1 is the same as record 2.
 */

int8
mdns_rdata_cmp(uint16 rd1_len, uint8 *rdata1, uint16 rd2_len, uint8 *rdata2)
{
    int16   i, len;

    /* 3. Compare the Rdata */
    if (rd1_len > rd2_len) {
        len = rd2_len;
    } else {
        len = rd1_len;
    }

    for (i = 0; i < len; i++) {
        if (rdata1[i] > rdata2[i]) {
            return (1);
        } else if (rdata1[i] < rdata2[i]) {
            return (-1);
        }
    }

    if (rd1_len > rd2_len) {
        return (1);
    } else if (rd1_len < rd2_len) {
        return (-1);
    }

    return (0);
}

/* 
 * Function:
 *   mdns_same_domain
 * Description:
 *   Check the name1 and name2 are the same domain name
 * Parameters:
 *   name1 : domain name 1
 *   name2 : domain name 2
 *   len : compare length 
 * Returns:
 *   TRUE : name1 and name2 are identical
 *   FALSE : name1 and name 2 are different domain name
 */

BOOL
mdns_same_domain(const uint8 *name1, const uint8 *name2, uint16 len)
{
    uint16  i;
    uint8  
c1, c2;
    
    if ((name1 == NULL) || (name2 == NULL)) {
        return FALSE;
    }

    for (i = 0; i < len; i++) {
        c1 = name1[i];
        c2 = name2[i];
        if (MDNS_IS_UPPER_CASE(c1)) {
            c1 += 'a' - 'A';
        }
        if (MDNS_IS_UPPER_CASE(c2)) {
            c2 += 'a' - 'A';
        }
        if (c1 != c2) {
            return FALSE;
        }
    }

    return TRUE;
}


/* 
 * Function:
 *   mdns_local_domain_check
 * Description:
 *   Check if the input name is in local doamin
 * Parameters:
 *   name : input domain name
 * Returns:
 *   TRUE: the name is in local domain.
 *   FALSE : the name is not in local domain
 */

BOOL
mdns_local_domain_check(uint8 *name)
{
    uint8   *p1, *p2, *p3, *p4, *p5, *p;
    
    /* 
     * Check if the input name is in local doman 
     * or the reserved IPv4 and IPv6 link local domain
     * IPv4 : 254.169.in-addr.arpa
     * IPv6 : 8.e.f.ip6.arpa, 9.e.f.ip6.arpa, a.e.f.ip6.arpa and b.e.f.ip6.arpa
     */
    /* get the last 5 labels to compare with */
    p1 = p2 = p3 = p4 = p5 = NULL;
    p = name;
    while (p != 0x0) {
        p5 = p4;
        p4 = p3;
        p3 = p2;
        p2 = p1;
        p1 = p;
        p += (*p);
            
    }
    
    /* check local doamin */
    if (mdns_same_domain(dns_local_domain, p1, MDNS_LOCAL_DOMAIN_LEN)) {
        return TRUE;
    }

    /* check IPv4 reserved link-local address */
    if (mdns_same_domain(ipv4_rev_domain, p4, MDNS_IPV4_REV_DOMAIN_LEN)) {
        return TRUE;
    }

    /* check IPv6 reserved link-local address */
    if (mdns_same_domain(ipv6_rev_domain1, p5, MDNS_IPV6_REV_DOMAIN_LEN)) {
        return TRUE;
    }
    if (mdns_same_domain(ipv6_rev_domain2, p5, MDNS_IPV6_REV_DOMAIN_LEN)) {
        return TRUE;
    }
    if (mdns_same_domain(ipv6_rev_domain3, p5, MDNS_IPV6_REV_DOMAIN_LEN)) {
        return TRUE;
    }
    if (mdns_same_domain(ipv6_rev_domain4, p5, MDNS_IPV6_REV_DOMAIN_LEN)) {
        return TRUE;
    }

    return FALSE;
}

#endif /* CFG_ZEROCONF_MDNS_INCLUDED */

