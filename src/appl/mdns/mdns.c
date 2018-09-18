/*
 * $Id: mdns.c,v 1.23 Broadcom SDK $
 *
 * $Copyright: Copyright 2016 Broadcom Corporation.
 * This program is the proprietary software of Broadcom Corporation
 * and/or its licensors, and may only be used, duplicated, modified
 * or distributed pursuant to the terms and conditions of a separate,
 * written license agreement executed between you and Broadcom
 * (an "Authorized License").  Except as set forth in an Authorized
 * License, Broadcom grants no license (express or implied), right
 * to use, or waiver of any kind with respect to the Software, and
 * Broadcom expressly reserves all rights in and to the Software
 * and all intellectual property rights therein.  IF YOU HAVE
 * NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
 * IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
 * ALL USE OF THE SOFTWARE.  
 *  
 * Except as expressly set forth in the Authorized License,
 *  
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof,
 * and to use this information only in connection with your use of
 * Broadcom integrated circuit products.
 *  
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
 * PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
 * OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
 * INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
 * ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
 * TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
 * THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
 * ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$
 *
 */


#include "system.h"

#ifdef CFG_ZEROCONF_MDNS_INCLUDED
#include "utils/net.h"
#include "uip.h"
#ifdef __BOOTLOADER__
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
#include "uip_arp.h"
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
#endif /* __BOOTLOADER__ */

#include "appl/mdns.h"
#include "appl/serialize.h"


#define MDNS_DEBUG 0

#if MDNS_DEBUG
#define MDNS_DBG(x)    do { sal_printf("MDNS: "); sal_printf x; } while(0)
#else
#define MDNS_DBG(x)
#endif


/* uIP packet buffer */
#define DNSBUF ((struct dns_hdr *)&uip_buf[UIP_IPUDPH_LEN + UIP_LLH_LEN])
#define UDPBUF ((struct uip_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#if CFG_UIP_IPV6_ENABLED
#define DNS6BUF ((struct dns_hdr *)&uip_buf[UIP6_IPUDPH_LEN + UIP_LLH_LEN])
#define UDP6BUF ((struct uip6_udpip_hdr *)&uip_buf[UIP_LLH_LEN])
#endif /* CFG_UIP_IPV6_ENABLED */



STATIC mdns_state_t mdns;
STATIC int      dns_buf_offset;
STATIC uint8    *dns_buf_ptr;



STATIC CODE const char local_domain[MDNS_LOCAL_DOMAIN_LEN] =
    {"\x5local"};


STATIC CODE const char dns_query[MDNS_GENERAL_QUERY_LEN] =
    {"\x9_services\x7_dns-sd\x4_udp\x5local\x0"};


STATIC CODE const uint8 mdns_ip4[] = {0xe0, 0x00, 0x00, 0xfb};
STATIC CODE const uint8 mdns_mac[] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0xfb};
#if CFG_UIP_IPV6_ENABLED
STATIC CODE const uint8 mdns_ip6[] = {0xff, 0x02, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0x00,
                                     0x00, 0x00, 0x00, 0xfb};
#endif /* CFG_UIP_IPV6_ENABLED */


STATIC dns_service_info_t *http_srv;
STATIC CODE const char  http_srv_name[] = CFG_MDNS_DEFAULT_HTTP_INSTANCE;
STATIC CODE const char  http_srv_type[] = "_http._tcp.";
STATIC CODE const char  http_srv_domain[] = "local.";
STATIC  uint8   http_txt_rdata[DNS_RR_RDATA_MAX_LEN];


extern int32
mdns_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT;

extern void udp_conns_periodic(void);


STATICFN void mdns_probe_task(uint8 type);
STATICFN void mdns_announce_task(uint8 type);
STATICFN void mdns_timer_trigger(void);
#ifndef __BOOTLOADER__
STATICFN void mdns_reply_list_add(void);
STATICFN void mdns_answer_task(uint8 type);
STATICFN void mdns_goodbye_task(uint8 type);
#endif /* !__BOOTLOADER__ */



/*
 * Function:
 *   mdns_packet_domain_name_ignore
 * Description:
 *   Ingone domain name and return the new data pointer
 * Parameters:
 *    data_ptr : current section pointer
 * Returns:
 *    NULL : error or not enough question to bypass
 *    ptr : new data pointer
 */
STATICFN uint8*
mdns_packet_domain_name_ignore(uint8 *data_ptr)
{
    uint8 len, *ptr;

    if (data_ptr == NULL) {
        return NULL;
    }

    ptr = data_ptr;
    len = *ptr; /* label length */
    while (len != 0x0) {

        switch (len & DNS_COMPRESS_FLAG) {
            case 0x00:
                ptr += (len + 1);
                break;
            case 0x40:
            case 0x80:
                /* not supported */
                return NULL;
                break;
            case 0xc0:
                /*compression tag */
                ptr += 2;
                return ptr;
        }
        len = *ptr;
    }

    ptr++; /* zero length byte */
    return ptr;

}

#if MDNS_DEBUG
STATICFN void
mdns_domain_name_print(uint8 *name)
{
    uint8 *ptr,i;
    uint8 index;

    index = 0;
    ptr = name;

    while (ptr[index]) {
        for (i = 1; i <= ptr[index]; i++) {
            sal_printf("%c",ptr[index + i]);
        }
        index += ptr[index] + 1;
        sal_printf(".");

    }
    sal_printf("\n");
}
#endif /* MDNS_DEBUG */

/*
 * Function:
 *   mdns_packet_domain_name_parse
 * Description:
 *   Retrive the domain name from the data pointer
 * Parameters:
 *    data_ptr : current section pointer
 *    name : domain name pointer
 *    name_len : the length of domain name
 * Returns:
 *    NULL : error or not enough question to bypass
 *    ptr : new data pointer
 */
STATICFN sys_error_t
mdns_packet_domain_name_parse(uint8 *data_ptr, uint8 *name,
        uint8 *name_len)        REENTRANT
{
    uint8 len, index, offset, sub_name_len;
    uint8 *ptr, *name_ptr, *sub_name_ptr;
    sys_error_t rv;

    if ((data_ptr == NULL) || (name == NULL)) {
        return NULL;
    }

    ptr = data_ptr;
    name_ptr = name;
    len = *ptr; /* label length */
    index = 0;
    while (len != 0x0) {

        switch (len & DNS_COMPRESS_FLAG) {
            case 0x00:
                sal_memcpy(&name_ptr[index], ptr, (len + 1));
                ptr += (len + 1);
                index += (len + 1);
                break;
            case 0x40:
            case 0x80:
                /* not supported */
                return SYS_ERR;
                break;
            case 0xc0:
                /*compression tag */
                offset = *(ptr + 1);
                sub_name_len = 0;
                sub_name_ptr = (uint8 *)dns_buf_ptr;
                rv = mdns_packet_domain_name_parse(&sub_name_ptr[offset],
                    &name[index], &sub_name_len);
                if (rv) {
                    return rv;
                }
                index += 2;
                *name_len = index;
                return SYS_OK;
        }
        len = *ptr;
    }
    name[index++] = 0x0; /* zero length */
    *name_len = index;

    return SYS_OK;

}


/*
 * Function:
 *   mdns_packet_question_ignore
 * Description:
 *   Ingone the number of questions and advance the data pointer
 * Parameters:
 *    data_ptr : current question section pointer
 *    num_q : number of question to pass
 * Returns:
 *    NULL : error or not enough question to bypass
 *    ptr : new data pointer
 */
STATICFN uint8 *
mdns_packet_question_ignore(uint8 *data_ptr, int num_q)
{
    int i;
    uint8 *ptr;

    if (data_ptr == NULL) {
        return NULL;
    }

    ptr = data_ptr;
    for (i=0; i < num_q; i++) {
        /* ignore query name */
        ptr = mdns_packet_domain_name_ignore(ptr);
        if (ptr == NULL) {
            return NULL;
        }
        /* ignore type and class */
        ptr += 4;
    }

    return ptr;
}

/*
 * Function:
 *   mdns_packet_record_ignore
 * Description:
 *   Ingone the number of records and advance the data pointer
 * Parameters:
 *    data_ptr : current section pointer
 *    num_rr : number of record to bypass
 * Returns:
 *    NULL : error or not enough records to bypass
 *    ptr : new data pointer
 */
STATICFN uint8 *
mdns_packet_record_ignore(uint8 *data_ptr, int num_rr)
{
    int i;
    uint16 rd_len;
    uint8 *ptr;

    if (data_ptr == NULL) {
        return NULL;
    }

    ptr = data_ptr;
    for (i=0; i < num_rr; i++) {
        /* ignore record name */
        ptr = mdns_packet_domain_name_ignore(ptr);
        if (ptr == NULL) {
            return NULL;
        }
        /* ignore type, class, ttl */
        ptr += 8;

        /* ignore rdlength and rdata */
        rd_len = ((*ptr) << 8) | *(ptr + 1);
        ptr += (rd_len + 2);
    }

    return ptr;
}

/*
 * Function:
 *   mdns_packet_section_pointer
 * Description:
 *   Return the pointer of DNS section types
 * Parameters:
 *    dns_data : the pointer of DNS header
 *    sec_type : DNS section types
 * Returns:
 */
STATICFN uint8*
mdns_packet_section_pointer(struct dns_hdr *dns_data, int sec_type)
{
    int num;
    uint8 *ptr;

    if (dns_data == NULL) {
        return NULL;
    }

    if ((sec_type > MDNS_PKT_SEC_TYPE_ADDI) ||
        (sec_type < MDNS_PKT_SEC_TYPE_QUEST)) {
        return NULL;
    }

    ptr = (uint8 *)dns_data;
    ptr += DNS_HDR_LEN;
    /* Question section */
    if (sec_type == MDNS_PKT_SEC_TYPE_QUEST) {
        return ptr;
    }
    /* Ignore Question section */
    num = uip_htons(dns_data->qdcount);
    ptr = mdns_packet_question_ignore(ptr, num);
    if (sec_type == MDNS_PKT_SEC_TYPE_ANSWER) {
        return ptr;
    }

    /* Ignore Answer section */
    num = uip_htons(dns_data->ancount);
    ptr = mdns_packet_record_ignore(ptr, num);
    if (sec_type == MDNS_PKT_SEC_TYPE_AUTH) {
        return ptr;
    }

    /* Ignore Auth section */
    num = uip_htons(dns_data->nscount);
    ptr = mdns_packet_record_ignore(ptr, num);

    /* For additional section */
    return ptr;


}


/*
 * Function:
 *   mdns_packet_record_data_get
 * Description:
 *   Get the required rdata pointer by the name, type and class
 * Parameters:
 *    section : section type
 *    name : required record name
 *    rtype : required record type
 *    rclass : required record class
 * Returns:
 *    the data pointer started at the field of rdlength
 */
STATICFN uint8 *
mdns_packet_record_data_get(uint8 section, uint8 *name,
    uint16 rtype, uint16 rclass)            REENTRANT
{
    uint8           num_rr, i, name_len;
    uint8           *rr_ptr;
    uint8           rr_name[DNS_RR_NAME_MAX_LEN];
    uint16          rr_type, rr_class;
    sys_error_t     rv;
    struct dns_hdr *dns_hdr;


    dns_hdr = (struct dns_hdr *)dns_buf_ptr;
    /* Get the number resource records on the section */
    switch (section) {
        case MDNS_PKT_SEC_TYPE_ANSWER:
            num_rr = uip_htons(dns_hdr->ancount);
            break;
        case MDNS_PKT_SEC_TYPE_AUTH:
            num_rr = uip_htons(dns_hdr->nscount);
            break;
        case MDNS_PKT_SEC_TYPE_ADDI:
            num_rr = uip_htons(dns_hdr->arcount);
            break;
        default:
            return NULL;
    }
    if (num_rr == 0) {
        return NULL;
    }

    rr_ptr = mdns_packet_section_pointer((struct dns_hdr *)dns_buf_ptr, section);
    if (rr_ptr == NULL) {
        return NULL;
    }

    for (i = 0; i < num_rr; i++) {

        /* Parse name, type and class */
        rv = mdns_packet_domain_name_parse(rr_ptr, rr_name, &name_len);
        if (rv) {
            return NULL;
        }

        /* Check the name */
        if (mdns_same_domain(name, rr_name, name_len)) {
            rr_type = rr_ptr[name_len + 1];
            rr_class = (rr_ptr[name_len + 2] << 8) | rr_ptr[name_len + 3];
            if ((rr_type == rtype) && ((rr_class & 0xff) == (rclass & 0xff))) {
                /* return the pointer of rdlength */
                /* Bypass rtype, rclass and ttl */
                return (&rr_ptr[name_len + 8]);
            }
        }

        /* Move to the next*/
        rr_ptr = mdns_packet_record_ignore(rr_ptr, 1);
    }

    return NULL;
}


/*
 * Function:
 *   mdns_pkt_same_domain_name
 * Description:
 *   Check if the domain name is identical with the one in data buffer
 * Parameters:
 *   buf_offset : the offset of the domain name in data buffer
 *   name : domain name to compare with
 *   name_len : length of the domain name
 * Returns:
 *
 */
