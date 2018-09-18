/*
 * $Id: soc.h,v 1.34 Broadcom SDK $
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

#ifndef _SOC_H_
#define _SOC_H_


/* Chip type */
typedef enum {
    SOC_TYPE_SWITCH_ROBO,
    SOC_TYPE_SWITCH_XGS
} soc_chip_type_t;

/* Convenient macros */
#define SOC_IF_ERROR_RETURN(op) \
    do { sys_error_t __rv__; if ((__rv__ = (op)) != SYS_OK) return(__rv__); } while(0)

/* RX handler */
#define SOC_RX_FLAG_TIMESTAMP       (1 << 0)
#define SOC_RX_FLAG_TRAFFIC_CLASS   (1 << 1)
#define SOC_RX_FLAG_TRUNCATED       (1 << 4)
#define SOC_RX_FLAG_ERROR_CRC       (1 << 5)
#define SOC_RX_FLAG_ERROR_OTHER     (1 << 6)
typedef struct soc_rx_packet_s {
    /* Filled by upper caller */
    uint8   *buffer;
    uint16  buflen;

    /* Filled by soc RX engine */
    uint16  flags;
    uint16  pktlen;
    uint8   unit;
    uint8   lport;
    uint16  traffic_class;
    uint32  timestamp;
    uint32  reserved1;
    uint32  reserved2;

    /* For chaining */
    struct soc_rx_packet_s *next;
} soc_rx_packet_t;
typedef void (*SOC_RX_HANDLER)(soc_rx_packet_t *) REENTRANT;

/* TX callback */
#define SOC_TX_FLAG_TIMESTAMP_REQUEST               (1 << 0)
#define SOC_TX_FLAG_USE_UNTAG_PORT_BITMAP           (1 << 1)
#define SOC_TX_TAG_MODE_FOLLOW_SWITCH_RULES         (0)
#define SOC_TX_TAG_MODE_UNTAG_ALL                   (1)
#define SOC_TX_TAG_MODE_TAG_ALL                     (2)
struct soc_tx_packet_s;
typedef void (*SOC_TX_CALLBACK)(struct soc_tx_packet_s *pkt) REENTRANT;
typedef struct soc_tx_packet_s {

    /* Filled by caller */
    uint8 * buffer;
    uint16  pktlen;
    uint16  flags;
    uint16  traffic_class;
    pbmp_t  port_bitmap;    /* Follow switch ARL if empty */
    pbmp_t  untag_bitmap;   /* Valid only if FOLLOW_VLAN_UNTAG_RULES not set */
    uint8   tag_mode;       /* Valid only if port_bitmap is empty  */

    /* Reserved */
    uint32  reserved1;
    uint32  reserved2;
    uint32  reserved3;
    uint32  reserved4;

    /* Filled by switch driver */
    uint8 unit;
    sys_error_t status;

    /* Used by caller */
    SOC_TX_CALLBACK callback;   /* Called after packet sent; Must set */
    void *cookie;

    /* Used interally in SOC layer */
    uint32 internal0;

    /* For chaining */
    struct soc_tx_packet_s *next;
} soc_tx_packet_t;

/* physical bitmap check *:
 *  - this macro is designed for the CPU port is different between
 *      XGS and ROBO chips.
 *  p.s. CPU port is always at bit 0 in XGS
 */
#define SOC_PBMP_PORT_SET(_type, _pbmp, _pport)     \
        ((_pbmp) |= (0x1 << (_type == SOC_TYPE_SWITCH_XGS) ? \
                (_pport + 1) : (_pport)));
#define SOC_PBMP_PORT_CLEAR(_type, _pbmp, _pport)     \
        ((_pbmp) &= ~(0x1 << (_type == SOC_TYPE_SWITCH_XGS) ? \
                (_pport + 1) : (_pport)));
#define SOC_PBMP_PORT_CHECK(_type, _pbmp, _pport)   \
        ((_pbmp) && (0x1 << (_type == SOC_TYPE_SWITCH_XGS) ? \
                (_pport + 1) : (_pport)))

#ifdef CFG_SWITCH_VLAN_INCLUDED
typedef enum vlan_type_s {
    VT_NONE,
    VT_PORT_BASED,
    VT_DOT1Q,

    VT_COUNT
} vlan_type_t;

#endif  /* CFG_SWITCH_VLAN_INCLUDED */
/* Those SOC VLAN types are defined based on ROBO's chip spec.
 *  Any new chip feature on support new VLAN type can be expended.
 */
typedef enum soc_vlan_type_s {
    SOC_VT_PORT_BASED,
    SOC_VT_DOT1Q_BASED,
    SOC_VT_MAC_BASED,
    SOC_VT_PROTOCOL_BASED,

    SOC_VT_COUNT
} soc_vlan_type_t;

#define SOC_MIN_VLAN_ID     0
#define SOC_MAX_VLAN_ID     4095
#define SOC_MIN_1QVLAN_ID   1
#define SOC_MAX_1QVLAN_ID   4094

#define SOC_VLAN_ID_IS_VALID(_vid)  \
        ((_vid) >= SOC_MIN_VLAN_ID && (_vid) <= SOC_MAX_VLAN_ID)
