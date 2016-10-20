/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkHalf.h"
#include "SkOnce.h"
#include "SkOpts.h"

#if defined(SK_ARM_HAS_NEON)
    #if defined(SK_ARM_HAS_CRC32)
        #define SK_OPTS_NS neon_and_crc32
    #else
        #define SK_OPTS_NS neon
    #endif
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
    #define SK_OPTS_NS avx2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
    #define SK_OPTS_NS avx
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
    #define SK_OPTS_NS sse42
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
    #define SK_OPTS_NS sse41
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #define SK_OPTS_NS ssse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE3
    #define SK_OPTS_NS sse3
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #define SK_OPTS_NS sse2
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
    #define SK_OPTS_NS sse
#else
    #define SK_OPTS_NS portable
#endif

#include "SkBlend_opts.h"
#include "SkBlitMask_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlurImageFilter_opts.h"
#include "SkChecksum_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkRasterPipeline_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
#define DEFINE_DEFAULT(name) decltype(name) name = SK_OPTS_NS::name
    DEFINE_DEFAULT(create_xfermode);
    DEFINE_DEFAULT(color_cube_filter_span);

    DEFINE_DEFAULT(box_blur_xx);
    DEFINE_DEFAULT(box_blur_xy);
    DEFINE_DEFAULT(box_blur_yx);

    DEFINE_DEFAULT(dilate_x);
    DEFINE_DEFAULT(dilate_y);
    DEFINE_DEFAULT( erode_x);
    DEFINE_DEFAULT( erode_y);

    DEFINE_DEFAULT(texture_compressor);
    DEFINE_DEFAULT(fill_block_dimensions);

    DEFINE_DEFAULT(blit_mask_d32_a8);

    DEFINE_DEFAULT(blit_row_color32);
    DEFINE_DEFAULT(blit_row_s32a_opaque);

    DEFINE_DEFAULT(RGBA_to_BGRA);
    DEFINE_DEFAULT(RGBA_to_rgbA);
    DEFINE_DEFAULT(RGBA_to_bgrA);
    DEFINE_DEFAULT(RGB_to_RGB1);
    DEFINE_DEFAULT(RGB_to_BGR1);
    DEFINE_DEFAULT(gray_to_RGB1);
    DEFINE_DEFAULT(grayA_to_RGBA);
    DEFINE_DEFAULT(grayA_to_rgbA);
    DEFINE_DEFAULT(inverted_CMYK_to_RGB1);
    DEFINE_DEFAULT(inverted_CMYK_to_BGR1);

    DEFINE_DEFAULT(srcover_srgb_srgb);

    DEFINE_DEFAULT(hash_fn);

    DEFINE_DEFAULT(run_pipeline);
