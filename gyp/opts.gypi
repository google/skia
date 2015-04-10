# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
        'none_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_none.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_none.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_none.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_none.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_none.cpp',
        ],

        'armv7_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_arm.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_arm.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_arm.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_arm.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_arm.cpp',
        ],
        'neon_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_matrixProcs_neon.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_neon.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_neon.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_neon.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_arm_neon.cpp',
        ],
        'arm64_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_matrixProcs_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_neon.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_arm.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_neon.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_none.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_none.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_arm.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_arm_neon.cpp',
        ],

        'mips_dsp_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_mips_dsp.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_mips_dsp.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_none.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_none.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_none.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_none.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_none.cpp',
        ],

        'sse2_sources': [
            '<(skia_src_path)/opts/SkBitmapFilter_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkBlurImage_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkMorphology_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkTextureCompression_opts_none.cpp',
            '<(skia_src_path)/opts/SkUtils_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkXfermode_opts_SSE2.cpp',
            '<(skia_src_path)/opts/opts_check_x86.cpp',
        ],
        'ssse3_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_SSSE3.cpp',
        ],
        'sse41_sources': [
            '<(skia_src_path)/opts/SkBlurImage_opts_SSE4.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_SSE4.cpp',
        ],
}