#define SOC_1QVLAN_ID_IS_VALID(_vid)  \
        ((_vid) >= SOC_MIN_1QVLAN_ID && (_vid) <= SOC_MAX_1QVLAN_ID)

#ifdef CFG_SWITCH_DOS_INCLUDED

/* if new chip add and new DOS type is supported, extend below definitions
 * to the higher bit(or bits) and the SOC_DOS_ALL_COUNT need be increased for
 * including those new DOS type(or types).
 */
#define SOC_DOS_ALL_COUNT                    13

#define SOC_DOS_ICMPV6_LONG_PING_DROP       ((uint16)(1U << 12))
#define SOC_DOS_ICMPV4_LONG_PING_DROP       ((uint16)(1U << 11))
#define SOC_DOS_ICMPV6_FRAGMENT_DROP        ((uint16)(1U << 10))
#define SOC_DOS_ICMPV4_FRAGMENT_DROP        ((uint16)(1U << 9))
#define SOC_DOS_TCP_FRAG_ERR_DROP           ((uint16)(1U << 8))
#define SOC_DOS_TCP_SHORT_HDR_DROP          ((uint16)(1U << 7))
#define SOC_DOS_TCP_SYN_ERR_DROP            ((uint16)(1U << 6))
#define SOC_DOS_TCP_SYNFIN_SCAN_DROP        ((uint16)(1U << 5))
#define SOC_DOS_TCP_XMASS_SCAN_DROP         ((uint16)(1U << 4))
#define SOC_DOS_TCP_NULL_SCAN_DROP          ((uint16)(1U << 3))
#define SOC_DOS_UDP_BLAT_DROP               ((uint16)(1U << 2))
#define SOC_DOS_TCP_BLAT_DROP               ((uint16)(1U << 1))
#define SOC_DOS_IP_LAND_DROP                ((uint16)(1U << 0))

#define SOC_DOS_DISABLED                    0

/* check valid bit in 16 bits width only, return 0 means all valid */
#define SOC_DOS_VALID_CHECK(dos_list)       \
        (~((uint16)((0x1 << SOC_DOS_ALL_COUNT) - 1)) & ((uint16)(dos_list)))

#endif  /* CFG_SWITCH_DOS_INCLUDED */

#define SAL_MAC_ADDR_FROM_UINT32(mac, src) do {\
        (mac)[0] = (uint8) ((src)[1] >> 8 & 0xff); \
        (mac)[1] = (uint8) ((src)[1] & 0xff); \
        (mac)[2] = (uint8) ((src)[0] >> 24); \
        (mac)[3] = (uint8) ((src)[0] >> 16 & 0xff); \
        (mac)[4] = (uint8) ((src)[0] >> 8 & 0xff); \
        (mac)[5] = (uint8) ((src)[0] & 0xff); \
    } while (0)


/* bcm_port_info_s */
typedef struct bcm_port_info_s {
    char phy_name[256];  
    int enable; 
    int linkstatus; 
    int autoneg; 
    uint32 speed; 
    int duplex; 
    int linkscan; 
    uint32 learn; 
    int discard; 
    uint32 vlanfilter; 
    int untagged_priority; 
    int untagged_vlan; 
    int stp_state;
    int loopback;
    int interface; 
    int pause_tx;
    int pause_rx;  
    uint32 frame_max;

}bcm_port_info_t;

typedef enum bcm_stg_stp_e {
    BCM_STG_STP_DISABLE, /* Disabled. */
    BCM_STG_STP_BLOCK, /* BPDUs/no learns. */
    BCM_STG_STP_LISTEN, /* BPDUs/no learns. */
    BCM_STG_STP_LEARN, /* BPDUs/learns. */
    BCM_STG_STP_FORWARD, /* Normal operation. */
    BCM_STG_STP_COUNT
} bcm_stg_stp_t;

typedef enum bcm_port_discard_e {
    BCM_PORT_DISCARD_NONE, 
    BCM_PORT_DISCARD_ALL, 
    BCM_PORT_DISCARD_UNTAG, 
    BCM_PORT_DISCARD_TAG, 
    BCM_PORT_DISCARD_INGRESS, 
    BCM_PORT_DISCARD_EGRESS, 
    BCM_PORT_DISCARD_COUNT 
} bcm_port_discard_t;

typedef enum bcm_port_loopback_e {
    BCM_PORT_LOOPBACK_NONE = 0, 
    BCM_PORT_LOOPBACK_MAC  = 1, 
    BCM_PORT_LOOPBACK_PHY  = 2, 
    BCM_PORT_LOOPBACK_PHY_REMOTE = 3, 
    BCM_PORT_LOOPBACK_COUNT = 4 
} bcm_port_loopback_t;


/*
 *  L2 ARL related definitions
 */
#define SOC_ARL_INVALID_BIN_ID      (-1)

/*
 * SOC switch class
 */
