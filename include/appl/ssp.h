/*
 * $Id: ssp.h,v 1.7 Broadcom SDK $
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

#ifndef _SSP_H_
#define _SSP_H_

#include "system.h"

/* 
 * Parameters used in SSP tag (loop/var) callback functions
 */
typedef unsigned short SSPTAG_PARAM;

/* Per-Session-Memory-Handle for ssp functions */
typedef void *SSP_PSMH;

/* mask/flags of opbyte in data entries */
typedef enum {
    SSP_DATA_TYPE_MASK      = 0x03,
    SSP_DATA_TYPE_TEXT      = 0x00,
    SSP_DATA_TYPE_LOOP      = 0x01,
    SSP_DATA_TYPE_VAR       = 0x02,
    SSP_DATA_TYPE_CUSTOM    = 0x03,

    SSP_DATA_PARAM1_LOOP    = 0x10,

    SSP_DATA_FLAG_LAST      = 0x80,
} SSP_DATA_OPFLAG;

/* Data entry for SSP pages */
typedef struct SSP_DATA_ENTRY_s{
    /* OP byte: 
     *   [1:0] - data type: 0 - text; 1 - loop start/end; 2 - var; 3 - custom;
     *   [6:4] - whether any of params is loop index;
     *   [7]   - last entry
     */
    unsigned char opbyte;
    
    /* Peer Index: index offset for matching loop start/end */
    unsigned char pidx;
    
    /* Parameters (for loop start and var) */
    SSPTAG_PARAM param1; /* Also used as length for text entry */
    SSPTAG_PARAM param2;
    SSPTAG_PARAM param3;
    
    /* Data/Function pointer: for loop start, var and custom */
    RES_CONST_DECL void *dataptr;
} SSP_DATA_ENTRY;

/* File flags */
typedef uint32 SSP_FILE_FLAGS;


/****************************************************************
 *                                                              *
 * URL Handler - Handle request data or change reponse          *
 *                                                              *
 ****************************************************************/

/*
 * Types of URL handler - first field in handler context
 */
typedef enum {
    
    /* Request-phase handlers */
    SSP_HANDLER_GLOBAL_OPEN,
    SSP_HANDLER_OPEN,
    SSP_HANDLER_REQ_HEADER,
    SSP_HANDLER_REQ_COOKIE,
    SSP_HANDLER_QUERY_STRINGS,
    SSP_HANDLER_FILE_UPLOAD,
    SSP_HANDLER_REQ_DONE,
    
    /* Response-phase handlers */
    SSP_HANDLER_SET_HEADER,
    SSP_HANDLER_SET_COOKIE,
    SSP_HANDLER_SET_FILENAME,
    SSP_HANDLER_CLOSE
    
} SSP_HANDLER_TYPE;

/*
 * Query String pair (reference when type is SSP_HANDLER_QUERY_STRINGS)
 */
typedef struct
{
    const char *name;
    const char *value;
} SSP_STRING_PAIR;

/*
 * Basic URL handler context (for type SSP_HANDLER_QUERY_STRINGS)
 */
typedef struct {
    SSP_HANDLER_TYPE                type;
    
    /*
     * page & flags can only be changed in "Request-phase" handlers.
     */
    RES_CONST_DECL SSP_DATA_ENTRY *                page;
    SSP_FILE_FLAGS                  flags;
    
    /*
     * Query Strings 
     */
    SSP_STRING_PAIR *pairs;
    int count;
    
} SSP_HANDLER_CONTEXT;

/*
 *  Return value of URL handler
 */
typedef enum {
    SSP_HANDLER_RET_INTACT,
    SSP_HANDLER_RET_MODIFIED
} SSP_HANDLER_RETVAL;

/*
 * Prototype of URL handler callback function 
 */
typedef SSP_HANDLER_RETVAL (*SSP_FILE_HANDLER)(SSP_HANDLER_CONTEXT *, SSP_PSMH) REENTRANT;


/****************************************************************
 *                                                              *
 * SSP LOOP TAG FUNCTION - Called when reaching SSP loop tag    *
 *                                                              *
 ****************************************************************/

/* 
 * Return value of SSP Loop Tag function 
 */
typedef enum {
    SSPLOOP_STOP,
    SSPLOOP_PROCEED
} SSPLOOP_RETVAL;

/*
 * Current loop index for SSP loop tag 
 */
typedef unsigned short SSPLOOP_INDEX;

/* 
 * Prototype of SSP Loop Tag function 
 */
typedef SSPLOOP_RETVAL (*SSPLOOP_CBKFUNC)(SSPTAG_PARAM *, SSPLOOP_INDEX, SSP_PSMH) REENTRANT;


/****************************************************************
 *                                                              *
 * SSP VAR TAG FUNCTION - Called when reaching SSP variable tag *
 *                                                              *
 ****************************************************************/