STATICFN BOOL
mdns_pkt_same_domain_name(uint16 buf_offset,
                                        uint8 *name, uint8 name_len) REENTRANT
{
    uint16  i;
    uint8  c1, c2;
    uint8   *ptr;
    uint8   len;


    if (name == NULL) {
        return FALSE;
    }

    ptr = &dns_buf_ptr[buf_offset];
    len = name_len;

    for (i = 0; i < len; i++) {
        c1 = ptr[i];
        c2 = name[i];
        if (c1 == DNS_COMPRESS_FLAG) {
            return mdns_pkt_same_domain_name((uint16)ptr[i+1], &name[i], (len - i));

        } else {
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
    }

    return TRUE;
}

/*
 * Function:
 *   mdns_pkt_record_add
 * Description:
 *   Add the domain name in the DNS data buffer
 * Parameters:
 *   name : domain name that need to be add to data buffer
 *   name_len : length of the domain name
 * Returns:
 *
 */
STATICFN void
mdns_pkt_domain_name_add(uint8 *name, uint8 name_len)
{
    uint8       name_idx, cmp_len, label_len, data_len;
    uint8       *ptr;
    uint16      i;

    name_idx = 0;
    cmp_len = name_len;
    ptr = name;
    label_len = ptr[name_idx];
    data_len = dns_buf_offset;
    while (label_len && (name_idx < name_len)) {
        for (i = 0; i < data_len; i++) {
            if (dns_buf_ptr[i] == DNS_COMPRESS_FLAG) {
                if (mdns_pkt_same_domain_name((uint16)dns_buf_ptr[i+1],
                        &ptr[name_idx], cmp_len)) {
                    /* Compression the domain name */
                    dns_buf_ptr[dns_buf_offset++] = DNS_COMPRESS_FLAG;
                    dns_buf_ptr[dns_buf_offset++] = i & 0xff;
                    return;
                }
            } else if (dns_buf_ptr[i] == ptr[name_idx]){

            /* Check if the first label length and the last zero byte */
                if (mdns_pkt_same_domain_name(i, &ptr[name_idx], cmp_len)) {
                    /* Compression the domain name */
                    dns_buf_ptr[dns_buf_offset++] = DNS_COMPRESS_FLAG;
                    dns_buf_ptr[dns_buf_offset++] = i & 0xff;
                    return;
                }
            }
        }
        /* Put the label on the packet buffer */
        label_len++;
        sal_memcpy(&dns_buf_ptr[dns_buf_offset], &ptr[name_idx], label_len);
        dns_buf_offset += label_len;
        name_idx += label_len;
        cmp_len -= label_len;
        label_len = ptr[name_idx];

    }
    /* append zero bype */
    dns_buf_ptr[dns_buf_offset++] = 0x0;

    return;

}


/*
 * Function:
 *   mdns_pkt_record_add
 * Description:
 *   Add the resource record in the DNS data buffer
 * Parameters:
 *   flags : MDNS_PKT_FLAG_XXX
 *   srv : service info
 *   rr_type : record type
 * Returns:
 *
 */
STATICFN void
mdns_pkt_record_add(uint8 flags, dns_service_info_t *srv, uint8 rr_type)
{
    dns_rr_info_t   *rr_info = 0;
    uint8       bitmap_len, rd_len;
    int         temp;

    /* 1. name */
    if (flags & MDNS_PKT_FLAG_ADDR_RECORD) {
        rr_info = &mdns.addr_rr;
        if (rr_type == DNS_TYPE_AAAA) {
            rr_info = &mdns.addr6_rr;
        }
        mdns_pkt_domain_name_add(mdns.host_dn, mdns.host_dn_len);
    } else if (flags & MDNS_PKT_FLAG_DNS_GQUERY) {
        rr_info = srv->dns_rr;
        mdns_pkt_domain_name_add((uint8 *)dns_query, MDNS_GENERAL_QUERY_LEN);
    } else {
        switch (rr_type) {
            case DNS_TYPE_SRV:
                rr_info = srv->srv_rr;
                mdns_pkt_domain_name_add(srv->name, srv->name_len);
                break;
            case DNS_TYPE_TXT:
                rr_info = srv->txt_rr;
                mdns_pkt_domain_name_add(srv->name, srv->name_len);
                break;
            case DNS_TYPE_PTR:
                rr_info = srv->ptr_rr;
                mdns_pkt_domain_name_add(&srv->name[srv->instance_len],
                    (srv->name_len - srv->instance_len));
                break;
            default:
                return;
        }
    }

    /* 2. type */
    dns_buf_ptr[dns_buf_offset++] = 0;
    if (flags & MDNS_PKT_FLAG_NONEXIST) {
        /* Use NSEC record to indicate the record doesn't exist */
        dns_buf_ptr[dns_buf_offset++] = DNS_TYPE_NSEC;
    } else {
        dns_buf_ptr[dns_buf_offset++] = (rr_info->rtype & 0xff);
    }
    /* 3. class */
    if (rr_info->unique) {
        /* Set cache flash bit*/
        dns_buf_ptr[dns_buf_offset++] = (DNS_CACHE_FLASH >> 8);
    } else {
        dns_buf_ptr[dns_buf_offset++] = 0x0;
    }
    dns_buf_ptr[dns_buf_offset++] = (rr_info->rclass & 0xff);

    /* 4. ttl */
    if (mdns.send_pkt_type == MDNS_SEND_GOODBYE_PKT) {
        dns_buf_ptr[dns_buf_offset++] = 0x0;
        dns_buf_ptr[dns_buf_offset++] = 0x0;
        dns_buf_ptr[dns_buf_offset++] = 0x0;
        dns_buf_ptr[dns_buf_offset++] = 0x0;
    } else {
        dns_buf_ptr[dns_buf_offset++] = rr_info->ttl >> 24;
        dns_buf_ptr[dns_buf_offset++] = (rr_info->ttl >> 16) & 0xff;
        dns_buf_ptr[dns_buf_offset++] = (rr_info->ttl >> 8) & 0xff;
        dns_buf_ptr[dns_buf_offset++] = (rr_info->ttl & 0xff);
    }

    if (flags & MDNS_PKT_FLAG_NONEXIST) {
        /* NSEC record */
        bitmap_len = (DNS_TYPE_AAAA / 8) + 1;
        /*
         * The next domain name is record's name, it always takes 2 bytes in the packet.
         * The rdlength = 2(next domain name) + 1(bitmap block) + 1(bitmap length)
         * + bitmap_len (bitmap data)
         */
        /* 5. rdlength */
        rd_len = 2 + 1 + 1 + bitmap_len;
        dns_buf_ptr[dns_buf_offset++] = 0;
        dns_buf_ptr[dns_buf_offset++] = rd_len;
        /* 6. rdata */
        /* Next domain name field */
        if (flags & MDNS_PKT_FLAG_ADDR_RECORD) {
            mdns_pkt_domain_name_add(mdns.host_dn, mdns.host_dn_len);
        } else {
            mdns_pkt_domain_name_add(srv->name, srv->name_len);
        }
        /* Bit Map Block */
        dns_buf_ptr[dns_buf_offset++] = 0; /* Always zero */
        /* Bit Map Length */
        dns_buf_ptr[dns_buf_offset++] = bitmap_len;
        /* Bit Map Data */
        sal_memset(&dns_buf_ptr[dns_buf_offset], 0, bitmap_len);
        if (mdns.addr_rr.enabled) {
            dns_buf_ptr[dns_buf_offset] = 1 << (7 - (DNS_TYPE_A % 8));
        }
        if (mdns.addr6_rr.enabled) {
            dns_buf_ptr[dns_buf_offset + bitmap_len - 1] =
                1 << (7 - (DNS_TYPE_AAAA % 8));
        }
        dns_buf_offset += bitmap_len;

    } else {
        if (rr_type == DNS_TYPE_PTR) {
            /* Compress the rdata of PTR record */
            temp = dns_buf_offset; /* Record the position of rdlength */
            dns_buf_offset += 2;
            mdns_pkt_domain_name_add(rr_info->rdata, rr_info->rdlength);
            dns_buf_ptr[temp] = 0;
            dns_buf_ptr[temp + 1] = (dns_buf_offset - temp - 2);
        } else if (rr_type == DNS_TYPE_SRV) {
            /* Compress the target field in Rdata */
            temp = dns_buf_offset; /* Record the position of rdlength */
            dns_buf_offset += 2;
            /* Copy the priority, weight and port */
            sal_memcpy (&dns_buf_ptr[dns_buf_offset], rr_info->rdata, 6);
            dns_buf_offset += 6;
            /* Compress the target field */
            mdns_pkt_domain_name_add(rr_info->rdata + 6, rr_info->rdlength - 6);
            dns_buf_ptr[temp] = 0;
            dns_buf_ptr[temp + 1] = (dns_buf_offset - temp - 2);
        } else {
            /* 5. rdlength */
            dns_buf_ptr[dns_buf_offset++] = (rr_info->rdlength >> 8) & 0xff;
            dns_buf_ptr[dns_buf_offset++] = rr_info->rdlength & 0xff;

            /* 6. rdata */
            sal_memcpy(&dns_buf_ptr[dns_buf_offset], rr_info->rdata,
                rr_info->rdlength);
            dns_buf_offset += rr_info->rdlength;
        }
    }
}


/*
 * Function:
 *   mdns_pkt_question_add
 * Description:
 *   Add the question in the DNS data buffer
 * Parameters:
 *   is_addr : 1->address, 0->service
 * Returns:
 *
 */
STATICFN void
mdns_pkt_question_add(BOOL is_addr, dns_service_info_t *srv)
{

    /* 1. name */
    if (is_addr) {
        mdns_pkt_domain_name_add(mdns.host_dn, mdns.host_dn_len);
    } else {
        mdns_pkt_domain_name_add(srv->name, srv->name_len);
    }

    /* 2. type */
    dns_buf_ptr[dns_buf_offset++] = 0;
    dns_buf_ptr[dns_buf_offset++] = DNS_TYPE_ANY;
    /* 3. class */
    /* request for unicast response */
    dns_buf_ptr[dns_buf_offset++] = (DNS_UNICAST_RESPONSE >> 8);
    dns_buf_ptr[dns_buf_offset++] = DNS_CLASS_IN;
}


/*
 * Function:
 *   mdns_pkt_send_dest_check
 * Description:
 *   Check the reply host address for the resource record
 * Parameters:
 *   rr : resource record
 * Returns:
 *
 */
STATICFN void
mdns_pkt_send_dest_check(dns_rr_info_t *rr)
{
    if (!mdns.multicast_pkt_send) {
        if (rr->send_flags & DNS_RR_SEND_FLAG_UNICAST) {
            if (!mdns.unicast_pkt_send) {
                /*
                 * First record in DNS packet.
                 * Update the destination info
                 */
                mdns.unicast_pkt_send = TRUE;
#if CFG_UIP_IPV6_ENABLED
                if (uip_ipv6) {
                    sal_memcpy(mdns.dst_ip6, rr->peer_ip6, 16);
                } else
#endif /* CFG_UIP_IPV6_ENABLED */
                {
                    sal_memcpy(mdns.dst_ip4, rr->peer_ip4, 4);
                }

            } else {
                /* Check the destnication is the same as last record's */
                /* Otherwise, the packet should be sent by multicast */
#if CFG_UIP_IPV6_ENABLED
                if (uip_ipv6) {
                    if (sal_memcpy(mdns.dst_ip6, rr->peer_ip6, 16)) {
                        mdns.multicast_pkt_send = TRUE;
                    }
                } else {
                    if (sal_memcpy(mdns.dst_ip4, rr->peer_ip4, 4)) {
                        mdns.multicast_pkt_send = TRUE;
                    }
                }
#else /* !CFG_UIP_IPV6_ENABLED */
                if (sal_memcpy(mdns.dst_ip4, rr->peer_ip4, 4)) {
                    mdns.multicast_pkt_send = TRUE;
                }
#endif /* CFG_UIP_IPV6_ENABLED */

            }
        } else {
            /* Send by Multicast DNS */
            mdns.multicast_pkt_send = TRUE;
        }
    }

    return;
}

/*
 * Function:
 *   mdns_packet_out
 * Description:
 *   Construct the DNS packet to send
 * Parameters:
 * Returns:
 *
 */
void
mdns_packet_out(void)
{
    dns_service_info_t  *srv;
    struct dns_hdr *dns_hdr;
    dns_rr_info_t   *adr_rr;
#if CFG_UIP_IPV6_ENABLED
    BOOL            is_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
    dns_rr_info_t   *adr6_rr;
    uint16          val16;

    dns_hdr = (struct dns_hdr *)dns_buf_ptr;
    /* ID Flags */
    if (mdns.send_pkt_type == MDNS_SEND_QUERY_PKT) {
        dns_hdr->dns_flag = 0; /* Standard query */
    } else if ((mdns.send_pkt_type == MDNS_SEND_RESP_PKT) ||
        (mdns.send_pkt_type == MDNS_SEND_GOODBYE_PKT)){
        dns_hdr->dns_flag = uip_htons(DNS_HDR_FLAG_QR_MASK | DNS_HDR_FLAG_AA_MASK);
    } else {
        return;
    }

    /* Clear send packet type */
    mdns.unicast_pkt_send = FALSE;
    mdns.multicast_pkt_send = FALSE;

    adr_rr = &mdns.addr_rr;
    adr6_rr = &mdns.addr6_rr;
#if CFG_UIP_IPV6_ENABLED
    is_ipv6 = FALSE;
    if (uip_ipv6) {
        is_ipv6 = TRUE;
    }
#endif /* CFG_UIP_IPV6_ENABLED */

    /* Question Section */
    if (adr_rr->send_flags & DNS_RR_SEND_FLAG_IN_QUEST) {
        mdns_pkt_send_dest_check(adr_rr);
        val16 = uip_htons(dns_hdr->qdcount);
        val16++;
        dns_hdr->qdcount = uip_htons(val16);
        mdns_pkt_question_add(TRUE, NULL);
    }
    if (adr6_rr->send_flags & DNS_RR_SEND_FLAG_IN_QUEST) {
        mdns_pkt_send_dest_check(adr6_rr);
        val16 = uip_htons(dns_hdr->qdcount);
        val16++;
        dns_hdr->qdcount = uip_htons(val16);
        mdns_pkt_question_add(TRUE, NULL);
    }

    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            if (srv->srv_rr->send_flags & DNS_RR_SEND_FLAG_IN_QUEST) {
                mdns_pkt_send_dest_check(srv->srv_rr);
                val16 = uip_htons(dns_hdr->qdcount);
                val16++;
                dns_hdr->qdcount = uip_htons(val16);
                mdns_pkt_question_add(FALSE, srv);
            }
        }
        srv = srv->next;
    }

    /* Answer Section */
    if (adr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
        mdns_pkt_send_dest_check(adr_rr);
        val16 = uip_htons(dns_hdr->ancount);
        val16++;
        dns_hdr->ancount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_A);
    }

    if (adr6_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
        mdns_pkt_send_dest_check(adr6_rr);
        val16 = uip_htons(dns_hdr->ancount);
        val16++;
        dns_hdr->ancount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_AAAA);
    }

    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            if (srv->srv_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                mdns_pkt_send_dest_check(srv->srv_rr);
                val16 = uip_htons(dns_hdr->ancount);
                val16++;
                dns_hdr->ancount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_SRV);
            }
        }
        if (srv->txt_rr) {
            if (srv->txt_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                mdns_pkt_send_dest_check(srv->txt_rr);
                val16 = uip_htons(dns_hdr->ancount);
                val16++;
                dns_hdr->ancount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_TXT);
            }
        }
        if (srv->ptr_rr) {
            if (srv->ptr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                mdns_pkt_send_dest_check(srv->ptr_rr);
                val16 = uip_htons(dns_hdr->ancount);
                val16++;
                dns_hdr->ancount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_PTR);
            }
        }
        if (srv->dns_rr) {
            if (srv->dns_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                mdns_pkt_send_dest_check(srv->dns_rr);
                val16 = uip_htons(dns_hdr->ancount);
                val16++;
                dns_hdr->ancount = uip_htons(val16);
                mdns_pkt_record_add(MDNS_PKT_FLAG_DNS_GQUERY, srv,
                    DNS_TYPE_PTR);
            }
        }
        srv = srv->next;
    }

    /* Auth Section */
    if (adr_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
        mdns_pkt_send_dest_check(adr_rr);
        val16 = uip_htons(dns_hdr->nscount);
        val16++;
        dns_hdr->nscount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_A);
    }

    if (adr6_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
        mdns_pkt_send_dest_check(adr6_rr);
        val16 = uip_htons(dns_hdr->nscount);
        val16++;
        dns_hdr->nscount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_AAAA);
    }

    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            if (srv->srv_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
                mdns_pkt_send_dest_check(srv->srv_rr);
                val16 = uip_htons(dns_hdr->nscount);
                val16++;
                dns_hdr->nscount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_SRV);
            }
        }
        if (srv->txt_rr) {
            if (srv->txt_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
                mdns_pkt_send_dest_check(srv->txt_rr);
                val16 = uip_htons(dns_hdr->nscount);
                val16++;
                dns_hdr->nscount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_TXT);
            }
        }
        if (srv->ptr_rr) {
            if (srv->ptr_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
                mdns_pkt_send_dest_check(srv->ptr_rr);
                val16 = uip_htons(dns_hdr->nscount);
                val16++;
                dns_hdr->nscount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_PTR);
            }
        }
        if (srv->dns_rr) {
            if (srv->dns_rr->send_flags & DNS_RR_SEND_FLAG_IN_AUTH) {
                mdns_pkt_send_dest_check(srv->dns_rr);
                val16 = uip_htons(dns_hdr->nscount);
                val16++;
                dns_hdr->nscount = uip_htons(val16);
                mdns_pkt_record_add(MDNS_PKT_FLAG_DNS_GQUERY, srv,
                    DNS_TYPE_PTR);
            }
        }
        srv = srv->next;
    }

    /* Additional Section */
    if ((adr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) &&
        (!(adr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER))) {
        /* Not present in Answer section */
        val16 = uip_htons(dns_hdr->arcount);
        val16++;
        dns_hdr->arcount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_A);
    }

    if ((adr6_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) &&
        (!(adr6_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER))) {
        /* Not present in Answer section */
        val16 = uip_htons(dns_hdr->arcount);
        val16++;
        dns_hdr->arcount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD, NULL, DNS_TYPE_AAAA);
    }

    if ((adr_rr->send_flags & DNS_RR_SEND_FLAG_NSEC) ||
        (adr6_rr->send_flags & DNS_RR_SEND_FLAG_NSEC)){
        val16 = uip_htons(dns_hdr->arcount);
        val16++;
        dns_hdr->arcount = uip_htons(val16);
        mdns_pkt_record_add(MDNS_PKT_FLAG_ADDR_RECORD | MDNS_PKT_FLAG_NONEXIST,
            NULL, DNS_TYPE_AAAA);
    }

    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            if (srv->srv_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) {
                val16 = uip_htons(dns_hdr->arcount);
                val16++;
                dns_hdr->arcount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_SRV);
            }
        }
        if (srv->txt_rr) {
            if (srv->txt_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) {
                val16 = uip_htons(dns_hdr->arcount);
                val16++;
                dns_hdr->arcount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_TXT);
            }
        }
        if (srv->ptr_rr) {
            if (srv->ptr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) {
                val16 = uip_htons(dns_hdr->arcount);
                val16++;
                dns_hdr->arcount = uip_htons(val16);
                mdns_pkt_record_add(0x0, srv, DNS_TYPE_PTR);
            }
        }
        if (srv->dns_rr) {
            if (srv->dns_rr->send_flags & DNS_RR_SEND_FLAG_IN_ADDI) {
                val16 = uip_htons(dns_hdr->arcount);
                val16++;
                dns_hdr->arcount = uip_htons(val16);
                mdns_pkt_record_add(MDNS_PKT_FLAG_DNS_GQUERY, srv,
                    DNS_TYPE_PTR);
            }
        }
        srv = srv->next;
    }

    if (mdns.unicast_pkt_send && !mdns.multicast_pkt_send) {
        /* Override the Dest IP address */
#if CFG_UIP_IPV6_ENABLED
        if (is_ipv6) {
            sal_memcpy(&mdns.conn_v6->ripaddr, mdns.dst_ip6, 16);
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            sal_memcpy(&mdns.conn->ripaddr, mdns.dst_ip4, 4);
        }
    } else {
#if CFG_UIP_IPV6_ENABLED
        if (is_ipv6) {
            sal_memcpy(&mdns.conn_v6->ripaddr, mdns_ip6, 16);
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            sal_memcpy(&mdns.conn->ripaddr, mdns_ip4, 4);
        }
    }


    /* Clean send_flags */
    mdns.addr_rr.send_flags = 0;
    mdns.addr6_rr.send_flags = 0;
    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            srv->srv_rr->send_flags = 0;
        }
        if (srv->txt_rr) {
            srv->txt_rr->send_flags = 0;
        }
        if (srv->ptr_rr) {
            srv->ptr_rr->send_flags = 0;
        }
        if (srv->dns_rr) {
            srv->dns_rr->send_flags = 0;
        }
        srv = srv->next;
    }
    return;
}


