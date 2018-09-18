/*
 * $Id: xcmd_core.c,v 1.4 Broadcom SDK $
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
#include "system.h"
 
#ifdef CFG_XCOMMAND_INCLUDED

#include "appl/xcmd/xcmd_internal.h"
#include "xcmd_core.h"

#define XCDEBUG 0

#if XCDEBUG
#define XCDEBUG_PRINT(...)  do { sal_printf("XCDEBUG:"); sal_printf(__VA_ARGS__); sal_printf("\n"); } while(0)  
#else
#define XCDEBUG_PRINT(str, ...)  
#endif 



/***********************************************************************
 *
 *  Structures
 *
 **********************************************************************/

typedef struct {
    unsigned int count;
    XCEXE_PARAM items[MAX_PARAM_STACK_SIZE];
} XCSTACK_EXE_PARAM;

typedef struct {
    unsigned int count;
    XCGEN_PARAM items[MAX_PARAM_STACK_SIZE];
} XCSTACK_GEN_PARAM;

typedef struct _XCMEM_ENTRY {
    void *key;
    int size;
    unsigned int flags;
    struct _XCMEM_ENTRY *next;
} XCMEM_ENTRY;

typedef struct {
    XCMEM_ENTRY *psmem;
} XCMEM_SERVICE, *PCMEMH;

typedef struct {
    XCMD_INPUT *pin;
    const XCNODE_CONTEXT *cxt;
    PCMEMH memh;
} XCEXE_CXT_HANDLE;



/***********************************************************************
 *
 *  NODE handler declaration
 *
 **********************************************************************/

