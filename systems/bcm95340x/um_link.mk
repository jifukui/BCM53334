# $Id: um_link.mk,v 1.13 Broadcom SDK $
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
# This Makefile snippet takes care of linking the firmware.
#

.PHONY: $(LDSCRIPT)

ifeq ($(strip ${CFG_GNU_TOOLCHAIN}),1)
LDFLAGS +=  -e reset -$(ENDIAN_FLAG) --gc-sections -L$(TOOLCHAIN_DIR)/$(ARMEB)-elf/lib/$(MULTILIB) -L$(TOOLCHAIN_DIR)/lib/gcc/$(ARMEB)-elf/$(GCC_VERSION)/$(MULTILIB) 
LDLIBS +=  -lc -lgcc
else
LDFLAGS +=  -e reset -$(ENDIAN_FLAG) --gc-sections -L$(TOOLCHAIN_DIR)/lib/gcc/arm-none-eabi/$(GCC_VERSION)/armv7-ar/thumb
LDLIBS += -lgcc
endif

#$(LDSCRIPT) : $(LDSCRIPT_TEMPLATE) 
#	$(GCC) -E $(CFLAGS) -ansi -P - <$(LDSCRIPT_TEMPLATE) >$(LDSCRIPT)

#
# If code is to be copied out to TCM, we need a bootloader
#
CFETARGET = cfe
BOOTTARGET = boot

#
# ZIPSTART linker stuff
#

ZZSOBJS = $(patsubst %,ZS_%,$(ZSOBJS))
ZZCRT0OBJS = $(patsubst %,ZS_%,$(ZCRT0OBJS))

$(LIBZIPSTART) : $(ZZSOBJS) $(BSPOBJS)
	rm -f $(LIBZIPSTART)
	$(GAR) cr $(LIBZIPSTART) $(ZZSOBJS)
	$(GRANLIB) $(LIBZIPSTART)

$(ZIPSTART) : $(ZZCRT0OBJS) $(LIBZIPSTART) $(ZIPSTART_LDSCRIPT) $(CFETARGET).bin.o
	$(GLD) $(ENDIAN) -o $@ -Map $@.map -g --script $(ZIPSTART_LDSCRIPT) $(ZZCRT0OBJS) $(CFETARGET).bin.o -L. -lzipstart 
	$(GOBJDUMP) -d $@ > $@.dis
	$(GOBJCOPY) $(OCBINFLAGS) --output-target=binary $@ $@.bin
	$(GOBJCOPY) --input-target=binary --output-target=srec $@.bin $@.srec

#
# CFE linker stuff
#


$(CFETARGET) : $(CRT0OBJS) $(BSPOBJS) $(LIBCFE) $(LIBPHYECD)  $(LDSCRIPT)
	$(GLD) -o $(CFETARGET) -Map $(CFETARGET).map $(LDFLAGS) --defsym reset_addr=$(CFG_TEXT_START) -T $(BSP_SRC)/$(LD_SCRIPT) $(CRT0OBJS) --start-group $(BSPOBJS) -L. -lcfe -L$(LIB_PATH) -lphyecd -lphypkgsrc -lphyutil -lphygeneric $(LDLIBS) --end-group
#	$(GLD) -o $(CFETARGET) -Map $(CFETARGET).map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -lcfe -L$(LIB_PATH) -lphyecd $(PLATFORM_LIBS) $(LDLIBS)
#	$(GLD) $(ENDIAN) -o $(CFETARGET) -Map $(CFETARGET).map $(CFE_LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -L./lib -lcfe -lpower $(LDLIBS)
	$(GOBJDUMP) -d $(CFETARGET) > $(CFETARGET).dis
	$(GNM) $(CFETARGET) | sort > $(CFETARGET).nm

$(CFETARGET).bin : $(CFETARGET)
	$(GOBJCOPY) -O binary $(OBJCOPYOPTION) -S $(CFETARGET) $(CFETARGET).bin
#	$(GOBJCOPY) -O binary -R .reginfo -R .note -R .comment -R .mdebug -R .packet_buf -S $(CFETARGET) $(CFETARGET).bin
#	$(GOBJCOPY) $(OCBINFLAGS) --output-target=binary $(CFETARGET) $(CFETARGET).bin
#	$(GOBJCOPY) --input-target=binary --output-target=srec $(CFETARGET).bin $(CFETARGET).srec
#ifeq ($(strip ${CFG_LOADER}),1)
#	perl ${TOP}/tools/mkheader.pl $@ $(CFETARGET)-loader.bin ${CFE_VER_MAJ} ${CFE_VER_MIN} ${CFE_VER_ECO}
#endif