#ifndef __BOOTLOADER__

/*
 * Function:
 *   mdns_reply_list_add
 * Description:
 *   Add the answers to the reply list
 * Parameters:
 * Returns:
 *
 */
STATICFN void
mdns_reply_list_add(void)
{
    dns_service_info_t  *srv;
#if CFG_UIP_IPV6_ENABLED
    BOOL            is_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
    mdns_reply_answer_t     *reply_list, *reply, *new_reply;


#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        is_ipv6 = TRUE;
        reply_list = mdns.reply_list_ipv6;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    {
#if CFG_UIP_IPV6_ENABLED
        is_ipv6 = FALSE;
#endif /* CFG_UIP_IPV6_ENABLED */
        reply_list = mdns.reply_list_ipv4;
    }

    /* Create a reply node */
    new_reply = NULL;

    new_reply = sal_malloc(sizeof(mdns_reply_answer_t));
    if (new_reply == NULL) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_reply_list_add failed to allocate memory!\n");
#endif /* CFG_CONSOLE_ENABLED */
        return;
    }
    sal_memset(new_reply, 0, sizeof(mdns_reply_answer_t));

    /* Answer Section */
    /* Only PTR and DNS-SD records were sent with delays */
    srv = mdns.srv_list;
    while (srv) {
        if (srv->ptr_rr) {
            if (srv->ptr_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                new_reply->asnwer_rr = srv->ptr_rr;
                break;
            }
        }
        if (srv->dns_rr) {
            if (srv->dns_rr->send_flags & DNS_RR_SEND_FLAG_IN_ANSWER) {
                new_reply->asnwer_rr = srv->dns_rr;
                break;
            }
        }
        srv = srv->next;
    }

    /* No answer is found due to known answer list */
    if (new_reply->asnwer_rr == NULL) {
        sal_free(new_reply);
        return;
    }

    if (reply_list == NULL) {
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            mdns.reply_list_ipv6 = new_reply;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            mdns.reply_list_ipv4 = new_reply;
        }
    } else {
        reply = reply_list;
        while (reply) {
            reply = reply->next;
        }
        reply = new_reply;
    }

    new_reply->send_flags = new_reply->asnwer_rr->send_flags;
#if CFG_UIP_IPV6_ENABLED
    if (is_ipv6) {
        sal_memcpy(new_reply->peer_ip, new_reply->asnwer_rr->peer_ip6, 16);

    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    {
        sal_memcpy(new_reply->peer_ip, new_reply->asnwer_rr->peer_ip4, 4);
    }
    new_reply->start = sal_get_ticks();
    new_reply->delay = mdns.send_delay;
    new_reply->next = NULL;

    /* Clean send_flags */
    srv = mdns.srv_list;
    while (srv) {
        if (srv->srv_rr) {
            srv->srv_rr->send_flags = 0;
        }
        if (srv->txt_rr) {
            srv->txt_rr->send_flags = 0;
        }
        if (srv->ptr_rr) {
            srv->ptr_rr->send_flags = 0;
        }
        if (srv->dns_rr) {
            srv->dns_rr->send_flags = 0;
        }
        srv = srv->next;
    }
    /* Speed up the answer task */
    mdns_timer_trigger();
    return;
}
#endif /* __BOOTLOADER__ */


/*
 * Function:
 *   mdns_response_address_conflict_check
 * Description:
 *   Check if this record is conflict with our address records.
 * Parameters:
 *    name : record name
 *    rtype : record type
 *    rclass : record class
 *    rr_ptr : the data pointer of the record
 * Returns:
 */
STATICFN void
mdns_response_address_conflict_check(uint8 *name,
        uint16 rtype, uint16 rclass, uint8 *rr_ptr)
{
    uint16  class_value;
    mdns_naming_state_t *name_state;

    /* Check type */
    if ((rtype != DNS_TYPE_A) && (rtype != DNS_TYPE_ANY) &&
        (rtype != DNS_TYPE_AAAA)) {
        return;
    }

    /* Check class */
    class_value = rclass & 0xff;
    if ((class_value != mdns.addr_rr.rclass) &&
            (class_value != DNS_CLASS_ANY)) {
        return;
    }

    if (!rr_ptr) {
        return;
    }


#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        name_state = &mdns.name_ipv6;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    {
        name_state = &mdns.name_ipv4;
    }

    /* Check name */
    if (mdns_same_domain(mdns.host_dn, name, mdns.host_dn_len)) {

        /* Conflict event */
        switch (name_state->state) {
            case MDNS_SRV_STATE_INIT:
            case MDNS_SRV_STATE_PROBE:
            case MDNS_SRV_STATE_ANNOUNCE:
                name_state->state = MDNS_SRV_STATE_CONFLICT;
                break;
            default:
                break;
        }
    }

    return;
}

/*
 * Function:
 *   mdns_response_service_conflict_check
 * Description:
 *   Check if this record is conflict with our service records.
 * Parameters:
 *    name : record name
 *    rtype : record type
 *    rclass : record class
 *    from_response : if this record is come from response packets or not
 *    rr_ptr : the pointer of the checking record
 * Returns:
 */
STATICFN void
mdns_response_service_conflict_check(uint8 *name,
    uint16 rtype, uint16 rclass, BOOL from_response, uint8 *rdata)
{
    dns_service_info_t  *service;
    uint16          class_value;
    dns_rr_info_t       *rr;
    uint8           cname[DNS_RR_NAME_MAX_LEN], cname_len;
    mdns_naming_state_t     *name_state;

    /* Check the SRV and TXT */
    if ((rtype != DNS_TYPE_SRV) && (rtype != DNS_TYPE_ANY) &&
        (rtype != DNS_TYPE_TXT) && (rtype != DNS_TYPE_PTR)) {
        return;
    }

    /* Check class */
    class_value = rclass & 0xff;
    if ((class_value != DNS_CLASS_IN) &&
            (class_value != DNS_CLASS_ANY)) {
        return;
    }

    if (!rdata) {
        return;
    }

    service = mdns.srv_list;
    while (service) {
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            name_state = &service->name_ipv6;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            name_state = &service->name_ipv4;
        }
        /* Check name */
        if (from_response) {
            if (mdns_same_domain(service->name, name, service->name_len)) {
                switch (name_state->state) {
                    case MDNS_SRV_STATE_INIT:
                    case MDNS_SRV_STATE_PROBE:
                    case MDNS_SRV_STATE_ANNOUNCE:
                        name_state->state = MDNS_SRV_STATE_CONFLICT;
                        break;
                    default:
                        break;
                }
            }
        } else {
            /* Handle the Known Answer List of the query packets */
            if (name_state->state == MDNS_SRV_STATE_ANNOUNCE) {
                /* Check the service instance and service type enumeration */
                /* Current only these two queries are for the shared records */
                if (!sal_memcmp(&service->name[service->instance_len],
                        name, (service->name_len - service->instance_len))) {
                    switch (rtype) {
                        case DNS_TYPE_PTR:
                            rr = service->ptr_rr;
                            break;
                        case DNS_TYPE_SRV:
                            rr = service->srv_rr;
                            break;
                        case DNS_TYPE_TXT:
                            rr = service->txt_rr;
                            break;
                        default:
                            rr = NULL;
                            break;
                    }
                } else if (!sal_memcmp(dns_query, name, MDNS_GENERAL_QUERY_LEN)) {
                    if (rtype == DNS_TYPE_PTR) {
                        rr = service->dns_rr;
                    } else {
                        rr = NULL;
                    }
                } else {
                    rr = NULL;
                }

                if (rr != NULL) {
                    /* Check if the rdata is the same as the record's */
                    if (!mdns_packet_domain_name_parse(
                                    rdata, cname, &cname_len)) {
                        if (!mdns_rdata_cmp(rr->rdlength, rr->rdata,
                            rr->rdlength, cname)) {
                            /* Known answer suppression */
                            rr->send_flags &= ~DNS_RR_SEND_FLAG_IN_ANSWER;
                        }
                    }
                }
            }
        }

        service = service->next;
    }

    return;
}


#ifndef __BOOTLOADER__

STATICFN BOOL
mdns_answers_reply_list_check(uint8 *name,
    uint16 rtype, uint16 rclass, BOOL from_response, uint8 *rdata)
{
    dns_rr_info_t       *rr;
    uint8           cname[DNS_RR_NAME_MAX_LEN], cname_len;
    mdns_reply_answer_t     *reply_list, *reply, *pre;

    if (!rdata) {
        return FALSE;
    }

#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        reply_list = mdns.reply_list_ipv6;
    } else
#endif /* CFG_UIP_IPV6_ENABLED*/
    {
        reply_list = mdns.reply_list_ipv4;
    }

    reply = reply_list;
    pre = reply_list;
    while (reply) {
        /* Check the answers that matched the queried name */
        if ((reply->queried) || (from_response)) {
            rr = reply->asnwer_rr;
            /* Check if the query name is identical with answer's name */
            if((mdns_same_domain((const uint8 *)name,
                (const uint8 *)rr->rname, (uint16)rr->rname_len)) &&
                (rr->rtype == rtype) &&
                (rr->rclass == (rclass & 0xFF))) {

                /* Check the rdata is the same as PTR or DNS-SD record's */
                if (!mdns_packet_domain_name_parse(
                                    rdata, cname, &cname_len)) {
                    if (!mdns_rdata_cmp(rr->rdlength, rr->rdata,
                            rr->rdlength, cname)) {
                        /* Known answer suppression */
                        /* Remove this reply from the list */
                        if (reply == reply_list) {
#if CFG_UIP_IPV6_ENABLED
                            if (uip_ipv6) {
                                mdns.reply_list_ipv6 = reply->next;
                            } else
#endif /* CFG_UIP_IPV6_ENABLED*/
                            {
                                mdns.reply_list_ipv4 = reply->next;
                            }

                        } else {
                            pre->next = reply->next;
                            sal_free(reply);
                        }
                        return TRUE;
                    }
                }
            }

        }
        pre = reply;
        reply = reply->next;
    }

    return FALSE;
}