static XCMD_ERROR xc_handler_select(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_optional(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_cmd_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_cmd_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_word(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_line(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_integer(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_options(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_handler_param_custom(XCEXE_SESSION *ps, void *properties);

static XCMD_ERROR xc_completer_select(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_optional(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_cmd_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_cmd_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_word(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_line(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_integer(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_options(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_completer_param_custom(XCEXE_SESSION *ps, void *properties);

static XCMD_ERROR xc_lister_select(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_optional(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_cmd_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_cmd_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_eol(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_keyword(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_word(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_line(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_integer(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_options(XCEXE_SESSION *ps, void *properties);
static XCMD_ERROR xc_lister_param_custom(XCEXE_SESSION *ps, void *properties);

static XCMD_ERROR xc_builder_select(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_optional(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_cmd_keyword(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_cmd_eol(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_eol(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_keyword(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_word(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_line(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_integer(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_options(XCGEN_SESSION *ps, void *properties);
static XCMD_ERROR xc_builder_param_custom(XCGEN_SESSION *ps, void *properties);

XCMD_NODE_HANDLERS xcnode_handler_select = {
    xc_handler_select,
    xc_completer_select,
    xc_lister_select,
    xc_builder_select,
};

XCMD_NODE_HANDLERS xcnode_handler_optional = {
    xc_handler_optional,
    xc_completer_optional,
    xc_lister_optional,
    xc_builder_optional,
};

XCMD_NODE_HANDLERS xcnode_handler_cmd_eol = {
    xc_handler_cmd_eol,
    xc_completer_cmd_eol,
    xc_lister_cmd_eol,
    xc_builder_cmd_eol,
};

XCMD_NODE_HANDLERS xcnode_handler_cmd_keyword = {
    xc_handler_cmd_keyword,
    xc_completer_cmd_keyword,
    xc_lister_cmd_keyword,
    xc_builder_cmd_keyword,
};

XCMD_NODE_HANDLERS xcnode_handler_param_eol = {
    xc_handler_param_eol,
    xc_completer_param_eol,
    xc_lister_param_eol,
    xc_builder_param_eol,
};

XCMD_NODE_HANDLERS xcnode_handler_param_keyword = {
    xc_handler_param_keyword,
    xc_completer_param_keyword,
    xc_lister_param_keyword,
    xc_builder_param_keyword,
};

XCMD_NODE_HANDLERS xcnode_handler_param_word = {
    xc_handler_param_word,
    xc_completer_param_word,
    xc_lister_param_word,
    xc_builder_param_word,
};

XCMD_NODE_HANDLERS xcnode_handler_param_line = {
    xc_handler_param_line,
    xc_completer_param_line,
    xc_lister_param_line,
    xc_builder_param_line,
};

XCMD_NODE_HANDLERS xcnode_handler_param_integer = {
    xc_handler_param_integer,
    xc_completer_param_integer,
    xc_lister_param_integer,
    xc_builder_param_integer,
};

XCMD_NODE_HANDLERS xcnode_handler_param_custom = {
    xc_handler_param_custom,
    xc_completer_param_custom,
    xc_lister_param_custom,
    xc_builder_param_custom,
};

XCMD_NODE_HANDLERS xcnode_handler_param_options = {
    xc_handler_param_options,
    xc_completer_param_options,
    xc_lister_param_options,
    xc_builder_param_options,
};

XCMD_NODE_HANDLERS xcnode_handler_option = {
    NULL,
    NULL,
    NULL,
    NULL,
};


/***********************************************************************
 *
 *  Type converter declaration
 *
 **********************************************************************/

static XCMD_ERROR xc_strarg_from_string(
            const char *string, 
            unsigned int len, 
            void *prop, 
            void **pdata
            );

static XCMD_ERROR xc_stackstr_from_string(
            const char *string, 
            unsigned int len, 
            void *prop, 
            void **pdata
            );

static XCMD_ERROR xc_integer_from_string(
            const char *string, 
            unsigned int len, 
            void *prop, 
            void **pdata
            );

static XCMD_ERROR xc_word_to_string(
            void *value,
            void *properties,
            char **pstring
            );
       
static XCMD_ERROR xc_line_to_string(
            void *value,
            void *properties,
            char **pstring
            );
       
static XCMD_ERROR xc_integer_to_string(
            void *value,
            void *properties,
            char **pstring
            );

static XCMD_ERROR xc_options_to_string(
            void *value,
            void *properties,
            char **pstring
            );

static XCMD_TYPE_CONVERTER xccvt_word_converters = {
    xc_strarg_from_string,
    xc_word_to_string,
    xccvt_common_free_data,
    xccvt_common_free_string,
};

static XCMD_TYPE_CONVERTER xccvt_stack_word_converters = {
    xc_stackstr_from_string,
    xc_word_to_string,
    NULL,
    xccvt_common_free_string,
};

static XCMD_TYPE_CONVERTER xccvt_line_converters = {
    xc_strarg_from_string,
    xc_line_to_string,
    xccvt_common_free_data,
    xccvt_common_free_string,
};

static XCMD_TYPE_CONVERTER xccvt_stack_line_converters = {
    xc_stackstr_from_string,
    xc_line_to_string,
    NULL,
    xccvt_common_free_string,
};

static XCMD_TYPE_CONVERTER xccvt_integer_converters = {
    xc_integer_from_string,
    xc_integer_to_string,
    NULL,
    xccvt_common_free_string,
};

static XCMD_TYPE_CONVERTER xccvt_options_converters = {
    NULL,
    xc_options_to_string,
    NULL,
    NULL,
};


/***********************************************************************
 *
 *  Macros
 *
 **********************************************************************/

#define PUSH_EXE_PARAM(stack, pparam) xcmdi_push_exe_stack(stack, pparam)
#define POP_EXE_PARAM(stack, pparam) xcmdi_pop_exe_stack(stack, pparam)

#define PUSH_GEN_PARAM(stack, pparam) xcmdi_push_gen_stack(stack, pparam)
#define POP_GEN_PARAM(stack, pparam) xcmdi_pop_gen_stack(stack, pparam)

#define XCPRINT sal_printf

#define LIST_MATCH_DESC_IDENT(x)   ((((2 + (x)) + 1) + 3) / 4 * 4)

#define PRINT_DETAILED_MATCHED(name, desc, align) do {\
    XCPRINT("  %s", (name)); \
    if ((desc) != NULL) { \
        int d, c; \
        d = (align) - (2 + sal_strlen(name)); \
        for(c=0; c<d; c++) { \
            XCPRINT(" "); \
        } \
        XCPRINT("%s", (desc)); \
    } \
    XCPRINT("\n"); \
} while(0)


/* To check the type of xch handle is */
enum {
    XC_HANDLE_EXE = 1,
    XC_HANDLE_GEN = 2,
};

/***********************************************************************
 *
 *  Forwards
 *
 **********************************************************************/

static XCMD_ERROR xcmdi_goto_next_node(XCMDI_OP op, const XCMD_NODE_PTR *pnode, void *data);

static XCMD_ERROR xcmdi_push_exe_stack(void *stack, XCEXE_PARAM *pparam);
static XCMD_ERROR xcmdi_pop_exe_stack(void *stack, XCEXE_PARAM *pparam);
static XCMD_ERROR xcmdi_get_from_exe_stack(void *stack, int id, XCEXE_PARAM **pparam);

static XCMD_ERROR xcmdi_push_gen_stack(void *stack, XCGEN_PARAM *pparam);
static XCMD_ERROR xcmdi_pop_gen_stack(void *stack, XCGEN_PARAM *pparam);
static XCMD_ERROR xcmdi_get_from_gen_stack(void *stack, int id, XCGEN_PARAM **pparam);

static int char_is_boundary(char ch);
static int char_is_eol(char ch);
static int char_is_space(char ch);

static XCMD_ERROR xcmdi_str_remove_leading_spaces(XCEXE_SESSION *ps);
static XCMD_ERROR xcmdi_str_remove_leading_eols(XCEXE_SESSION *ps);
static XCMD_ERROR xcmdi_str_trymatch_token(XCEXE_SESSION *ps, const char *tk);
static XCMD_ERROR xcmdi_str_match_token(XCEXE_SESSION *ps, const char *tk);
static XCMD_ERROR xcmdi_str_get_token(XCEXE_SESSION *ps, int *plen);
static XCMD_ERROR xcmdi_str_get_line(XCEXE_SESSION *ps, int *plen);
static XCMD_ERROR xcmdi_str_reach_word_boundary(XCEXE_SESSION *ps, int off);
static XCMD_ERROR xcmdi_str_reach_line_boundary(XCEXE_SESSION *ps, int off);

static XCMD_ERROR xcmdi_context_generate(
    const XCNODE_CONTEXT *cxt, XCMD_OUTPUT *ps, PCMEMH memh);


/***********************************************************************
 *
 *  Default converters
 *
 **********************************************************************/

static XCMD_ERROR 
xc_strarg_from_string(const char *string, unsigned int len, void *prop, void **pdata)
{
    if (prop == NULL || string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* We've done validation in handlers, nothing else to check */
    
    /* Everything's OK */
    if (pdata == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    *pdata = (void *)xcmd_alloc(len + 1, "xc_strarg_from_string");
    if (*pdata == NULL) {
        return XCMD_ERR_OUT_OF_RESOURCE;
    }
    sal_strncpy((char *)*pdata, string, len);
    ((char *)*pdata)[len] = 0;
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_stackstr_from_string(const char *string, unsigned int len, void *prop, void **pdata)
{
    if (prop == NULL || string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* We've done validation in handlers, nothing else to check */
    
    /* Everything's OK */
    if (pdata == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    /* Copy data for user */
    sal_strncpy((void *)pdata, string, len);
    ((char *)pdata)[len] = 0;
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_word_to_string(void *value, void *prop, char **pstring)
{
    XCNODE_PARAM_WORD *pnode = (XCNODE_PARAM_WORD *)prop;
    unsigned int i;
    unsigned int vlen;

    if (prop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (value == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
    }
    
    /* Range checking */
    vlen = sal_strlen((char *)value);
    if (vlen  < pnode->minlen || vlen > pnode->maxlen) {
        
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
    }
    
    /* Content checking */
    for(i=0; i<vlen; i++) {
        char ch = ((char *)value)[i];
        if (char_is_boundary(ch)) {

            return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
        }
    }
    
    /* Everything's OK */
    if (pstring == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    /* Get a copy of it */
    *pstring = xcmd_alloc(vlen + 1, "xcmd_to_string");
    
    if (*pstring == NULL) {
        return XCMD_ERR_OUT_OF_RESOURCE;
    }
    sal_strncpy(*pstring, (const char *)value, vlen + 1);

    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_line_to_string(void *value, void *prop, char **pstring)
{
    XCNODE_PARAM_LINE *pnode = (XCNODE_PARAM_LINE *)prop;
    unsigned int vlen;

    if (prop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (value == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
    }
    
    /* Range checking */
    vlen = sal_strlen((char *)value);
    if (vlen < pnode->minlen || vlen > pnode->maxlen) {
        
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
    }
    
    /* Everything's OK */
    if (pstring == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    /* Get a copy of it */
    *pstring = xcmd_alloc(vlen + 1, "xcmd_to_string");
    
    if (*pstring == NULL) {
        return XCMD_ERR_OUT_OF_RESOURCE;
    }
    sal_strncpy(*pstring, (const char *)value, vlen + 1);

    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_integer_from_string(const char *string, unsigned int len, void *prop, void **pdata)
{
    const XCNODE_PARAM_INTEGER *pnode = (const XCNODE_PARAM_INTEGER *)prop;
    unsigned int number = 0;
    
    if (prop == NULL || string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    /* For not being ambiguous, HEX must start with '0x' */
    if (pnode->radix == 16) {
        if (len <= 2 || sal_strncmp(string, "0x", 2)) {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
    }
    
    /* Range checking */
    if (len > 12) {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }
    if (pnode->bsigned) {
        int num;
        char buf[13];
        char *endptr = buf;
        int minval = (int)pnode->min;
        int maxval = (int)pnode->max;
        sal_strncpy(buf, string, len);
        buf[len] = 0;
        num = sal_strtol(buf, &endptr, (int)pnode->radix);
         if (*endptr != '\0') {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        if (num < minval || num > maxval) {
            return XCMD_ERR_INCORRECT_PARAM_RANGE;
        }
        number = (unsigned int)num;
    } else {
        unsigned int num;
        char buf[13];
        const char *endptr = buf;
        unsigned int minval = pnode->min;
        unsigned int maxval = pnode->max;
        sal_strncpy(buf, string, len);
        buf[len] = 0;
        num = sal_strtoul(buf, &endptr, (int)pnode->radix);
        if (*endptr != '\0') {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        if (num < minval || num > maxval) {
            return XCMD_ERR_INCORRECT_PARAM_RANGE;
        }
        number = num;
    }
    
    /* Everything's OK */
    if (pdata == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    *pdata = (void *)number;
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_integer_to_string(void *value, void *prop, char **pstring)
{
    XCNODE_PARAM_INTEGER *pnode = (XCNODE_PARAM_INTEGER *)prop;
    char buffer[16];
    unsigned int vlen;

    if (prop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Range checking */
    if (pnode->bsigned) {
        int n = (int)value;
        if (n < (int)pnode->min || n > (int)pnode->max) {
            return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
        }
    } else {
        unsigned int n = (unsigned int)value;
        if (n < pnode->min || n > pnode->max) {
            return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
        }
    }
    
    /* Everything's OK */
    if (pstring == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    if (pnode->radix == 16) {
        sprintf(buffer, "0x%x", (unsigned int)value);
    } else {
        sprintf(buffer, "%d", (unsigned int)value);
    }
    
    /* Get a copy of it */
    vlen = sal_strlen(buffer) + 1;
    *pstring = xcmd_alloc(vlen, "xcmd_to_string");
    
    if (*pstring == NULL) {
        return XCMD_ERR_OUT_OF_RESOURCE;
    }
    sal_strncpy(*pstring, buffer, vlen);

    return XCMD_ERR_OK;
}

static XCMD_ERROR 
xc_options_to_string(void *value, void *prop, char **pstring)
{
    XCNODE_PARAM_OPTIONS *pnode = (XCNODE_PARAM_OPTIONS *)prop;
    const XCNODE_OPTION *p = NULL;
    unsigned int i;

    if (prop == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    for(i=0; i<pnode->count; i++) {
        p = pnode->options[i].properties;
        if (p == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }
        
        if ((int)value == p->value) {
            break;
        }
    }
    if (i == pnode->count || p == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_VALUE;
    }
    
    if (pstring == NULL) {

        /* Means to do validation only */
        return XCMD_ERR_OK;
    }
    
    /* Use the string in table (so we cannot have a free_string call) */
    *pstring = (char *)p->name;
    
    return XCMD_ERR_OK;
}

void
xccvt_common_free_string(char *string, void *properties)
{
    if (string) {
        xcmd_free(string);
    }
}

void
xccvt_common_free_data(void *data, void *properties)
{
    if (data) {
        xcmd_free(data);
    }
}


/***********************************************************************
 *
 *  Execute commands
 *
 **********************************************************************/
 
XCMD_ERROR 
xcmd_get_parameter(XCMD_HANDLE xch, int id, void **pvalue)
{
    XCEXE_SESSION *ps = (XCEXE_SESSION *)xch;
    XCEXE_PARAM *param;
    XCMD_ERROR r;
    
    if (xch == NULL || pvalue == NULL) {
        ps->error_code = XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
        return XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
    }
    if (ps->type == XC_HANDLE_GEN) {
        return XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
    }
    
    r = xcmdi_get_from_exe_stack(ps->params, id, &param);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            ps->error_code = XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
            return XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
        }
        return r;
    }
    
    /* Already visited */
    if (param->visited) {
        return XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
    }
    param->visited = 1;
    
    /* If converter is not supplied, means the value is already there */
    if (param->converter == NULL) {
        *pvalue = param->value;
        return XCMD_ERR_OK;
    }
    
    /* Call converter for value */
    if (param->converter->from_string == NULL || param->string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
    }
    r = (*param->converter->from_string)(
            param->string, 
            param->len, 
            param->properties, 
            pvalue
            );
    if (r != XCMD_ERR_OK) {
        return r;
    }
   
    /* Store returned pointer in case we need to free it */
    param->value = *pvalue;
    
    return XCMD_ERR_OK;
}


static XCMD_ERROR
xcmdi_invoke_execute_callback(XCMD_HANDLER handler, XCEXE_SESSION *ps, int path)
{
    XCSTACK_EXE_PARAM *pstack;
    XCMD_ERROR r;
    unsigned int i;
    
    if (ps == NULL || ps->params == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Clear visitied flags so we can check it's visited or not later */
    pstack = (XCSTACK_EXE_PARAM *)ps->params;
    for(i=0; i<pstack->count; i++) {
        
        pstack->items[i].visited = 0;
    }
    
    /* Clear the error that could happen during callback */
    ps->error_code = XCMD_ERR_OK;
    
    /* Invoke callback */
    r = (*handler)(path, (XCMD_HANDLE)ps);
    
    /* Take care of callback bug */
    if (ps->error_code != XCMD_ERR_OK) {
        return r;
    }

    /* Check if all parameters has been visited */
    for(i=0; i<pstack->count; i++) {
        if (!pstack->items[i].visited) {
            return XCMD_ERR_INTERNAL_INVALID_GET_PARAM_CALL;
        }
    }
    
    /* Free allocated data if any */
    for(i=0; i<pstack->count; i++) {
        XCEXE_PARAM *param = &pstack->items[i];
        if (param->value != NULL && 
            param->converter != NULL && 
            param->converter->free_data != NULL) {
                
            (*param->converter->free_data)(param->value, param->properties);
            param->value = NULL;
        }
    }
    
    return r;
}

XCMD_ERROR
xcmd_set_parameter(XCMD_HANDLE xch, int id, void *value)
{
    XCGEN_SESSION *ps = (XCGEN_SESSION *)xch;
    XCGEN_PARAM *param;
    XCMD_ERROR r;
    char *str;
    
    if (xch == NULL) {
        ps->error_code = XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
    }
    if (ps->type == XC_HANDLE_EXE) {
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
    }
    
    r = xcmdi_get_from_gen_stack(ps->params, id, &param);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            ps->error_code = XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
            return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
        }
        return r;
    }

    /* Check if the parameter has already been set */
    if (param->string != NULL) {

        ps->error_code = XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
        return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
    }
    
    if (param->converter == NULL || param->converter->to_string == NULL) {
        ps->error_code = XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
        return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
    }
    
    /* Do the "to string" convertion */
    r = (*param->converter->to_string)(value, param->properties, &str);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    param->string = str;
        
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xcmdi_build_command(XCGEN_SESSION *ps, XCMD_CMDFLAGS flags)
{
    XCSTACK_GEN_PARAM *pstack;
    XCMD_OUTPUT *pout;
    XCMD_ERROR r;
    unsigned int i;
    int len;
    
    if (ps == NULL || ps->params == NULL || ps->pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    pstack = (XCSTACK_GEN_PARAM *)ps->params;
    pout = (XCMD_OUTPUT *)ps->pstream;
    
    if (flags & XCFLAG_BEHAVIOR_MARKED) {
        char ch = DEFAULT_MARKED_LINE_PREFIX;
        r = pout->write_str(pout, &ch, 1);
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    for(i=0; i<pstack->count; i++) {
        
        if (pstack->items[i].string == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_SET_PARAM_CALL;
        }
        
        /* Write command/parameter */
        len = sal_strlen(pstack->items[i].string);
        r = pout->write_str(pout, pstack->items[i].string, len);
        if (r != XCMD_ERR_OK) {
            return r;
        }
        
        /* Write space to delimit commands/parameters */
        if (i < pstack->count - 1) {
            char ch = ' ';
            r = pout->write_str(pout, &ch, 1);
            if (r != XCMD_ERR_OK) {
                return r;
            }
        }
    }
    
    /* Write EOL */
    r = pout->write_cr(pout);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    return XCMD_ERR_OK;
}
static XCMD_ERROR
xcmdi_build_error_description(XCGEN_SESSION *ps, XCMD_ERROR err)
{
    XCSTACK_GEN_PARAM *pstack;
    XCMD_OUTPUT *pout;
    unsigned int i;
    int len;
    char *pstr;
    char buffer[128];
    int curpos = 0;
    int maxlen = sizeof(buffer) - 1;

    if (ps == NULL || ps->params == NULL || ps->pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    buffer[0] = 0;
    pstack = (XCSTACK_GEN_PARAM *)ps->params;
    pout = (XCMD_OUTPUT *)ps->pstream;
    
    /* Output error command */
    for(i=0; i<pstack->count; i++) {
        
        if (pstack->items[i].string == NULL) {

            /* Invalid parameter */
            pstr = DEFAULT_OUTPUT_INVALID_PARAM;

        } else {

            /* determined string */
            pstr = pstack->items[i].string;
        }
        
        len = sal_strlen(pstr);
        if (curpos + len > maxlen) {
            break;
        }
        sal_strncpy(&buffer[curpos], pstr, len);
        curpos += len;

        if (i < pstack->count - 1) {
            if (curpos + 1 > maxlen) {
                break;
            }
            buffer[curpos++] = ' ';
        }
    }
    
    buffer[curpos] = 0;
    return pout->report_error(pout, err, buffer);
}

static void
xcmdi_free_builder_strings(XCGEN_SESSION *ps)
{
    XCSTACK_GEN_PARAM *pstack;
    unsigned int i;
    
    if (ps == NULL || ps->params == NULL) {
        return;
    }
    pstack = (XCSTACK_GEN_PARAM *)ps->params;
    
    for(i=0; i<pstack->count; i++) {
        
        if (pstack->items[i].string == NULL) {
            continue;
        }
        
        if (pstack->items[i].converter == NULL) {
            continue;
        }
        
        if (pstack->items[i].converter->free_string == NULL) {

            /* Since a converter exists, we should assume it's not 'keyword' */
            pstack->items[i].string = NULL;
            continue;
        }
        
        (*pstack->items[i].converter->free_string)(
            pstack->items[i].string, 
            pstack->items[i].properties
            );
        pstack->items[i].string = NULL;
    }
}

static XCMD_ERROR
xcmdi_invoke_builder_callback(
    XCMD_BUILDER builder, 
    XCGEN_SESSION *ps, 
    int path, 
    const XCNODE_CONTEXT *subcxt,
    XCMD_CMDFLAGS flags
    )
{
    XCMD_ACTION r;
    unsigned int index = 0;
    
    for(;; index++) {
        
        /* Clear the error that could happen during callback */
        ps->error_code = XCMD_ERR_OK;
        
        /* Invoke callback */
        r = (*builder)(path, index, (XCMD_HANDLE)ps);
        
        if (r == XCMD_ACTION_SET_AND_STOP || 
            r == XCMD_ACTION_SET_AND_CONTINUE) {
                
            XCMD_ERROR err;

            /* Take care of callback bug when setting parameter*/
            if (ps->error_code != XCMD_ERR_OK) {
                r = ps->error_code;
                break;
            }

            err = xcmdi_build_command(ps, flags);

            if (err != XCMD_ERR_OK) {
                r = err;
                break;
            }
            
            /* Handle sub-context */
            if (subcxt) {
                err = xcmdi_context_generate(
                    subcxt, 
                    (XCMD_OUTPUT *)ps->pstream, 
                    (PCMEMH)ps->context
                    );
                if (err != XCMD_ERR_OK) {
                    xcmdi_free_builder_strings(ps);
                    return err;
                }
            }
            
            if (r == XCMD_ACTION_SET_AND_STOP) {
                r = XCMD_ERR_OK;
                break;
            }

            xcmdi_free_builder_strings(ps);
            continue;
            
        } else if (r == XCMD_ACTION_SKIP_AND_CONTINUE) {
            
            xcmdi_free_builder_strings(ps);
            continue;
            
        } else if (r == XCMD_ACTION_SKIP_AND_STOP) {

            r = XCMD_ERR_OK;
            break;

        } else {
            break;
        }
    }
    
    if ((XCMD_ERROR)r != XCMD_ERR_OK) {
        r = xcmdi_build_error_description(ps, r);
    }
    
    xcmdi_free_builder_strings(ps);
    return r;
}


/***********************************************************************
 *
 *  NODE handlers
 *
 **********************************************************************/

static XCMD_ERROR
xc_handler_select(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_SELECT *pnode = (XCNODE_SELECT *)properties;
    unsigned int i;
    XCMD_ERROR r = XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
  
    xcmdi_str_remove_leading_spaces(ps);
    
    for(i=0; i<pnode->count; i++) {

        r = xcmdi_goto_next_node(
                XCMDI_OP_EXECUTE, 
                &pnode->children[i],
                ps);
                
        if (r == XCMD_ERR_OK) {
            
            return r;
            
        } else if (r == XCMD_ERR_INVALID_COMMAND_INPUT) {

            /* The branches thinks it's invalid, so try another */
            continue;
            
        } else if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT) {
            
            /* Maybe we have  <CR> branch? */
            continue;
            
        } else {
            
            /* Other error should be sent back to upper layer */
            return r;
        }
    }
    
    return r;
}

static XCMD_ERROR
xc_handler_optional(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_OPTIONAL *pnode = (XCNODE_OPTIONAL *)properties;
    unsigned int i, j, incomplete_input_false = 0, valid_input = 0;
    /* This allow max optional parameter is 32 */
    unsigned int travel_mask=0; 
    XCMD_ERROR r = XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    xcmdi_str_remove_leading_spaces(ps);
        
    for(j=0; j<(pnode->count - 1); j++) {
        /* Iterat (pnode->count - 1) counts to process all possible optional parameters */
        incomplete_input_false = 0;
        
        for(i=0; i<(pnode->count - 1); i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;

            if ((j != 0) &&
                (pnode->children[i].handlers == &xcnode_handler_cmd_eol)) {
                /* If customer allows user can specify no optional parameter, 
                   Customer needs put COMMAND-EOL in OPTIONAL tag in XML file. 
                   At this time, It means CR path could be called by first iterate only. */
                incomplete_input_false++;
                continue;
            }

            if (travel_mask & (1 << i)) {
                /* Don't allow same optional parameter appears twice */
                r = xcmdi_str_trymatch_token(ps, p->name);
                if (r != XCMD_ERR_OK) {
                    if (r == XCMD_ERR_FALSE) {
                        if (xcmdi_str_reach_line_boundary(ps, 0) == XCMD_ERR_OK) {
                            incomplete_input_false++;
                            r = XCMD_ERR_INCOMPLETE_COMMAND_INPUT_FALSE;
                        } else {
                            r = XCMD_ERR_INVALID_COMMAND_INPUT;
                        }
                    }
                } else {
                    r = XCMD_ERR_INVALID_COMMAND_INPUT;
                }
                continue;
            }

            ps->optional_tag_flag |= XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;
            /* Use flag XCFLAG_OPTIONAL_TAG_GENERAL_MARKED to 
               1. distinguish it is real incomplete input or not
               2. skip line boundary check on parameter EOL */
            r = xcmdi_goto_next_node(
                    XCMDI_OP_EXECUTE, 
                    &pnode->children[i],
                    ps);
            ps->optional_tag_flag &= ~XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;

            if (r == XCMD_ERR_OK) {

                if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                    return r;
                }

                travel_mask |= (1 << i);
                valid_input++;

                break;
                
            } else if (r == XCMD_ERR_INVALID_COMMAND_INPUT) {

                continue;
                
            } else if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT) {

                return r;
                
            } else if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT_FALSE) {
            
                incomplete_input_false++;
                continue;

            } else {
                
                /* Other error should be sent back to upper layer */
                return r;
            }
        }

        if ((valid_input) && (incomplete_input_false == (pnode->count - 1))) {
            r = XCMD_ERR_OK;
            break;
        }

        if (r != XCMD_ERR_OK) {
            if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT_FALSE) {
                return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
            } else {
                return r;
            }
        }

    }   

    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Need to call ALL path to process all input optional parameters without token match */
    ps->optional_tag_flag |= XCFLAG_OPTIONAL_TAG_WO_TK_MATCH_CHECK;
    r = xcmdi_goto_next_node(
                XCMDI_OP_EXECUTE, 
                &pnode->children[pnode->count - 1],
                ps);
    ps->optional_tag_flag &= ~XCFLAG_OPTIONAL_TAG_WO_TK_MATCH_CHECK;
    return r;
}

static XCMD_ERROR
xc_handler_cmd_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_CMD_KEYWORD *pnode = (XCNODE_CMD_KEYWORD *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (ps->optional_tag_flag & XCFLAG_OPTIONAL_TAG_WO_TK_MATCH_CHECK) {
        /* For ALL path of optinal tag, don't do "match token" check */
        xcmdi_str_remove_leading_spaces(ps);
        return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
    }

    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            if (xcmdi_str_reach_line_boundary(ps, 0) == XCMD_ERR_OK) {
                if (ps->optional_tag_flag & XCFLAG_OPTIONAL_TAG_GENERAL_MARKED) {
                    return XCMD_ERR_INCOMPLETE_COMMAND_INPUT_FALSE;
                } else {
                    return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
                }
            }
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        return r;
    }

    if (xcli_auth_check(ps, pnode->flags) != XCMD_ERR_OK) {
           return XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL;
    };

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_handler_cmd_eol(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_CMD_EOL *pnode = (XCNODE_CMD_EOL *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    r = xcmdi_str_reach_line_boundary(ps, 0);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        return r;
    }

    if (xcli_auth_check(ps, pnode->flags) != XCMD_ERR_OK) {
           return XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL;
    };
    
    xcmdi_str_remove_leading_eols(ps);
    
    if (pnode->callbacks.handler == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }

    /* If it's associated with a sub-context, assign it now */
    if (pnode->sub_context != NULL) {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = pnode->sub_context;
    } else {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = NULL;
    }
    
    /* Call handler to process this command */
    r = xcmdi_invoke_execute_callback(pnode->callbacks.handler, ps, pnode->path);

    /* Unassigne context from the context handle */
    if (pnode->sub_context != NULL) {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = NULL;
    }
    
    return r;
}

static XCMD_ERROR
xc_handler_param_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_KEYWORD *pnode = (XCNODE_PARAM_KEYWORD *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            if (xcmdi_str_reach_line_boundary(ps, 0) == XCMD_ERR_OK) {
                return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
            }
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        return r;
    }

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_handler_param_word(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_WORD *pnode = (XCNODE_PARAM_WORD *)properties;
    XCEXE_PARAM param;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK || len <= 0) {
        if (len == 0) {
            return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
        } 
        return r;
    }
    
    /* Range checking */
    if (len < pnode->minlen || len > pnode->maxlen) {
        return XCMD_ERR_INCORRECT_PARAM_LENGTH;
    }
    
    /* Prepare variable parameter */
    param.id = pnode->id;
    if (pnode->memtype) {
        param.converter = &xccvt_stack_word_converters;
    } else {
        param.converter = &xccvt_word_converters;
    }
    param.string = &ps->line[ps->curpos];
    param.len = (unsigned int)len;
    param.properties = properties;
    param.value = NULL;
    
    /* Push to stack */
    r = PUSH_EXE_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        xcmd_free(param.value);
        return r;
    }

    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    /* Pop from stack and free the memory */
    POP_EXE_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_handler_param_line(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_LINE *pnode = (XCNODE_PARAM_LINE *)properties;
    XCEXE_PARAM param;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    r = xcmdi_str_get_line(ps, &len);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Range checking */
    if (len < pnode->minlen || len > pnode->maxlen) {
        return XCMD_ERR_INCORRECT_PARAM_LENGTH;
    }
    
    /* Prepare variable parameter */
    param.id = pnode->id;
    if (pnode->memtype) {
        param.converter = &xccvt_stack_line_converters;
    } else {
        param.converter = &xccvt_line_converters;
    }
    param.string = &ps->line[ps->curpos];
    param.len = (unsigned int)len;
    param.properties = properties;
    param.value = NULL;
    
    /* Push to stack */
    r = PUSH_EXE_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        xcmd_free(param.value);
        return r;
    }

    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    /* Pop from stack and free the memory */
    POP_EXE_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_handler_param_integer(XCEXE_SESSION *ps, void *properties)
{
    const XCNODE_PARAM_INTEGER *pnode = (const XCNODE_PARAM_INTEGER *)properties;
    XCEXE_PARAM param;
    XCMD_ERROR r;
    int len = 0;

    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK || len <= 0) {
        if (len == 0) {
            return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
        } 
        return r;
    }
    
    /* Check the string using data converter */
    r = xc_integer_from_string(&ps->line[ps->curpos], len, properties, NULL);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Prepare variable parameter */
    param.id = pnode->id;
    param.converter = &xccvt_integer_converters;
    param.string = &ps->line[ps->curpos];
    param.len = (unsigned int)len;
    param.properties = properties;
    param.value = NULL;
    
    /* Push to stack */
    r = PUSH_EXE_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    /* Pop from stack */
    POP_EXE_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_handler_param_custom(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_CUSTOM *pnode = (XCNODE_PARAM_CUSTOM *)properties;
    XCEXE_PARAM param;
    XCMD_ERROR r;
    int len = 0;
    BOOL linef;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Whether it's a LINE format (characters to the EOL) */
    linef = (pnode->format == 1);
    
    if (linef) {
        r = xcmdi_str_get_line(ps, &len);
        if (r != XCMD_ERR_OK) {
            return r;
        }
    } else {
        r = xcmdi_str_get_token(ps, &len);
        if (r != XCMD_ERR_OK || len <= 0) {
            if (len == 0) {
                return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
            } 
            return r;
        }
    }
    
    /* Check data using custom converters */
    if (pnode->converters == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }
    if (pnode->converters->from_string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_CUSTOM_CONVERTERS;
    }
    r = (*pnode->converters->from_string)(
            &ps->line[ps->curpos], 
            len, 
            pnode, 
            NULL
            );
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }
    
    /* Prepare variable parameter */
    param.id = pnode->id;
    param.converter = pnode->converters;
    param.string = &ps->line[ps->curpos];
    param.len = (unsigned int)len;
    param.properties = properties;
    param.value = NULL;
    
    /* Push to stack */
    r = PUSH_EXE_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        xcmd_free(param.value);
        return r;
    }

    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    /* Pop from stack and free the memory */
    POP_EXE_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_handler_param_options(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_OPTIONS *pnode = (XCNODE_PARAM_OPTIONS *)properties;
    XCEXE_PARAM param;
    XCMD_ERROR r;
    int len = 0;
    int value = -1;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (xcmdi_str_reach_line_boundary(ps, 0) == XCMD_ERR_OK) {
        return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
    }

    /* Iterate options and see if anyone matched */
    r = XCMD_ERR_INVALID_COMMAND_INPUT;
    for(i=0; i<pnode->count; i++) {
        const XCNODE_OPTION *p = pnode->options[i].properties;
        if (p == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }
        r = xcmdi_str_match_token(ps, p->name);
        if (r == XCMD_ERR_OK) {
            value = p->value;
            break;
        }
        if (r != XCMD_ERR_FALSE) {
            return r;
        }
    }
    
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }

    /* Prepare variable parameter */
    param.id = pnode->id;
    param.converter = NULL;
    param.string = NULL;
    param.len = 0;
    param.properties = NULL;
    param.value = (void *)value;
    
    /* Push to stack */
    r = PUSH_EXE_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    /* Pop from stack */
    POP_EXE_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_handler_param_eol(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_EOL *pnode = (XCNODE_PARAM_EOL *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (!(ps->optional_tag_flag & XCFLAG_OPTIONAL_TAG_GENERAL_MARKED)) {
        r = xcmdi_str_reach_line_boundary(ps, 0);
        if (r != XCMD_ERR_OK) {
            if (r == XCMD_ERR_FALSE) {
                return XCMD_ERR_INVALID_COMMAND_INPUT;
            }
            return r;
        }
    }
    xcmdi_str_remove_leading_eols(ps);
    
    if (pnode->callbacks.handler == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }

    /* If it's associated with a sub-context, assign it now */
    if (pnode->sub_context != NULL) {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = pnode->sub_context;
    } else {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = NULL;
    }
    
    /* Call handler to process this command */
    r = xcmdi_invoke_execute_callback(pnode->callbacks.handler, ps, pnode->path);

    /* Unassigne context from the context handle */
    if (pnode->sub_context != NULL) {
        XCEXE_CXT_HANDLE *exeh = (XCEXE_CXT_HANDLE *)ps->context;
        exeh->cxt = NULL;
    }
    
    return r;
}


/***********************************************************************
 *
 *  NODE completers
 *
 **********************************************************************/

static XCMD_ERROR
xc_completer_select(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_SELECT *pnode = (XCNODE_SELECT *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    xcmdi_str_remove_leading_spaces(ps);
    
    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {

        int found = 0;
        
        for(i=0; i<pnode->count; i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
            unsigned int slen;
            
            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
             
            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                continue;
            }

            /* Authentication check */ 
            if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                      continue;
                };
            }
            
            slen = sal_strlen(p->name);
            if (slen < tlen) {
                continue;
            }
            if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                continue;
            }
            if (found == 0) {
                
                /* Copy the whole token to buffer */
                if (slen - tlen > ps->buflen - ps->curpos - 1) {
                    slen = ps->buflen - ps->curpos - 1 + tlen;
                }
                sal_strncpy(&ps->line[ps->curpos], p->name, slen);
                ps->line[ps->curpos + slen] = 0;
                
            } else {
                
                /* Determine the common set using '\0' */
                unsigned int j;
                unsigned int clen = sal_strlen(&ps->line[ps->curpos]);
                
                for(j=0; j<clen; j++) {
                    if (ps->line[ps->curpos + j] != p->name[j]) {
                        break;
                    }
                }
                ps->line[ps->curpos + j] = 0;
            }
            
            found++;
        }
        
        /* If not ambiguous, add a space at the end */
        if (found == 1) {
            if (ps->buflen - 1 > ps->curpos + sal_strlen(&ps->line[ps->curpos])) {
                sal_strcat(&ps->line[ps->curpos], " ");
            }
        }
        
        return XCMD_ERR_OK;
    }

    /* Try to find the keyword */
    for(i=0; i<pnode->count; i++) {

        const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;

        if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
            continue;
        }

        if (p->name == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }

        /* Authentication check */ 
        if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
            if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                      continue;
            };
        }        
        r = xcmdi_str_trymatch_token(ps, p->name);
        if (r == XCMD_ERR_OK) {
            r = xcmdi_goto_next_node(
                    XCMDI_OP_EXECUTE, 
                    &pnode->children[i],
                    ps);
            return r;
        } else if (r != XCMD_ERR_FALSE) {
            return r;
        }
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_completer_optional(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_OPTIONAL *pnode = (XCNODE_OPTIONAL *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    unsigned int i, k;
    unsigned int travel_mask=0; 
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    for (k=0; k<(pnode->count - 1); k++) {
        /* Iterat (pnode->count - 1) counts to process all possible optional parameters */         
        xcmdi_str_remove_leading_spaces(ps);
        
        if (char_is_eol(ps->line[ps->curpos])) {
            /* No prefix specifed */
            return XCMD_ERR_OK;
        }
        
        r = xcmdi_str_get_token(ps, (int *)&tlen);
        if (r != XCMD_ERR_OK) {
            return r;
        }

        /* Do partial match/complete only if it's the ending token */
        if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {

            int found = 0;
            
            for(i=0; i<(pnode->count - 1); i++) {

                const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
                unsigned int slen;
                
                if (p->name == NULL) {
                    return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
                }

                if (travel_mask & (1 << i)) {
                    /* Don't allow same optional parameter appears twice */
                    continue;
                }
            
                if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                    continue;
                }

                /* Authentication check */ 
                if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                          continue;
                    };
                }
                
                slen = sal_strlen(p->name);
                if (slen < tlen) {
                    continue;
                }
                if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                    continue;
                }
                
                if (found == 0) {
                    
                    /* Copy the whole token to buffer */
                    if (slen - tlen > ps->buflen - ps->curpos - 1) {
                        slen = ps->buflen - ps->curpos - 1 + tlen;
                    }
                    sal_strncpy(&ps->line[ps->curpos], p->name, slen);
                    ps->line[ps->curpos + slen] = 0;
                    
                } else {
                    
                    /* Determine the common set using '\0' */
                    unsigned int j;
                    unsigned int clen = sal_strlen(&ps->line[ps->curpos]);
                    
                    for(j=0; j<clen; j++) {
                        if (ps->line[ps->curpos + j] != p->name[j]) {
                            break;
                        }
                    }
                    ps->line[ps->curpos + j] = 0;
                }
                
                found++;
            }
            
            /* If not ambiguous, add a space at the end */
            if (found == 1) {
                if (ps->buflen - 1 > ps->curpos + sal_strlen(&ps->line[ps->curpos])) {
                    sal_strcat(&ps->line[ps->curpos], " ");
                }
            }
            
            return XCMD_ERR_OK;
        }

        /* Try to find the keyword */
        for(i=0; i<(pnode->count - 1); i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;

            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                continue;
            }

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }

            /* Authentication check */ 
            if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                     continue;
                };
            }
            
            r = xcmdi_str_trymatch_token(ps, p->name);
            if (r == XCMD_ERR_OK) {
                travel_mask |= (1 << i);
                /* Use flag XCFLAG_OPTIONAL_TAG_GENERAL_MARKED to skip EOL return */
                if (k != (pnode->count - 2)) {
                    ps->optional_tag_flag |= XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;
                }
                r = xcmdi_goto_next_node(
                        XCMDI_OP_EXECUTE, 
                        &pnode->children[i],
                        ps);
                if (k != (pnode->count - 2)) {
                    ps->optional_tag_flag &= ~XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;
                }
                if (r == XCMD_ERR_OK) {
                    return r;
                } else {
                    break;
                }
            } else if (r != XCMD_ERR_FALSE) {
                return r;
            }
        }
    }
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_completer_cmd_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_CMD_KEYWORD *pnode = (XCNODE_CMD_KEYWORD *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
   
    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            /* Should not happen */
            return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
        }
        return r;
    }

    if (xcli_auth_check(ps, pnode->flags) != XCMD_ERR_OK) {
        return XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL;
    };

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_completer_cmd_eol(XCEXE_SESSION *ps, void *properties)
{
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Should not reach here */
    return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
}

static XCMD_ERROR
xc_completer_param_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_KEYWORD *pnode = (XCNODE_PARAM_KEYWORD *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {

        unsigned int slen;
        
        slen = sal_strlen(pnode->name);
        if (slen >= tlen && 
           !sal_strncmp(pnode->name, &ps->line[ps->curpos], tlen)) {
            
            /* Copy the whole token to buffer */
            if (slen - tlen > ps->buflen - ps->curpos - 1) {
                slen = ps->buflen - ps->curpos - 1 + tlen;
            }
            sal_strncpy(&ps->line[ps->curpos], pnode->name, slen);
            ps->line[ps->curpos + slen] = 0;
            if (ps->buflen - 1 > ps->curpos + slen) {
                ps->line[ps->curpos + slen] = ' ';
                ps->line[ps->curpos + slen + 1] = 0;
            }
        }
        
        return XCMD_ERR_OK;
    }

    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            return XCMD_ERR_OK;
        }
        return r;
    }

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_completer_param_word(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_WORD *pnode = (XCNODE_PARAM_WORD *)properties;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT) {
            return XCMD_ERR_OK;
        }
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        return XCMD_ERR_OK;
    }
    
    /* Range checking */
    if (len < pnode->minlen || len > pnode->maxlen) {
        return XCMD_ERR_OK;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_completer_param_line(XCEXE_SESSION *ps, void *properties)
{
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_completer_param_integer(XCEXE_SESSION *ps, void *properties)
{
    const XCNODE_PARAM_INTEGER *pnode = (const XCNODE_PARAM_INTEGER *)properties;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT) {
            return XCMD_ERR_OK;
        }
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        return XCMD_ERR_OK;
    }
    
    /* Check the string using data converter */
    r = xc_integer_from_string(&ps->line[ps->curpos], len, properties, NULL);
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_OK;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_completer_param_custom(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_CUSTOM *pnode = (XCNODE_PARAM_CUSTOM *)properties;
    XCMD_ERROR r;
    int len = 0;
    BOOL linef;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Whether it's a LINE format (characters to the EOL) */
    linef = (pnode->format == 1);
    if (linef) {
        return XCMD_ERR_OK;
    }

    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_INCOMPLETE_COMMAND_INPUT) {
            return XCMD_ERR_OK;
        }
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        return XCMD_ERR_OK;
    }
    
    /* Check data using custom converters */
    if (pnode->converters == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }
    if (pnode->converters->from_string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_CUSTOM_CONVERTERS;
    }
    r = (*pnode->converters->from_string)(
            &ps->line[ps->curpos], 
            len, 
            pnode, 
            NULL
            );
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_OK;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_completer_param_options(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_OPTIONS *pnode = (XCNODE_PARAM_OPTIONS *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {

        int found = 0;
        
        for(i=0; i<pnode->count; i++) {

            const XCNODE_OPTION *p = pnode->options[i].properties;
            unsigned int slen;
            
            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
        
            slen = sal_strlen(p->name);
            if (slen < tlen) {
                continue;
            }
            if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                continue;
            }
            
            if (found == 0) {
                
                /* Copy the whole token to buffer */
                if (slen - tlen > ps->buflen - ps->curpos - 1) {
                    slen = ps->buflen - ps->curpos - 1 + tlen;
                }
                sal_strncpy(&ps->line[ps->curpos], p->name, slen);
                ps->line[ps->curpos + slen] = 0;
                
            } else {
                
                /* Determine the common set using '\0' */
                unsigned int j;
                unsigned int clen = sal_strlen(&ps->line[ps->curpos]);
                
                for(j=0; j<clen; j++) {
                    if (ps->line[ps->curpos + j] != p->name[j]) {
                        break;
                    }
                }
                ps->line[ps->curpos + j] = 0;
            }
            
            found++;
        }
        
        /* If not ambiguous, add a space at the end */
        if (found == 1) {
            if (ps->buflen - 1 > ps->curpos + sal_strlen(&ps->line[ps->curpos])) {
                sal_strcat(&ps->line[ps->curpos], " ");
            }
        }
        
        return XCMD_ERR_OK;
    }

    /* Try to find the keyword */
    for(i=0; i<pnode->count; i++) {

        const XCNODE_OPTION *p = pnode->options[i].properties;

        if (p->name == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }
        
        r = xcmdi_str_match_token(ps, p->name);
        if (r == XCMD_ERR_OK) {

            xcmdi_str_remove_leading_spaces(ps);
            r = xcmdi_goto_next_node(
                    XCMDI_OP_EXECUTE, 
                    &pnode->next_node,
                    ps);
            return r;
        } else if (r != XCMD_ERR_FALSE) {
            return r;
        }
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_completer_param_eol(XCEXE_SESSION *ps, void *properties)
{
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (!(ps->optional_tag_flag & XCFLAG_OPTIONAL_TAG_GENERAL_MARKED)) {
        return XCMD_ERR_OK;
    } else {
        return XCMD_ERR_FALSE;
    }
}


/***********************************************************************
 *
 *  NODE listers
 *
 **********************************************************************/

static XCMD_ERROR
xc_lister_select(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_SELECT *pnode = (XCNODE_SELECT *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    unsigned int i;

    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    xcmdi_str_remove_leading_spaces(ps);
    
    /* If no prefix specifed */
    if (char_is_eol(ps->line[ps->curpos])) {
        
        int mlen = 0;
        
        /* Pass 1: finding the max length of names */
        for(i=0; i<pnode->count; i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
            int slen;

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }               

            /* Authentication check */ 
            if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                     continue;
                };
            } else if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_EOL *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                     continue;
                };
            }

            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                slen = sal_strlen("<cr>");
            } else {
                slen = sal_strlen(p->name);
            }
            if (slen > mlen) {
                mlen = slen;
            }
        }
        
        /* <2-char space> + <name> + <4-aligned> */
        mlen = LIST_MATCH_DESC_IDENT(mlen);

        /* Pass 2: output the name and description */
        for(i=0; i<pnode->count; i++) {
            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
            const char *name;
            const char *desc;

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }

             /* Authentication check */ 
             if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                 if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                      continue;
                 };
             } else if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                 if (xcli_auth_check(ps, ((XCNODE_CMD_EOL *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                      continue;
                 };
             }

            
            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                name = "<cr>";
                desc = NULL;
            } else {
                name = p->name;
                desc = p->desc;
            }
            PRINT_DETAILED_MATCHED(name, desc, mlen);
        }
        
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {
        
        BOOL found = FALSE;

        for(i=0; i<pnode->count; i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
            unsigned int slen;
            
            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
            

            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                continue;
            }

            /* Authentication check */ 
            if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                     continue;
                };
            } 
            
            slen = sal_strlen(p->name);
            if (slen < tlen) {
                continue;
            }
            if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                continue;
            }
            
            found = TRUE;
            XCPRINT("%s  ", p->name);
        }
        
        if (found) {
            XCPRINT("\n");
            return XCMD_ERR_OK;
        }
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }

    /* Try to find the keyword */
    for(i=0; i<pnode->count; i++) {

        const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;

        if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
            continue;
        }

        /* Authentication check */ 
        if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
            if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                 continue;
            };
        }
        
        if (p->name == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }
        
        r = xcmdi_str_trymatch_token(ps, p->name);
        if (r == XCMD_ERR_OK) {
            r = xcmdi_goto_next_node(
                    XCMDI_OP_EXECUTE, 
                    &pnode->children[i],
                    ps);
            return r;
        } else if (r != XCMD_ERR_FALSE) {
            return r;
        }
    }

    return XCMD_ERR_INVALID_COMMAND_INPUT;
}

static XCMD_ERROR
xc_lister_optional(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_OPTIONAL *pnode = (XCNODE_OPTIONAL *)properties;
    XCMD_ERROR r = XCMD_ERR_OK;
    unsigned int tlen = 0;
    unsigned int i, j;
    unsigned int travel_mask=0; 

    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    for (j=0; j<(pnode->count - 1); j++) {
        /* Iterat (pnode->count - 1) counts to process all possible optional parameters */
        xcmdi_str_remove_leading_spaces(ps);
        
        /* If no prefix specifed */
        if (char_is_eol(ps->line[ps->curpos])) {
            
            int mlen = 0;
            int cr_existed = 0;
            
            /* Pass 1: finding the max length of names */
            for(i=0; i<pnode->count; i++) {

                const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
                int slen;

                if (p->name == NULL) {
                    return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
                }

                /* Authentication check */ 
                if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                         continue;
                    };
                } else if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_EOL *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                         continue;
                    };
                }

                if ((travel_mask & (1 << i)) || cr_existed) {
                    continue;
                }

                if ((i == (pnode->count-1)) && (j == 0)) {
                    continue;
                } else if (((i == (pnode->count-1)) && (j != 0)) || 
                    (pnode->children[i].handlers == &xcnode_handler_cmd_eol)) {
                    slen = sal_strlen("<cr>");
                    cr_existed = 1;
                } else {
                    slen = sal_strlen(p->name);
                }
                if (slen > mlen) {
                    mlen = slen;
                }
            }

            cr_existed = 0;
            
            /* <2-char space> + <name> + <4-aligned> */
            mlen = LIST_MATCH_DESC_IDENT(mlen);

            /* Pass 2: output the name and description */
            for(i=0; i<pnode->count; i++) {
                const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
                const char *name;
                const char *desc;

                if (p->name == NULL) {
                    return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
                }

                /* Authentication check */ 
                if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                         continue;
                    };
                } else if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_EOL *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                         continue;
                    };
                }

                if ((travel_mask & (1 << i)) || cr_existed) {
                    continue;
                }

                if ((i == (pnode->count-1)) && (j == 0)) {
                    continue;
                } else if (((i == (pnode->count-1)) && (j != 0)) ||
                    (pnode->children[i].handlers == &xcnode_handler_cmd_eol)) {
                    name = "<cr>";
                    desc = NULL;
                    cr_existed = 1;
                } else {
                    name = p->name;
                    desc = p->desc;
                }
                PRINT_DETAILED_MATCHED(name, desc, mlen);
            }
            
            return XCMD_ERR_OK;
        }
        
        r = xcmdi_str_get_token(ps, (int *)&tlen);
        if (r != XCMD_ERR_OK) {
            return r;
        }
        
        /* Do partial match/complete only if it's the ending token */
        if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {
            
            BOOL found = FALSE;

            for(i=0; i<(pnode->count - 1); i++) {

                const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;
                unsigned int slen;
                
                if (p->name == NULL) {
                    return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
                }

                if (travel_mask & (1 << i)) {
                    /* Don't allow same optional parameter appears twice */
                    continue;
                }
            
                if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                    continue;
                }

                /* Authentication check */ 
                if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                    if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                         continue;
                    };
                }
                
                slen = sal_strlen(p->name);
                if (slen < tlen) {
                    continue;
                }
                if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                    continue;
                }
                
                found = TRUE;
                XCPRINT("%s  ", p->name);
            }
            
            if (found) {
                travel_mask |= (1 << i);
                XCPRINT("\n");
                return XCMD_ERR_OK;
            }
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }

        /* Try to find the keyword */
        for(i=0; i<(pnode->count - 1); i++) {

            const XCNODE_CMD_KEYWORD *p = pnode->children[i].properties;

            if (travel_mask & (1 << i)) {
                /* Don't allow same optional parameter appears twice */
                continue;
            }

            if (pnode->children[i].handlers == &xcnode_handler_cmd_eol) {
                continue;
            }

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
            
            /* Authentication check */ 
            if (pnode->children[i].handlers == &xcnode_handler_cmd_keyword) {
                if (xcli_auth_check(ps, ((XCNODE_CMD_KEYWORD *) (pnode->children[i].properties))->flags) != XCMD_ERR_OK) {
                     continue;
                };
            }

            r = xcmdi_str_trymatch_token(ps, p->name);
            if (r == XCMD_ERR_OK) {
                travel_mask |= (1 << i);
                /* Use flag XCFLAG_OPTIONAL_TAG_GENERAL_MARKED to skip EOL return */
                if (j != (pnode->count - 2)) {
                    ps->optional_tag_flag |= XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;
                }
                r = xcmdi_goto_next_node(
                        XCMDI_OP_EXECUTE, 
                        &pnode->children[i],
                        ps);
                if (j != (pnode->count - 2)) {
                    ps->optional_tag_flag &= ~XCFLAG_OPTIONAL_TAG_GENERAL_MARKED;
                }

                if (r == XCMD_ERR_OK) {
                    return r;
                } else {
                    break;
                }
            } else if (r != XCMD_ERR_FALSE) {
                return r;
            }
        }
    }

    return r;
    /* return XCMD_ERR_INVALID_COMMAND_INPUT; */
}

static XCMD_ERROR
xc_lister_cmd_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_CMD_KEYWORD *pnode = (XCNODE_CMD_KEYWORD *)properties;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (xcli_auth_check(ps, pnode->flags) != XCMD_ERR_OK) {
        return XCMD_ERR_INTERNAL_INVALID_PRIVILEGE_LEVEL;
    };
    

    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            /* Should not happen */
            return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
        }
        return r;
    }

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_lister_cmd_eol(XCEXE_SESSION *ps, void *properties)
{
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Should not reach here */
    return XCMD_ERR_INTERNAL_INVALID_ENGINE_ROUTE;
}

static XCMD_ERROR
xc_lister_param_keyword(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_KEYWORD *pnode = (XCNODE_PARAM_KEYWORD *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        int mlen = LIST_MATCH_DESC_IDENT(sal_strlen(pnode->name));
        PRINT_DETAILED_MATCHED(pnode->name, pnode->desc, mlen);
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {

        unsigned int slen;
        
        slen = sal_strlen(pnode->name);
        if (slen >= tlen && 
           !sal_strncmp(pnode->name, &ps->line[ps->curpos], tlen)) {
            
            XCPRINT("%s  \n", pnode->name);
            return XCMD_ERR_OK;
        }

        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }

    r = xcmdi_str_match_token(ps, pnode->name);
    if (r != XCMD_ERR_OK) {
        if (r == XCMD_ERR_FALSE) {
            return XCMD_ERR_INVALID_COMMAND_INPUT;
        }
        return r;
    }

    xcmdi_str_remove_leading_spaces(ps);
    return xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);
}

static XCMD_ERROR
xc_lister_param_word(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_WORD *pnode = (XCNODE_PARAM_WORD *)properties;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        const char *name = pnode->name;
        int mlen;
        if (name == NULL) {
            name = "WORD";
        }
        mlen = LIST_MATCH_DESC_IDENT(sal_strlen(name));
        PRINT_DETAILED_MATCHED(name, pnode->desc, mlen);
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        const char *name = pnode->name;
        if (name == NULL) {
            name = "WORD";
        }
        XCPRINT("%s \n", name);
        return XCMD_ERR_OK;
    }
    
    /* Range checking */
    if (len < pnode->minlen || len > pnode->maxlen) {
        return XCMD_ERR_INCORRECT_PARAM_LENGTH;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_lister_param_line(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_LINE *pnode = (XCNODE_PARAM_LINE *)properties;
    const char *name;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        const char *name = pnode->name;
        const char *desc = NULL;
        int mlen;
        int slen;
        if (name == NULL) {
            name = "LINE";
        }
        mlen = sal_strlen("<cr>");
        slen = sal_strlen(name);
        if (slen > mlen) {
            mlen = slen;
        }
        mlen = LIST_MATCH_DESC_IDENT(mlen);
        PRINT_DETAILED_MATCHED(name, pnode->desc, mlen);
        PRINT_DETAILED_MATCHED("<cr>", desc, mlen);
        return XCMD_ERR_OK;
    }
    
    name = pnode->name;
    if (name == NULL) {
        name = "LINE";
    }
    XCPRINT("%s  <cr>\n", name);
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_lister_param_integer(XCEXE_SESSION *ps, void *properties)
{
    const XCNODE_PARAM_INTEGER *pnode = (const XCNODE_PARAM_INTEGER *)properties;
    XCMD_ERROR r;
    int len = 0;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (char_is_eol(ps->line[ps->curpos])) {
        /* No prefix specifed */
        const char *name = pnode->name;
        int mlen;
        if (name == NULL) {
            name = "<number>";
        }
        mlen = LIST_MATCH_DESC_IDENT(sal_strlen(name));
        PRINT_DETAILED_MATCHED(name, pnode->desc, mlen);
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        const char *name = pnode->name;
        if (name == NULL) {
            name = "<number>";
        }
        XCPRINT("%s \n", name);
        return XCMD_ERR_OK;
    }
    
    /* Check the string using data converter */
    r = xc_integer_from_string(&ps->line[ps->curpos], len, properties, NULL);
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_lister_param_custom(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_CUSTOM *pnode = (XCNODE_PARAM_CUSTOM *)properties;
    XCMD_ERROR r;
    int len = 0;
    BOOL linef;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Whether it's a LINE format (characters to the EOL) */
    linef = (pnode->format == 1);

    if (char_is_eol(ps->line[ps->curpos])) {
        
        /* No prefix specifed */
        if (linef) {
            const char *name = pnode->name;
            const char *desc = NULL;
            int mlen;
            int slen;
            if (name == NULL) {
                name = "LINE";
            }
            mlen = sal_strlen("<cr>");
            slen = sal_strlen(name);
            if (slen > mlen) {
                mlen = slen;
            }
            mlen = LIST_MATCH_DESC_IDENT(mlen);
            PRINT_DETAILED_MATCHED(name, pnode->desc, mlen);
            PRINT_DETAILED_MATCHED("<cr>", desc, mlen);
            return XCMD_ERR_OK;

        } else {
        
            const char *name = pnode->name;
            int mlen;
            if (name == NULL) {
                if (linef) {
                    name = "LINE";
                } else {
                    name = "WORD";
                }
            }
            mlen = LIST_MATCH_DESC_IDENT(sal_strlen(name));
            PRINT_DETAILED_MATCHED(name, pnode->desc, mlen);
            return XCMD_ERR_OK;
        }
    }
    
    if (linef) {
        const char *name = pnode->name;
        if (name == NULL) {
            name = "LINE";
        }
        XCPRINT("%s  <cr>\n", name);
        return XCMD_ERR_OK;
    }

    r = xcmdi_str_get_token(ps, &len);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, len) == XCMD_ERR_OK) {
        const char *name = pnode->name;
        if (name == NULL) {
            name = "WORD";
        }
        XCPRINT("%s \n", name);
        return XCMD_ERR_OK;
    }
    
    /* Check data using custom converters */
    if (pnode->converters == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }
    if (pnode->converters->from_string == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_CUSTOM_CONVERTERS;
    }
    r = (*pnode->converters->from_string)(
            &ps->line[ps->curpos], 
            len, 
            pnode, 
            NULL
            );
    if (r != XCMD_ERR_OK) {
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }
    
    /* Advance line pointer */
    ps->curpos += len;    
    xcmdi_str_remove_leading_spaces(ps);
    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &pnode->next_node, ps);

    return r;
}

static XCMD_ERROR
xc_lister_param_options(XCEXE_SESSION *ps, void *properties)
{
    XCNODE_PARAM_OPTIONS *pnode = (XCNODE_PARAM_OPTIONS *)properties;
    XCMD_ERROR r;
    unsigned int tlen = 0;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* If no prefix specifed */
    if (char_is_eol(ps->line[ps->curpos])) {
        
        int mlen = 0;
        
        /* Pass 1: finding the max length of names */
        for(i=0; i<pnode->count; i++) {

            const XCNODE_OPTION *p = pnode->options[i].properties;
            int slen;

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
            
            slen = sal_strlen(p->name);
            if (slen > mlen) {
                mlen = slen;
            }
        }
        
        /* <2-char space> + <name> + <4-aligned> */
        mlen = LIST_MATCH_DESC_IDENT(mlen);

        /* Pass 2: output the name and description */
        for(i=0; i<pnode->count; i++) {
            const XCNODE_OPTION *p = pnode->options[i].properties;
            const char *name;
            const char *desc;

            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
            
            name = p->name;
            desc = p->desc;
            PRINT_DETAILED_MATCHED(name, desc, mlen);
        }
        
        return XCMD_ERR_OK;
    }
    
    r = xcmdi_str_get_token(ps, (int *)&tlen);
    if (r != XCMD_ERR_OK) {
        return r;
    }

    /* Do partial match/complete only if it's the ending token */
    if (xcmdi_str_reach_line_boundary(ps, tlen) == XCMD_ERR_OK) {
    
        BOOL found = FALSE;

        for(i=0; i<pnode->count; i++) {

            const XCNODE_OPTION *p = pnode->options[i].properties;
            unsigned int slen;
            
            if (p->name == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
            }
        
            slen = sal_strlen(p->name);
            if (slen < tlen) {
                continue;
            }
            if (sal_strncmp(p->name, &ps->line[ps->curpos], tlen)) {
                continue;
            }
            
            found = TRUE;
            XCPRINT("%s  ", p->name);
        }
        
        if (found) {
            XCPRINT("\n");
            return XCMD_ERR_OK;
        }
        return XCMD_ERR_INVALID_COMMAND_INPUT;
    }

    /* Try to find the keyword */
    for(i=0; i<pnode->count; i++) {

        const XCNODE_OPTION *p = pnode->options[i].properties;

        if (p->name == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
        }
        
        r = xcmdi_str_match_token(ps, p->name);
        if (r == XCMD_ERR_OK) {

            xcmdi_str_remove_leading_spaces(ps);
            r = xcmdi_goto_next_node(
                    XCMDI_OP_EXECUTE, 
                    &pnode->next_node,
                    ps);
            return r;
        } else if (r != XCMD_ERR_FALSE) {
            return r;
        }
    }

    return XCMD_ERR_INVALID_COMMAND_INPUT;
}

static XCMD_ERROR
xc_lister_param_eol(XCEXE_SESSION *ps, void *properties)
{
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (char_is_eol(ps->line[ps->curpos])) {
        if (!(ps->optional_tag_flag & XCFLAG_OPTIONAL_TAG_GENERAL_MARKED)) {
            XCPRINT("  <cr>\n");
            return XCMD_ERR_OK;
        } else {
            return XCMD_ERR_FALSE;
        }
    }

    return XCMD_ERR_INVALID_COMMAND_INPUT;
}


/***********************************************************************
 *
 *  NODE builders
 *
 **********************************************************************/

static XCMD_ERROR
xc_builder_select(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_SELECT *pnode = (XCNODE_SELECT *)properties;
    XCMD_ERROR r;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    for(i=0; i<pnode->count; i++) {
        
        r = xcmdi_goto_next_node(
                XCMDI_OP_BUILD, 
                &pnode->children[i],
                ps);
                
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_builder_optional(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_OPTIONAL *pnode = (XCNODE_OPTIONAL *)properties;
    XCMD_ERROR r;
    unsigned int i;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    for(i=0; i<pnode->count; i++) {
        
        r = xcmdi_goto_next_node(
                XCMDI_OP_BUILD, 
                &pnode->children[i],
                ps);
                
        if (r != XCMD_ERR_OK) {
            return r;
        }
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xc_builder_cmd_keyword(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_CMD_KEYWORD *pnode = (XCNODE_CMD_KEYWORD *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Build command line only if it's "config" command */
    if ((pnode->flags & XCFLAG_CMDTYPE_CONFIG) == 0) {
        return XCMD_ERR_OK;
    }
    
    param.id = -1;
    param.string = (char *)pnode->name;
    param.converter = NULL;
    param.properties = NULL;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_cmd_eol(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_CMD_EOL *pnode = (XCNODE_CMD_EOL *)properties;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    /* Build command line only if it's "config" command */
    if ((pnode->flags & XCFLAG_CMDTYPE_CONFIG) == 0) {
        return XCMD_ERR_OK;
    }
    
    if (pnode->callbacks.builder == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }
    
    /* Call handler to process this command */
    return xcmdi_invoke_builder_callback(
            pnode->callbacks.builder, 
            ps, 
            pnode->path,
            pnode->sub_context,
            pnode->flags
            );
}

static XCMD_ERROR
xc_builder_param_keyword(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_KEYWORD *pnode = (XCNODE_PARAM_KEYWORD *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = -1;
    param.string = (char *)pnode->name;
    param.converter = NULL;
    param.properties = NULL;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_word(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_WORD *pnode = (XCNODE_PARAM_WORD *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = pnode->id;
    param.string = NULL;
    param.converter = &xccvt_word_converters;
    param.properties = properties;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_line(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_LINE *pnode = (XCNODE_PARAM_LINE *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = pnode->id;
    param.string = NULL;
    param.converter = &xccvt_line_converters;
    param.properties = properties;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_integer(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_INTEGER *pnode = (XCNODE_PARAM_INTEGER *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = pnode->id;
    param.string = NULL;
    param.converter = &xccvt_integer_converters;
    param.properties = properties;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_custom(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_CUSTOM *pnode = (XCNODE_PARAM_CUSTOM *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = pnode->id;
    param.string = NULL;
    param.converter = (XCMD_TYPE_CONVERTER *)pnode->converters;
    param.properties = properties;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_options(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_OPTIONS *pnode = (XCNODE_PARAM_OPTIONS *)properties;
    XCGEN_PARAM param;
    XCMD_ERROR r;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    param.id = pnode->id;
    param.string = NULL;
    param.converter = &xccvt_options_converters;
    param.properties = properties;

    /* Push to stack */
    r = PUSH_GEN_PARAM(ps->params, &param);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &pnode->next_node, ps);
    
    /* Pop from stack */
    POP_GEN_PARAM(ps->params, NULL);

    return r;
}

static XCMD_ERROR
xc_builder_param_eol(XCGEN_SESSION *ps, void *properties)
{
    XCNODE_PARAM_EOL *pnode = (XCNODE_PARAM_EOL *)properties;
    
    if (ps == NULL || properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (pnode->callbacks.builder == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_TABLE_DATA;
    }
    
    /* Call handler to process this command */
    return xcmdi_invoke_builder_callback(
            pnode->callbacks.builder, 
            ps, 
            pnode->path,
            pnode->sub_context,
            pnode->flags
            );
}


/***********************************************************************
 *
 *  Common utilites
 *
 **********************************************************************/

static XCMD_ERROR
xcmdi_goto_next_node(XCMDI_OP op, const XCMD_NODE_PTR *pnode, void *data)
{
    const XCMD_NODE_HANDLERS *handlers;
    const void * properties;
    
    if (pnode == NULL || op < 0 || op >= XCMDI_OP_COUNT || data == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    handlers = pnode->handlers;
    properties = pnode->properties;
    if (handlers == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
    }
    if (properties == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (op == XCMDI_OP_EXECUTE) {
        XCEXE_SESSION *ps = (XCEXE_SESSION *)data;
        if (ps->op == XCMDI_OP_COMPLETE) {
            XCHANDLER_COMPLETE func = handlers->complete;
            if (func == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
            }
            XCMD_ERROR r = (*func)(data, (void *)properties);
            return r;
        } else if (ps->op == XCMDI_OP_FINDMATCH) {
            XCHANDLER_LIST func = handlers->list;
            if (func == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
            }
            XCMD_ERROR r = (*func)(data, (void *)properties);
            return r;
        } else {
            XCHANDLER_EXECUTE func = handlers->execute;
            if (func == NULL) {
                return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
            }
            XCMD_ERROR r = (*func)(data, (void *)properties);
            return r;
        }
    } else {
        XCHANDLER_BUILD func = handlers->build;
        if (func == NULL) {
            return XCMD_ERR_INTERNAL_INVALID_NOT_IMPLEMENTED;
        }
        XCMD_ERROR r = (*func)(data, (void *)properties);
        return r;
    }
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcmdi_push_exe_stack(void *stack, XCEXE_PARAM *pparam)
{
    
    XCSTACK_EXE_PARAM *pstack = (XCSTACK_EXE_PARAM *)stack;
    if (pstack == NULL || pparam == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (pstack->count >= MAX_PARAM_STACK_SIZE) {
        return XCMD_ERR_TOO_MANY_COMMAND_LEVELS;
    }
    
    sal_memcpy(&pstack->items[pstack->count], pparam, sizeof(XCEXE_PARAM));
    pstack->count++;
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcmdi_pop_exe_stack(void *stack, XCEXE_PARAM *pparam)
{

    XCSTACK_EXE_PARAM *pstack = (XCSTACK_EXE_PARAM *)stack;
    if (pstack == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    pstack->count--;
    if (pparam != NULL) {
        sal_memcpy(pparam, &pstack->items[pstack->count], sizeof(XCEXE_PARAM));
    }
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcmdi_get_from_exe_stack(void *stack, int id, XCEXE_PARAM **pparam)
{
    XCSTACK_EXE_PARAM *pstack = (XCSTACK_EXE_PARAM *)stack;
    unsigned int i;
    
    if (pstack == NULL || pparam == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    for(i=0; i<pstack->count; i++) {
        if (pstack->items[i].id == id) {
            *pparam = &pstack->items[i];
            return XCMD_ERR_OK;
        }
    }
    
    /* Not found */
    return XCMD_ERR_FALSE;
}

XCMD_ERROR
xcmdi_push_gen_stack(void *stack, XCGEN_PARAM *pparam)
{
    
    XCSTACK_GEN_PARAM *pstack = (XCSTACK_GEN_PARAM *)stack;
    if (pstack == NULL || pparam == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (pstack->count >= MAX_PARAM_STACK_SIZE) {
        return XCMD_ERR_TOO_MANY_COMMAND_LEVELS;
    }
    
    sal_memcpy(&pstack->items[pstack->count], pparam, sizeof(XCGEN_PARAM));
    pstack->count++;
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcmdi_pop_gen_stack(void *stack, XCGEN_PARAM *pparam)
{
    XCSTACK_GEN_PARAM *pstack = (XCSTACK_GEN_PARAM *)stack;
    if (pstack == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    pstack->count--;
    if (pparam != NULL) {
        sal_memcpy(pparam, &pstack->items[pstack->count], sizeof(XCGEN_PARAM));
    }
    
    return XCMD_ERR_OK;
}

XCMD_ERROR
xcmdi_get_from_gen_stack(void *stack, int id, XCGEN_PARAM **pparam)
{
    XCSTACK_GEN_PARAM *pstack = (XCSTACK_GEN_PARAM *)stack;
    unsigned int i;
    
    if (pstack == NULL || pparam == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    for(i=0; i<pstack->count; i++) {
        if (pstack->items[i].id == id) {
            *pparam = &pstack->items[i];
            return XCMD_ERR_OK;
        }
    }
    
    /* Not found */
    return XCMD_ERR_FALSE;
}

static int
char_is_boundary(char ch)
{
    if (ch ==  ' ' || 
        ch == '\t' ||
        ch == '\r' ||
        ch == '\n' ||
        ch == '\0') {
        
        return 1;
    }
    
    return 0;
}

static int
char_is_space(char ch)
{
    if (ch ==  ' ' || 
        ch == '\t') {
        
        return 1;
    }
    
    return 0;
}

static int
char_is_eol(char ch)
{
    if (ch == '\r' || 
        ch == '\n' ||
        ch == '\0') {
        
        return 1;
    }
    
    return 0;
}

static XCMD_ERROR
xcmdi_str_remove_leading_spaces(XCEXE_SESSION *ps)
{
    if (ps == NULL || ps->line == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    while(ps->curpos < ps->maxlen && 
          char_is_space(ps->line[ps->curpos])) {

        ps->curpos++;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xcmdi_str_remove_leading_eols(XCEXE_SESSION *ps)
{
    if (ps == NULL || ps->line == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    while(ps->curpos < ps->maxlen && 
          char_is_eol(ps->line[ps->curpos])) {

        ps->curpos++;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xcmdi_str_trymatch_token(XCEXE_SESSION *ps, const char *tk)
{
    int r;
    if (ps == NULL || ps->line == NULL || tk == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    r = sal_strlen(tk);
    if (ps->maxlen - ps->curpos < r) {
        return XCMD_ERR_FALSE;
    }
    
    if (sal_strncmp(&ps->line[ps->curpos], tk, r)) {
        return XCMD_ERR_FALSE;
    }
    
    return xcmdi_str_reach_word_boundary(ps, r);
}

static XCMD_ERROR
xcmdi_str_match_token(XCEXE_SESSION *ps, const char *tk)
{
    XCMD_ERROR r = xcmdi_str_trymatch_token(ps, tk);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    ps->curpos += sal_strlen(tk);
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xcmdi_str_reach_word_boundary(XCEXE_SESSION *ps, int off)
{
    if (ps == NULL || ps->line == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (ps->curpos + off >= ps->maxlen || 
        char_is_boundary(ps->line[ps->curpos + off])) {

        return XCMD_ERR_OK;
    }

    return XCMD_ERR_FALSE;
}

static XCMD_ERROR
xcmdi_str_reach_line_boundary(XCEXE_SESSION *ps, int off)
{
    if (ps == NULL || ps->line == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    if (ps->curpos + off >= ps->maxlen ||
        char_is_eol(ps->line[ps->curpos + off])) {

        return XCMD_ERR_OK;
    }
    
    return XCMD_ERR_FALSE;
}

static XCMD_ERROR
xcmdi_str_get_token(XCEXE_SESSION *ps, int *plen)
{
    int off;
    if (ps == NULL || ps->line == NULL || plen == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    off = 0;
    for(;;) {
        if (ps->curpos + off >= ps->maxlen ||
            char_is_boundary(ps->line[ps->curpos + off])) {

            break;
        }
        off++;
    }
    *plen = off;
    
    if (off == 0) {
        return XCMD_ERR_INCOMPLETE_COMMAND_INPUT;
    }
    
    return XCMD_ERR_OK;
}

static XCMD_ERROR
xcmdi_str_get_line(XCEXE_SESSION *ps, int *plen)
{
    int off;
    if (ps == NULL || ps->line == NULL || plen == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    off = 0;
    for(;;) {
        if (ps->curpos + off >= ps->maxlen ||
            char_is_eol(ps->line[ps->curpos + off])) {

            break;
        }
        off++;
    }
    *plen = off;
    
    return XCMD_ERR_OK;
}

void *
xcmd_alloc(unsigned int size, char *desc)
{
    return sal_malloc(size);
}

void
xcmd_free(void *addr)
{
    sal_free(addr);
}


/***********************************************************************
 *
 *  Per-context memory
 *
 **********************************************************************/
 
static XCMEM_SERVICE*
get_xcmem_service(XCMD_HANDLE xch)
{
    XCEXE_SESSION *pexe;
    
    if (xch == NULL) {
        return NULL;
    }
    
    pexe = (XCEXE_SESSION *)xch;
    if (pexe->type == XC_HANDLE_EXE) {
        XCEXE_CXT_HANDLE *pcxt = (XCEXE_CXT_HANDLE *)pexe->context;
        if (pcxt == NULL) {
            return NULL;
        }
        return pcxt->memh;
        
    } else if (pexe->type == XC_HANDLE_GEN) {
        XCGEN_SESSION *pgen = (XCGEN_SESSION *)xch;
        return (XCMEM_SERVICE *)pgen->context;

    }
    
    return NULL;
}

void *
xcmd_pcmem_alloc(XCMD_HANDLE handle, void *key, int size)
{
    XCMEM_ENTRY *pm;
    XCMEM_SERVICE *pservice = get_xcmem_service(handle);

    if (pservice == NULL) {
        return NULL;
    }
    
    /* Check if we already have the key */
    pm = pservice->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            break;
        }
    }

    /* Found existing psmem for this key */
    if (pm != NULL) {
        /* If the new request wants more size, then we re-alloc it.
         * Otherwise (new size <= old size), we return the old mem. */
        if (size <= pm->size) {
            return ((void *)pm) + sizeof(XCMEM_ENTRY);
        } else {
            /* Free it first for re-allocation */
            xcmd_pcmem_free(handle, key);
        }
    }

    /* XXX: Use static memory as pool */
    pm = (XCMEM_ENTRY *)xcmd_alloc(sizeof(XCMEM_ENTRY) + size, "xcmd_pcmem");
    if (pm == NULL) {
        return NULL;
    }

    /* Insert into the chain */
    if (pservice->psmem == NULL) {
        pservice->psmem = pm;
        pm->next = NULL;
    } else {
        pm->next = pservice->psmem;
        pservice->psmem = pm;
    }
    pm->key = key;
    pm->size = size;

    return ((void *)pm) + sizeof(XCMEM_ENTRY);
}

void *
xcmd_pcmem_get(XCMD_HANDLE handle, void *key)
{
    XCMEM_ENTRY *pm;
    XCMEM_SERVICE *pservice = get_xcmem_service(handle);

    if (pservice == NULL) {
        return NULL;
    }

    /* Try to find the key */
    pm = pservice->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            return ((void *)pm) + sizeof(XCMEM_ENTRY);
        }
    }

    return NULL;
}

int
xcmd_pcmem_getsize(XCMD_HANDLE handle, void *key)
{
    XCMEM_ENTRY *pm;
    XCMEM_SERVICE *pservice = get_xcmem_service(handle);

    if (pservice == NULL) {
        return 0;
    }

    /* Try to find the key */
    pm = pservice->psmem;
    for(; pm != NULL; pm = pm->next) {
        if (pm->key == key) {
            return pm->size;
        }
    }

    return 0;
}

void 
xcmd_pcmem_free(XCMD_HANDLE handle, void *key) 
{
    XCMEM_ENTRY *ppm = NULL; /* previous one */
    XCMEM_ENTRY *pm;
    XCMEM_SERVICE *pservice = get_xcmem_service(handle);

    if (pservice == NULL) {
        return;
    }

    /* Try to find the key */
    pm = pservice->psmem;
    for(; pm != NULL; ppm = pm, pm = pm->next) {
        if (pm->key == key) {
            if (ppm == NULL) {
                pservice->psmem = pm->next;
            } else {
                ppm->next = pm->next;
            }

            /* Free it */
            xcmd_free(pm);
            return;
        }
    }

    /* Not found */
    return;
}

static void 
xcmdi_pcmem_free_all(XCMEM_SERVICE *pservice) 
{
    XCMEM_ENTRY *pm;
    XCMEM_ENTRY *npm;

    if (pservice == NULL) {
        return;
    }

    pm = pservice->psmem;
    for(; pm != NULL; pm = npm) {
        npm = pm->next;
        xcmd_free(pm);
    }
    
    pservice->psmem = NULL;
}


/***********************************************************************
 *
 *  Context-level functions
 *
 **********************************************************************/

XCMD_ERROR
xcmdi_context_generate(const XCNODE_CONTEXT *cxt, XCMD_OUTPUT *pout, PCMEMH memh)
{
    XCMD_ERROR r;
    XCSTACK_GEN_PARAM stack;
    XCGEN_SESSION session = {
        XC_HANDLE_GEN,
        pout,
        (void *)&stack,
        XCMD_ERR_OK,
        memh 
    };

    if (cxt == NULL || pout == NULL || memh == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    stack.count = 0; /* MUST initialize count to 0 */
    pout->open(pout);
    r = xcmdi_goto_next_node(XCMDI_OP_BUILD, &cxt->next_node, &session);
    pout->close(pout);

    return r;
}

XCMD_ERROR
xcmd_context_generate(const XCNODE_CONTEXT *cxt, XCMD_OUTPUT *pout)
{
    XCMD_ERROR r;
    XCMEM_SERVICE xcmem = {
        NULL
    };
    
    r = xcmdi_context_generate(cxt, pout, &xcmem);
    
    xcmdi_pcmem_free_all(&xcmem);
    return r;
}

static XCMD_ERROR
xcmdi_context_execute(
    XCMDI_OP op,
    const XCNODE_CONTEXT *cxt, 
    char *buffer, 
    unsigned int len,
    unsigned int alt_len,
    XCMD_INPUT *pstream,
    PCMEMH pcmem
    )
{
    XCMD_ERROR r;
    XCSTACK_EXE_PARAM stack;
    XCEXE_CXT_HANDLE ch = {
        pstream,
        NULL,
        pcmem
    };
    XCEXE_SESSION session = {
        XC_HANDLE_EXE,
        0,
        (int)op,
        buffer,
        len,
        0,
        alt_len,
        (void *)&stack,
        XCMD_ERR_OK,
        NULL,
        0,
        0,
        &ch,
        NULL,
        pstream
    };
    XCMEM_SERVICE xcmem = {
        NULL
    };

    if (cxt == NULL || buffer == NULL || pstream == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }

    stack.count = 0; /* MUST initialize count to 0 */
    /* If this is the top-level shell, prepare PCMEM engine */
    if (pcmem == NULL) {
        ch.memh = &xcmem;
    }
    
    /* Handle marked line */
    if (session.line[session.curpos] == DEFAULT_MARKED_LINE_PREFIX) {
        session.curpos++;
    }

    r = xcmdi_goto_next_node(XCMDI_OP_EXECUTE, &cxt->next_node, &session);
    
    /* Free the memory only if it's the top-level shell */
    if (pcmem == NULL) {
        xcmdi_pcmem_free_all(&xcmem);
    }
    
    return r;
}

static XCMD_ERROR
xcmdi_process_inputs(
    const char *prompt,
    XCMD_INPUT *ps,
    const XCNODE_CONTEXT *cxt, 
    PCMEMH pcmem
    )
{
    char *line = NULL;
    unsigned int len = 0;
    unsigned int alen = 0;
    XCMD_ERROR r;
    XCMDI_OP op = XCMDI_OP_EXECUTE;

    if (cxt == NULL || ps == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_FUNC_PARAM;
    }
    
    r = ps->open(ps, prompt);
    if (r != XCMD_ERR_OK) {
        return r;
    }
    
    for(;;) {
        
        r = ps->get_line(ps, (const char **)&line, &len, &op);

        /* The cli may decide to exit */
        if (r == XCMD_ERR_EXIT) {
            r = XCMD_ERR_OK;
            break;
        }

        if (r != XCMD_ERR_OK) {
            break;
        }

        if (line == NULL) {
            r = XCMD_ERR_INTERNAL_INVALID_STREAM_DATA;
            break;
        }
        
        if (op == XCMDI_OP_EXECUTE ||
            op == XCMDI_OP_FINDMATCH ||
            op == XCMDI_OP_COMPLETE) {

            /* For execute, len shouldn't be 0 */
            if (op == XCMDI_OP_EXECUTE && len == 0) {
                r = XCMD_ERR_INTERNAL_INVALID_STREAM_DATA;
                break;
            }

            /* AUTOCOMPLETE */
            alen = 0;
            if (op == XCMDI_OP_COMPLETE) {
                alen = len;
                len = sal_strlen(line);
                if (len >= alen) {
                    r = XCMD_ERR_INTERNAL_INVALID_STREAM_DATA;
                    break;
                }
            }

            r = xcmdi_context_execute(op, cxt, line, len, alen, ps, pcmem);
            
            /* The command may decide to exit */
            if (r == XCMD_ERR_EXIT) {
                r = XCMD_ERR_OK;
                break;
            }

            /* Let the INPUT stream show error if applicable */
            if (r != XCMD_ERR_OK) {
                ps->report_error(ps, r, line, len);
            }
            
            if (op == XCMDI_OP_EXECUTE) {
                /* Discard the content after executed */
                line = NULL;
                len = 0;
            } else if (op == XCMDI_OP_COMPLETE) {
                /* Return modified line back */
                if (line != NULL) {
                    len = sal_strlen(line);
                }
            }
            
            if (r > XCMD_ERR_CMD_ERROR_START && r < XCMD_ERR_CMD_ERROR_END) {
                continue;
            }
            
            if (r == XCMD_ERR_OK) {
                continue;
            }

            break;
        }
    }
    
    ps->close(ps);
    return r;
}

XCMD_ERROR
xcmd_process_inputs(
    const char *prompt,
    XCMD_INPUT *ps,
    const XCNODE_CONTEXT *cxt
    )
{
    return xcmdi_process_inputs(prompt, ps, cxt, NULL);
}

XCMD_ERROR
xcmd_start_sub_context(const char *prompt, XCMD_HANDLE xch)
{
    XCMD_ERROR r;
    XCEXE_SESSION *ps = (XCEXE_SESSION *)xch;
    XCEXE_CXT_HANDLE *exeh;
    
    if (xch == NULL) {
        return XCMD_ERR_INTERNAL_INVALID_SUB_CONTEXT_CALL;
    }
    if (ps->type == XC_HANDLE_GEN) {
        return XCMD_ERR_INTERNAL_INVALID_SUB_CONTEXT_CALL;
    }
    
    exeh = (XCEXE_CXT_HANDLE *)ps->context;
    
    if (exeh == NULL || 
        exeh->pin == NULL || 
        exeh->cxt == NULL || 
        exeh->memh == NULL) {

        return XCMD_ERR_INTERNAL_INVALID_SUB_CONTEXT_CALL;
    }
    
    r = xcmdi_process_inputs(
            prompt, 
            exeh->pin, 
            exeh->cxt, 
            exeh->memh
            );
    
    return r;
}

#endif /* CFG_XCOMMAND_INCLUDED */
