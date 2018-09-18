#!/usr/bin/perl -w

#
# $Id: mkflashimage.pl,v 1.9 Broadcom SDK $
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
# Append one file to another file at fixed offset
# 
 
use integer;
use File::Basename;
local $script_name = basename(__FILE__);

################################################################################
#
# sub:   error ($error_string)
#
# Display how to use this script
#
sub error
{
    my ($error_string) = @_;
    die  $script_name . " error:" . $! . ": " . $error_string . "\n";
    #croak  $script_name . ":" . $! . ": " . $error_string . "\n";
}

################################################################################
#
# sub:    void bcm_image_info_get($image_file_name, \%firmware_info)
#
# Parse firmware information at the beginning of image
#

sub bcm_image_info_get {
	   my ($image_file_name, $firmware_info) = @_;
	   open(IMAGE_FILE, "<", $image_file_name) or error "Can't open $image_file_name";	   
	   seek(IMAGE_FILE, 64, 0);	   
	   #readline(IMAGE_FILE, $header);    
	   $head = <IMAGE_FILE>;
	   if ($head !~ /^UM$/) {
         $firmware_info->{"UM"} = 0;
         close(IMAGE_FILE);	
	       return;
	   }
	   $firmware_info->{"UM"} = 1;
#	   print "header gound\n";  
     while($board_name = <IMAGE_FILE>) {    
  
        if ($board_name =~ /^END$/) {
        	  last;
        }
        
        if ($board_name =~ /(.+)_W$/g) {                      	  
            $board_name = $1;
#            print $1 . "\n";
            binmode IMAGE_FILE, ::raw;
            read(IMAGE_FILE, $val8, 1);
            $multiple = 1;
            $value = hex(unpack("H*", $val8));            
            read(IMAGE_FILE, $val8, 1);
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            read(IMAGE_FILE, $val8, 1);            
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            read(IMAGE_FILE, $val8, 1);
            $multiple = $multiple *256;
            $value = $value + hex(unpack("H*", $val8)) * $multiple;                                   
            binmode IMAGE_FILE, ::crlf;               
            $firmware_info->{$board_name} = $value;
     
            next;
        }	   

        if ($board_name =~ /(.+)_S$/g) {               
            $string = <IMAGE_FILE>;
            $board_name = $1;
            $firmware_info->{$board_name} = $string;
            next;
        }	   

     }	   
	   	   
	   
	   close(IMAGE_FILE);	
}

#
# Check and parse command line parameters
#
local %image_info_hash;

die "Usage: ".__FILE__." <input_file1> <input_file2> <offset> <output_file>\n     " .__FILE__." <input_file1> <input_file2> <output_file>\n" unless (@ARGV == 4 || @ARGV == 3);

if (@ARGV == 4) {
   ($in_file, $in_file2, $offset, $out_file) = @ARGV
              or die "Usage: ".__FILE__." <input_file1> <input_file2> <offset> <output_file>\n";
    $_ = $offset;
    if (/^0x/) {
        $offset = hex($offset);
    }

} else { 
  if (@ARGV == 3) {
      ($in_file, $in_file2, $out_file) = @ARGV
       or die "Usage: ".__FILE__." <input_file1> <input_file2> <output_file>\n";
       $offset = -1;
  }
}

bcm_image_info_get($in_file, \%image_info_hash);

if ($image_info_hash{"UM"} == 1) {
    print "UM signature is found at " . $in_file . "\n";
    printf "Board name = %s", $image_info_hash{"CFG_BOARDNAME"};
    printf "Firmware Type = %s", $image_info_hash{"target"};
    printf "Version = %x\n", $image_info_hash{"CFE_VER"};
    $firmware_offset =  $image_info_hash{"BOARD_FIRMWARE_ADDR"} - $image_info_hash{"BOARD_LOADER_ADDR"};
    printf "firmware offset = 0x%x\n", $firmware_offset;
    if ($offset == -1) {
        $offset = $firmware_offset;
    } else {
        if ($firmware_offset != $offset) {
             error "The detected firmware offset is conflict with your offset setting, Please confirm it\.";
        }
    }
}

if ($offset == -1) {
    error "There is no UM signature found\nPlease key-in the offset value of the umweb firmware\n"; 
}
    
#
# Open files
#
open (IN_FILE, "< $in_file") or die "Cannot open input file $in_file!";
binmode(IN_FILE);

open (IN_FILE2, "< $in_file2") or die "Cannot open input file $in_file2!";
binmode(IN_FILE2);

open (OUT_FILE, "> $out_file") or die "Cannot open output file $out_file!";
binmode(OUT_FILE);

#
# File length in byte count
#
$count = -s IN_FILE;


if ($count > $offset) {
    die "Offset $offset is less than first file length $count!\n"
}

#
# Copy data
#
while(read(IN_FILE, $buf, 256)) {
    print OUT_FILE $buf;
}

#
# Append second file at offset
#
$rem = $offset - $count;
while($rem > 0) {
    printf OUT_FILE "\0";
    $rem--;
} 

while(read(IN_FILE2, $buf, 256)) {
    print OUT_FILE $buf;
}
#
# Close all files
#
close(OUT_FILE);
close(IN_FILE);
close(IN_FILE2);

