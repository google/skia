/* Copyright (c) 2011,  NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of the NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define JPEG_INTERNALS
#include "jinclude.h"
#include "jpeglib.h"
#include "jdct.h"
#include <machine/cpu-features.h>


#if defined(NV_ARM_NEON) && defined(__ARM_HAVE_NEON)

EXTERN(void) jsimd_ycc_rgba8888_convert_neon
        JPP((JDIMENSION out_width,
             JSAMPIMAGE input_buf, JDIMENSION input_row,
             JSAMPARRAY output_buf, int num_rows));
EXTERN(void) jsimd_ycc_rgb565_convert_neon
        JPP((JDIMENSION out_width,
             JSAMPIMAGE input_buf, JDIMENSION input_row,
             JSAMPARRAY output_buf, int num_rows));

EXTERN(void) jsimd_idct_ifast_neon JPP((void * dct_table,
                                        JCOEFPTR coef_block,
                                        JSAMPARRAY output_buf,
                                        JDIMENSION output_col));

EXTERN(void) jsimd_idct_2x2_neon JPP((void * dct_table,
                                        JCOEFPTR coef_block,
                                        JSAMPARRAY output_buf,
                                        JDIMENSION output_col));

EXTERN(void) jsimd_idct_4x4_neon JPP((void * dct_table,
                                        JCOEFPTR coef_block,
                                        JSAMPARRAY output_buf,
                                        JDIMENSION output_col));

GLOBAL(void)
jsimd_ycc_rgba8888_convert (j_decompress_ptr cinfo,
                       JSAMPIMAGE input_buf, JDIMENSION input_row,
                       JSAMPARRAY output_buf, int num_rows)
{
    void (*neonfct)(JDIMENSION, JSAMPIMAGE, JDIMENSION, JSAMPARRAY, int);

    neonfct=jsimd_ycc_rgba8888_convert_neon;

    neonfct(cinfo->output_width, input_buf,
        input_row, output_buf, num_rows);
}

GLOBAL(void)
jsimd_ycc_rgb565_convert (j_decompress_ptr cinfo,
                       JSAMPIMAGE input_buf, JDIMENSION input_row,
                       JSAMPARRAY output_buf, int num_rows)
{
    void (*neonfct)(JDIMENSION, JSAMPIMAGE, JDIMENSION, JSAMPARRAY, int);

    neonfct=jsimd_ycc_rgb565_convert_neon;

    neonfct(cinfo->output_width, input_buf,
        input_row, output_buf, num_rows);
}

GLOBAL(void)
jsimd_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col)
{
    jsimd_idct_ifast_neon(compptr->dct_table, coef_block, output_buf, output_col);
}


GLOBAL(void)
jsimd_idct_2x2 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col)
{
    jsimd_idct_2x2_neon(compptr->dct_table, coef_block, output_buf, output_col);
}

GLOBAL(void)
jsimd_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col)
{
    jsimd_idct_4x4_neon(compptr->dct_table, coef_block, output_buf, output_col);
}


GLOBAL(int)
cap_neon_idct_2x2 (void)
{
  if (  (DCTSIZE != 8)              ||
        (sizeof(JCOEF) != 2)        ||
        (BITS_IN_JSAMPLE != 8)      ||
        (sizeof(JDIMENSION) != 4)   ||
        (sizeof(ISLOW_MULT_TYPE) != 2))
    return 0;

    return 1;
}

GLOBAL(int)
cap_neon_idct_4x4 (void)
{

  if (  (DCTSIZE != 8)              ||
        (sizeof(JCOEF) != 2)        ||
        (BITS_IN_JSAMPLE != 8)      ||
        (sizeof(JDIMENSION) != 4)   ||
        (sizeof(ISLOW_MULT_TYPE) != 2))
    return 0;

    return 1;
}

GLOBAL(int)
cap_neon_idct_ifast (void)
{

  if (  (DCTSIZE != 8)                  ||
        (sizeof(JCOEF) != 2)            ||
        (BITS_IN_JSAMPLE != 8)          ||
        (sizeof(JDIMENSION) != 4)       ||
        (sizeof(IFAST_MULT_TYPE) != 2)  ||
        (IFAST_SCALE_BITS != 2))
    return 0;

    return 1;

}

GLOBAL(int)
cap_neon_ycc_rgb (void)
{

  if(   (BITS_IN_JSAMPLE != 8)                          ||
        (sizeof(JDIMENSION) != 4)                       ||
        ((RGB_PIXELSIZE != 3) && (RGB_PIXELSIZE != 4)))
    return 0;

    return 1;
}

#endif


