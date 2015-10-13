/*
 * jsimd_arm64.c
 *
 * Copyright 2009 Pierre Ossman <ossman@cendio.se> for Cendio AB
 * Copyright 2009-2011, 2013-2014 D. R. Commander
 *
 * Based on the x86 SIMD extension for IJG JPEG library,
 * Copyright (C) 1999-2006, MIYASAKA Masaru.
 * For conditions of distribution and use, see copyright notice in jsimdext.inc
 *
 * This file contains the interface between the "normal" portions
 * of the library and the SIMD implementations when running on a
 * 64-bit ARM architecture.
 */

#define JPEG_INTERNALS
#include "../jinclude.h"
#include "../jpeglib.h"
#include "../jsimd.h"
#include "../jdct.h"
#include "../jsimddct.h"
#include "jsimd.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

static unsigned int simd_support = ~0;

/*
 * Check what SIMD accelerations are supported.
 *
 * FIXME: This code is racy under a multi-threaded environment.
 */

/* 
 * ARMv8 architectures support NEON extensions by default.
 * It is no longer optional as it was with ARMv7.
 */ 


LOCAL(void)
init_simd (void)
{
  char *env = NULL;

  if (simd_support != ~0U)
    return;

  simd_support = 0;

  simd_support |= JSIMD_ARM_NEON;

  /* Force different settings through environment variables */
  env = getenv("JSIMD_FORCENEON");
  if ((env != NULL) && (strcmp(env, "1") == 0))
    simd_support &= JSIMD_ARM_NEON;
  env = getenv("JSIMD_FORCENONE");
  if ((env != NULL) && (strcmp(env, "1") == 0))
    simd_support = 0;
}

