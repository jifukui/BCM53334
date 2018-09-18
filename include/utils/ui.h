/*
 * $Id: ui.h,v 1.6 Broadcom SDK $
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

#ifndef _UTILS_UI_H_
#define _UTILS_UI_H_

/* Keyboard codes */
#define UI_KB_ESC       0x1B
#define UI_KB_LF        0x0A
#define UI_KB_CR        0x0D
#define UI_KB_BS        0x08
#define UI_KB_CTRL_C    0x03
#define UI_KB_DEL       0x7F

typedef enum {
    UI_RET_OK,     /* OK with input data */
    UI_RET_EMPTY,  /* ENTER pressed w/o data (i.e., using suggested value) */
    UI_RET_CANCEL, /* Cancelled by ESC or Ctrl-C */
    UI_RET_ERROR   /* Error occurs, usually invalid parameters */
} ui_ret_t;

#if 1
typedef uint8	sal_mac_addr_t[6];		/* MAC address */


#define SAL_MAC_ADDR_FROM_UINT32(mac, src) do {\
        (mac)[0] = (uint8) ((src)[1] >> 8 & 0xff); \
        (mac)[1] = (uint8) ((src)[1] & 0xff); \
        (mac)[2] = (uint8) ((src)[0] >> 24); \
        (mac)[3] = (uint8) ((src)[0] >> 16 & 0xff); \
        (mac)[4] = (uint8) ((src)[0] >> 8 & 0xff); \
        (mac)[5] = (uint8) ((src)[0] & 0xff); \
    } while (0)
    
#define BYTES2WORDS(x)          (((x) + 3) / 4)
#define BITS2BYTES(x)        (((x) + 7) / 8)
#define BITS2WORDS(x)        (((x) + 31) / 32)
#define SOC_MAX_MEM_FIELD_BITS          384
#define _E2S(x, max, string_array) \
				((((size_t)(x) < (max))) ?	(string_array)[(x)] : "?")

extern char *discard_mode[];
extern char *forward_mode[];

#define DISCARD_MODE(x)    _E2S(x, BCM_PORT_DISCARD_COUNT, discard_mode)

#define FORWARD_MODE(x)	   _E2S(x, BCM_STG_STP_COUNT, forward_mode)



#define SOC_MAX_MEM_FIELD_WORDS         BITS2WORDS(SOC_MAX_MEM_FIELD_BITS)


#define soc_mem_entry_bytes(mem) \
			mem_arr[mem]->bytes

#define soc_mem_entry_words(mem) \
		BYTES2WORDS(soc_mem_entry_bytes(mem))

#define soc_reg_entry_bytes(reg) \
				reg_arr[reg]->bytes
	
