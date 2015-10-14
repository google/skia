/*
 * Generic Options Support Header File
 *
 * Copyright (c) 2001  Stanislav Karchebny <berk@madfire.net>
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
#ifndef TASM_OPTIONS_H
#define TASM_OPTIONS_H

/* an option structure
 * operate on either -sopt, --lopt, -sopt <val> or --lopt=<val>
 */
typedef struct opt_option_s
{
    /* option */
    const char *opt;

    /* !=0 if option requires parameter, 0 if not */
    int takes_param;

    int (*handler) (char *cmd, /*@null@*/ char *param, int extra);
    int extra;                 /* extra value for handler */

    /* description to use in help_msg() */
    /*@observer@*/ const char *description;

    /* optional description for the param taken (NULL if not present) */
    /*  (short - will be printed after option sopt/lopt) */
    /*@observer@*/ /*@null@*/ const char *param_desc;
} opt_option;

/* handle everything that is not an option */
int not_an_option_handler(char *param);

/* parse command line calling handlers when appropriate
 * argc, argv - pass directly from main(argc,argv)
 * options - array of options
 * nopts - options count
 */
int parse_cmdline(int argc, char **argv, opt_option *options, size_t nopts,
                  void (*print_error) (const char *fmt, ...));

/* display help message msg followed by list of options in options and followed
 * by tail
 */
void help_msg(const char *msg, const char *tail, opt_option *options,
              size_t nopts);

#endif
