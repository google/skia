/*
 * YASM module loader header file
 *
 *  Copyright (C) 2002-2007  Peter Johnson
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND OTHER CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR OTHER CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef YASM_MODULE_H
#define YASM_MODULE_H

#ifndef YASM_LIB_DECL
#define YASM_LIB_DECL
#endif

typedef enum yasm_module_type {
    YASM_MODULE_ARCH = 0,
    YASM_MODULE_DBGFMT,
    YASM_MODULE_OBJFMT,
    YASM_MODULE_LISTFMT,
    YASM_MODULE_PARSER,
    YASM_MODULE_PREPROC
} yasm_module_type;

YASM_LIB_DECL
/*@dependent@*/ /*@null@*/ void *yasm_load_module
    (yasm_module_type type, const char *keyword);

#define yasm_load_arch(keyword) \
    yasm_load_module(YASM_MODULE_ARCH, keyword)
#define yasm_load_dbgfmt(keyword)       \
    yasm_load_module(YASM_MODULE_DBGFMT, keyword)
#define yasm_load_objfmt(keyword)       \
    yasm_load_module(YASM_MODULE_OBJFMT, keyword)
#define yasm_load_listfmt(keyword)      \
    yasm_load_module(YASM_MODULE_LISTFMT, keyword)
#define yasm_load_parser(keyword)       \
    yasm_load_module(YASM_MODULE_PARSER, keyword)
#define yasm_load_preproc(keyword)      \
    yasm_load_module(YASM_MODULE_PREPROC, keyword)

YASM_LIB_DECL
void yasm_list_modules
    (yasm_module_type type,
     void (*printfunc) (const char *name, const char *keyword));

#define yasm_list_arch(func)            \
    yasm_list_modules(YASM_MODULE_ARCH, func)
#define yasm_list_dbgfmt(func)          \
    yasm_list_modules(YASM_MODULE_DBGFMT, func)
#define yasm_list_objfmt(func)          \
    yasm_list_modules(YASM_MODULE_OBJFMT, func)
#define yasm_list_listfmt(func)         \
    yasm_list_modules(YASM_MODULE_LISTFMT, func)
#define yasm_list_parser(func)          \
    yasm_list_modules(YASM_MODULE_PARSER, func)
#define yasm_list_preproc(func)         \
    yasm_list_modules(YASM_MODULE_PREPROC, func)

YASM_LIB_DECL
void yasm_register_module(yasm_module_type type, const char *keyword,
                          void *data);

#endif
