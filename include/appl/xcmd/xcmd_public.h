/*
 * $Id: xcmd_public.h,v 1.3 Broadcom SDK $
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

#ifndef _XCMD_PUBLIC_H_
#define _XCMD_PUBLIC_H_

#include "system.h"

#if defined(CFG_XCOMMAND_INCLUDED) || defined(CFG_VENDOR_CONFIG_SUPPORT_INCLUDED)

/****************************************************************
 *                                                                                                             *
 * Error code                                                                                             *
 *                                                                                                             *
 ****************************************************************/
 
typedef enum {
    XCMD_ERR_OK = 0,
    
    /* Non-errors */
    XCMD_ERR_FALSE,
    XCMD_ERR_EXIT,

    /* Non-masked errors */
    XCMD_ERR_OUT_OF_RESOURCE,
    XCMD_ERR_TOO_MANY_COMMAND_LEVELS,
    XCMD_ERR_BUFFER_OVERFLOW,
    
    /* Command parsing/handling errors */
    XCMD_ERR_CMD_ERROR_START,
    XCMD_ERR_CUSTOM_ERROR,
    XCMD_ERR_FAILED_TO_OPERATE,
    XCMD_ERR_INVALID_COMMAND_INPUT,
    XCMD_ERR_INCOMPLETE_COMMAND_INPUT,
    XCMD_ERR_INCOMPLETE_COMMAND_INPUT_FALSE,
    XCMD_ERR_INCORRECT_PARAM_LENGTH,
    XCMD_ERR_INCORRECT_PARAM_RANGE,
    /* Authetication errors */
    XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL,
    XCMD_ERR_AUTH_FAIL,    
    XCMD_ERR_CMD_ERROR_END,
  
    /* Internal errors */
    XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED,
    XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL,
    XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL,
    XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE,
    XCMD_ERR_INTERNAL_INVALID_SUB_CONTEXT_CALL,
    XCMD_ERR_INTERNAL_INVALID_CUSTOM_CONVERTERS,
    XCMD_ERR_INTERNAL_INVALID_PATH,
    XCMD_ERR_INTERNAL_INVALID_OPTION,
    XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM,
    XCMD_ERR_INTERNAL_INVALID_TABLE_DATA,
    XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE,
    XCMD_ERR_INTERNAL_INVALID_STREAM_CALL,
    XCMD_ERR_INTERNAL_INVALID_STREAM_DATA,


    
} XCMD_ERROR;

/****************************************************************
 *                                                              *
 * Type convertor                                               *
 *                                                              *
 ****************************************************************/
 
typedef XCMD_ERROR (*XCCONV_FROM_STRING)(
            const char *string,
            unsigned int len,
            void *properties,
            void **pdata                    /* NULL means to validate only */
            );
            
typedef XCMD_ERROR (*XCCONV_TO_STRING)(
            void *value,
            void *properties,
            char **pstring                  /* NULL means to validate only */
            );
            
typedef void (*XCCONV_FREE_DATA)(void *data, void *properties);

typedef void (*XCCONV_FREE_STRING)(char *string, void *properties);

typedef struct {
    XCCONV_FROM_STRING  from_string;
    XCCONV_TO_STRING    to_string;
    XCCONV_FREE_DATA    free_data;      /* NULL: freeing is not required */
    XCCONV_FREE_STRING  free_string;    /* NULL: freeing is not required */
} XCMD_TYPE_CONVERTER;

/* Converters MUST use these functions for memory allocation/free */
extern void *xcmd_alloc(unsigned int size, char *desc);
extern void xcmd_free(void *addr);

extern void xccvt_common_free_string(char *string, void *properties);
extern void xccvt_common_free_data(void *data, void *properties);



/****************************************************************
 *                                                              *
 * Utilities required for handlers/generators                   *
 *                                                              *
 ****************************************************************/

typedef void *XCMD_HANDLE;

extern XCMD_ERROR 
xcmd_get_parameter(XCMD_HANDLE xch, int id, void **pvalue);

extern XCMD_ERROR
xcmd_set_parameter(XCMD_HANDLE xch, int id, void *value);

extern XCMD_ERROR
xcmd_start_sub_context(const char *prompt, XCMD_HANDLE xch);


/****************************************************************
 *                                                              *
 * Callback - Handler                                           *
 *                                                              *
 ****************************************************************/
 
typedef XCMD_ERROR (*XCMD_HANDLER)(
            int path,
            XCMD_HANDLE xch
            );
            
#define XCMD_GET_NUMBER(id, pvalue) do { \
    XCMD_ERROR r = xcmd_get_parameter((xch), (id), (void **)(pvalue)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)

#define XCMD_GET_STRING(id, pstring) do { \
    XCMD_ERROR r = xcmd_get_parameter((xch), (id), (void **)(pstring)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)