#endif /* __BOOTLOADER__ */

/*
 * Function:
 *   mdns_process_response
 * Description:
 *   Handle the MDNS response packets
 * Parameters:
 *
 * Returns:
 */
STATICFN void
mdns_process_response(void)
{
    uint8           *ans_ptr, name_len, *rdata_ptr;
    int             num_ans, i;
    uint8           rname[DNS_RR_NAME_MAX_LEN];
    sys_error_t     rv;
    uint16          rtype, rclass;
    struct dns_hdr  *dns_header;

    dns_header = (struct dns_hdr *)dns_buf_ptr;
    num_ans = uip_htons(dns_header->ancount);

    /* Check if the query name match our device address or service records */
    ans_ptr = mdns_packet_section_pointer(dns_header, MDNS_PKT_SEC_TYPE_ANSWER);
    if (ans_ptr == NULL) {
        return;
    }

    for (i = 0; i < num_ans; i++) {

        /* Parse name, type and class */
        rv = mdns_packet_domain_name_parse(ans_ptr, rname, &name_len);
        if (rv) {
            continue;
        }


#if MDNS_DEBUG
        MDNS_DBG(("Answer %d : ", i));
        mdns_domain_name_print(rname);
#endif /* MDNS_DEBUG */


        rtype = (ans_ptr[name_len] << 8) | ans_ptr[name_len + 1];
        rclass = (ans_ptr[name_len + 2] << 8) | ans_ptr[name_len + 3];
        rdata_ptr = &ans_ptr[name_len + 10];
        /* Check address record */
        mdns_response_address_conflict_check(rname, rtype, rclass, rdata_ptr);

#ifdef __BOOTLOADER__
        /* Check service records */
        mdns_response_service_conflict_check(rname, rtype,
                                                rclass, TRUE, rdata_ptr);
#else /* !__BOOTLOADER__ */
        if (!mdns_answers_reply_list_check(rname, rtype, rclass,
                                                    TRUE, rdata_ptr)) {
            /* Check service records */
            mdns_response_service_conflict_check(rname, rtype,
                                                rclass, TRUE, rdata_ptr);
        }
#endif /* __BOOTLOADER__ */

        /* Move to the next*/
        ans_ptr = mdns_packet_record_ignore(ans_ptr, 1);
    }

    return;
}

/*
 * Function:
 *   mdns_query_address_check
 * Description:
 *   Check if any address records need to reply for this question
 * Parameters:
 *    qname : qestion name
 *    qtype : question type
 *    qclass : question class
 * Returns:
 */
STATICFN void
mdns_query_address_check(uint8 *qname, uint16 qtype, uint16 qclass)
{
    uint16  rd_len;
    uint8   *rdata, *ptr;
    int8    rv;
    mdns_naming_state_t *name_state;
    dns_rr_info_t       *adr_rr, *adr_rr2;
    BOOL                is_ipv6;

    /* Check type */
    if ((qtype != DNS_TYPE_A) && (qtype != DNS_TYPE_ANY) &&
        (qtype != DNS_TYPE_AAAA)) {
        return;
    }

    /* Check class */
    if (((qclass & 0xFF)  != mdns.addr_rr.rclass) &&
        ((qclass & 0xFF) != DNS_CLASS_ANY)) {
        return;
    }

    if (qtype == DNS_TYPE_AAAA) {
        adr_rr = &mdns.addr6_rr; /* the record query for */
        adr_rr2 = &mdns.addr_rr;
    } else {
        adr_rr = &mdns.addr_rr; /* the record query for */
        adr_rr2 = &mdns.addr6_rr;
    }

#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        name_state = &mdns.name_ipv6;
        is_ipv6 = TRUE;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    {
        name_state = &mdns.name_ipv4;
        is_ipv6 = FALSE;
    }

    /* Check name */
    if (mdns_same_domain(mdns.host_dn, qname, mdns.host_dn_len)) {
        switch (name_state->state) {
            case MDNS_SRV_STATE_INIT:
                mdns.name_ipv4.state = MDNS_SRV_STATE_CONFLICT;
#if CFG_UIP_IPV6_ENABLED
                mdns.name_ipv6.state = MDNS_SRV_STATE_CONFLICT;
#endif /* CFG_UIP_IPV6_ENABLED */
                break;
            case MDNS_SRV_STATE_PROBE:
                /* race condition */
                ptr = mdns_packet_record_data_get(MDNS_PKT_SEC_TYPE_AUTH,
                    qname, qtype, DNS_CLASS_IN);
                if (ptr == NULL) {
                    return;
                }
                rd_len = (*ptr << 8) | *(ptr + 1);
                rdata = ptr + 2;
                rv = mdns_rdata_cmp(mdns.host_dn_len, mdns.host_dn,
                    rd_len, rdata);
                if (rv > 0) {
                    /* The querier has high priority for the name */
                    mdns.name_ipv4.state = MDNS_SRV_STATE_CONFLICT;
#if CFG_UIP_IPV6_ENABLED
                    mdns.name_ipv6.state = MDNS_SRV_STATE_CONFLICT;
#endif /* CFG_UIP_IPV6_ENABLED */
                }

                break;
            case MDNS_SRV_STATE_ANNOUNCE:
                /* Need to reply */
                if (qclass & DNS_UNICAST_RESPONSE) {
                    /* unicast response request */
                        /* First unicast reply request */
                        /* And save the peer infomation */
                    adr_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                    adr_rr2->send_flags |= DNS_RR_SEND_FLAG_UNICAST;

                    if (!is_ipv6) {
                        sal_memcpy(adr_rr->peer_ip4,
                            &UDPBUF->srcipaddr, 4);
                    }
#if CFG_UIP_IPV6_ENABLED
                    else {
                        sal_memcpy(adr_rr->peer_ip6,
                            &UDP6BUF->srcipaddr, 16);
                    }
#endif /* CFG_UIP_IPV6_ENABLED */

                } else {
                    /* Multicast reply */
                    adr_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                    adr_rr2->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                }

                if ((qtype == DNS_TYPE_AAAA) || (qtype == DNS_TYPE_A)) {
                    if (adr_rr->enabled) {
                        adr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    }
                    if (adr_rr2->enabled) {
                        adr_rr2->send_flags |= DNS_RR_SEND_FLAG_IN_ADDI;
                    }
                } else if (qtype == DNS_TYPE_ANY) {
                    if (adr_rr->enabled) {
                        adr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    }
                    if (adr_rr2->enabled) {
                        adr_rr2->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    }
                }

                /* Use NSEC record to indicate the nonexistence */
                if ((!adr_rr->enabled) || (!adr_rr2->enabled)) {
                    adr_rr->send_flags |= DNS_RR_SEND_FLAG_NSEC;
                }
                mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
                break;
            default:
                break;
        }
    }

    return;
}

/*
 * Function:
 *   mdns_query_service_check
 * Description:
 *   Check if any service records need to reply for this question
 * Parameters:
 *    qname : qestion name
 *    qtype : question type
 *    qclass : question class
 * Returns:
 */
