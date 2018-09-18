# $Id: Makefile,v 1.16 Broadcom SDK $
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

CFG_BOARDNAME = SmartSwitch
CFG_IMG := bcm95333x

#
#
#  Config change detection  
#
#

ifneq ($(target), )
$(shell ln -s -f configs/config.$(target).old .config)
$(shell ln -s -f configs/config_$(target).h  include/config.h)
CFG_CONFIG_UPDATE = clean_wo_config
endif

target ?= umweb

ifeq ($(strip ${CFG_LOADER}),1)
target = loader
$(shell ln -s -f configs/config.$(target).old .config)
$(shell ln -s -f configs/config_$(target).h  include/config.h)
CFG_CONFIG_UPDATE = clean_wo_config
endif 

ifeq ($(wildcard .config), ) 
$(shell ln -s -f configs/config.$(target).old .config)
$(shell ln -s -f configs/config_$(target).h  include/config.h)
CFG_CONFIG_UPDATE = clean_wo_config
endif 

ifeq ($(wildcard include/config.h), )
$(shell ln -s -f configs/config.$(target).old .config)
$(shell ln -s -f configs/config_$(target).h  include/config.h)
CFG_CONFIG_UPDATE = clean_wo_config
endif


include .config

CFG_INTERRUPTS ?= 0
CFG_LITTLE ?= 1
CFG_RELOC ?= 0
CFG_UNCACHED ?= 0
CFG_VAPI ?= 0 
CFG_BOOTRAM ?= 0
CFG_PCI ?= 0
CFG_ZLIB ?= 0
CFG_LDT ?= 0
CFG_WEB ?= 0
CFG_GNU_TOOLCHAIN ?= 0


TOP = $(abspath  ../..)
BUILD_DIR =  $(abspath  ./)
TOOL_DIR =  $(abspath  ../../tools)



CLEANOBJS += cfe.* cfez.* cfez zipstart.* conf.h  
CONFIG_FILES = .config include/config.h 
ALL : $(CFG_CONFIG_UPDATE) conf.h img_name cfe cfe.bin ${FLASH_IMG}
	mv cfe.bin ${CFG_IMG}-${target}.bin


img_name:

conf.h: .config
	perl $(TOOL_DIR)/config2h.pl


include $(BUILD_DIR)/um.mk
  
-include ${BUILD_DIR}/um_gen_phylibs.mk

include ${BUILD_DIR}/um_link.mk