#undef DEFINE_DEFAULT

    SkOpts::VoidFn body[] = {
        (SkOpts::VoidFn)SK_OPTS_NS::just_return,
        (SkOpts::VoidFn)SK_OPTS_NS::swap_src_dst,

        (SkOpts::VoidFn)SK_OPTS_NS::store_565,
        (SkOpts::VoidFn)SK_OPTS_NS::store_srgb,
        (SkOpts::VoidFn)SK_OPTS_NS::store_f16,

        (SkOpts::VoidFn)SK_OPTS_NS::load_s_565,
        (SkOpts::VoidFn)SK_OPTS_NS::load_s_srgb,
        (SkOpts::VoidFn)SK_OPTS_NS::load_s_f16,

        (SkOpts::VoidFn)SK_OPTS_NS::load_d_565,
        (SkOpts::VoidFn)SK_OPTS_NS::load_d_srgb,
        (SkOpts::VoidFn)SK_OPTS_NS::load_d_f16,

        (SkOpts::VoidFn)SK_OPTS_NS::scale_u8,

        (SkOpts::VoidFn)SK_OPTS_NS::lerp_u8,
        (SkOpts::VoidFn)SK_OPTS_NS::lerp_565,
        (SkOpts::VoidFn)SK_OPTS_NS::lerp_constant_float,

        (SkOpts::VoidFn)SK_OPTS_NS::constant_color,

        (SkOpts::VoidFn)SK_OPTS_NS::dst,
        (SkOpts::VoidFn)SK_OPTS_NS::dstatop,
        (SkOpts::VoidFn)SK_OPTS_NS::dstin,
        (SkOpts::VoidFn)SK_OPTS_NS::dstout,
        (SkOpts::VoidFn)SK_OPTS_NS::dstover,
        (SkOpts::VoidFn)SK_OPTS_NS::srcatop,
        (SkOpts::VoidFn)SK_OPTS_NS::srcin,
        (SkOpts::VoidFn)SK_OPTS_NS::srcout,
        (SkOpts::VoidFn)SK_OPTS_NS::srcover,
        (SkOpts::VoidFn)SK_OPTS_NS::clear,
        (SkOpts::VoidFn)SK_OPTS_NS::modulate,
        (SkOpts::VoidFn)SK_OPTS_NS::multiply,
        (SkOpts::VoidFn)SK_OPTS_NS::plus_,
        (SkOpts::VoidFn)SK_OPTS_NS::screen,
        (SkOpts::VoidFn)SK_OPTS_NS::xor_,
        (SkOpts::VoidFn)SK_OPTS_NS::colorburn,
        (SkOpts::VoidFn)SK_OPTS_NS::colordodge,
        (SkOpts::VoidFn)SK_OPTS_NS::darken,
        (SkOpts::VoidFn)SK_OPTS_NS::difference,
        (SkOpts::VoidFn)SK_OPTS_NS::exclusion,
        (SkOpts::VoidFn)SK_OPTS_NS::hardlight,
        (SkOpts::VoidFn)SK_OPTS_NS::lighten,
        (SkOpts::VoidFn)SK_OPTS_NS::overlay,
        (SkOpts::VoidFn)SK_OPTS_NS::softlight,
    };
    static_assert(SK_ARRAY_COUNT(body) == SkRasterPipeline::kNumStockStages, "");

    SkOpts::VoidFn tail[] = {
        (SkOpts::VoidFn)SK_OPTS_NS::just_return_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::swap_src_dst_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::store_565_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::store_srgb_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::store_f16_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::load_s_565_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::load_s_srgb_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::load_s_f16_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::load_d_565_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::load_d_srgb_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::load_d_f16_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::scale_u8_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::lerp_u8_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::lerp_565_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::lerp_constant_float_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::constant_color_tail,

        (SkOpts::VoidFn)SK_OPTS_NS::dst_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::dstatop_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::dstin_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::dstout_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::dstover_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::srcatop_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::srcin_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::srcout_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::srcover_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::clear_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::modulate_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::multiply_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::plus__tail,
        (SkOpts::VoidFn)SK_OPTS_NS::screen_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::xor__tail,
        (SkOpts::VoidFn)SK_OPTS_NS::colorburn_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::colordodge_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::darken_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::difference_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::exclusion_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::hardlight_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::lighten_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::overlay_tail,
        (SkOpts::VoidFn)SK_OPTS_NS::softlight_tail,
    };
    static_assert(SK_ARRAY_COUNT(tail) == SkRasterPipeline::kNumStockStages, "");

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_ssse3();
    void Init_sse41();
    void Init_sse42();
    void Init_avx();
    void Init_hsw();
    void Init_crc32();

    static void init() {
#if !defined(SK_BUILD_NO_OPTS)
    #if defined(SK_CPU_X86)
        if (SkCpu::Supports(SkCpu::SSSE3)) { Init_ssse3(); }
        if (SkCpu::Supports(SkCpu::SSE41)) { Init_sse41(); }
        if (SkCpu::Supports(SkCpu::SSE42)) { Init_sse42(); }
        if (SkCpu::Supports(SkCpu::AVX  )) { Init_avx();   }
        if (SkCpu::Supports(SkCpu::HSW  )) { Init_hsw();   }

    #elif defined(SK_CPU_ARM64)
        if (SkCpu::Supports(SkCpu::CRC32)) { Init_crc32(); }

    #endif
#endif
    }

    void Init() {
        static SkOnce once;
        once(init);
    }
}  // namespace SkOpts
