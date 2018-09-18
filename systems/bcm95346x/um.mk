# $Id: um.mk,v 1.13 Broadcom SDK $
# $Copyright: Copyright 2016 Broadcom Corporation.
# This program is the proprietary software of Broadcom Corporation
# and/or its licensors, and may only be used, duplicated, modified
# or distributed pursuant to the terms and conditions of a separate,
# written license agreement executed between you and Broadcom
# (an "Authorized License").  Except as set forth in an Authorized
# License, Broadcom grants no license (express or implied), right
# to use, or waiver of any kind with respect to the Software, and
# Broadcom expressly reserves all rights in and to the Software
# and all intellectual property rights therein.  IF YOU HAVE
# NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE
# IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE
# ALL USE OF THE SOFTWARE.  
#  
# Except as expressly set forth in the Authorized License,
#  
# 1.     This program, including its structure, sequence and organization,
# constitutes the valuable trade secrets of Broadcom, and you shall use
# all reasonable efforts to protect the confidentiality thereof,
# and to use this information only in connection with your use of
# Broadcom integrated circuit products.
#  
# 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS
# PROVIDED "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
# REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY,
# OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
# DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
# NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
# ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
# CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
# OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
# 
# 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
# BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL,
# INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER
# ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY
# TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF
# THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR USD 1.00,
# WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
# ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.$

#
# UM's version number
#

include ${TOP}/systems/bcm95346x/um_version.mk

#
# Default values for certain parameters
#

CFG_MLONG64 ?= 0
CFG_RELOC ?= 0
CFG_UNCACHED ?= 0
CFG_BOOTRAM ?= 0
CFG_PCI ?= 0
CFG_LDT ?= 0
CFG_LDT_REV_017 ?= 0
CFG_PCIDEVICE ?= 0
CFG_DOWNLOAD ?= 0
CFG_USB ?= 0
CFG_MSYS ?= 0
CFG_ZLIB ?= 0
CFG_VGACONSOLE ?= 0
CFG_BIENDIAN ?= 0
CFG_RAMAPP ?= 0
CFG_USB ?= 0
CFG_ZIPSTART ?= 0
CFG_ZIPPED_CFE ?= 0
CFG_RELEASE_STAGE ?= 0
CFG_COMPRESSED_IMAGE ?= 0
CFG_UART1 ?= 0

#
# Override some settings based on the value of CFG_RELOC.
#
# 'STATIC' relocation means no biendian, no bootram, ZIPstart
# '1' is SVR4 Relocation,  no RAMAPP, no BOOTRAM
# '0' (no relocation) : no changes
#

ifeq ($(strip ${CFG_RELOC}),1)
  CFG_RAMAPP = 0
  CFG_BOOTRAM = 0
endif

ifeq ($(strip ${CFG_RELOC}),STATIC)
  CFG_RAMAPP = 0
  CFG_BIENDIAN = 0
  CFG_BOOTRAM = 0
  CFG_ZIPSTART = 1
  CFE_CFLAGS += -DCFG_ZIPSTART=1
endif

#
# Default goal.
#

all : ALL

#
# Paths to other parts of the firmware.  Everything's relative to ${TOP}
# so that you can actually do a build anywhere you want.
#

BSP_SRC    = ${TOP}/systems/bcm95346x/src
BSP_INC    = ${TOP}/systems/bcm95346x/include
LIB_PATH   = ${BUILD_DIR}/lib
MAIN_SRC   = ${TOP}/src/kernel
MAIN_INC   = ${TOP}/include
SOC_SRC    = ${TOP}/src/driver/soc/bcm5346x
UIP_SRC    = ${TOP}/src/net
FLASH_SRC  = ${TOP}/src/driver/flash
XCMD_INC   = ${TOP}/src/appl/xcommands
SAL_CONFIG_H = ${TOP}/include/arm/sal_config.h
#
# Preprocessor defines for CFE's version number
#

VDEF = -DCFE_VER_MAJOR=${CFE_VER_MAJ} -DCFE_VER_MINOR=${CFE_VER_MIN} -DCFE_VER_BUILD=${CFE_VER_ECO}
VDEF += -DCFE_VER_SDK=${CFE_VER_SDK}