STATICFN void
mdns_query_service_check(uint8 *qname, uint16 qtype, uint16 qclass)
{
#if 0
    uint16  rd_len;
#endif
    uint8   *rdata, *ptr, target_len;
    int8    rv;
    dns_service_info_t  *service;
    BOOL    uni_resp;
    uint8   target_name[DNS_RR_RDATA_MAX_LEN];
    mdns_naming_state_t     *name_state;
    BOOL    is_ipv6;


    /* Check type */
    if ((qtype != DNS_TYPE_SRV) && (qtype != DNS_TYPE_ANY) &&
        (qtype != DNS_TYPE_TXT) && (qtype != DNS_TYPE_PTR)) {
        return;
    }

    /* Check class */
    if (((qclass & 0xFF) != DNS_CLASS_IN) &&
        ((qclass & 0xFF) != DNS_CLASS_ANY)) {
        return;
    }
    if (qclass & DNS_UNICAST_RESPONSE) {
        uni_resp = TRUE;
    } else {
        uni_resp = FALSE;
    }

    /* Check name */
    service = mdns.srv_list;
    while (service) {

#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            name_state = &service->name_ipv6;
            is_ipv6 = TRUE;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            name_state = &service->name_ipv4;
            is_ipv6 = FALSE;
        }

        if (mdns_same_domain(service->name, qname, service->name_len)) {
            switch (name_state->state) {
                case MDNS_SRV_STATE_INIT:
                    name_state->state = MDNS_SRV_STATE_CONFLICT;
                    break;
                case MDNS_SRV_STATE_PROBE:
                    /* race condition */
                    ptr = mdns_packet_record_data_get(MDNS_PKT_SEC_TYPE_AUTH,
                        qname, DNS_TYPE_SRV, DNS_CLASS_IN);
                    if (ptr == NULL) {
                        return;
                    }
#if 0
                    rd_len = (*ptr << 8) | *(ptr + 1);
#endif
                    rdata = ptr + 2;
                    /* Parse the compressed target field */
                    if (!mdns_packet_domain_name_parse(
                            rdata, target_name, &target_len)) {
                        rv = mdns_rdata_cmp(service->name_len, service->name,
                            service->name_len, target_name);
                        if (rv > 0) {
                            /* The querier has high priority for the name */
                            name_state->state = MDNS_SRV_STATE_CONFLICT;
                        }
                    }
                    break;
                case MDNS_SRV_STATE_ANNOUNCE:
                    if (qtype == DNS_TYPE_ANY) {
                        service->srv_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                        service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                        service->txt_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    } else if (qtype == DNS_TYPE_SRV) {
                        service->srv_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    } else if (qtype == DNS_TYPE_PTR) {
                        service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    } else if (qtype == DNS_TYPE_TXT) {
                        service->txt_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                    }
                    if (uni_resp) {
                        /* unicast response request */
                        service->srv_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                        service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                        service->txt_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                        switch (qtype) {
                            case DNS_TYPE_SRV:
                                if (!is_ipv6) {
                                    sal_memcpy(service->srv_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                }
#if CFG_UIP_IPV6_ENABLED
                                else {
                                    sal_memcpy(service->srv_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                }
#endif /* CFG_UIP_IPV6_ENABLED */
                                break;
                            case DNS_TYPE_PTR:
                                if (!is_ipv6) {
                                    sal_memcpy(service->ptr_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                }
#if CFG_UIP_IPV6_ENABLED
                                else {
                                    sal_memcpy(service->ptr_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                }
#endif /* CFG_UIP_IPV6_ENABLED */
                                break;
                            case DNS_TYPE_TXT:
                                if (!is_ipv6) {
                                    sal_memcpy(service->txt_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                }
#if CFG_UIP_IPV6_ENABLED
                                else {
                                    sal_memcpy(service->txt_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                }
#endif /* CFG_UIP_IPV6_ENABLED */
                                break;
                            default:
                                /* ANY */
                                if (!is_ipv6) {
                                    sal_memcpy(service->srv_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                    sal_memcpy(service->ptr_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                    sal_memcpy(service->txt_rr->peer_ip4,
                                        &UDPBUF->srcipaddr, 4);
                                }
#if CFG_UIP_IPV6_ENABLED
                                else {
                                    sal_memcpy(service->srv_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                    sal_memcpy(service->ptr_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                    sal_memcpy(service->txt_rr->peer_ip6,
                                        &UDP6BUF->srcipaddr, 16);
                                }
#endif /* CFG_UIP_IPV6_ENABLED */
                                break;
                        }
                    } else {
                        service->srv_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                        service->ptr_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                        service->txt_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                    }
                    mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
                    break;
                default:
                    break;
            }
        } else if (!sal_memcmp(&service->name[service->instance_len],
            qname, (service->name_len - service->instance_len))) {
            /* Protocol checking */
            if (name_state->state == MDNS_SRV_STATE_ANNOUNCE) {
                service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                if (uni_resp) {
                    service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                    if (!is_ipv6) {
                        sal_memcpy(service->ptr_rr->peer_ip4,
                            &UDPBUF->srcipaddr, 4);
                    }
#if CFG_UIP_IPV6_ENABLED
                    else {
                        sal_memcpy(service->ptr_rr->peer_ip6,
                            &UDP6BUF->srcipaddr, 16);
                    }
#endif /* CFG_UIP_IPV6_ENABLED */
                } else {
                    service->ptr_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                }
                mdns.send_delay += SAL_USEC_TO_TICKS(
                    sal_get_ticks() % MDNS_RESPONSE_DELAY);
                mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
            }
        } else if (!sal_memcmp(dns_query, qname, MDNS_GENERAL_QUERY_LEN)) {
            /* General DNS query */
            if (name_state->state == MDNS_SRV_STATE_ANNOUNCE) {
                service->dns_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                if (uni_resp) {
                    service->dns_rr->send_flags |= DNS_RR_SEND_FLAG_UNICAST;
                    if (!is_ipv6) {
                        sal_memcpy(service->dns_rr->peer_ip4,
                            &UDPBUF->srcipaddr, 4);
                    }
#if CFG_UIP_IPV6_ENABLED
                    else {
                        sal_memcpy(service->dns_rr->peer_ip6,
                            &UDP6BUF->srcipaddr, 16);
                    }
#endif /* CFG_UIP_IPV6_ENABLED */
                } else {
                    service->dns_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                }
                mdns.send_delay += SAL_USEC_TO_TICKS(
                    sal_get_ticks() % MDNS_RESPONSE_DELAY);
                mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
            }
        }

        service = service->next;
    }


    return;
}


#ifndef __BOOTLOADER__
STATICFN BOOL
mdns_query_reply_list_check(uint8 *qname, uint16 qtype, uint16 qclass)
{
    mdns_reply_answer_t     *reply_list;
    uint8                   addr_size, *ip_ptr;
    dns_rr_info_t           *ans_rr;


#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        reply_list = mdns.reply_list_ipv6;
        addr_size = 16;
        ip_ptr = (uint8 *)&UDP6BUF->srcipaddr;
    } else
#endif /* CFG_UIP_IPV6_ENABLED*/
    {
        reply_list = mdns.reply_list_ipv4;
        addr_size = 4;
        ip_ptr = (uint8 *)&UDPBUF->srcipaddr;
    }

    /* Check if this query was scheduled into the reply list */
    while (reply_list) {
        ans_rr = reply_list->asnwer_rr;
        /* Check if the asnwer record match the question or not*/
        if (ans_rr) {
            if ((mdns_same_domain(qname, ans_rr->rname, ans_rr->rname_len)) &&
                (qtype == ans_rr->rtype) &&
                ((qclass & 0xFF) == ans_rr->rclass)) {

                /* Check IP or multicast response */
                if (((qclass & DNS_UNICAST_RESPONSE) == 0x0) ||
                    (!sal_memcmp(ip_ptr, reply_list->peer_ip, addr_size))) {
                    reply_list->queried = TRUE; /* Queried by current packet */
                    reply_list->delay += mdns.send_delay;
                    return TRUE;
                }

            }
        }
        reply_list = reply_list->next;
    }

    return FALSE;
}
#endif /* __BOOTLOADER__ */



/*
 * Function:
 *   mdns_process_query
 * Description:
 *   Handle the MDNS query packets
 * Parameters:
 *
 * Returns:
 */
STATICFN void
mdns_process_query(void)
{
#if 0
    BOOL        legacy_query;
#endif
    uint8       *rr_ptr, que_num, ans_num, name_len, i, *rdata_ptr;
    uint8       rname[DNS_RR_NAME_MAX_LEN];
    uint16      rtype, rclass;
    sys_error_t rv;
    struct dns_hdr *dns_header;
    mdns_reply_answer_t     *reply;

    dns_header = (struct dns_hdr *)dns_buf_ptr;

#if 0
    legacy_query = FALSE;
    /* Check if it is legacy unicast query */
#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        if (UDP6BUF->srcport != uip_htons(DNS_MULTICAST_PORT)) {
            legacy_query = TRUE;
        }
    } else
#endif
    if (UDPBUF->srcport != uip_htons(DNS_MULTICAST_PORT)) {
        legacy_query = TRUE;
    }
#endif

    /* Check TC */
    if (dns_header->dns_flag & uip_htons(DNS_HDR_FLAG_TC_MASK)) {
        /* add a random delay 400~500ms */
        mdns.send_delay = SAL_USEC_TO_TICKS(MDNS_TC_DELAY +
            (sal_get_ticks() % 100000));
    } else {
        mdns.send_delay = 0;
    }

    que_num = uip_htons(dns_header->qdcount);
    ans_num = uip_htons(dns_header->ancount);

    rr_ptr = mdns_packet_section_pointer(dns_header, MDNS_PKT_SEC_TYPE_QUEST);
    if (rr_ptr == NULL) {
        return;
    }

    for (i = 0; i < que_num; i++) {

        rv = mdns_packet_domain_name_parse(rr_ptr, rname, &name_len);
        if (rv) {
            /* packet malformed */
            return;
        }
#if MDNS_DEBUG
        MDNS_DBG(("Question %bd : ", i));
        mdns_domain_name_print(rname);
#endif /* MDNS_DEBUG */
        rtype = rr_ptr[name_len] << 8 | rr_ptr[name_len + 1];
        rclass = rr_ptr[name_len + 2] << 8 | rr_ptr[name_len + 3];
#ifdef __BOOTLOADER__
        mdns_query_address_check(rname, rtype, rclass);
        mdns_query_service_check(rname, rtype, rclass);
#else /* !__BOOTLOADER__ */
        if (!mdns_query_reply_list_check(rname, rtype, rclass)) {
            mdns_query_address_check(rname, rtype, rclass);
            mdns_query_service_check(rname, rtype, rclass);
        }
#endif /* __BOOTLOADER__ */
        /* Move to the next*/
        rr_ptr = mdns_packet_question_ignore(rr_ptr, 1);
    }

    rr_ptr = mdns_packet_section_pointer(dns_header, MDNS_PKT_SEC_TYPE_ANSWER);
    if (rr_ptr == NULL) {
        return;
    }

    /* Known Answer List checking */
    for (i = 0; i < ans_num; i++) {

        rv = mdns_packet_domain_name_parse(rr_ptr, rname, &name_len);
        if (rv) {
            /* packet malformed */
            return;
        }
        /* Shared records only */
        if (rr_ptr[name_len + 2] & (DNS_CACHE_FLASH >> 8)) {
            rr_ptr = mdns_packet_record_ignore(rr_ptr, 1);
            continue;
        }

#if MDNS_DEBUG
        MDNS_DBG(("Known Answer %bd : ", i));
        mdns_domain_name_print(rname);
#endif /* MDNS_DEBUG */

        rtype = (rr_ptr[name_len] << 8) | rr_ptr[name_len + 1];
        rclass = (rr_ptr[name_len + 2] << 8) | rr_ptr[name_len + 3];
        rdata_ptr = &rr_ptr[name_len + 10];
        /* Check address record */
        mdns_response_address_conflict_check(rname, rtype, rclass, rdata_ptr);

#ifdef __BOOTLOADER__
        /* Check service records */
        mdns_response_service_conflict_check(rname, rtype,
                                            rclass, FALSE, rdata_ptr);
#else /* !_BOOTLOADER__ */
        if (!mdns_answers_reply_list_check(rname, rtype, rclass,
                                                    FALSE, rdata_ptr)) {
        /* Check service records */
            mdns_response_service_conflict_check(rname, rtype,
                                            rclass, FALSE, rdata_ptr);
        }
#endif /* __BOOTLOADER__ */

        /* Move to the next*/
        rr_ptr = mdns_packet_record_ignore(rr_ptr, 1);
    }

    /* Clear "queried" flag of reply list */
#if CFG_UIP_IPV6_ENABLED
    if (uip_ipv6) {
        reply = mdns.reply_list_ipv6;
    } else
#endif
    {
        reply = mdns.reply_list_ipv4;
    }

    while (reply) {
        reply->queried = FALSE;
        reply = reply->next;
    }

    return;
}

/*
 * Function:
 *   mdns_packet_in
 * Description:
 *   Handler of the MDNS packets
 * Parameters:
 *
 * Returns:
 *
 */
void
mdns_packet_in(void)
{
    struct dns_hdr *dns_hdr;

    dns_hdr = (struct dns_hdr *)dns_buf_ptr;
    /* Check opcode */
    if (dns_hdr->dns_flag & uip_htons(DNS_HDR_FLAG_OPCODE_MASK)) {
        return;
    }
    /* Response code */
    if (dns_hdr->dns_flag & uip_htons(DNS_HDR_FLAG_RC_MASK)) {
        return;
    }

    /* query/response */
    if (dns_hdr->dns_flag & uip_htons(DNS_HDR_FLAG_QR_MASK)) {
        MDNS_DBG(("Received MDNS response packet.\n"));
        mdns_process_response();
    } else {
        MDNS_DBG(("Received MDNS query packet.\n"));
        mdns_process_query();
    }

    return;

}


/*
 * Function:
 *   mdns_service_add
 * Description:
 *   Add new service to advertise
 * Parameters:
 *    new_srv : new service info
 * Returns:
 *   SYS_OK
 *   SYS_ERR_XXX
 */
STATICFN sys_error_t
mdns_service_add(dns_service_info_t *new_srv)
{
    dns_service_info_t *cur_srv, *pre;
    if (!new_srv) {
        return SYS_ERR_PARAMETER;
    }

    if (mdns.srv_list == NULL) {
        mdns.srv_list = new_srv;
        return SYS_OK;
    }
    cur_srv = mdns.srv_list;

    /* check if this service is registered */
    while (cur_srv) {
        if (mdns_same_domain(cur_srv->name,
            new_srv->name, cur_srv->name_len) &&
            (cur_srv->name_len == new_srv->name_len)) {
            /* The service is already existed */
            return SYS_ERR_EXISTS;
        }
        pre = cur_srv;
        cur_srv = cur_srv->next;
    }

    /* Add this new service to the list */
    pre->next = new_srv;
    return SYS_OK;

}

/*
 * Function:
 *   mdns_service_host_name_update
 * Description:
 *   Schedule to advertise the service records when the host name is changed
 * Parameters:
 *
 * Returns:
 *
 */
STATICFN void
mdns_service_host_name_update(void)
{
    dns_service_info_t *srv;
    uint8   *new_ptr, size;

    srv = mdns.srv_list;

    while ((srv) && (srv->srv_rr)) {

        /* rdata */
        size = mdns.host_dn_len + 6;
        /* allocate new buffer */
        new_ptr = sal_malloc(size);
        if (new_ptr == NULL) {
#if CFG_CONSOLE_ENABLED
            sal_printf("mdns_host_name_change: Failed!\n");
#endif /* CFG_CONSOLE_ENABLED */
            return;
        }
        srv->srv_rr->rdlength = size;
        /* Copy priority, weight and port value from original one */
        sal_memcpy(new_ptr, srv->srv_rr->rdata, 6);
        /* host domain name */
        sal_memcpy((new_ptr + 6), mdns.host_dn, mdns.host_dn_len);
        /* Free old one */
        sal_free(srv->srv_rr->rdata);
        srv->srv_rr->rdata = new_ptr;

        if (srv->name_ipv4.state == MDNS_SRV_STATE_ANNOUNCE) {
            srv->name_ipv4.ad_count = 0;
            srv->name_ipv4.last_ad = sal_get_ticks();
            srv->name_ipv4.ad_interval =
                SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);
        }
#if CFG_UIP_IPV6_ENABLED
        if (srv->name_ipv6.state == MDNS_SRV_STATE_ANNOUNCE) {
            srv->name_ipv6.ad_count = 0;
            srv->name_ipv6.last_ad = sal_get_ticks();
            srv->name_ipv6.ad_interval =
                SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);
        }
#endif /* CFG_UIP_IPV6_ENABLED */

        srv= srv->next;
    }

    if (http_srv) {
        if (http_srv->txt_rr) {
            mdns_http_service_txt_rdata_build(http_srv->txt_rr);
        }
    }

    return;
}

/*
 * Function:
 *   mdns_service_instance_name_change
 * Description:
 *
 * Parameters:
 *
 * Returns:
 *
 */
STATICFN void
mdns_service_instance_name_update(dns_service_info_t *srv)
{
    uint8   *newbuf, size;
    dns_rr_info_t   *ptr;

    ptr = srv->ptr_rr;
    if (ptr) {

        /* rdata */
        size = srv->name_len;
        /* allocate new buffer */
        newbuf = sal_malloc(size);
        if (newbuf == NULL) {
#if CFG_CONSOLE_ENABLED
            sal_printf("mdns_service_instance_name_change: Failed!\n");
#endif /* CFG_CONSOLE_ENABLED */
            return;
        }
        ptr->rdlength = size;
        ptr->rname = &srv->name[srv->instance_len];
        ptr->rname_len = srv->name_len - srv->instance_len;
        /* Copy service name */
        sal_memcpy(newbuf, srv->name, size);
        /* Free old one */
        sal_free(ptr->rdata);
        ptr->rdata = newbuf;
    }


    return;
}


/*
 * Function:
 *   mdns_addr_rr_initialized
 * Description:
 *   Initialize the system address records
 * Parameters:
 *
 * Returns:
 *
 */
STATICFN void
mdns_addr_rr_initialized(uint8 type)
{
    if (type == MDNS_ADVERTISE_IPV4) {
        /* Add the address records */
        mdns.addr_rr.rtype = DNS_TYPE_A;
        mdns.addr_rr.rclass = DNS_CLASS_IN;
        mdns.addr_rr.ttl = DNS_RR_TTL_ADDR;
        mdns.addr_rr.rdlength = 4; /* IPv4 */
        mdns.addr_rr.unique = TRUE;
        mdns.addr_rr.rdata = sal_malloc(mdns.addr_rr.rdlength);
        sal_memcpy(mdns.addr_rr.rdata, mdns.sys_ip, mdns.addr_rr.rdlength);
        mdns.addr_rr.send_flags = 0;
        mdns.addr_rr.enabled = TRUE;

    } else if (type == MDNS_ADVERTISE_IPV6) {
        /* TYPE AAAA */
        mdns.addr6_rr.rtype = DNS_TYPE_AAAA;
        mdns.addr6_rr.rclass = DNS_CLASS_IN;
        mdns.addr6_rr.ttl = DNS_RR_TTL_ADDR;
        mdns.addr6_rr.rdlength = 16; /* IPv6 */
        mdns.addr6_rr.rdata = sal_malloc(mdns.addr6_rr.rdlength);
#if CFG_UIP_IPV6_ENABLED
        sal_memcpy(mdns.addr6_rr.rdata, mdns.sys_ip6, mdns.addr6_rr.rdlength);
#endif /* CFG_UIP_IPV6_ENABLED */
        mdns.addr6_rr.unique = TRUE;
        mdns.addr6_rr.enabled = TRUE;
        mdns.addr6_rr.send_flags = 0;
    }

}

/*
 * Function:
 *   mdns_name_reassign
 * Description:
 *   Assign the new host/instance name for probing
 * Parameters:
 *    str : the original string that contains the full domain name
 *    str_len : the length of str
 *    name_start : the start position of the probing name
 *    name_len : the langth of the probing name
 * Returns:
 *
 */
STATICFN void
mdns_name_reassign(uint8 *str, uint8 *str_len, uint8 name_start,
    uint8 *name_len)
{
    int8    i;
    uint8   len;

    /* Check if the last char is bracket or not */
    i = name_start + *name_len - 1;
    len = *name_len;
    if (str[i] == ')') {
        i--;
        if (MDNS_IS_DIGIT(str[i])) {
            i--;
            if (str[i] == '(') {
                /* (x) */
                i++;
                if (str[i] == '9') {
                    /* change to 2-digit number */

                    /* shift one char space */
                    mdns_string_shift_down(str, (name_start + len),
                        (*str_len - name_start- len - 1), 1);
                    str[i] = '1';
                    str[i + 1] = '0';
                    str[i + 2] = ')';
                    str[name_start] += 1;
                    *str_len += 1;
                    *name_len += 1;
                } else {
                    /* increase the index */
                    str[i]++;
                }
                return;
            } else if (MDNS_IS_DIGIT(str[i]) && (str[i - 1] == '(')) {
                /* (xx) */
                if (str[i + 1] == '9') {
                    str[i]++;
                    str[i + 1] = '0';
                } else {
                    /* increase the index */
                    str[i + 1]++;
                }
                return;
            }

        }
    }

    /* No any brackets
     * append (2) at the end of the name
     */

    mdns_string_shift_down(str, (name_start + len), (*str_len - name_start- len - 1), 3);
    i = name_start + *name_len;
    str[i++] = '(';
    str[i++] = '2';
    str[i] = ')';
    str[name_start] += 3;
    *str_len += 3;
    *name_len += 3;


    return;
}


STATICFN void
mdns_hostname_reassign(uint8 *str, uint8 name_start, uint8 name_len)
{
    int8    i;

    /* Check if the last char is bracket or not */
    i = name_start + name_len - 1;
    while (str[i] == 'f') {
        str[i] = '0';
        i--;
    }
    str[i]++;
    return;
}


/*
 * Function:
 *   mdns_addr_advertise
 * Description:
 *   The main task of address records' advertisement
 * Parameters:
 *
 * Returns:
 *
 */
STATICFN void
mdns_addr_advertise(uint8       type)
{
    mdns_naming_state_t *name_state;

#if CFG_UIP_IPV6_ENABLED
    if (type == MDNS_ADVERTISE_IPV6) {
        name_state = &(mdns.name_ipv6);
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    {
        name_state = &(mdns.name_ipv4);
    }

    switch(name_state->state) {
        case MDNS_SRV_STATE_INIT:
            /* Add the delay to the first probe packet */
            name_state->probe_count = 0;
            name_state->ad_count = 0;
            name_state->state = MDNS_SRV_STATE_PROBE;
            /* Schedule next probe time */
            name_state->last_probe = sal_get_ticks();
            name_state->probe_interval = SAL_USEC_TO_TICKS(MDNS_PROBE_DELAY);

            /* Construct the address record */
            mdns_addr_rr_initialized(type);

            /* Enable the probe timer */
            mdns_timer_trigger();
            break;
        case MDNS_SRV_STATE_PROBE:
            if (name_state->probe_count >= MDNS_PROBE_NUM) {
                /* Schedule the next mDNS annoucements */
                name_state->ad_count = 0;
                name_state->state = MDNS_SRV_STATE_ANNOUNCE;
                name_state->last_ad = sal_get_ticks();
                name_state->ad_interval =
                    SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);

            }
            break;
        case MDNS_SRV_STATE_ANNOUNCE:
        case MDNS_SRV_STATE_GOODBYE:
            break;
        case MDNS_SRV_STATE_CONFLICT:
            name_state->conflict_count++;
            if (name_state->conflict_count > MDNS_MAX_RPOBE_CONFLICT) {
                name_state->state = MDNS_SRV_STATE_WAIT;
                break;
            }
            /* change the name */
            mdns_hostname_reassign(mdns.host_dn, 0, mdns.host_name_len);
            mdns_service_host_name_update();
            mdns.name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
            mdns.name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED */
            break;
        case MDNS_SRV_STATE_DISABLED:
            break;
        case MDNS_SRV_STATE_FAILED:
            break;
        case MDNS_SRV_STATE_WAIT:
            /* wait 5 seconds */
            if (SAL_TIME_EXPIRED(name_state->last_probe,
                SAL_USEC_TO_TICKS(MDNS_PROBE_CONFLICT_WAIT))) {
                /* change the name */
                mdns_hostname_reassign(mdns.host_dn, 0, mdns.host_name_len);
                mdns_service_host_name_update();
                name_state->conflict_count = 0;
                mdns.name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
                mdns.name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED */
            }
            break;
        default:
            break;
    }

    return;
}

/*
 * Function:
 *   mdns_service_advertise
 * Description:
 *   The main task of service records' advertisement
 * Parameters:
 *
 * Returns:
 *
 */
STATICFN void
mdns_service_advertise(uint8 type)
{
    dns_service_info_t *service;
    mdns_naming_state_t *name_state;

    service = mdns.srv_list;

    while (service) {
        if (type == MDNS_ADVERTISE_IPV4) {
            name_state = &service->name_ipv4;
#if CFG_UIP_IPV6_ENABLED
        } else if (type == MDNS_ADVERTISE_IPV6) {
            name_state = &service->name_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
        } else {
            return;
        }
        switch(name_state->state) {
            case MDNS_SRV_STATE_INIT:
                name_state->probe_count = 0;
                name_state->state = MDNS_SRV_STATE_PROBE;
                /* Schedule next probe time */
                name_state->last_probe = sal_get_ticks();
                name_state->probe_interval =
                    (name_state->last_probe %
                    SAL_USEC_TO_TICKS(MDNS_PROBE_DELAY));
                mdns_timer_trigger();
                break;
            case MDNS_SRV_STATE_PROBE:
                if (name_state->probe_count >= MDNS_PROBE_NUM) {
                    /* Schedule the sending time of annoucements */
                    name_state->ad_count = 0;
                    name_state->state = MDNS_SRV_STATE_ANNOUNCE;
                    name_state->last_ad = sal_get_ticks();
                    name_state->ad_interval =
                        SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);

                }
                break;
            case MDNS_SRV_STATE_ANNOUNCE:
            case MDNS_SRV_STATE_GOODBYE:
                break;
            case MDNS_SRV_STATE_CONFLICT:
                name_state->conflict_count++;
                if (name_state->conflict_count > MDNS_MAX_RPOBE_CONFLICT) {
                    name_state->state = MDNS_SRV_STATE_WAIT;
                    break;
                }
                /* change the name */
                mdns_name_reassign(service->name, &service->name_len, 0,
                    &service->instance_len);
                mdns_service_instance_name_update(service);
                service->name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
                service->name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED */
                break;
            case MDNS_SRV_STATE_DISABLED:
                break;
            case MDNS_SRV_STATE_FAILED:
                break;
            case MDNS_SRV_STATE_WAIT:
                /* wait 5 seconds */
                if (SAL_TIME_EXPIRED(name_state->last_probe,
                    MDNS_PROBE_CONFLICT_WAIT)) {
                    /* change the name */
                    mdns_name_reassign(service->name, &service->name_len, 0,
                        &service->instance_len);
                    mdns_service_instance_name_update(service);
                    name_state->conflict_count = 0;
                    service->name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
                    service->name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED */
                }
                break;
            default:
                break;
        }
        service = service->next;
    }

    return;
}


void
mdns_http_service_txt_rdata_build(dns_rr_info_t *txt_rr)
{
    uint16  index;
#ifndef __BOOTLOADER__
    uint8   len;
    char    *ptr;
    uint8   v1, v2, v3, v4;
    char    ser[MAX_SERIAL_NUM_LEN + 1];

    len = 0; /* the length of each key-value pair */
    index = 0;

    /* 1. version */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "version=1.0");
    http_txt_rdata[index] = len;
    index += (len + 1);


    /* 2. model */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "model=%s",CFG_MDNS_MODEL_NAME);
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 3. deviceType */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "deviceType=Switch");
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 4. deviceDescr */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "deviceDescr=%s",CFG_MDNS_MODEL_DESC);
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 5. fmVersion */
    ptr = (char *)&http_txt_rdata[index + 1];
    board_firmware_version_get(&v1, &v2, &v3, &v4);
    len = sal_sprintf(ptr, "fmVersion=%u.%u.%u.%u",
        (int)v1, (int)v2, (int)v3, (int)v4);
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 6. PIDVID */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "PIDVID=N/A");
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 7. MACAddress */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "MACAddress=%02bX%02bX%02bX%02bX%02bX%02bX",
        mdns.sys_mac[0],mdns.sys_mac[1],mdns.sys_mac[2],
        mdns.sys_mac[3],mdns.sys_mac[4],mdns.sys_mac[5]);
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 8. serialNo */
    ptr = (char *)&http_txt_rdata[index + 1];
    get_serial_num(&v1, ser);
    if (v1 == TRUE) {
        len = sal_sprintf(ptr, "serialNo=%s",ser);
    } else {
        len = sal_sprintf(ptr, "serialNo=N/A");
    }
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 9. hostname */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "hostname=%s",mdns.sys_name);
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 10. slave */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "slave=0");
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 11. MDFID */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "MDFID=N/A");
    http_txt_rdata[index] = len;
    index += (len + 1);

    /* 12. accessType */
    ptr = (char *)&http_txt_rdata[index + 1];
    len = sal_sprintf(ptr, "accessType=http,console");
    http_txt_rdata[index] = len;
    index += (len + 1);