/*
 * Maximal number of raw data (in bytes) per tag
 */
#define SSPVAR_MAX_RAWDATA_BYTES    (1024)

/*
 * Data type returned from sspvar functions to be sent to client 
 */
typedef enum {
    SSPVAR_RET_NULL,
    SSPVAR_RET_STRING,
    SSPVAR_RET_INTEGER,
    SSPVAR_RET_BINARY
} SSPVAR_RET_TYPE;

/*
 * Actual data returned from sspvar functions to be sent to client 
 */
typedef struct {
    SSPVAR_RET_TYPE type;
    union {
        const char *string;                 /* SSPVAR_RET_STRING */
        int   integer;                      /* SSPVAR_RET_INTEGER */
        const unsigned char *address;       /* SSPVAR_RET_BINARY */
    } val_data;     /* 8051 reserved word. change 'data' to 'val_data' */
    uint32 length;                          /* SSPVAR_RET_BINARY */
} SSPVAR_RET;
/* 
 * Prototype of SSP Variable function 
 */
typedef void (*SSPVAR_CBKFUNC)(SSPTAG_PARAM *, SSPVAR_RET *, SSP_PSMH) REENTRANT;


/****************************************************************
 *                                                              *
 * ADVANCED / MISC.                                             *
 *                                                              *
 ****************************************************************/

/*
 * Upload context for URL handler SSP_HANDLER_FILE_UPLOAD 
 */
typedef struct {
    /* 
     * Calling order:
     *    1. buf=NULL,index=-1: total length (in length) notification
     *    2. buf != NULL: data
     *    3. buf=NULL,lenght=0: End of File, total length in index */
    const unsigned char *buf;
    int32 index;
    int32 length;
} SSP_UPLOAD_CONTEXT;

/*
 * Extended URL handler context (types other than SSP_HANDLER_QUERY_STRINGS)
 */
typedef struct {
    SSP_HANDLER_TYPE                type;
    
    /*
     * page & flags can only be changed in "Request-phase" handlers.
     */
    RES_CONST_DECL SSP_DATA_ENTRY *                page;
    SSP_FILE_FLAGS                  flags;
    
    union {
        /* 
         * string should be used only for
         *      SSP_HANDLER_REQ_DONE     (write only)
         *      SSP_HANDLER_SET_FILENAME (write only)
         */
        const char *string;
        
        /* 
         * this string pair should be used only for
         *      SSP_HANDLER_REQ_HEADER   (read only)
         *      SSP_HANDLER_REQ_COOKIE   (read only)
         *      SSP_HANDLER_SET_HEADER   (write only)
         *      SSP_HANDLER_SET_COOKIE   (write only)
         */
        SSP_STRING_PAIR string_pair;
                                    
        /* 
         * upload should be used only for
         *      SSP_HANDLER_FILE_UPLOAD  (read/write)
         */
        SSP_UPLOAD_CONTEXT upload;
        
    } url_data; /* 8051 reserved word. change 'data' to 'url_data' */                          
     
} SSP_HANDLER_CONTEXT_EXT;

/* 8051 Code-Bank identifier */
typedef enum {
    SSP_BANK0,
    SSP_BANK1,
    SSP_BANK2,
    SSP_BANK3,
    SSP_BANK4,
    SSP_BANK5,
    SSP_BANK6,
    SSP_BANK7
} SSP_DATA_BANK;

#ifdef  __C51__
/* pre-defined valid bank_id to place SSP_DATA */
#define SSP_DATA_BANK_VALID(id)     \
            ((id) == SSP_BANK4 || (id) == SSP_BANK5)
#endif


/* Entry for global page table */
typedef struct {
    const char *                name;
    SSP_FILE_HANDLER            handler;
    RES_CONST_DECL SSP_DATA_ENTRY *            file_data;  /* 8051 reserved word. change 'data' to 'file_data' */                      
    SSP_FILE_FLAGS              flags;

    /* bank_id is for 8051's code-bank switch only */
    SSP_DATA_BANK               bank_id;
} SSP_FILE_ENTRY;

/*
 * Used in global handler
 */
typedef struct {
    SSP_STRING_PAIR *pairs;
    int count;
} SSP_QSTRING_CONTEXT;

/*
 * Global URL handler context
 */