typedef struct soc_switch_s {

    /* Chip type */
    soc_chip_type_t (*chip_type)(void) REENTRANT;

    /* Chip revision */
    sys_error_t (*chip_revision)(uint8 unit, uint8 *rev) REENTRANT;

    /* Number of ports */
    uint8 (*port_count)(uint8 unit) REENTRANT;

    /* Robo register read/write (for SOC_TYPE_SWITCH_ROBO) */
    sys_error_t (*robo_switch_reg_get)(uint8 unit,
                                       uint8 page,
                                       uint8 offset,
                                       uint8 *buf,
                                       uint8 len) REENTRANT;
    sys_error_t (*robo_switch_reg_set)(uint8 unit,
                                       uint8 page,
                                       uint8 offset,
                                       uint8 *buf,
                                       uint8 len) REENTRANT;

#if CFG_RXTX_SUPPORT_ENABLED

    /*
     * Set (the one and only) RX handler.
     * If intr is TRUE, the handler will be called in interrupt context
     */
    sys_error_t (*rx_set_handler)(uint8 unit, SOC_RX_HANDLER fn, BOOL intr) REENTRANT;


    /*
     * Fill (or refill) one packet buffer to RX engine.
     * You can fill more than one buffers until it returns SYS_ERR_FULL
     */
    sys_error_t (*rx_fill_buffer)(uint8 unit, soc_rx_packet_t *pkt) REENTRANT;

    /*
     * Packet TX
     */
    sys_error_t (*tx)(uint8 unit, soc_tx_packet_t *pkt) REENTRANT;

#endif /* CFG_RXTX_SUPPORT_ENABLED */

    /* Link status of a port */
    sys_error_t (*link_status)(uint8 unit, uint8 lport, BOOL *link) REENTRANT;

    /* XGS chip revision */
    sys_error_t (*xgs_chip_revision)(uint8 unit, uint16 *dev, uint16 *rev) REENTRANT;

    /* XGS register/memory read/write */
#ifdef CFG_SWITCH_XGS_NEW_SBUS_FORMAT_INCLUDED
    /* New sbus format(separated block id) */
    sys_error_t (*xgs_switch_reg_get)(uint8 unit,
                                      uint8 block,
                                      uint32 addr,
                                      uint32 *val) REENTRANT;
    sys_error_t (*xgs_switch_reg_set)(uint8 unit,
                                      uint8 block,
                                      uint32 addr,
                                      uint32 val) REENTRANT;
    sys_error_t (*xgs_switch_mem_get)(uint8 unit,
                                      uint8 block,
                                      uint32 addr,
                                      uint32 *buf,
                                      int len) REENTRANT;
    sys_error_t (*xgs_switch_mem_set)(uint8 unit,
                                      uint8 block,
                                      uint32 addr,
                                      uint32 *buf,
                                      int len) REENTRANT;
#else
    sys_error_t (*xgs_switch_reg_get)(uint8 unit,
                                      uint32 addr,
                                      uint32 *val) REENTRANT;
    sys_error_t (*xgs_switch_reg_set)(uint8 unit,
                                      uint32 addr,
                                      uint32 val) REENTRANT;
    sys_error_t (*xgs_switch_mem_get)(uint8 unit,
                                      uint32 addr,
                                      uint32 *buf,
                                      int len) REENTRANT;
    sys_error_t (*xgs_switch_mem_set)(uint8 unit,
                                      uint32 addr,
                                      uint32 *buf,
                                      int len) REENTRANT;
#endif /* CFG_SWITCH_XGS_NEW_SBUS_FORMAT_INCLUDED */

#ifdef CFG_SWITCH_VLAN_INCLUDED
    sys_error_t (*pvlan_egress_set)(uint8 unit,
                                       uint8 lport,
                                       pbmp_t lpbmp)   REENTRANT;

    sys_error_t (*pvlan_egress_get)(uint8 unit,
                                       uint8 lport,
                                       pbmp_t *lpbmp)   REENTRANT;
    sys_error_t (*qvlan_port_set)(uint8 unit,
                                       uint16 vlan_id,
                                       pbmp_t lpbmp,
                                       pbmp_t tag_lpbmp)   REENTRANT;

    sys_error_t (*qvlan_port_get)(uint8 unit,
                                       uint16 vlan_id,
                                       pbmp_t *lpbmp,
                                       pbmp_t *tag_lpbmp)   REENTRANT;

    sys_error_t (*vlan_create)(uint8 unit,
                                       vlan_type_t type,
                                       uint16 vlan_id)   REENTRANT;

    sys_error_t (*vlan_destroy)(uint8 unit,
                                       uint16 vlan_id)   REENTRANT;
    sys_error_t (*vlan_type_set)(uint8 unit,
                                       vlan_type_t type)   REENTRANT;
    sys_error_t (*vlan_reset)(uint8 unit)   REENTRANT;
#endif  /* CFG_SWITCH_VLAN_INCLUDED */
    sys_error_t (*phy_reg_get)(uint8 unit, uint8 lport, uint16 reg_addr, uint16 *p_value);
    sys_error_t (*phy_reg_set)(uint8 unit, uint8 lport, uint16 reg_addr, uint16 value);
    sys_error_t (*port_info_get)(uint8 unit, uint8 lport, bcm_port_info_t *info)   REENTRANT;

} soc_switch_t;

#endif /* _SOC_H_ */

