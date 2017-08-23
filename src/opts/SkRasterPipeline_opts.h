/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include "SkNx.h"

namespace SK_OPTS_NS {

#if defined(__AVX__)
    static const int N = 8;
#else
    static const int N = 4;
#endif

    using F   = SkNx<N,   float>;
    using U32 = SkNx<N,uint32_t>;
    using I32 = SkNx<N, int32_t>;

    using Stage = void(*)(void** program, F x, F y);

    static inline void foo(void** program, F x, F y) {
        x *= y;
        auto next = (Stage)*program++;
        next(program, x,y);
    }

    static inline void bar(void** program, F x, F y) {
        x /= y;
        auto next = (Stage)*program++;
        next(program, x,y);
    }

#define M(st) +1
    static const std::array<void(*)(), SK_RASTER_PIPELINE_STAGES(M)> raster_pipeline_stages = {{
#undef M
        (void(*)())foo,
        (void(*)())bar,
    }};

}

#endif//SkRasterPipeline_opts_DEFINED
