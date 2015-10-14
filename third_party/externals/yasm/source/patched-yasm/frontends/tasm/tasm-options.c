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
 * 3. Neither the name of the author nor the names of other contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
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
#include <util.h>
#include <ctype.h>

#include "tasm-options.h"


#ifdef __DEBUG__
#define DEBUG(x) fprintf ## x ;
#else
#define DEBUG(x)
#endif


/* Options Parser */
int
parse_cmdline(int argc, char **argv, opt_option *options, size_t nopts,
              void (*print_error) (const char *fmt, ...))
{
    int errors = 0, warnings = 0;
    size_t i;
    int got_it;

    DEBUG((stderr, "parse_cmdline: entered\n"));

  fail:
    while (--argc) {
        argv++;

        if (argv[0][0] == '/' && argv[0][1]) {        /* opt */
            got_it = 0;
            for (i = 0; i < nopts; i++) {
                char *cmd = &argv[0][1];
                size_t len = strlen(options[i].opt);
                if (yasm__strncasecmp(cmd, options[i].opt, len) == 0) {
                    char *param;

                    param = &argv[0][1+len];
                    if (options[i].takes_param) {
                        if (param[0] == '\0') {
                            print_error(
                                _("option `-%c' needs an argument!"),
                                options[i].opt);
                            errors++;
                            goto fail;
                        } else {
                            argc--;
                            argv++;
                        }
                    } else
                        param = NULL;

                    if (!options[i].handler(cmd, param, options[i].extra))
                        got_it = 1;
                    break;
                }
            }
            if (!got_it) {
                print_error(_("warning: unrecognized option `%s'"),
                            argv[0]);
                warnings++;
            }
        } else {    /* not an option, then it should be a file or something */

            if (not_an_option_handler(argv[0]))
                errors++;
        }
    }

    DEBUG((stderr, "parse_cmdline: finished\n"));
    return errors;
}

void
help_msg(const char *msg, const char *tail, opt_option *options, size_t nopts)
{
    char optbuf[100], optopt[100];
    size_t i;

    printf("%s", gettext(msg));

    for (i = 0; i < nopts; i++) {
        optbuf[0] = 0;
        optopt[0] = 0;

        if (options[i].takes_param) {
            if (options[i].opt)
                sprintf(optbuf, "/%s <%s>", options[i].opt,
                        options[i].param_desc ? options[i].
                        param_desc : _("param"));
        } else {
            if (options[i].opt)
                sprintf(optbuf, "/%s", options[i].opt);
        }

        printf("    %-22s  %s\n", optbuf, gettext(options[i].description));
    }

    printf("%s", gettext(tail));
}
