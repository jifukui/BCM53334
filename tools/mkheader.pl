#!/usr/bin/perl -w

#
# $Id: mkheader.pl,v 1.4 Broadcom SDK $
# 
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
# Add a header including checksum, file length and board to a binary file
# 
 
use integer;

#
# Check and parse command line parameters
#

if (@ARGV == 5) {
    ($in_file, $out_file, $majver, $minver, $build) = @ARGV
       or die "Usage: ".__FILE__." <input_file> <output_file> <maj> <min> <build>\n";
    
    open (IN_FILE, "< $in_file") or die "Cannot open input file $in_file!";
    binmode(IN_FILE);

    open (OUT_FILE, "> $out_file") or die "Cannot open output file $out_file!";
    binmode(OUT_FILE);

    read(IN_FILE, $buf, 0x10);
    print OUT_FILE $buf;

# Insert version into image at offset 0x10
    printf OUT_FILE "%01X%01X%01X0", $majver, $minver, $build;

    read(IN_FILE, $buf, 4);
#
# Copy remaining data
#
    while(read(IN_FILE, $buf, 256)) {
        print OUT_FILE $buf;
    }
    
    close(IN_FILE);
    close(OUT_FILE);
    exit;
}

die "Usage: ".__FILE__." <input_file> <output_file> <board_name> <maj> <min> <build>\n" unless (@ARGV == 6);

($in_file, $out_file, $board_name, $majver, $minver, $build) = @ARGV
    or die "Usage: ".__FILE__." <input_file> <output_file> <board_name> <maj> <min> <build>\n";
    
#
# Open files
#
open (IN_FILE, "< $in_file") or die "Cannot open input file $in_file!";
binmode(IN_FILE);

open (OUT_FILE, "> $out_file") or die "Cannot open output file $out_file!";
binmode(OUT_FILE);

#
# Insert header seal
#
print OUT_FILE "UMHD";

#
# Remember where we are (to copy later)
#
$pos = tell(IN_FILE);

#
# Calculate byte count and checksum
#
$flag = 0;
$sum = 0;
$count = 0;
while(read(IN_FILE, $buf, 1)) {
    if (($count%50) == 0) {
        $sum += ord($buf);
        $sum &= 0xFFFF;
    }
    $count++;
}

#
# Insert size and checksum
#

printf OUT_FILE "%08X%04X%04X", $count, $flag, $sum;

#
# Insert board name
#
printf OUT_FILE $board_name;

$count = length($board_name);
$rem = 32 - $count;
while($rem > 0) {
    printf OUT_FILE "\0";
    $rem--;
} 

#
# Insert version
#
printf OUT_FILE "%01X%01X%01X0", $majver, $minver, $build;

$rem = 8;

while($rem > 0) {
    printf OUT_FILE "\0";
    $rem--;
}

#
# Seek back for remaining data
#
seek IN_FILE, $pos, 0;

#
# Copy remaining data
#
while(read(IN_FILE, $buf, 256)) {
    print OUT_FILE $buf;
}

#
# Close all files
#
close(OUT_FILE);
close(IN_FILE);

