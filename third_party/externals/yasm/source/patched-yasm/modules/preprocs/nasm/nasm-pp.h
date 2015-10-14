/* preproc.h  header file for preproc.c
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the licence given in the file "Licence"
 * distributed in the NASM archive.
 */

#ifndef YASM_NASM_PREPROC_H
#define YASM_NASM_PREPROC_H

void pp_pre_include (const char *);
void pp_pre_define (char *);
void pp_pre_undefine (char *);
void pp_builtin_define (char *);
void pp_extra_stdmac (const char **);

extern Preproc nasmpp;

void nasm_preproc_add_dep(char *);

#endif
