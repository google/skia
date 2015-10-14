/*
 *
 *  Copyright (C) 2001-2007  Peter Johnson
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

#include "libyasm/bitvect.h"

static int
test_boot(void)
{
    if (BitVector_Boot() != ErrCode_Ok)
        return 1;
    return 0;
}

typedef struct Val_s {
    const char *ascii;
    unsigned char result[10];   /* 80 bit result, little endian */
} Val;

Val oct_small_vals[] = {
    {   "0",
        {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
    {   "1",
        {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
    {   "77",
        {0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
};

Val oct_large_vals[] = {
    {   "7654321076543210",
        {0x88, 0xC6, 0xFA, 0x88, 0xC6, 0xFA, 0x00, 0x00, 0x00, 0x00}
    },
    {   "12634727612534126530214",
        {0x8C, 0xB0, 0x5A, 0xE1, 0xAA, 0xF8, 0x3A, 0x67, 0x05, 0x00}
    },
    {   "61076543210",
        {0x88, 0xC6, 0xFA, 0x88, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00}
    },
};

wordptr testval;

static void
num_family_setup(void)
{
    BitVector_Boot();
    testval = BitVector_Create(80, FALSE);
}

static void
num_family_teardown(void)
{
    BitVector_Destroy(testval);
}

static char result_msg[1024];

static int
num_check(Val *val)
{
    unsigned char ascii[64], *result;
    unsigned int len;
    int i;
    int ret = 0;

    strcpy((char *)ascii, val->ascii);
    strcpy(result_msg, "parser failure");
    if(BitVector_from_Oct(testval, ascii) != ErrCode_Ok)
        return 1;

    result = BitVector_Block_Read(testval, &len);

    for (i=0; i<10; i++)
        if (result[i] != val->result[i])
            ret = 1;

    if (ret) {
        strcpy(result_msg, val->ascii);
        for (i=0; i<10; i++)
            sprintf((char *)ascii+3*i, "%02x ", result[i]);
        strcat(result_msg, ": ");
        strcat(result_msg, (char *)ascii);
    }
    free(result);
    
    return ret;
}

static int
test_oct_small_num(void)
{
    Val *vals = oct_small_vals;
    int i, num = sizeof(oct_small_vals)/sizeof(Val);

    for (i=0; i<num; i++) {
        if (num_check(&vals[i]) != 0)
            return 1;
    }
    return 0;
}

static int
test_oct_large_num(void)
{
    Val *vals = oct_large_vals;
    int i, num = sizeof(oct_large_vals)/sizeof(Val);

    for (i=0; i<num; i++) {
        if (num_check(&vals[i]) != 0)
            return 1;
    }
    return 0;
}

char failed[1000];

static int
runtest_(const char *testname, int (*testfunc)(void), void (*setup)(void),
         void (*teardown)(void))
{
    int nf;
    if (setup)
        setup();
    nf = testfunc();
    if (teardown)
        teardown();
    printf("%c", nf>0 ? 'F':'.');
    fflush(stdout);
    if (nf > 0)
        sprintf(failed, "%s ** F: %s failed!\n", failed, testname);
    return nf;
}
#define runtest(x,y,z)  runtest_(#x,test_##x,y,z)

int
main(void)
{
    int nf = 0;

    failed[0] = '\0';
    printf("Test bitvect_test: ");
    nf += runtest(boot, NULL, NULL);
    nf += runtest(oct_small_num, num_family_setup, num_family_teardown);
    nf += runtest(oct_large_num, num_family_setup, num_family_teardown);
    printf(" +%d-%d/3 %d%%\n%s",
           3-nf, nf, 100*(3-nf)/3, failed);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