#ifneq ("$(strip ${CFE_VER_SDK})","")
#  VDEF += -DCFE_VER_SDK=${CFE_VER_SDK}
#endif

ifeq ($(strip ${CFG_UART1}),1)
CFE_CFLAGS += -DCFG_UART1=1
endif

ifdef CFG_SERIAL_BAUD_RATE_OVERRIDE
  CFE_CFLAGS += -DCFG_SERIAL_BAUD_RATE=${CFG_SERIAL_BAUD_RATE_OVERRIDE}
endif

#
# Construct the list of paths that will eventually become the include
# paths and VPATH.
#

SRCDIRS = ${BSP_SRC} ${MAIN_SRC} ${TOP}/include

# ${TOP}/net ${TOP}/dev ${TOP}/ui ${TOP}/lib ${TOP}/httpd ${TOP}/httpd/callbacks

CFE_INC = ${TOP}/include ${TOP}/include/${ARCH} ${TOP}/src/net ${TOP}/src/appl/web/content ${XCMD_INC}

#
# Configure tools and basic tools flags.  This include sets up
# macros for calling the C compiler, basic flags,
# and linker scripts.
#

include ${BUILD_DIR}/src/tools.mk

#
# Add some common flags that are used on any architecture.
#

CFLAGS += -I. $(INCDIRS)
CFLAGS += -D_CFE_ ${VDEF} -DCFG_BOARDNAME=\"${CFG_BOARDNAME}\" 

ifeq ($(strip ${CFG_QT}), 1)
CFLAGS += -D__EMULATION__
endif
#
# Gross - allow more options to be supplied from command line
#

ifdef CFG_OPTIONS
OPTFLAGS = $(patsubst %,-D%,$(subst :, ,$(CFG_OPTIONS)))
CFLAGS += ${OPTFLAGS}
endif


#
# Include the makefiles for the architecture-common, cpu-specific,
# and board-specific directories.  Each of these will supply
# some files to "ALLOBJS".  The BOARD and CHIPSET directories are optional
# as some ports are so simple they don't need boad-specific stuff.
#

# include ${ARCH_SRC}/Makefile
# include ${CPU_SRC}/Makefile

# ifneq ("$(strip ${BOARD})","")
# include ${BOARD_SRC}/Makefile
# endif

include ${TOP}/systems/bcm95346x/src/Makefile

#
# Pull in the common directories
#

# include ${MAIN_SRC}/Makefile
# include ${TOP}/lib/Makefile

KERNELOBJS = background.o link.o main.o rx.o timer.o tx.o

SALOBJS = sal_alloc.o sal_chksum.o sal_console.o sal_init.o sal_libc.o\
          sal_printf.o sal_timer.o sal_config.o

SRCDIRS += ${TOP}/src/sal/${ARCH}

UIPOBJS = uip.o uip6.o uip_arp.o uip-ds6.o uip-nd6.o uip-icmp6.o uip_arch.o\
          uip_task.o

SRCDIRS += ${TOP}/src/net

APPLOBJS = app_init.o dhcpc.o igmpsnoop.o igmpsnoop_cbk.o mdns.o mdns_utils.o
ifndef CFG_SOC_SNAKE_TEST
APPLOBJS += snaketest.o
endif
SRCDIRS += ${TOP}/src/appl ${TOP}/src/appl/dhcpc ${TOP}/src/appl/snaketest
SRCDIRS += ${TOP}/src/appl/igmpsnoop ${TOP}/src/appl/net 
SRCDIRS += ${TOP}/src/appl/mdns


#
# XCOMANND core file include  
#

XCMDOBJS = xc_input_buffer.o xc_input_cli.o xc_output_buf.o xcmd_cli.o xcmd_core.o xcmd_auth.o xcmd_display_page.o
SRCDIRS += ${TOP}/src/appl/xcmd

#
# XCOMANND XML Tables generator : please use make xcommand to generate callback function  
#