GLOBAL(int)
jsimd_can_rgb_ycc (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_rgb_gray (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_ycc_rgb (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;
  if ((RGB_PIXELSIZE != 3) && (RGB_PIXELSIZE != 4))
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(int)
jsimd_can_ycc_rgb565 (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(void)
jsimd_rgb_ycc_convert (j_compress_ptr cinfo,
                       JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
                       JDIMENSION output_row, int num_rows)
{
}

GLOBAL(void)
jsimd_rgb_gray_convert (j_compress_ptr cinfo,
                        JSAMPARRAY input_buf, JSAMPIMAGE output_buf,
                        JDIMENSION output_row, int num_rows)
{
}

GLOBAL(void)
jsimd_ycc_rgb_convert (j_decompress_ptr cinfo,
                       JSAMPIMAGE input_buf, JDIMENSION input_row,
                       JSAMPARRAY output_buf, int num_rows)
{
  void (*neonfct)(JDIMENSION, JSAMPIMAGE, JDIMENSION, JSAMPARRAY, int);

  switch(cinfo->out_color_space) {
    case JCS_EXT_RGB:
      neonfct=jsimd_ycc_extrgb_convert_neon;
      break;
    case JCS_EXT_RGBX:
    case JCS_EXT_RGBA:
      neonfct=jsimd_ycc_extrgbx_convert_neon;
      break;
    case JCS_EXT_BGR:
      neonfct=jsimd_ycc_extbgr_convert_neon;
      break;
    case JCS_EXT_BGRX:
    case JCS_EXT_BGRA:
      neonfct=jsimd_ycc_extbgrx_convert_neon;
      break;
    case JCS_EXT_XBGR:
    case JCS_EXT_ABGR:
      neonfct=jsimd_ycc_extxbgr_convert_neon;
      break;
    case JCS_EXT_XRGB:
    case JCS_EXT_ARGB:
      neonfct=jsimd_ycc_extxrgb_convert_neon;
      break;
    default:
      neonfct=jsimd_ycc_extrgb_convert_neon;
      break;
  }

  if (simd_support & JSIMD_ARM_NEON)
    neonfct(cinfo->output_width, input_buf, input_row, output_buf, num_rows);
}

GLOBAL(void)
jsimd_ycc_rgb565_convert (j_decompress_ptr cinfo,
                          JSAMPIMAGE input_buf, JDIMENSION input_row,
                          JSAMPARRAY output_buf, int num_rows)
{
  if (simd_support & JSIMD_ARM_NEON)
    jsimd_ycc_rgb565_convert_neon(cinfo->output_width, input_buf, input_row,
                                  output_buf, num_rows);
}

GLOBAL(int)
jsimd_can_h2v2_downsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_h2v1_downsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_h2v2_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
                       JSAMPARRAY input_data, JSAMPARRAY output_data)
{
}

GLOBAL(void)
jsimd_h2v1_downsample (j_compress_ptr cinfo, jpeg_component_info * compptr,
                       JSAMPARRAY input_data, JSAMPARRAY output_data)
{
}

GLOBAL(int)
jsimd_can_h2v2_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_h2v1_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_h2v2_upsample (j_decompress_ptr cinfo,
                     jpeg_component_info * compptr,
                     JSAMPARRAY input_data,
                     JSAMPARRAY * output_data_ptr)
{
}

GLOBAL(void)
jsimd_h2v1_upsample (j_decompress_ptr cinfo,
                     jpeg_component_info * compptr,
                     JSAMPARRAY input_data,
                     JSAMPARRAY * output_data_ptr)
{
}

GLOBAL(int)
jsimd_can_h2v2_fancy_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_h2v1_fancy_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_h2v2_fancy_upsample (j_decompress_ptr cinfo,
                           jpeg_component_info * compptr,
                           JSAMPARRAY input_data,
                           JSAMPARRAY * output_data_ptr)
{
}

GLOBAL(void)
jsimd_h2v1_fancy_upsample (j_decompress_ptr cinfo,
                           jpeg_component_info * compptr,
                           JSAMPARRAY input_data,
                           JSAMPARRAY * output_data_ptr)
{
}

GLOBAL(int)
jsimd_can_h2v2_merged_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_h2v1_merged_upsample (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_h2v2_merged_upsample (j_decompress_ptr cinfo,
                            JSAMPIMAGE input_buf,
                            JDIMENSION in_row_group_ctr,
                            JSAMPARRAY output_buf)
{
}

GLOBAL(void)
jsimd_h2v1_merged_upsample (j_decompress_ptr cinfo,
                            JSAMPIMAGE input_buf,
                            JDIMENSION in_row_group_ctr,
                            JSAMPARRAY output_buf)
{
}

GLOBAL(int)
jsimd_can_convsamp (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_convsamp_float (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_convsamp (JSAMPARRAY sample_data, JDIMENSION start_col,
                DCTELEM * workspace)
{
}

GLOBAL(void)
jsimd_convsamp_float (JSAMPARRAY sample_data, JDIMENSION start_col,
                      FAST_FLOAT * workspace)
{
}

GLOBAL(int)
jsimd_can_fdct_islow (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_fdct_ifast (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_fdct_float (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_fdct_islow (DCTELEM * data)
{
}

GLOBAL(void)
jsimd_fdct_ifast (DCTELEM * data)
{
}

GLOBAL(void)
jsimd_fdct_float (FAST_FLOAT * data)
{
}

GLOBAL(int)
jsimd_can_quantize (void)
{
  init_simd();

  return 0;
}

GLOBAL(int)
jsimd_can_quantize_float (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_quantize (JCOEFPTR coef_block, DCTELEM * divisors,
                DCTELEM * workspace)
{
}

GLOBAL(void)
jsimd_quantize_float (JCOEFPTR coef_block, FAST_FLOAT * divisors,
                      FAST_FLOAT * workspace)
{
}

GLOBAL(int)
jsimd_can_idct_2x2 (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (DCTSIZE != 8)
    return 0;
  if (sizeof(JCOEF) != 2)
    return 0;
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;
  if (sizeof(ISLOW_MULT_TYPE) != 2)
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(int)
jsimd_can_idct_4x4 (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (DCTSIZE != 8)
    return 0;
  if (sizeof(JCOEF) != 2)
    return 0;
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;
  if (sizeof(ISLOW_MULT_TYPE) != 2)
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(void)
jsimd_idct_2x2 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col)
{
  if (simd_support & JSIMD_ARM_NEON)
    jsimd_idct_2x2_neon(compptr->dct_table, coef_block, output_buf,
                        output_col);
}

GLOBAL(void)
jsimd_idct_4x4 (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                JCOEFPTR coef_block, JSAMPARRAY output_buf,
                JDIMENSION output_col)
{
  if (simd_support & JSIMD_ARM_NEON)
    jsimd_idct_4x4_neon(compptr->dct_table, coef_block, output_buf,
                        output_col);
}

GLOBAL(int)
jsimd_can_idct_islow (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (DCTSIZE != 8)
    return 0;
  if (sizeof(JCOEF) != 2)
    return 0;
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;
  if (sizeof(ISLOW_MULT_TYPE) != 2)
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(int)
jsimd_can_idct_ifast (void)
{
  init_simd();

  /* The code is optimised for these values only */
  if (DCTSIZE != 8)
    return 0;
  if (sizeof(JCOEF) != 2)
    return 0;
  if (BITS_IN_JSAMPLE != 8)
    return 0;
  if (sizeof(JDIMENSION) != 4)
    return 0;
  if (sizeof(IFAST_MULT_TYPE) != 2)
    return 0;
  if (IFAST_SCALE_BITS != 2)
    return 0;

  if (simd_support & JSIMD_ARM_NEON)
    return 1;

  return 0;
}

GLOBAL(int)
jsimd_can_idct_float (void)
{
  init_simd();

  return 0;
}

GLOBAL(void)
jsimd_idct_islow (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                  JCOEFPTR coef_block, JSAMPARRAY output_buf,
                  JDIMENSION output_col)
{
  if (simd_support & JSIMD_ARM_NEON)
    jsimd_idct_islow_neon(compptr->dct_table, coef_block, output_buf,
                          output_col);
}

GLOBAL(void)
jsimd_idct_ifast (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                  JCOEFPTR coef_block, JSAMPARRAY output_buf,
                  JDIMENSION output_col)
{
  if (simd_support & JSIMD_ARM_NEON)
    jsimd_idct_ifast_neon(compptr->dct_table, coef_block, output_buf,
                          output_col);
}

GLOBAL(void)
jsimd_idct_float (j_decompress_ptr cinfo, jpeg_component_info * compptr,
                  JCOEFPTR coef_block, JSAMPARRAY output_buf,
                  JDIMENSION output_col)
{
}
