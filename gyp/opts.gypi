# Copyright 2015 Google Inc.
#
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
{
        'none_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_none.cpp',
        ],

        'armv7_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm.cpp',
        ],
        'neon_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_matrixProcs_neon.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm_neon.cpp',
        ],
        'arm64_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_matrixProcs_neon.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_arm_neon.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_arm_neon.cpp',
        ],
        'crc32_sources': [
            '<(skia_src_path)/opts/SkOpts_crc32.cpp',
        ],

        'mips_dsp_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_mips_dsp.cpp',
            '<(skia_src_path)/opts/SkBlitMask_opts_none.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_mips_dsp.cpp',
        ],

        'sse2_sources': [
            '<(skia_src_path)/opts/SkBitmapFilter_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkBitmapProcState_opts_SSE2.cpp',
            '<(skia_src_path)/opts/SkBlitRow_opts_SSE2.cpp',
            '<(skia_src_path)/opts/opts_check_x86.cpp',
        ],
        'ssse3_sources': [
            '<(skia_src_path)/opts/SkBitmapProcState_opts_SSSE3.cpp',
            '<(skia_src_path)/opts/SkOpts_ssse3.cpp',
        ],
        'sse41_sources': [
            '<(skia_src_path)/opts/SkOpts_sse41.cpp',
        ],
        'sse42_sources': [
            '<(skia_src_path)/opts/SkOpts_sse42.cpp',
        ],
        'avx_sources': [
            '<(skia_src_path)/opts/SkOpts_avx.cpp',
        ],
        'hsw_sources': [
            '<(skia_src_path)/opts/SkOpts_hsw.cpp',
        ],
}
