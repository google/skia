/*
 *
 *  Copyright (C) 2005-2007  Peter Johnson
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libyasm/intnum.c"

typedef struct Test_Entry {
    /* signedness (0=unsigned, 1=signed) */
    int sign;

    /* whether input value should be negated */
    int negate;

    /* input value (as hex string) */
    const char *input;

    /* correct size returned from both size_leb128 and get_leb128 */
    unsigned long outsize;

    /* correct return data from get_leb128 */
    const unsigned char *result;
} Test_Entry;

static Test_Entry tests[] = {
    /* Unsigned values */
    {0, 0, "0", 1, (const unsigned char *)"\x00"},
    {0, 0, "2", 1, (const unsigned char *)"\x02"},
    {0, 0, "7F", 1, (const unsigned char *)"\x7F"},
    {0, 0, "80", 2, (const unsigned char *)"\x80\x01"},
    {0, 0, "81", 2, (const unsigned char *)"\x81\x01"},
    {0, 0, "82", 2, (const unsigned char *)"\x82\x01"},
    {0, 0, "3239", 2, (const unsigned char *)"\xB9\x64"},
    /* Signed zero value */
    {1, 0, "0", 1, (const unsigned char *)"\x00"},
    /* Signed positive values */
    {1, 0, "2", 1, (const unsigned char *)"\x02"},
    {1, 0, "7F", 2, (const unsigned char *)"\xFF\x00"},
    {1, 0, "80", 2, (const unsigned char *)"\x80\x01"},
    {1, 0, "81", 2, (const unsigned char *)"\x81\x01"},
    /* Signed negative values */
    {1, 1, "2", 1, (const unsigned char *)"\x7E"},
    {1, 1, "7F", 2, (const unsigned char *)"\x81\x7F"},
    {1, 1, "80", 2, (const unsigned char *)"\x80\x7F"},
    {1, 1, "81", 2, (const unsigned char *)"\xFF\x7E"},
};

static char failed[1000];
static char failmsg[100];

static int
run_output_test(Test_Entry *test)
{
    char *valstr = yasm__xstrdup(test->input);
    yasm_intnum *intn = yasm_intnum_create_hex(valstr);
    unsigned long size, i;
    unsigned char out[100];
    int bad;

    yasm_xfree(valstr);

    if (test->negate)
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);

    size = yasm_intnum_size_leb128(intn, test->sign);
    if (size != test->outsize) {
        yasm_intnum_destroy(intn);
        sprintf(failmsg, "%ssigned %s%s size() bad size: expected %lu, got %lu!",
                test->sign?"":"un", test->negate?"-":"", test->input,
                test->outsize, size);
        return 1;
    }

    for (i=0; i<sizeof(out); i++)
        out[i] = 0xFF;
    size = yasm_intnum_get_leb128(intn, out, test->sign);
    if (size != test->outsize) {
        yasm_intnum_destroy(intn);
        sprintf(failmsg, "%ssigned %s%s get() bad size: expected %lu, got %lu!",
                test->sign?"":"un", test->negate?"-":"", test->input,
                test->outsize, size);
        return 1;
    }

    bad = 0;
    for (i=0; i<test->outsize && !bad; i++) {
        if (out[i] != test->result[i])
            bad = 1;
    }
    if (bad) {
        yasm_intnum_destroy(intn);
        sprintf(failmsg, "%ssigned %s%s get() bad output!",
                test->sign?"":"un", test->negate?"-":"", test->input);
        return 1;
    }

    yasm_intnum_destroy(intn);
    return 0;
}

static int
run_input_test(Test_Entry *test)
{
    char *valstr = yasm__xstrdup(test->input);
    yasm_intnum *intn = yasm_intnum_create_hex(valstr);
    yasm_intnum *testn;
    unsigned long size;

    yasm_xfree(valstr);

    if (test->negate)
        yasm_intnum_calc(intn, YASM_EXPR_NEG, NULL);

    testn = yasm_intnum_create_leb128(test->result, test->sign, &size);
    if (size != test->outsize) {
        yasm_intnum_destroy(testn);
        yasm_intnum_destroy(intn);
        sprintf(failmsg, "%ssigned %s%s create() bad size: expected %lu, got %lu!",
                test->sign?"":"un", test->negate?"-":"", test->input,
                test->outsize, size);
        return 1;
    }

    yasm_intnum_calc(intn, YASM_EXPR_EQ, testn);
    if (!yasm_intnum_is_pos1(intn)) {
        yasm_intnum_destroy(testn);
        yasm_intnum_destroy(intn);
        sprintf(failmsg, "%ssigned %s%s create() bad output!",
                test->sign?"":"un", test->negate?"-":"", test->input);
        return 1;
    }

    yasm_intnum_destroy(testn);
    yasm_intnum_destroy(intn);
    return 0;
}

int
main(void)
{
    int nf = 0;
    int numtests = sizeof(tests)/sizeof(Test_Entry);
    int i;

    if (BitVector_Boot() != ErrCode_Ok)
        return EXIT_FAILURE;
    yasm_intnum_initialize();

    failed[0] = '\0';
    printf("Test leb128_test: ");
    for (i=0; i<numtests; i++) {
        int fail;

        fail = run_output_test(&tests[i]);
        printf("%c", fail>0 ? 'F':'.');
        fflush(stdout);
        if (fail)
            sprintf(failed, "%s ** F: %s\n", failed, failmsg);
        nf += fail;

        fail = run_input_test(&tests[i]);
        printf("%c", fail>0 ? 'F':'.');
        fflush(stdout);
        if (fail)
            sprintf(failed, "%s ** F: %s\n", failed, failmsg);
        nf += fail;
    }

    yasm_intnum_cleanup();

    printf(" +%d-%d/%d %d%%\n%s",
           numtests*2-nf, nf, numtests*2, 100*(numtests*2-nf)/(numtests*2),
           failed);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