XCOMMANDS_XMLDIR = ${TOP}/src/appl/xcommands
XCOMMANDS_PARSER = ${TOP}/tools/xcommands/parse_context.pl
XCOMMAND_XML_TABLES := \
    $(XCOMMANDS_XMLDIR)/global.xml
    
XCOMMAND_DEF_TABLES := $(XCOMMANDS_XMLDIR)/defines.xml    
    
XCOMMANDSDIRS = ${TOP}/src/appl/xcommands/callback ${TOP}/src/appl/xcommands/generated

SRCDIRS += ${XCOMMANDSDIRS}
XCOMMANDSOBJS = $(patsubst %.c,%.o,$(notdir $(foreach dir, $(XCOMMANDSDIRS), $(wildcard $(dir)/*.c))))
HTTPDOBJS = httpd.o httpd_arch.o ssp.o ssp_fs_root.o ssp_fstab.o
SRCDIRS += ${TOP}/src/appl/httpd ${TOP}/src/appl/web

ifeq ($(strip ${CFG_WEB}), 1) 
GUIDIRS = ${TOP}/src/appl/web/callback ${TOP}/src/appl/web/content
SRCDIRS += ${GUIDIRS}
GUIOBJS = $(patsubst %.c,%.o,$(notdir $(foreach dir, $(GUIDIRS), $(wildcard $(dir)/*.c))))
endif

UIOBJS = cli.o ui_switch.o ui_system.o ui_flash.o ui_igmpsnoop.o ui_rx.o ui_tx.o 


SRCDIRS += ${TOP}/src/appl/cli

PERSISOBJS = persistence.o serialize.o flash_medium.o ramtxt_medium.o mcast.o\
             mirror.o network.o qos.o serializers.o vlan.o lag.o loopdetect.o\
             system.o
SRCDIRS += ${TOP}/src/appl/persistence ${TOP}/src/appl/persistence/media/flash\
           ${TOP}/src/appl/persistence/media/ramtxt\
           ${TOP}/src/appl/persistence/serialize ${TOP}/src/serializers\

BRDIMPLOBJ = brd_misc.o brd_rxtx.o brd_vlan.o
SRCDIRS += ${TOP}/src/board

FLASHOBJ = iproc_qspi.o flash.o
SWITCHOBJ = mlswitch.o mlrxtx.o mlvlan.o mlport.o mlloop.o \
            mlmdns.o xlmac.o

SWITCHOBJ += mdk_phy.o cdk_debug.o bcm5346x_miim_int.o bcm95346xk_miim_ext.o
ifeq ($(strip ${CFG_SOC_SNAKE_TEST}), 1)
SWITCHOBJ += mlsnaketest.o
endif
ifeq ($(strip ${CFG_QT}), 1)
SWITCHOBJ += mlqt.o
endif

SRCDIRS += ${TOP}/src/driver/flash
SRCDIRS += ${TOP}/src/driver/soc/bcm5346x 

UTILSOBJS = ui_utils.o net_utils.o ports_utils.o factory_utils.o pbmp.o nvram_utils.o system_utils.o

SRCDIRS += ${TOP}/src/utils/net ${TOP}/src/utils/ports ${TOP}/src/utils/ui\
           ${TOP}/src/utils/nvram ${TOP}/src/utils/system

SRCDIRS += ${TOP}/src/driver/phyecd

#
# Add the common object files here.
#
ALLOBJS += $(KERNELOBJS) $(SALOBJS) $(UIPOBJS) $(APPLOBJS) $(UTILSOBJS)\
           $(UIOBJS) $(BRDIMPLOBJ) $(PERSISOBJS) $(FLASHOBJ) $(SWITCHOBJ)\
           $(HTTPDOBJS) $(GUIOBJS) $(XCMDOBJS) $(XCOMMANDSOBJS)

ifneq (,$(wildcard ${TOP}/src/driver/phyecd/phyecd.c))
LIBPHYECDOBJ = phyecd.o
LIBPHYECD = libphyecd.a
endif

#
# Add optional code.  Each option will add to ALLOBJS with its Makefile,
# and some append to SRCDIRS and/or CFE_INC.
#

ifeq ($(strip ${CFG_INTERRUPTS}),1)
CFLAGS += -DCFG_INTERRUPTS=1
endif

# Add phy support list
ifndef BCM_PHY_LIST
BCM_PHY_LIST=TSCE VIPER
endif
CFLAGS += $(foreach phy,$(BCM_PHY_LIST), -DINCLUDE_PHY_$(phy))

# specify endian of system and setup the flag 

ifeq ($(strip ${CFG_LITTLE}),1) 
CFLAGS += -DCFG_LITTLE_ENDIAN=1 -DCFG_BIG_ENDIAN=0
else
CFLAGS += -DCFG_LITTLE_ENDIAN=0 -DCFG_BIG_ENDIAN=1 
endif
 
#
# Make the paths
#

INCDIRS = $(patsubst %,-I%,$(subst :, ,$(BSP_INC) $(CFE_INC)))

VPATH = $(SRCDIRS)

#
# This is the makefile's main target.  Note that we actually
# do most of the work in 'ALL' (from the build Makefile) not 'all'.
#

#all : build_date.c makereg pcidevs_data2.h ALL
all : build_date.c ALL

.PHONY : all 
.PHONY : ALL
.PHONY : build_date.c

#
# Build the local tools that we use to construct other source files
#

HOST_CC = gcc
HOST_CFLAGS = -g -Wall -Werror -Wstrict-prototypes -Wmissing-prototypes

bin2codefile : ${TOP}/tools/bin2codefile.c
	$(HOST_CC) $(HOST_CFLAGS) -o bin2codefile ${TOP}/tools/bin2codefile.c

build_date.c :
	@echo "const char *builddate = \"`date`\";" > build_date.c
	@echo "const char *builduser = \"`whoami`@`hostname`\";" >> build_date.c

#
# Make a define for the board name
#

CFLAGS += -D_$(patsubst "%",%,${CFG_BOARDNAME})_

#
# Rules for building normal CFE files
#

LIBCFE = $(LIB_PATH)/libcfe.a


xcommands : $(patsubst %, %_cbkgen, ${XCOMMAND_XML_TABLES}) 
%.xml_cbkgen:
	@echo Convert $(patsubst %_cbkgen, %, $@) 
	@perl $(XCOMMANDS_PARSER) $(patsubst %_cbkgen, %, $@) $(XCOMMAND_DEF_TABLES) 


#
# Dependcy Rule 
#

-include $(wildcard $(patsubst %.o, %.d, $(CRT0OBJS) $(BSPOBJS) $(ALLOBJS)))

#
# Generic Complile 
#

%.o : %.c
	@echo ""
	$(GCC) $(CFE_CFLAGS) $(CFLAGS) -include $(BUILD_DIR)/conf.h -MP -MD -o $@ $< -c

%.o : %.S
	@echo ""
	$(GCCAS) -D__ASSEMBLER__ $(CFE_CFLAGS) $(CFLAGS) -include $(BUILD_DIR)/conf.h -MP -MD -o $@ $< -c

#
# Rules for building ZIPSTART
#

LIBZIPSTART = libzipstart.a

ZS_%.o : %.c
	$(CC) $(ENDIAN) $(ZIPSTART_CFLAGS) -D_ZIPSTART_ $(CFLAGS) -o $@ $<

ZS_%.o : %.S
	$(CC) $(ENDIAN) $(ZIPSTART_CFLAGS) -D_ZIPSTART_ $(CFLAGS) -o $@ $<

$(LIBPHYECD) : $(LIBPHYECDOBJ)
ifneq (,$(LIBPHYECDOBJ))
	rm -f ${LIB_PATH}/$(LIBPHYECD)
	$(GAR) cr ${LIB_PATH}/$(LIBPHYECD) $(LIBPHYECDOBJ)
	$(GRANLIB) ${LIB_PATH}/$(LIBPHYECD)
endif

#
# This rule constructs "libcfe.a" which contains most of the object
# files.
#

$(LIBCFE) : $(ALLOBJS)
	rm -f $(LIBCFE)
	$(GAR) cr $(LIBCFE) $(ALLOBJS)
	$(GRANLIB) $(LIBCFE)