typedef struct {
    SSP_HANDLER_TYPE                type;
    
    /*
     * URL file entry can only be changed with type SSP_HANDLER_GLOBAL_OPEN
     */
    SSP_FILE_ENTRY *                pfile;
    
    /*
     * page & flags can only be changed in normal "Request-phase" handlers 
     * 
     */
    RES_CONST_DECL SSP_DATA_ENTRY *                page;
    SSP_FILE_FLAGS                  flags;
    
    union {

        /* 
         * string should be used only for
         *      SSP_HANDLER_GLOBAL_OPEN  (read only)
         *      SSP_HANDLER_REQ_DONE     (write only)
         *      SSP_HANDLER_SET_HEADER   (write only)
         *      SSP_HANDLER_SET_COOKIE   (write only)
         *      SSP_HANDLER_SET_FILENAME (write only)
         */
        const char *string;
        
        /* 
         * this string pair should be used only for
         *      SSP_HANDLER_REQ_HEADER   (read only)
         */
        SSP_STRING_PAIR string_pair;
                                    
        /*
         * Query strings for 
         *      SSP_HANDLER_QUERY_STRINGS (read only)
         */
        SSP_QSTRING_CONTEXT qstrings;
        
    } url_data;     /* 8051 reserved word. change 'data' to 'url_data' */                        
    
} SSP_GLOBAL_HANDLER_CONTEXT;

/*
 * Prototype of Global URL handler callback function 
 */
typedef SSP_HANDLER_RETVAL (*SSP_GLOBAL_HANDLER)(SSP_GLOBAL_HANDLER_CONTEXT *, SSP_PSMH) REENTRANT; 

/*
 * Filesystem table 
 */
struct SSP_FILESYSTEM {
    const char *dir_prefix;     /* eg. "/data/images/", "/data/", "/", ... */
    SSP_FILE_ENTRY *filetable;
    SSP_GLOBAL_HANDLER globalhandler;
    SSP_FILE_FLAGS flags;
};


/****************************************************************
 *                                                              *
 * FILE FLAGS - used in *.wfs file                              *
 *                                                              *
 ****************************************************************/

/* 
 * File flags: displaying properties 
 */
#define SSPF_NO_CACHE               (0x00000001)
#define SSPF_FORCE_CACHE            (0x00000002)

/* 
 * File flags: behavior 
 */
#define SSPF_NO_DIRECT_ACCESS       (0x00000100)
#define SSPF_PUBLIC_RESOURCE        (0x00000200)
#define SSPF_IF_MODIFIED_SINCE      (0x00000400)
#define SSPF_LOCATION_REDIRECT      (0x00000800)
#define SSPF_MANAGER_RESOURCE       (0x00000080)

/* 
 * File flags: handler 
 */
#define SSPF_REQUEST_HEADER_H       (0x00001000)
#define SSPF_REQUEST_COOKIE_H       (0x00002000)
#define SSPF_QUERY_STRINGS_H        (0x00004000)
#define SSPF_FILE_UPLOAD_H          (0x00008000)
#define SSPF_REQUEST_DONE_H         (0x00010000)
#define SSPF_SET_HEADER_H           (0x00020000)
#define SSPF_SET_COOKIE_H           (0x00040000)
#define SSPF_SET_ATTACH_FILENAME_H  (0x00080000)
#define SSPF_OPEN_CLOSE_H           (0x00100000)
#define SSPF_GLOBAL_OPEN_H          (0x00200000)

/*
 * MASKs for file flags 
 */
#define SSPFMASK_DISPLAY_FLAGS      (0x0000003F)
#define SSPFMASK_BEHAVIOR_FLAGS     (0x00000FC0)
#define SSPFMASK_HANDLER_FLAGS      (0x00FFF000)
#define SSPFMASK_USER_FLAGS         (0xFF000000)


/****************************************************************
 *                                                              *
 * UTILITIES - Used in SSP callback functions                   *
 *                                                              *
 ****************************************************************/

/* 
 * SSP-callback-shared temp buffer, can be used by all callbacks 
 */
#define SSPUTIL_CBK_SHARED_SIZE (1024)
extern char ssputil_shared_buffer[SSPUTIL_CBK_SHARED_SIZE];

/* 
 * Per-Session memory - alloc by key (any unique pointer) 
 * NOTE: All per-session memory will be freed when session closed. 
 */
void *ssputil_psmem_alloc(SSP_PSMH handle, void *key, int size);

/*
 * Per-Session memory - get pre-allocated memory by key 
 */
void *ssputil_psmem_get(SSP_PSMH handle, void *key);

/* 
 * Per-Session memory - get pre-allocated memory size by key 
 */
int ssputil_psmem_getsize(SSP_PSMH handle, void *key);

/*
 * Per-Session memory - free per-session memory by key 
 */
void ssputil_psmem_free(SSP_PSMH handle, void *key);

/*
 * Get the TCP socket handle for this session
 * NOTE: 'secure' parameter returns whether this connection is secure or not.
 */
int ssputil_get_session_socket(SSP_PSMH handle, int *secure);

/* 
 * locate file - return data entries pointer and flags.
 *               NOTE: The URL(filename) must start with '/'.
 */
RES_CONST_DECL SSP_DATA_ENTRY *
ssputil_locate_file(SSP_PSMH psmem, const char *path, SSP_FILE_FLAGS *pflag);

#endif /* _SSP_H_ */

