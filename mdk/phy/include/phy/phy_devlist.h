#ifdef PHY_DEVLIST_ENTRY
/*
 * PHY_DEVLIST_ENTRY MACROS
 * 
 * Before including this file, define PHY_DEVLIST_ENTRY
 * as a macro to operate on the following parameters:
 *     #define PHY_DEVLIST_ENTRY(_nm,_bd,_fl,_desc,_r0,_r1)
 * 
 *     _nm: Chip Name
 *     _bd: SW Base Driver
 *     _fl: Flags
 *     _desc: Description
 *     _r0: Reserved
 *     _r1: Reserved
 * 
 * Note that this macro will be undefined at the end of this file.
 */

#if PHY_CONFIG_INCLUDE_BCM54282 == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCM54282, bcm54282, 0, "Octal QSGMII 10/100/1000BASE-T Gigabit Ethernet Transceiver", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCM54680 == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCM54680, bcm54680, 0, "Octal-Port 10/100/1000BASE-T Gigabit Ethernet Transceiver", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCM54680E == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCM54680E, bcm54680e, 0, "Octal Gigabit Ethernet Transceiver (EEE)", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCM54880E == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCM54880E, bcm54880e, 0, "Octal Gigabit Ethernet Transceiver (EEE)", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCM56150 == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCM56150, bcm56150, PHY_DRIVER_F_INTERNAL, "Internal BCMEGPHY40 Embedded Gigabit PHY Driver", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCMI_QSGMII_SERDES == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCMI_QSGMII_SERDES, bcmi_qsgmii_serdes, PHY_DRIVER_F_INTERNAL, "Internal Octal QSGMII 1.25G SerDes PHY Driver", 0, 0)
#endif
#if PHY_CONFIG_INCLUDE_BCMI_TSC_XGXS == 1 || defined(PHY_DEVLIST_INCLUDE_ALL)
PHY_DEVLIST_ENTRY(BCMI_TSC_XGXS, bcmi_tsc_xgxs, PHY_DRIVER_F_INTERNAL, "Internal TSC 40G SerDes PHY Driver", 0, 0)
#endif
/* End PHY_DEVLIST_ENTRY Macros */

#ifdef PHY_DEVLIST_INCLUDE_ALL
#undef PHY_DEVLIST_INCLUDE_ALL
#endif
#undef PHY_DEVLIST_ENTRY
#endif /* PHY_DEVLIST_ENTRY */