#else /* !__BOOTLOADER__ */
    /* Empty */
    index = 0;
    http_txt_rdata[index++] = 0x0;
#endif /* __BOOTLOADER__ */

    txt_rr->rdlength = index;
    txt_rr->rdata = http_txt_rdata;


}

/*
 * Function:
 *   mdns_init
 * Description:
 *   Construct the HTTP service records
 * Parameters:
 *
 * Returns:
 *   SYS_OK
 *   SYS_ERR
 */
STATICFN sys_error_t
mdns_http_service_init(void)
{
    uint8 *ptr, index;
    sys_error_t rv;
    dns_rr_info_t   *temp_rr;

    /*
     * Instance name : Broadcom UM-switch web
     * sub-service type : none
     * service type : "_http._tcp"
     * domain name :  local
     */

    http_srv = sal_malloc(sizeof(dns_service_info_t));
    if (!http_srv) {
        goto http_srv_error;
    }
    sal_memset(http_srv, 0, sizeof(dns_service_info_t));

    /* Instance name */
    ptr = http_srv->name;
    http_srv->instance_len = sal_strlen(http_srv_name);
    mdns_convert_string2label(http_srv_name, ptr);
    ptr += http_srv->instance_len;

    http_srv->sub_proto_len = 0; /* Null sub-type name */

    /* Protocol name */
    http_srv->proto_len = sal_strlen(http_srv_type);
    mdns_convert_string2label(http_srv_type, ptr);
    ptr += http_srv->proto_len;

    /* Domain name */
    http_srv->domain_len = sal_strlen(http_srv_domain);
    mdns_convert_string2label(http_srv_domain, ptr);
    ptr += http_srv->domain_len;
    *ptr = '\0';
    http_srv->name_len = http_srv->instance_len + http_srv->sub_proto_len +
        http_srv->proto_len + http_srv->domain_len + 1;

    /* Construct resource records */
    /* SRV record */
    http_srv->srv_rr = sal_malloc(sizeof(dns_rr_info_t));
    if (!http_srv->srv_rr) {
        goto http_srv_error;
    }
    temp_rr = http_srv->srv_rr;
    sal_memset(temp_rr, 0, sizeof(dns_rr_info_t));

    temp_rr->rname = http_srv->name;
    temp_rr->rname_len = http_srv->name_len;
    temp_rr->unique = TRUE;
    temp_rr->rtype = DNS_TYPE_SRV;
    temp_rr->rclass = DNS_CLASS_IN;
    temp_rr->ttl = DNS_RR_TTL_SRV;
    temp_rr->enabled = TRUE;

    /* rdata */
    temp_rr->rdlength = mdns.host_dn_len + 6;
    /* Dynamic allocate to reduce the RAM coonsumption */
    temp_rr->rdata = sal_malloc(temp_rr->rdlength);
    index = 0;
    /* priority */
    temp_rr->rdata[index++] = 0;
    temp_rr->rdata[index++] = 0;
    /* weight */
    temp_rr->rdata[index++] = 0;
    temp_rr->rdata[index++] = 0;
    /* port */
    temp_rr->rdata[index++] = (HTTP_TCP_PORT >> 8);
    temp_rr->rdata[index++] = HTTP_TCP_PORT & 0xff;
    /* host domain name */
    sal_memcpy(&temp_rr->rdata[index], mdns.host_dn, mdns.host_dn_len);
    /* end of SRV record */

    /* PTR record */
    http_srv->ptr_rr = sal_malloc(sizeof(dns_rr_info_t));
    if (!http_srv->ptr_rr) {
        goto http_srv_error;
    }
    temp_rr = http_srv->ptr_rr;
    sal_memset(temp_rr, 0, sizeof(dns_rr_info_t));

    temp_rr->rname = &(http_srv->name[http_srv->instance_len]);
    temp_rr->rname_len = http_srv->name_len - http_srv->instance_len;
    temp_rr->unique = FALSE;
    temp_rr->rtype = DNS_TYPE_PTR;
    temp_rr->rclass = DNS_CLASS_IN;
    temp_rr->ttl = DNS_RR_TTL_PTR;
    temp_rr->enabled = TRUE;
    temp_rr->rdlength = http_srv->name_len;
    temp_rr->rdata = sal_malloc(http_srv->name_len);
    if (temp_rr->rdata == NULL) {
        goto http_srv_error;
    }
    sal_memcpy(temp_rr->rdata, http_srv->name, temp_rr->rdlength);

    /* end of PTR record */

    /* TXT record */
    http_srv->txt_rr = sal_malloc(sizeof(dns_rr_info_t));
    if (!http_srv->txt_rr) {
        goto http_srv_error;
    }
    temp_rr = http_srv->txt_rr;
    sal_memset(temp_rr, 0, sizeof(dns_rr_info_t));
    temp_rr->rname = http_srv->name;
    temp_rr->rname_len = http_srv->name_len;
    temp_rr->unique = TRUE;
    temp_rr->rtype = DNS_TYPE_TXT;
    temp_rr->rclass = DNS_CLASS_IN;
    temp_rr->ttl = DNS_RR_TTL_TXT;
    temp_rr->enabled = TRUE;


    /* rdata */
    mdns_http_service_txt_rdata_build(temp_rr);
    /* end of TXT record */

    /* DNS-SD PTR record */
    http_srv->dns_rr = sal_malloc(sizeof(dns_rr_info_t));
    if (!http_srv->dns_rr) {
        goto http_srv_error;
    }
    temp_rr = http_srv->dns_rr;
    sal_memset(temp_rr, 0, sizeof(dns_rr_info_t));

    temp_rr->rname = (uint8 *)dns_query;
    temp_rr->rname_len = MDNS_GENERAL_QUERY_LEN;
    temp_rr->unique = FALSE;
    temp_rr->rtype = DNS_TYPE_PTR;
    temp_rr->rclass = DNS_CLASS_IN;
    temp_rr->ttl = DNS_RR_TTL_PTR;
    temp_rr->rdlength = http_srv->name_len - http_srv->instance_len;
    temp_rr->rdata = sal_malloc(temp_rr->rdlength);
    if (temp_rr->rdata == NULL) {
        goto http_srv_error;
    }
    sal_memcpy(temp_rr->rdata,
        &http_srv->name[http_srv->instance_len], temp_rr->rdlength);
    /* end of DNS PTR record */


    rv = mdns_service_add(http_srv);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("Add HTTP service failed\n");
#endif /* CFG_CONSOLE_ENABLED */
    }
    return SYS_OK;

http_srv_error:
    if (!http_srv->srv_rr) {
        if (!http_srv->srv_rr->rdata) {
            sal_free(http_srv->srv_rr->rdata);
        }
        sal_free(http_srv->srv_rr);
    }
    if (!http_srv->ptr_rr) {
        if (!http_srv->ptr_rr->rdata) {
            sal_free(http_srv->ptr_rr->rdata);
        }
        sal_free(http_srv->ptr_rr);
    }
    if (!http_srv->txt_rr) {
        if (!http_srv->txt_rr->rdata) {
            sal_free(http_srv->txt_rr->rdata);
        }
        sal_free(http_srv->txt_rr);
    }
    if (!http_srv->dns_rr) {
        if (!http_srv->dns_rr->rdata) {
            sal_free(http_srv->dns_rr->rdata);
        }
        sal_free(http_srv->dns_rr);
    }

#if CFG_CONSOLE_ENABLED
    sal_printf("HTTP service record initilization failed.\n");
#endif /* CFG_CONSOLE_ENABLED */
    return SYS_ERR;
}

