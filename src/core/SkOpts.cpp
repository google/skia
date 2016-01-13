/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOnce.h"
#include "SkOpts.h"

#define SK_OPTS_NS sk_default
#include "SkBlitMask_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlurImageFilter_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkFloatingPoint_opts.h"
#include "SkMatrix_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkUtils_opts.h"
#include "SkXfermode_opts.h"

#if defined(SK_CPU_X86) && !defined(SK_BUILD_FOR_IOS)
    #if defined(SK_BUILD_FOR_WIN32)
        #include <intrin.h>
        static void cpuid (uint32_t abcd[4]) { __cpuid  ((int*)abcd, 1);    }
        static void cpuid7(uint32_t abcd[4]) { __cpuidex((int*)abcd, 7, 0); }
        static uint64_t xgetbv(uint32_t xcr) { return _xgetbv(xcr); }
    #else
        #include <cpuid.h>
        #if !defined(__cpuid_count)  // Old Mac Clang doesn't have this defined.
            #define  __cpuid_count(eax, ecx, a, b, c, d) \
                __asm__("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "0"(eax), "2"(ecx))
        #endif
        static void cpuid (uint32_t abcd[4]) { __get_cpuid(1, abcd+0, abcd+1, abcd+2, abcd+3); }
        static void cpuid7(uint32_t abcd[4]) {
            __cpuid_count(7, 0, abcd[0], abcd[1], abcd[2], abcd[3]);
        }
        static uint64_t xgetbv(uint32_t xcr) {
            uint32_t eax, edx;
            __asm__ __volatile__ ( "xgetbv" : "=a"(eax), "=d"(edx) : "c"(xcr));
            return (uint64_t)(edx) << 32 | eax;
        }
    #endif
#elif !defined(SK_ARM_HAS_NEON)      && \
       defined(SK_CPU_ARM32)         && \
       defined(SK_BUILD_FOR_ANDROID) && \
      !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
    #include <cpu-features.h>
#endif

namespace SkOpts {
    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
    decltype(rsqrt)                     rsqrt = sk_default::rsqrt;
    decltype(memset16)               memset16 = sk_default::memset16;
    decltype(memset32)               memset32 = sk_default::memset32;
    decltype(create_xfermode) create_xfermode = sk_default::create_xfermode;
    decltype(color_cube_filter_span) color_cube_filter_span = sk_default::color_cube_filter_span;

    decltype(box_blur_xx) box_blur_xx = sk_default::box_blur_xx;
    decltype(box_blur_xy) box_blur_xy = sk_default::box_blur_xy;
    decltype(box_blur_yx) box_blur_yx = sk_default::box_blur_yx;

    decltype(dilate_x) dilate_x = sk_default::dilate_x;
    decltype(dilate_y) dilate_y = sk_default::dilate_y;
    decltype( erode_x)  erode_x = sk_default::erode_x;
    decltype( erode_y)  erode_y = sk_default::erode_y;

    decltype(texture_compressor)       texture_compressor = sk_default::texture_compressor;
    decltype(fill_block_dimensions) fill_block_dimensions = sk_default::fill_block_dimensions;

    decltype(blit_mask_d32_a8) blit_mask_d32_a8 = sk_default::blit_mask_d32_a8;

    decltype(blit_row_color32) blit_row_color32 = sk_default::blit_row_color32;

    decltype(matrix_translate)       matrix_translate       = sk_default::matrix_translate;
    decltype(matrix_scale_translate) matrix_scale_translate = sk_default::matrix_scale_translate;
    decltype(matrix_affine)          matrix_affine          = sk_default::matrix_affine;

    decltype(       premul_xxxa)        premul_xxxa = sk_default::       premul_xxxa;
    decltype(       swaprb_xxxa)        swaprb_xxxa = sk_default::       swaprb_xxxa;
    decltype(premul_swaprb_xxxa) premul_swaprb_xxxa = sk_default::premul_swaprb_xxxa;

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_ssse3();
    void Init_sse41();
    void Init_sse42() {}
    void Init_avx();
    void Init_avx2() {}
    void Init_neon();

    static void init() {
        // TODO: Chrome's not linking _sse* opts on iOS simulator builds.  Bug or feature?
    #if defined(SK_CPU_X86) && !defined(SK_BUILD_FOR_IOS)
        uint32_t abcd[] = {0,0,0,0};
        cpuid(abcd);
        if (abcd[2] & (1<< 9)) { Init_ssse3(); }
        if (abcd[2] & (1<<19)) { Init_sse41(); }
        if (abcd[2] & (1<<20)) { Init_sse42(); }

        // AVX detection's kind of a pain.  This is cribbed from Chromium.
        if ( (  abcd[2] & (7<<26)) == (7<<26) &&    // Check bits 26-28 of ecx are all set,
             (xgetbv(0) & 6      ) == 6          ){ // and  check the OS supports XSAVE.
            Init_avx();

            // AVX2 additionally needs bit 5 set on ebx after calling cpuid(7).
            uint32_t abcd7[] = {0,0,0,0};
            cpuid7(abcd7);
            if (abcd7[1] & (1<<5)) { Init_avx2(); }
        }

    #elif !defined(SK_ARM_HAS_NEON)      && \
           defined(SK_CPU_ARM32)         && \
           defined(SK_BUILD_FOR_ANDROID) && \
          !defined(SK_BUILD_FOR_ANDROID_FRAMEWORK)
        if (android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) { Init_neon(); }
    #endif
    }

    SK_DECLARE_STATIC_ONCE(gInitOnce);
    void Init() { SkOnce(&gInitOnce, init); }

#if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS
    static struct AutoInit {
        AutoInit() { Init(); }
    } gAutoInit;
#endif
}