#define soc_reg_entry_words(reg) \
			BYTES2WORDS(soc_reg_entry_bytes(reg))


    
  #define soc_mem_table_idx_to_pointer(mem, cast, table, index) \
    ((cast)(&(((uint32 *)(table))[soc_mem_entry_words(mem) * (index)]))
  


typedef int soc_field_t;

typedef struct soc_field_info_s {
	soc_field_t	field;
	uint16		len;	/* Bits in field */
	uint16		bp;	/* Least bit position of the field */
	uint16  	flags;	/* Logical OR of SOCF_* */
} soc_field_info_t;

typedef struct soc_mem_info_s {
    char    *name;
    uint32  flags;
    int     index_min;  /* Minimum table index */
    int     index_max;  /* Maximum table index */ 
    uint32  block_id;   /* valid blocks id*/
    uint32  base;       /* Includes region offset */
    uint32  gran;       /* Phys addr granularity by index */
    uint16  bytes;
    uint16  nFields;
    soc_field_info_t    *fields;
    char                **views;    
} soc_mem_info_t;


#define SOCF_LE         0x01    /* little endian */
#define SOCF_RO         0x02    /* read only */
#define SOCF_WO         0x04    /* write only */
#define SOCF_SC         0x08
#define SOCF_RES        0x10    /* reserved (do not test, etc.) */


#define SOC_REG_FLAG_64_BITS (1<<0)     /* Register is 64 bits wide */




typedef uint32  soc_block_t;

typedef struct soc_reg_info_s {
    char    *name;

    uint32  base;
    uint32  block_id;
    uint32  flags;
    int     nFields;
    soc_field_info_t  *fields;
    char   **views;
    uint16 bytes;
}soc_reg_info_t;

#define SOCF_LE		    0x01  
#define SOCF_RES        0x10
#define SOCF_GLOBAL     0x20


#define SOC_MEM_FLAG_VALID   0x00000002

 
typedef enum {
    L2X_M,
    PORT_TAB_M,
    VLAN_TAB_M,
    EGR_VLAN_TAB_M,
    VLAN_STG_TAB_M,
    EGR_VLAN_STG_TAB_M,    
    VLAN_PROFILE_TAB_M,
    VLAN_PROFILE_2_TAB_M,
    L2_MC_TAB_M, 
    MEM_MAX_COUNT,
} mem_tab_t;

typedef enum {
    REG_ING_CONFIG_64_RER,
    IGMP_MLD_PKT_CONTROL_REG,    
    L2_AGE_TIMER,
    PROTOCOL_PKT_CONTRO,    
    UNKNOWN_MCAST_BLOCK_MASK_64,
    IUNKNOWN_MCAST_BLOCK_MASK_64,
    REG_MAX_COUNT,
} reg_tab_t;


typedef enum {
    ASSOCIATED_DATAf,
    CLASS_IDf,
    CPUf,
    DESTINATIONf,
    DESTINATION_1f,
    DEST_TYPEf,
    DST_DISCARDf,
    DUMMY_INDEXf,
    L2X_EVEN_PARITYf,
    HITDAf,
    HITSAf,   
    KEY_TYPEf,    
    L2MC_PTRf,
    L3f,
    LIMIT_COUNTEDf,
    LOCAL_SAf,
    MAC_ADDRf,
    MAC_BLOCK_INDEXf,
    L2X_MIRRORf,
    L2X_MIRROR0f,
    MODULE_IDf,    
    L2X_OVIDf,
    PENDINGf,
    PORT_NUMf,
    PRIf,
    REMOTEf,
    REMOTE_TRUNKf,
    RPEf,
    SCPf,
    SRC_DISCARDf,
    STATIC_BITf,
    Tf,
    TGIDf,
    L2X_VALIDf, 
    VPGf,
    L2X_FIELD_MAX,
} soc_mem_l2x_m_t;

typedef enum {   
    ALLOW_SRC_MODf,
    CFI_0_MAPPINGf,
    CFI_1_MAPPINGf,
    PORT_TAB_CFI_AS_CNGf,
    CLASS_BASED_SM_ENABLEf,
    CML_FLAGS_MOVEf,
    CML_FLAGS_NEWf,
    CTRL_PROFILE_INDEX_1588f,
    DATA_0f,
    DATA_1f,
    DISABLE_STATIC_MOVE_DROPf,
    DROP_BPDUf,
    DUAL_MODID_ENABLEf,
    EN_IFILTERf,
    FILTER_ENABLEf,
    FP_PORT_SELECT_TYPEf,
    HIGIG2f,
    HIGIG_TRUNKf,
    HIGIG_TRUNK_IDf,
    IEEE_802_1AS_ENABLEf,
    IGNORE_IPMC_L2_BITMAPf,
    IGNORE_IPMC_L3_BITMAPf,
    INNER_TPID_ENABLEf,
    IPMC_DO_VLANf,
    IVIDf,
    MAC_BASED_VID_ENABLEf,
    MAC_IP_BIND_LOOKUP_MISS_DROPf,
    MAP_TAG_PKT_PRIORITYf,
    MDL_BITMAPf,
    MH_INGRESS_TAGGED_SELf,
    PORT_MIRRORf,
    PORT_MIRROR0f,
    MY_MODIDf,
    OAM_ENABLEf,
    OUTER_TPID_ENABLEf,
    OUTER_TPID_VERIFYf,
    PORT_OVIDf,
    PASS_CONTROL_FRAMESf,
    PORT_BRIDGEf,
    PORT_DIS_TAGf,
    PORT_DIS_UNTAGf,
    PORT_PRIf,
    PORT_TYPEf,
    PORT_VIDf,
    PRI_MAPPINGf,
    PROTOCOL_PKT_INDEXf,
    PVLAN_ENABLEf,
    REMOTE_CPU_ENf,
    REMOVE_HG_HDR_SRC_PORTf,
    PORT_TAB_RESERVED_0f,
    PORT_TAB_RESERVED_1f,
    PORT_TAB_RESERVED_2f,
    SUBNET_BASED_VID_ENABLEf,
    TAG_ACTION_PROFILE_PTRf,
    TRUST_DOT1P_PTRf,
    TRUST_DSCP_V4f,
    TRUST_DSCP_V6f,
    TRUST_INCOMING_VIDf,
    TRUST_OUTER_DOT1Pf,
    USE_INCOMING_DOT1Pf,
    USE_INNER_PRIf,
    USE_IVID_AS_OVIDf,
    USE_PORT_TABLE_GROUP_IDf,
    V4IPMC_ENABLEf,
    V4IPMC_L2_ENABLEf,
    V4L3_ENABLEf,
    V6IPMC_ENABLEf,
    V6IPMC_L2_ENABLEf,
    V6L3_ENABLEf,
    VFP_ENABLEf,
    VFP_PORT_GROUP_IDf,
    VLAN_PRECEDENCEf,
    VLAN_PROTOCOL_DATA_INDEXf,
    VT_ENABLEf,
    VT_KEY_TYPEf,
    VT_KEY_TYPE_2f,
    VT_KEY_TYPE_2_USE_GLPf,
    VT_KEY_TYPE_USE_GLPf,
    VT_MISS_DROPf, 
    PORT_TAB_FIELD_MAX,
} soc_mem_port_tab_m_t;

typedef enum {
	VIRTUAL_PORT_ENf,
    BC_IDXf,
    VLAN_EVEN_PARITYf,
    FID_IDf,
    HIGIG_TRUNK_OVERRIDEf,
    L2_ENTRY_KEY_TYPEf,
    L3_IIFf,
    VLAN_PORT_BITMAPf,
    VLAN_PORT_BITMAP_LOf,
    RESERVED0f,
    RESERVED1f,
    VLAN_STGf,
    UMC_IDXf,
    UUC_IDXf,
    VLAN_VALIDf,
    VLAN_CLASS_IDf,
    VLAN_PROFILE_PTRf,
} soc_mem_vlan_tab_m_t;

typedef enum {   
    DOT1P_MAPPING_PTRf, 
    EGR_VLAN_EVEN_PARITYf, 
    OUTER_TPID_INDEXf, 
    EGR_VLAN_PORT_BITMAPf, 
    EGR_VLAN_PORT_BITMAP_HIf, 
    EGR_VLAN_PORT_BITMAP_LOf, 
    REMARK_DOT1Pf, 
    EGR_VLAN_STGf, 
    EGR_VLAN_UT_BITMAPf, 
    EGR_VLAN_UT_BITMAP_HIf, 
    EGR_VLAN_UT_BITMAP_LOf, 
    UT_PORT_BITMAPf, 
    UT_PORT_BITMAP_HIf, 
    UT_PORT_BITMAP_LOf, 
    EGR_VLAN_VALIDf,
} soc_mem_egr_vlan_tab_m_t;

typedef enum {    
    SP_TREE_PORT0f,    
    SP_TREE_PORT1f,
    SP_TREE_PORT2f,
    SP_TREE_PORT3f,
    SP_TREE_PORT4f,
    SP_TREE_PORT5f,
    SP_TREE_PORT6f,
    SP_TREE_PORT7f,
    SP_TREE_PORT8f,
    SP_TREE_PORT9f,
    SP_TREE_PORT10f,
    SP_TREE_PORT11f,
    SP_TREE_PORT12f,
    SP_TREE_PORT13f,
    SP_TREE_PORT14f,
    SP_TREE_PORT15f,
    SP_TREE_PORT16f,
    SP_TREE_PORT17f,
    SP_TREE_PORT18f,
    SP_TREE_PORT19f,    
    SP_TREE_PORT20f,
    SP_TREE_PORT21f,
    SP_TREE_PORT22f,
    SP_TREE_PORT23f,
    SP_TREE_PORT24f,
    SP_TREE_PORT25f,
    SP_TREE_PORT26f,
    SP_TREE_PORT27f,
    SP_TREE_PORT28f,
    SP_TREE_PORT29f,
    SP_EVEN_PARITYf,
} soc_mem_stg_m_t;

    
typedef enum {
     ICMP_REDIRECT_TOCPUf,
     IPMCV4_ENABLEf,
     IPMCV4_L2_ENABLEf,
     IPMCV6_ENABLEf,
     IPMCV6_L2_ENABLEf,
     IPV4L3_ENABLEf,
     IPV6L3_ENABLEf,
     IPV6_ROUTING_HEADER_TYPE_0_DROPf,
     L2_MISS_DROPf,
     L2_MISS_TOCPUf,
     L2_NON_UCAST_DROPf,
     L2_NON_UCAST_TOCPUf,
     L2_PFMf,
     L3_IPV4_PFMf,
     L3_IPV6_PFMf,
     LEARN_DISABLEf,
     VLAN_PROFILE_OUTER_TPID_INDEXf,
     VLAN_PROFILE_RESERVED_0f,
     VLAN_PROFILE_RESERVED_1f,
     VLAN_PROFILE_RESERVED_2f,
     VLAN_PROFILE_RESERVED_3f,
     UNKNOWN_IPV4_MC_TOCPUf,
     UNKNOWN_IPV6_MC_TOCPUf,
} soc_mem_vlan_profile_t;

typedef enum {
    BCAST_MASK_SELf,
    BLOCK_MASK_Af,
    BLOCK_MASK_Bf,
    KNOWN_MCAST_MASK_SELf,
    UNKNOWN_MCAST_MASK_SELf,
    UNKNOWN_UCAST_MASK_SELf,
} soc_mem_vlan_profile_2_t;

typedef enum {   
   L2_MC_EVEN_PARITYf,
   L2_MC_HIGIG_TRUNK_OVERRIDEf,
   L2_MC_PORT_BITMAPf,
   L2_MC_PORT_BITMAP_LOf,
   L2_MC_RESERVED0f,
   L2_MC_VALIDf,
} soc_mem_l2_mc_tab_m_t;

typedef enum {
    IPMC_EVEN_PARITYf,
    IPMC_HIGIG_TRUNK_OVERRIDEf,
    IPMC_L2_BITMAPf,
    IPMC_L2_BITMAP_LOf,
    IPMC_L3_BITMAPf,
    IPMC_L3_BITMAP_LOf,
    IPMC_MODULE_IDf,
    IPMC_PORT_NUMf,
    IPMC_RESERVED0f,
    IPMC_Tf, 
    IPMC_TGIDf, 
    IPMC_VALIDf,
} soc_mem_ip_mc_tab_m_t;


typedef enum {
    TBYT,
    RBYT,
    TPKT,
    RPKT,
    RFCS,
    RUC,
    TUC,
    RMCA,
    TMCA,
    RBCA,
    TBCA,
    RXPF,
    TXPF,
    ROVR,
    TOVR,
    EgressDrop,
    R_MAX,    
} reg_port_status_t;

extern soc_mem_info_t *mem_arr[];
extern uint32
soc_memacc_field32_get(soc_field_info_t *fieldinfo, void *entry);
extern void soc_mem_dump(int tab_index, int change);
extern void soc_reg_dump(uint32 index, uint32 id);
extern void soc_port_status_dump();

#define  BITALLOCSIZE(_max) ((((_max) + 31) / 32) * sizeof (uint32))
#define  BITOP(a, b, op)    \
		 (((a)[(b) / 32]) op (1U << ((b) % 32)))


 typedef struct resource_bmp_s {
    uint32  *w;	
 } resource_bmp_t;
 
 typedef struct id_resource_s {
    resource_bmp_t resource_bmp;
    uint32             total_id;
 } id_resource_t;

extern int resource_alloc(id_resource_t resource, uint32 *id);
extern int resource_free(id_resource_t resource, int id);



/* Specific operations */
#define    BITGET(_a, _b)    BITOP(_a, _b, &)
#define    BITSET(_a, _b)    BITOP(_a, _b, |=)
#define    BITCLR(_a, _b)    BITOP(_a, _b, &= ~)
#define    BITWRITE(_a, _b, _val)    ((_val) ? BITSET(_a, _b) : BITCLR(_a, _b))
#define    BIT_ITER(_a, _max, _b)            \
           for ((_b) = 0; (_b) < (_max); (_b)++) \
               if ((_a)[(_b) / 32] == 0) \
                   (_b) += 31;     \
               else if (BITGET((_a), (_b)))
 

 #define RESOURCE_ID_GET(a, b)		\
    do {		\
        int rv;		\
        rv = resource_alloc(a, &b);		\
        if (rv != SYS_OK) {		\
            return  rv;		\
        }			\
    } while (0)

#define RESOURCE_ID_FREE(a, b)		\
    do {		\
        int rv;		\
        rv = resource_free(a, b);		\
        if (rv != SYS_OK) {		\
            return  rv;		\
        }			\
    } while (0)

#define RESOURCE_BMP_ADD(bmp, mtr)     BITSET(((bmp).w), (mtr))
#define RESOURCE_BMP_REMOVE(bmp, mtr)  BITCLR(((bmp).w), (mtr))
#define RESOURCE_BMP_TEST(bmp, mtr)    BITGET(((bmp).w), (mtr))

extern int igmp_resource_init(void);
extern int l3_ipmc_id_get(uint16 *id);
extern int l3_ipmc_id_free(     uint16 id);

#endif
/*  platform dependent C library IO function */
extern char put_char(char c);
extern char get_char(void);


/* Input hex number: byte (8bit), word (16bit), dword (32bit) with prompt */
extern ui_ret_t ui_get_byte(uint8 *val, const char *str) REENTRANT;
extern ui_ret_t ui_get_word(uint16 *val, const char *str) REENTRANT;
extern ui_ret_t ui_get_dword(uint32 *val, const char *str) REENTRANT;

/* Input address in hex */
extern ui_ret_t ui_get_address(uint8 **paddr, const char *str) REENTRANT;

/* Input a uint32 in decimal (str must not be NULL or "") */
extern ui_ret_t ui_get_decimal(uint32 *pvalue, const char *str) REENTRANT;

/* Dump memory as bytes in hex (can be cancelled using ESC or Ctrl-C) */
extern void ui_dump_memory(uint8 *addr, uint16 len) REENTRANT;

/* Backspace (go back and delete one character) */
extern void ui_backspace(void) REENTRANT;

/* Input heximal number */
extern ui_ret_t ui_get_hex(uint32 *value, uint8 size) REENTRANT;

/* Input a fixed number of bytes. 
 * Set show_org to TRUE to show original data as default.
 * Note: Even if user cancels, previous inputed bytes are still updated.
 */
extern ui_ret_t ui_get_bytes(uint8 *pbytes, 
                             uint8 len, 
                             const char *str, 
                             BOOL show_org) REENTRANT;

/* Input yes or no. 
   Suggested is the default answer when pressing ENTER:
     0: default No
     1: default Yes
     2: No default answer (must enter 'y' or 'n') 
   Return value: TRUE - yes, FALSE - no
 */
extern BOOL ui_yes_or_no(const char *str, uint8 suggested) REENTRANT;

/* Input a string with maximum number of bytes. */
extern ui_ret_t ui_get_string(char *pbytes, uint8 len, const char *str) REENTRANT;

/* Input a secure string with maximum number of bytes. */
extern ui_ret_t ui_get_secure_string(char *pbytes, uint8 len, const char *str) REENTRANT;

#endif /* _UTILS_UI_H_ */