#define XCMD_GET_VALUE(id, pdata) do { \
    XCMD_ERROR r = xcmd_get_parameter((xch), (id), (void **)(pdata)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)

#define XCMD_INVOKE_SUB_CONTEXT(prompt, xch) do { \
    XCMD_ERROR r = xcmd_start_sub_context((prompt), (xch)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)


/****************************************************************
 *                                                              *
 * Callback - Generator                                         *
 *                                                              *
 ****************************************************************/

typedef enum {
    XCMD_ACTION_SET_AND_STOP        = XCMD_ERR_OK,
    XCMD_ACTION_SET_AND_CONTINUE    = -1,
    XCMD_ACTION_SKIP_AND_STOP       = -2,
    XCMD_ACTION_SKIP_AND_CONTINUE   = -3,
} XCMD_ACTION;
 
typedef XCMD_ACTION (*XCMD_BUILDER)(
            int path,
            unsigned int index,
            XCMD_HANDLE xch
            );
            
#define XCMD_SET_NUMBER(id, value) do { \
    XCMD_ERROR r = xcmd_set_parameter((xch), (id), (void *)(value)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)

#define XCMD_SET_STRING(id, string) do { \
    XCMD_ERROR r = xcmd_set_parameter((xch), (id), (void *)(string)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)

#define XCMD_SET_VALUE(id, value) do { \
    XCMD_ERROR r = xcmd_set_parameter((xch), (id), (void *)&(value)); \
    if (r != XCMD_ERR_OK) return r; \
} while(0)


/****************************************************************
 *                                                              *
 * API to parse/build commands                                  *
 *                                                              *
 ****************************************************************/

extern XCMD_ERROR
xcli_start_shell(const char*prompt, const char *user, const char *password);

extern XCMD_ERROR
xcli_execute_commands_from_buffer(const char *buffer, unsigned int len);

extern XCMD_ERROR
xcli_build_commands_to_buffer(char *buffer, unsigned int *plen);


/****************************************************************
 *                                                              *
 * API to build a displaying pages                                 *
 *                                                              *
 ****************************************************************/

#define XC_DISPLAY_PAGES_DO(ps)  xc_display_pages_start(ps); \
                                 do 


#define XC_DISPLAY_PAGES_WHILE(ps)  while (xc_display_pages_end(ps) != XCMD_ERR_OK) 
extern XCMD_ERROR xc_display_pages(XCMD_HANDLE *xch, const char *fmt, ...);
extern void xc_display_pages_start(XCMD_HANDLE *xch);
extern XCMD_ERROR xc_display_pages_end(XCMD_HANDLE *xch);

/****************************************************************
 *                                                                                                             *
 * API to mange user account                                                                     *
 *                                                                                                             *
 ****************************************************************/
/*
  * Function:
  *     xcli_auth_adduser
  * Purpose:
  *      XCMD API:  add user in privilege context 
  * Parameters:
  *      user : exist user name 
  *      password : password of corresponding user 
  *      access : the permission level of user, "privilege", "normal" or "guest"
  * Returns:
  *      XCMD_ERR_OK: add user successful. 
  *      XCMD_ERR_XXX: fail 
  */
    
extern XCMD_ERROR xcli_auth_adduser(const char *user, const char* password, const char *access);

/*
 * Function:
 *      xcli_auth_deluser
 * Purpose:
 *      delete exist user in privilege context
 * Parameters:
 *      user : exist user name 
 *      password : password of corresponding user 
 *      
 * Returns:
 *      XCMD_ERR_OK: delete user successful. 
 *      XCMD_ERR_XXX: fail 
 */

extern XCMD_ERROR xcli_auth_deluser(const char *user, const char* password);

/*
 * Function:
 *      xcli_auth_change_password
 * Purpose:
 *      renew password of exist user in privilege context
 * Parameters:
 *      user : exist user name 
 *      old_password : old password
 *      new_password: new password 
 * Returns:
 *      XCMD_ERR_OK: change password successful. 
 *      XCMD_ERR_XXX: fail 
 */

extern XCMD_ERROR xcli_auth_change_password(const char *user, const char* old_password, const char *new_password);




/****************************************************************
 *                                                              *
 * API to make use of per-context memory                        *
 *                                                              *
 ****************************************************************/

extern void *xcmd_pcmem_alloc(XCMD_HANDLE handle, void *key, int size);

extern void *xcmd_pcmem_get(XCMD_HANDLE handle, void *key);

extern int xcmd_pcmem_getsize(XCMD_HANDLE handle, void *key);

extern void xcmd_pcmem_free(XCMD_HANDLE handle, void *key);

#endif /* CFG_XCOMMAND_INCLUDED */

#endif /* _XCMD_PUBLIC_H_ */