/* Check if any address or services is in PROBE state */
STATICFN BOOL
mdns_probe_state_check(void)
{
    dns_service_info_t *srv;

    /* Address */
    if (mdns.name_ipv4.state == MDNS_SRV_STATE_PROBE) {
        return TRUE;
    }
#if CFG_UIP_IPV6_ENABLED
    if (mdns.name_ipv6.state == MDNS_SRV_STATE_PROBE) {
        return TRUE;
    }
#endif /* CFG_UIP_IPV6_ENABLED */


    /* Services */
    srv = mdns.srv_list;

    while (srv) {
        if (srv->name_ipv4.state == MDNS_SRV_STATE_PROBE) {
            return TRUE;
        }
#if CFG_UIP_IPV6_ENABLED
        if (srv->name_ipv6.state == MDNS_SRV_STATE_PROBE) {
            return TRUE;
        }
#endif /* CFG_UIP_IPV6_ENABLED */
        srv = srv->next;
    }

    return FALSE;
}

/* Check if any reply list is empty. */
STATICFN BOOL
mdns_reply_list_empty(void)
{
    /* Address */
    if (mdns.reply_list_ipv4) {
        return FALSE;
    }
#if CFG_UIP_IPV6_ENABLED
    if (mdns.reply_list_ipv6) {
        return FALSE;
    }
#endif /* CFG_UIP_IPV6_ENABLED */

    return TRUE;
}


/*
 * Function:
 *   mdns_timer_task
 * Description:
 *   Handle the timing requirement of MDNS
 * Parameters:
 *
 * Returns:
 */
STATICCBK void
mdns_timer_task(void *param)
{
    UNREFERENCED_PARAMETER(param);

    /* Process the application event */
    udp_conns_periodic();

    /* Enable timer */
    mdns.probe_timer_enable = mdns_probe_state_check() |
                                        (!mdns_reply_list_empty());
    if (!mdns.probe_timer_enable) {
        timer_remove(mdns_timer_task);
    }
}

STATICFN void
mdns_timer_trigger(void)
{
    BOOL    enable;

    /* Check if any services or address in PROVE state */
    enable = mdns_probe_state_check() | (!mdns_reply_list_empty());

    if (enable != mdns.probe_timer_enable) {
        if (enable) {
            timer_add(mdns_timer_task, NULL, MDNS_TIMER_INTERVAL);
        } else {
            timer_remove(mdns_timer_task);
        }
        mdns.probe_timer_enable = enable;
    }

    return;
}


#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
void
mdns_intf_change_notify(BOOL up)
{
    dns_service_info_t  *service;
    if (mdns.enable) {
        if (!up) {
            /* Initilialized the IPv4/IPv6 address state */
            mdns.name_ipv4.state = MDNS_SRV_STATE_DISABLED;
#if CFG_UIP_IPV6_ENABLED
            mdns.name_ipv6.state = MDNS_SRV_STATE_DISABLED;
#endif /* CFG_UIP_IPV6_ENABLED */

            service = mdns.srv_list;

            while (service) {
                service->name_ipv4.state = MDNS_SRV_STATE_DISABLED;
#if CFG_UIP_IPV6_ENABLED
                service->name_ipv6.state = MDNS_SRV_STATE_DISABLED;
#endif /* CFG_UIP_IPV6_ENABLED */
                service = service->next;
            }
        } else {
            /* Interface up */
#ifdef __BOOTLOADER__
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
            if (board_loader_mode_get(NULL, FALSE) == LM_UPGRADE_FIRMWARE) {
                /* Start IPv4 MDNS */
                mdns_start();
            } else {
                /* Normal mode */
#ifdef CFG_ZEROCONF_AUTOIP_INCLUDED
                /* Start auto ip configuration */
                uip_autoip_start();
#else /* !CFG_ZEROCONF_AUTOIP_INCLUDED */
                /* Start IPv4 MDNS */
                mdns_start();
#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
            }

#endif /* CFG_ZEROCONF_AUTOIP_INCLUDED */
#else /* !__BOOTLOADER__ */
            /* Trigger the MDNS if the IPv4 address configuration is static */
            if (mdns.iconfig == INET_CONFIG_STATIC) {
                /* Start IPv4 MDNS */
                mdns_start();
            }
#endif /* __BOOTLOADER__ */
        }
    }

}

#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

/*
 * Function:
 *   mdns_init
 * Description:
 *   MDNS initialization
 * Parameters:
 *
 * Returns:
 */
/* Be called after the system IP configuration is done */
void
mdns_init(void)
{
    sys_error_t rv = SYS_OK;

    sal_memset(&mdns, 0, sizeof(mdns_state_t));
    /* Get system enable */
#ifdef __BOOTLOADER__
    mdns.enable = TRUE;
#else /* !__BOOTLOADER__ */
    mdns.enable = FALSE;
#endif /* __BOOTLOADER__ */

    /* device MAC address */
    rv = get_system_mac(mdns.sys_mac);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_init: Failed to get the system MAC address!\n");
#endif /* CFG_CONSOLE_ENABLED */
    }

    /* Services list */
    mdns.srv_list = NULL;

    /* Reply list */
    mdns.reply_list_ipv4 = NULL;
#if CFG_UIP_IPV6_ENABLED
    mdns.reply_list_ipv6 = NULL;
#endif /* CFG_UIP_IPV6_ENABLED */

    http_srv = NULL;
    mdns.iconfig = INET_CONFIG_OFF;

    mdns.name_ipv4.state = MDNS_SRV_STATE_DISABLED;
#if CFG_UIP_IPV6_ENABLED
    mdns.name_ipv6.state = MDNS_SRV_STATE_DISABLED;
#endif /* CFG_UIP_IPV6_ENABLED */

    /* Timer */
    mdns.probe_timer_enable = FALSE;

    /* Disable to receive the MDNS packets */
    rv = board_mdns_enable_set(mdns.enable);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_init: Failed to enable MDNS packets to CPU !\n");
#endif /* CFG_CONSOLE_ENABLED */
    }

#ifdef CFG_NET_LINKCHANGE_NOTIFY_INCLUDED
    net_register_linkchange(mdns_intf_change_notify);
#endif /* CFG_NET_LINKCHANGE_NOTIFY_INCLUDED */

    return;
}


void
mdns_host_domain_name_set(void)
{
    uint8   *ptr;
    sys_error_t rv = SYS_OK;
    uint8   sys_name[MAX_SYSTEM_NAME_LEN];

    if (!mdns.enable) {
        return;
    }

    /* Construct the address record of the device */
    rv = get_system_name((char *)sys_name, MAX_SYSTEM_NAME_LEN);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_init: Failed to get the system name!\n");
#endif /* CFG_CONSOLE_ENABLED */
        return;
    }
    /* Check if the system name is changed */
    if (!sal_strcmp((const char *)sys_name, (const char *)mdns.sys_name)) {
        return;
    }
    sal_strcpy((char *)mdns.sys_name, (const char *)sys_name);
    mdns.sys_name_len = sal_strlen((char *)mdns.sys_name);


    /* Hostname */
    mdns.host_dn[0] = mdns.sys_name_len + 7;
    sal_memcpy(&mdns.host_dn[1], mdns.sys_name, mdns.sys_name_len);
    /* Append last threee bytes of MAC address */
    ptr = &mdns.host_dn[mdns.sys_name_len + 1];
    sal_sprintf((char *)ptr, "-%02x%02x%02x",
        (int)mdns.sys_mac[3], (int)mdns.sys_mac[4], (int)mdns.sys_mac[5]);
    mdns.host_name_len = mdns.sys_name_len + 8;

    ptr = &mdns.host_dn[mdns.host_name_len];
    /* Domain name */
    sal_memcpy(ptr, local_domain, MDNS_LOCAL_DOMAIN_LEN);
    ptr += MDNS_LOCAL_DOMAIN_LEN;
    *ptr = 0x0; /* zero length byte */
    mdns.host_dn_len = mdns.host_name_len + MDNS_LOCAL_DOMAIN_LEN + 1;

    mdns_service_host_name_update();
}



/*
 * Function:
 *   mdns_start
 * Description:
 *   MDNS initialization
 * Parameters:
 *
 * Returns:
 */
/* Be called after the system IP configuration is done */
void
mdns_start(void)
{
    sys_error_t rv = SYS_OK;
    uip_ipaddr_t mdns_addr;

    /* device IP */
    mdns.iconfig = (uint8)get_network_interface_config(mdns.sys_ip,
                                        mdns.sys_netmask,
                                        mdns.sys_gateway);
    mdns_host_domain_name_set();

    /* Address record */
    mdns.name_ipv4.state = MDNS_SRV_STATE_INIT;

    /* Create UDP session for MDNS */
    if (mdns.conn == NULL) {
        uip_ipaddr(&mdns_addr, mdns_ip4[0], mdns_ip4[1],
            mdns_ip4[2], mdns_ip4[3]);
        mdns.conn = uip_udp_new((uip_ipaddr_t *)&mdns_addr,
                                            UIP_HTONS(DNS_MULTICAST_PORT));
        if (mdns.conn != NULL) {
            uip_udp_bind(mdns.conn, UIP_HTONS(DNS_MULTICAST_PORT));
        }
    }

    /* Construct the HTTP service records */
    if (http_srv == NULL) {
        rv = mdns_http_service_init();
        if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_start: HTTP service initialization failed!\n");
#endif /* CFG_CONSOLE_ENABLED */
        }
    }

    /* State */
    http_srv->name_ipv4.state = MDNS_SRV_STATE_INIT;

    /* Enable to receive the MDNS packets */
    rv = board_mdns_enable_set(mdns.enable);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_start: Failed to enable MDNS packets to CPU !\n");
#endif /* CFG_CONSOLE_ENABLED */
    }

    return;
}


#if CFG_UIP_IPV6_ENABLED
/*
 * Function:
 *   mdns_init
 * Description:
 *   MDNS initialization
 * Parameters:
 *
 * Returns:
 */
/* Be called after the system IP configuration is done */
void
mdns_start_ipv6(uip_ip6addr_t *ipv6_addr)
{
    sys_error_t rv = SYS_OK;

    if (mdns.conn_v6 && (mdns.name_ipv6.state != MDNS_SRV_STATE_DISABLED)) {
        /* Update the ipv6 address if device's is a link-local address */
        if ((mdns.sys_ip6[0] == 0xFE) && (mdns.sys_ip6[1] == 0x80)) {
            sal_memcpy(mdns.sys_ip6, ipv6_addr, 16);
            sal_memcpy(mdns.addr6_rr.rdata, ipv6_addr, 16);
            mdns.name_ipv6.state = MDNS_SRV_STATE_INIT;
        }
        return;
    }

    mdns_host_domain_name_set();
    mdns.name_ipv6.state = MDNS_SRV_STATE_INIT;

    sal_memcpy(mdns.sys_ip6, ipv6_addr, 16);

    if (mdns.conn_v6 == NULL) {
        /* Create UDP session for MDNS */
        mdns.conn_v6 = uip6_udp_new((uip_ip6addr_t *)&mdns_ip6, UIP_HTONS(DNS_MULTICAST_PORT));

        if (mdns.conn_v6 != NULL) {
            uip_udp_bind(mdns.conn_v6, UIP_HTONS(DNS_MULTICAST_PORT));
        }
    }

    /* Construct the HTTP service records */
    if (http_srv == NULL) {
        rv = mdns_http_service_init();
        if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_start_ipv6: HTTP service initialization failed!\n");
#endif /* CFG_CONSOLE_ENABLED */
        }
    }

    /* State */
    http_srv->name_ipv6.state = MDNS_SRV_STATE_INIT;

    /* Enable to receive the MDNS packets */
    rv = board_mdns_enable_set(mdns.enable);
    if (rv) {
#if CFG_CONSOLE_ENABLED
        sal_printf("mdns_startipv6: Failed to enable MDNS packets to CPU !\n");
#endif /* CFG_CONSOLE_ENABLED */
    }

    return;
}

#endif /* CFG_UIP_IPV6_ENABLED */

/*
 * Function:
 *   mdns_init
 * Description:
 *   MDNS initialization
 * Parameters:
 *
 * Returns:
 */
void
mdns_appcall(void)
{

    /* Reset the send DNS packet type */
    mdns.send_pkt_type = MDNS_SEND_NONE_PKT;

    if (uip_newdata()) {
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            dns_buf_ptr = (uint8 *)DNS6BUF;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            dns_buf_ptr = (uint8 *)DNSBUF;
        }
        mdns_packet_in();
        if (mdns.send_pkt_type == MDNS_SEND_RESP_PKT) {
            sal_memset(dns_buf_ptr, 0, DNS_HDR_LEN);
            dns_buf_offset = DNS_HDR_LEN;

#ifndef __BOOTLOADER__
            if (mdns.send_delay) {
                /* Add it to reply list */
                mdns_reply_list_add();
            } else
#endif /* __BOOTLOADER__ */
            {
                /* Send out the packet immediately */
                mdns_packet_out();
            }
            if (dns_buf_offset == DNS_HDR_LEN) {
                /* No DNS questions or records to be sent */
                return;
            }
#if CFG_UIP_IPV6_ENABLED
            if (uip_ipv6) {
                uip_appdata = &uip_buf[UIP6_IPUDPH_LEN + UIP_LLH_LEN];
                uip6_send(uip_appdata, dns_buf_offset);
            } else
#endif /* CFG_UIP_IPV6_ENABLED */
            {
                uip_appdata = &uip_buf[UIP_IPUDPH_LEN + UIP_LLH_LEN];
                uip_send(uip_appdata, dns_buf_offset);
            }
        }
    } else {

        /* Advertise the device IP address record first */
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            mdns_addr_advertise(MDNS_ADVERTISE_IPV6);
            if (mdns.name_ipv6.state == MDNS_SRV_STATE_ANNOUNCE) {
                mdns_service_advertise(MDNS_ADVERTISE_IPV6);
            }
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            mdns_addr_advertise(MDNS_ADVERTISE_IPV4);
            if (mdns.name_ipv4.state == MDNS_SRV_STATE_ANNOUNCE) {
                mdns_service_advertise(MDNS_ADVERTISE_IPV4);
            }
        }

        /* DNS buffer and header initialization */
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
            dns_buf_ptr = (uint8 *)DNS6BUF;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
            dns_buf_ptr = (uint8 *)DNSBUF;
        }
        sal_memset(dns_buf_ptr, 0, DNS_HDR_LEN);
        dns_buf_offset = DNS_HDR_LEN;

        /* Check if any probe and response packets need to be sent out */
#if CFG_UIP_IPV6_ENABLED
        if (uip_ipv6) {
#ifndef __BOOTLOADER__
            mdns_goodbye_task(MDNS_ADVERTISE_IPV6);
#endif /* __BOOTLOADER__ */
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_probe_task(MDNS_ADVERTISE_IPV6);
            }
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_announce_task(MDNS_ADVERTISE_IPV6);
            }
