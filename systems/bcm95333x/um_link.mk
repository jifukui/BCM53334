# $Id: um_link.mk,v 1.15 Broadcom SDK $
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


$(LDSCRIPT) : $(LDSCRIPT_TEMPLATE) 
	$(GCC) -E $(CFLAGS) -ansi -D__ASSEMBLY__ -P - <$(LDSCRIPT_TEMPLATE) >$(LDSCRIPT)

#
# If the relocation type is STATIC, we need ZIPSTART.
#
ifeq ($(strip ${CFG_RELOC}),STATIC)
  CFETARGET = ramcfe
  ZIPSTART  = cfe
else
  CFETARGET = cfe
  ZIPSTART  = zipstart
endif

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
	$(GLD) -o $(CFETARGET) -Map $(CFETARGET).map $(LDFLAGS) $(CRT0OBJS) --start-group $(BSPOBJS) -L. -lcfe -L$(LIB_PATH) -lphypkgsrc -lphyutil -lphygeneric $(LDLIBS) $(PLATFORM_LIBS) --end-group
#	$(GLD) -o $(CFETARGET) -Map $(CFETARGET).map $(LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -lcfe -L$(LIB_PATH) -lphyecd $(PLATFORM_LIBS) $(LDLIBS)
#	$(GLD) $(ENDIAN) -o $(CFETARGET) -Map $(CFETARGET).map $(CFE_LDFLAGS) $(CRT0OBJS) $(BSPOBJS) -L. -L./lib -lcfe -lpower $(LDLIBS)
	$(GOBJDUMP) -d $(CFETARGET) > $(CFETARGET).dis
	$(GNM) $(CFETARGET) | sort > $(CFETARGET).nm

$(CFETARGET).bin : $(CFETARGET)
	$(GOBJCOPY) -O binary -R .reginfo -R .note -R .comment -R .mdebug -R .sram_data -S $(CFETARGET) $(CFETARGET).bin
#	$(GOBJCOPY) $(OCBINFLAGS) --output-target=binary $(CFETARGET) $(CFETARGET).bin
	$(GOBJCOPY) --input-target=binary --output-target=srec $(CFETARGET).bin $(CFETARGET).srec
ifeq ($(strip ${CFG_LOADER}),1)
	perl ${TOP}/tools/mkheader.pl $@ $(CFETARGET)-loader.bin ${CFE_VER_MAJ} ${CFE_VER_MIN} ${CFE_VER_ECO}
endif

OFMT = $(shell $(OBJDUMP) -i | head -2 | grep elf)

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
	$(GOBJDUMP) -d cfe > cfe.dis

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
