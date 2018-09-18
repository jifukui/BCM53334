

#include <system.h>


void config_check(void) {
	
/* Checking for config dependence */
#if CFG_UIP_STACK_ENABLED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for UIP STACK
#endif
#endif

#ifdef CFG_SWITCH_SNAKETEST_INCLUDED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for CFG_SWITCH_SNAKETEST_INCLUDED
#endif
#if (!defined(CFG_SWITCH_VLAN_INCLUDED) && (!defined(CFG_SOC_SNAKE_TEST)))
#error Need to define CFG_SWITCH_VLAN_INCLUDED for CFG_SWITCH_SNAKETEST_INCLUDED
#endif
#endif

#ifdef CFG_SWITCH_MCAST_INCLUDED
#if !CFG_RXTX_SUPPORT_ENABLED
#error Need to set CFG_RXTX_SUPPORT_ENABLED to 1 for CFG_SWITCH_MCAST_INCLUDED
#endif
if (SAL_IS_UMWEB() || SAL_IS_RAMAPP()) {
#if !defined(CFG_SWITCH_VLAN_INCLUDED)
#error Need to define CFG_SWITCH_VLAN_INCLUDED for CFG_SWITCH_MCAST_INCLUDED
#endif
}
#endif

#if CFG_PERSISTENCE_SUPPORT_ENABLED
#if !CFG_FLASH_SUPPORT_ENABLED
#error Need to set CFG_FLASH_SUPPORT_ENABLED to 1 for CFG_PERSISTENCE_SUPPORT_ENABLED
#endif
#endif
    
#if CFG_CLI_ENABLED
#if !CFG_CONSOLE_ENABLED
#error Need to set CFG_CONSOLE_ENABLED to 1 for CLI
#endif
#endif
    
    
#if CFG_CLI_FLASH_CMD_ENABLED
#if !CFG_FLASH_SUPPORT_ENABLED
#error Need to set CFG_FLASH_SUPPORT_ENABLED to 1 for FLASH CLI
#endif
#endif

}