DATA_START = $(shell grep -m 1 data_start cfe.map | awk '{print $$1}' | sed 's/^0x0*/0x/')

DATA_OFFSET = $(shell grep -m 1 flashdata_offset cfe.map | awk '{print $$1}' | sed 's/^0x0*/0x/')

low_mem_redirect.o: low_mem.S
	$(GCCAS) -c -D__ASSEMBLER__ -DLOW_MEM_REDIRECT -DTEXT_START=0x00000000 -DDATA_START=$(DATA_START) $(CFLAGS) -o $@ $<

payload.c: $(CFETARGET)
	$(OBJCOPY) -O binary -j .text $(CFETARGET) um-payload-text
	$(OBJCOPY) -O binary -j .text2 $(CFETARGET) um-payload-text2
	$(OBJCOPY) -O binary -j .data $(CFETARGET) um-payload-data
	$(OBJCOPY) -O binary -j .flashdata $(CFETARGET) um-payload-flashdata
	$(TOP)/tools/mkpayload.py --text um-payload-text --text-start 0x00000000 --text2 um-payload-text2 --text2-start 0x00040000 --data um-payload-data --data-start $(DATA_START) --output payload.c
	rm um-payload-text um-payload-text2 um-payload-data

$(BOOTTARGET) : low_mem_redirect.o cache.o boot.o payload.o
	$(GLD) -o $(BOOTTARGET) -Map $(BOOTTARGET).map $(LDFLAGS) -T $(BSP_SRC)/um_load.lds --defsym reset_addr=$(CFG_TEXT_START) low_mem_redirect.o cache.o boot.o payload.o -L. -lcfe -L$(LIB_PATH) -lphyecd $(LDLIBS)
	$(GOBJDUMP) -d $(BOOTTARGET) > $(BOOTTARGET).dis

$(BOOTTARGET).bin : $(BOOTTARGET)
	$(GOBJCOPY) -O binary $(BOOTTARGET) $(BOOTTARGET).bin
	$(TOP)/tools/mkflashimage.pl $(BOOTTARGET).bin um-payload-flashdata $(DATA_OFFSET) $(CFG_IMG).bin
	rm um-payload-flashdata

OFMT = $(shell $(GOBJDUMP) -i | head -2 | grep elf)

$(CFETARGET).bin.o : $(CFETARGET).bin
ifeq ($(strip ${CFG_ZIPPED_CFE}),1)
	gzip -c $(CFETARGET).bin > $(CFETARGET).bin.gz
	$(GLD) $(ENDIAN) -T ${BSP_SRC}/binobj.lds -b binary --oformat $(OFMT) -o $(CFETARGET).bin.o $(CFETARGET).bin.gz
else
	$(GLD) $(ENDIAN) -T ${BSP_SRC}/binobj.lds -b binary --oformat $(OFMT) -o $(CFETARGET).bin.o $(CFETARGET).bin
endif

#
# Build the flash image
#

cfe.dis : cfe
	$(OBJDUMP) -d cfe > cfe.dis

${CFG_IMG}-${target}.flash : cfe.bin
	perl ${TOP}/tools/mkheader.pl $< $@ ${CFG_BOARDNAME} ${CFE_VER_MAJ} ${CFE_VER_MIN} ${CFE_VER_ECO}
	-cp $@ ${CFG_IMG}-fw.flash 
#	./mkflashimage -v ${ENDIAN} -B ${CFG_BOARDNAME} -V ${CFE_VER_MAJ}.${CFE_VER_MIN}.${CFE_VER_ECO} cfe.bin cfe.flash
#	$(GOBJCOPY) --input-target=binary --output-target=srec cfe.flash cfe.flash.srec

#
# Housecleaning
#

clean_wo_config:
	rm -f *.o *~ *.d $(CFETARGET) $(BOOTTARGET) *.bin *.flash *.dis *.map *.image um.lds
	rm -f build_date.c
	rm -f lib/libcfe.a
	rm -f payload.c
	rm -f $(CLEANOBJS)

clean: clean_wo_config
	rm -f $(CONFIG_FILES)

distclean : clean