#ifndef __BOOTLOADER__
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_answer_task(MDNS_ADVERTISE_IPV6);
            }
#endif /* __BOOTLOADER__ */
            if (mdns.send_pkt_type != MDNS_SEND_NONE_PKT) {
                uip_appdata = &uip_buf[UIP6_IPUDPH_LEN + UIP_LLH_LEN];
                uip6_send(uip_appdata, dns_buf_offset);
            }
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        {
#ifndef __BOOTLOADER__
            mdns_goodbye_task(MDNS_ADVERTISE_IPV4);
#endif /* __BOOTLOADER__*/
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_probe_task(MDNS_ADVERTISE_IPV4);
            }
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_announce_task(MDNS_ADVERTISE_IPV4);
            }
#ifndef __BOOTLOADER__
            if (mdns.send_pkt_type == MDNS_SEND_NONE_PKT) {
                mdns_answer_task(MDNS_ADVERTISE_IPV4);
            }
#endif /* __BOOTLOADER__ */
            if (mdns.send_pkt_type != MDNS_SEND_NONE_PKT) {
                uip_appdata = &uip_buf[UIP_IPUDPH_LEN + UIP_LLH_LEN];
                uip_send(uip_appdata, dns_buf_offset);
            }
        }
    }


    return;
}

#ifndef __BOOTLOADER__

STATICFN void
mdns_goodbye_task(uint8 type)
{
    dns_service_info_t      *srv;
    mdns_naming_state_t     *name_state;

    if (type == MDNS_ADVERTISE_IPV4) {
        name_state = &mdns.name_ipv4;
#if CFG_UIP_IPV6_ENABLED
    } else if (type == MDNS_ADVERTISE_IPV6) {
        name_state = &mdns.name_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
    } else {
        return;
    }

    if (name_state->state == MDNS_SRV_STATE_GOODBYE) {
        /* IPv4 address record */
        if (mdns.addr_rr.enabled) {
            /* Send goodbye packets for address record */
            mdns.addr_rr.send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
            mdns.addr_rr.send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
        }
        /* IPv6 address record */
        if (mdns.addr6_rr.enabled) {
            /* Send goodbye packets for address record */
            mdns.addr6_rr.send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
            mdns.addr6_rr.send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
        }
        name_state->state = MDNS_SRV_STATE_DISABLED;
        mdns.send_pkt_type = MDNS_SEND_GOODBYE_PKT;
    }

    /* Service */
    srv = mdns.srv_list;
    while (srv) {
         if (type == MDNS_ADVERTISE_IPV4) {
            name_state = &srv->name_ipv4;
#if CFG_UIP_IPV6_ENABLED
        } else if (type == MDNS_ADVERTISE_IPV6) {
            name_state = &srv->name_ipv6;
#endif /* CFG_UIP_IPV6_ENABLED */
        }
        if (name_state->state == MDNS_SRV_STATE_GOODBYE) {
            if (srv->ptr_rr) {
                srv->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                srv->ptr_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
            }
            name_state->state = MDNS_SRV_STATE_DISABLED;
            mdns.send_pkt_type = MDNS_SEND_GOODBYE_PKT;
        }
        srv = srv->next;
    }

    if (mdns.send_pkt_type == MDNS_SEND_GOODBYE_PKT) {
        mdns_packet_out();
    }

    return;
}

#endif /* __BOOTLOADER */

/*
 * Function:
 *   mdns_probe_task
 * Description:
 *   Check if any probe packets need to be sent out
 * Parameters:
 *
 * Returns:
 */
void
mdns_probe_task(uint8 type)
{
    dns_service_info_t  *service;
    mdns_naming_state_t *name_state;
    dns_rr_info_t       *rr;

    /* Check if the bonjour is enabled */
    if (!mdns.enable) {
        return;
    }

    if (mdns.send_pkt_type == MDNS_SEND_RESP_PKT) {
        return;
    }

#if CFG_UIP_IPV6_ENABLED
    if (type == MDNS_ADVERTISE_IPV6) {
        name_state = &mdns.name_ipv6;
        rr = &mdns.addr6_rr;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    if (type == MDNS_ADVERTISE_IPV4) {
        name_state = &mdns.name_ipv4;
        rr = &mdns.addr_rr;
    } else {
        return;
    }

    /* Check if any request need to be sent out */
    if (name_state->state == MDNS_SRV_STATE_PROBE) {
        if (SAL_TIME_EXPIRED(name_state->last_probe,
                    name_state->probe_interval) > 0) {
            /* Send the probe packet */
            rr->send_flags |= (DNS_RR_SEND_FLAG_IN_QUEST |
                DNS_RR_SEND_FLAG_IN_AUTH);
            rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
            mdns.send_pkt_type = MDNS_SEND_QUERY_PKT;
            name_state->probe_count++;
            name_state->last_probe = sal_get_ticks();
            if (name_state->probe_count == 1) {
                /* Record the first probe time */
                name_state->first_probe = name_state->last_probe;
            }
            /* schedule next probe time */
            name_state->probe_interval =
                (name_state->last_probe %
                    SAL_USEC_TO_TICKS(MDNS_PROBE_DELAY));
        }
    }


    /* Check the probe packets of services */
    service = mdns.srv_list;

    while (service) {
        /* Ipv4/IPv6 */
#if CFG_UIP_IPV6_ENABLED
        if (type == MDNS_ADVERTISE_IPV6) {
            name_state = &service->name_ipv6;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        if (type == MDNS_ADVERTISE_IPV4) {
            name_state = &service->name_ipv4;
        }

        if ((name_state->state == MDNS_SRV_STATE_PROBE) &&
            SAL_TIME_EXPIRED(name_state->last_probe,
                name_state->probe_interval) > 0) {
            /* Send the probe packet */
            service->srv_rr->send_flags |= (DNS_RR_SEND_FLAG_IN_QUEST |
                DNS_RR_SEND_FLAG_IN_AUTH);
            service->srv_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
            service->txt_rr->send_flags |= DNS_RR_SEND_FLAG_IN_AUTH;
            service->txt_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
            mdns.send_pkt_type = MDNS_SEND_QUERY_PKT;
            name_state->probe_count++;
            name_state->last_probe = sal_get_ticks();
            if (name_state->probe_count == 1) {
                /* Record the first probe time */
                name_state->first_probe = name_state->last_probe;
            }
            /* schedule next probe time */
            name_state->probe_interval =
                (name_state->last_probe %
                SAL_USEC_TO_TICKS(MDNS_PROBE_DELAY));

        }

        service = service->next;
    }

    if (mdns.send_pkt_type == MDNS_SEND_QUERY_PKT) {
        mdns_packet_out();
    }
    return;
}

/*
 * Function:
 *   mdns_announce_task
 * Description:
 *   Check if any announcement or response packets need to be sent out
 * Parameters:
 *
 * Returns:
 */

void
mdns_announce_task(uint8 type)
{
    dns_service_info_t  *service;
    mdns_naming_state_t *name_state;
    dns_rr_info_t       *rr;

    /* Check if the bonjour is enabled */
    if (!mdns.enable) {
        return;
    }
    if (mdns.send_pkt_type == MDNS_SEND_QUERY_PKT) {
        return;
    }

#if CFG_UIP_IPV6_ENABLED
    if (type == MDNS_ADVERTISE_IPV6) {
        name_state = &mdns.name_ipv6;
        rr = &mdns.addr6_rr;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    if (type == MDNS_ADVERTISE_IPV4) {
        name_state = &mdns.name_ipv4;
        rr = &mdns.addr_rr;
    } else {
        return;
    }

    /* Check if any annoucement need to be sent out */
    if ((name_state->state == MDNS_SRV_STATE_ANNOUNCE) &&
        (name_state->ad_count < MDNS_ANNOUNCE_NUM)) {
        if (SAL_TIME_EXPIRED(name_state->last_ad,
                    name_state->ad_interval) > 0) {
            /* Send out the announce packet */
            rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
            rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
            mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
            name_state->ad_count++;
            name_state->last_ad = sal_get_ticks();
            name_state->ad_interval =
                SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);
        }
    }

    /* Check the announce packets of services */
    service = mdns.srv_list;

    while (service) {
#if CFG_UIP_IPV6_ENABLED
        if (type == MDNS_ADVERTISE_IPV6) {
            name_state = &service->name_ipv6;
        } else
#endif /* CFG_UIP_IPV6_ENABLED */
        if (type == MDNS_ADVERTISE_IPV4) {
            name_state = &service->name_ipv4;
        }

        if ((name_state->state == MDNS_SRV_STATE_ANNOUNCE) &&
        (name_state->ad_count < MDNS_ANNOUNCE_NUM)) {
            if (SAL_TIME_EXPIRED(name_state->last_ad,
                name_state->ad_interval) > 0) {
                /* send MDNS announce packet */
                service->srv_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                service->txt_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                service->ptr_rr->send_flags |= DNS_RR_SEND_FLAG_IN_ANSWER;
                service->srv_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                service->txt_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                service->ptr_rr->send_flags &= ~DNS_RR_SEND_FLAG_UNICAST;
                mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
                name_state->ad_count++;
                /* schedule next probe time */
                name_state->last_ad = sal_get_ticks();
                name_state->ad_interval =
                    SAL_USEC_TO_TICKS(MDNS_ANNOUNCE_WAIT);

            }
        }

        service = service->next;
    }

    if (mdns.send_pkt_type == MDNS_SEND_RESP_PKT) {
        mdns_packet_out();
    }
    return;
}


#ifndef __BOOTLOADER__

/*
 * Function:
 *   mdns_answer_task
 * Description:
 *   Check if any answer on the reply list is needed to be sent out
 * Parameters:
 *
 * Returns:
 */

void
mdns_answer_task(uint8 type)
{
    dns_rr_info_t       *rr;
    mdns_reply_answer_t *reply_list, *reply, *pre;

    /* Check if the bonjour is enabled */
    if (!mdns.enable) {
        return;
    }

#if CFG_UIP_IPV6_ENABLED
    if (type == MDNS_ADVERTISE_IPV6) {
        reply_list = mdns.reply_list_ipv6;
    } else
#endif /* CFG_UIP_IPV6_ENABLED */
    if (type == MDNS_ADVERTISE_IPV4) {
        reply_list = mdns.reply_list_ipv4;
    } else {
        return;
    }

    /* No answer in the reply list */
    if (reply_list == NULL) {
        return;
    }

    reply = reply_list;
    pre = reply_list;
    while (reply) {
        if (SAL_TIME_EXPIRED(reply->start, reply->delay) > 0) {
            rr = reply->asnwer_rr;
            rr->send_flags = reply->send_flags;

            mdns.send_pkt_type = MDNS_SEND_RESP_PKT;
            /* Only one packet can be sent during appcall */
            mdns_packet_out();

            /* Remove this answer from reply list */
            if (reply == reply_list) {
#if CFG_UIP_IPV6_ENABLED
                if (type == MDNS_ADVERTISE_IPV6) {
                    mdns.reply_list_ipv6 = NULL;
                } else
#endif /* CFG_UIP_IPV6_ENABLED */
                {
                    mdns.reply_list_ipv4 = NULL;
                }
                reply_list = NULL;
            } else {
                pre->next = reply->next;
                sal_free(reply);
            }
            return;
        }
        pre = reply;
        reply = reply->next;
    }
    return;
}


void
mdns_bonjour_enable_set(BOOL enable)
{
    dns_service_info_t  *srv;
    sys_error_t     rv;

    if (enable != mdns.enable) {
        if (enable) {
            /* restart advertise */
            mdns.name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
            mdns.name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED */
            srv = mdns.srv_list;
            while (srv) {
                srv->name_ipv4.state = MDNS_SRV_STATE_INIT;
#if CFG_UIP_IPV6_ENABLED
                srv->name_ipv6.state = MDNS_SRV_STATE_INIT;
#endif /* CFG_UIP_IPV6_ENABLED*/
                srv = srv->next;
            }
        } else {
            if (mdns.name_ipv4.state == MDNS_SRV_STATE_ANNOUNCE) {

                mdns.name_ipv4.state = MDNS_SRV_STATE_GOODBYE;
            } else {
                mdns.name_ipv4.state = MDNS_SRV_STATE_DISABLED;
            }
#if CFG_UIP_IPV6_ENABLED
            if (mdns.name_ipv6.state == MDNS_SRV_STATE_ANNOUNCE) {

                mdns.name_ipv6.state = MDNS_SRV_STATE_GOODBYE;
            } else {
                mdns.name_ipv6.state = MDNS_SRV_STATE_DISABLED;
            }
#endif /* CFG_UIP_IPV6_ENABLED */
            srv = mdns.srv_list;
            while (srv) {
                if (srv->name_ipv4.state == MDNS_SRV_STATE_ANNOUNCE) {
                    srv->name_ipv4.state = MDNS_SRV_STATE_GOODBYE;
                } else {
                    srv->name_ipv4.state = MDNS_SRV_STATE_DISABLED;
                }
#if CFG_UIP_IPV6_ENABLED
                if (srv->name_ipv6.state == MDNS_SRV_STATE_ANNOUNCE) {
                    srv->name_ipv6.state = MDNS_SRV_STATE_GOODBYE;
                } else {
                    srv->name_ipv6.state = MDNS_SRV_STATE_DISABLED;
                }
#endif /* CFG_UIP_IPV6_ENABLED */
                srv = srv->next;
            }
        }

        rv = board_mdns_enable_set(enable);
        if (rv) {
#if CFG_CONSOLE_ENABLED
            sal_printf("Failed to enable MDNS packets to CPU.\n");
#endif /* CFG_CONSOLE_ENABLED */
        }

        mdns.enable = enable;
    }
}

void
mdns_bonjour_enable_get(BOOL *enable)
{
    *enable = mdns.enable;
    return;
}


int32
mdns_serializer(SERIALIZE_OP op, SERL_MEDIUM_BIN *medium) REENTRANT
{
    BOOL    enable;

    /*
     * NOTE: Update the version number if serializer code changes.
     */
    if (op == SERIALIZE_OP_VERSION) {
        return MAKE_SERIALIZER_VERSION(1);
    }

    if (op == SERIALIZE_OP_LOAD) {

        medium->read(&enable, MAX_SERIAL_BONJOUR_LEN);
        mdns_bonjour_enable_set(enable);

    } else if (op == SERIALIZE_OP_SAVE) {

        mdns_bonjour_enable_get(&enable);
        medium->write(&enable, MAX_SERIAL_BONJOUR_LEN);

    } else if (op == SERIALIZE_OP_LOAD_DEFAULTS) {

        enable = TRUE;
        mdns_bonjour_enable_set(enable);
        return 0;
    }

    return MAX_SERIAL_BONJOUR_LEN;
}
#endif /* __BOOTLOADER__ */


#endif /* CFG_ZEROCONF_MDNS_INCLUDED */



