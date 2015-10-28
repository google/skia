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

#ifndef __JSIMD_NEON_H__
#define __JSIMD_NEON_H__
#include "jinclude.h"
#include "jpeglib.h"
#include <machine/cpu-features.h>

#if defined(NV_ARM_NEON) && defined(__ARM_HAVE_NEON)

#if defined(ANDROID_ARMV6_IDCT)
#error "ANDROID_ARMV6_IDCT and NV_ARM_NEON are muturaly exclusive modes, both have been defined"
#endif

GLOBAL(void)
jsimd_idct_2x2 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col);

GLOBAL(void)
jsimd_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col);

GLOBAL(void)
jsimd_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col);

GLOBAL(int)
cap_neon_idct_2x2 (void);

GLOBAL(int)
cap_neon_idct_4x4 (void);

GLOBAL(int)
cap_neon_idct_ifast (void);

GLOBAL(int)
cap_neon_ycc_rgb (void);

GLOBAL(void)
jsimd_ycc_rgb565_convert (j_decompress_ptr cinfo,
                       JSAMPIMAGE input_buf, JDIMENSION input_row,
                       JSAMPARRAY output_buf, int num_rows);

GLOBAL(void)
jsimd_ycc_rgba8888_convert (j_decompress_ptr cinfo,
                       JSAMPIMAGE input_buf, JDIMENSION input_row,
                       JSAMPARRAY output_buf, int num_rows);
#endif

#endif
