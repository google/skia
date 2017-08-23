/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

#include <immintrin.h>

namespace SK_OPTS_NS {

#if defined(__AVX__)
    using F = float __attribute__((ext_vector_type(8)));
#else
    using F = float;
    //using F = float __attribute__((ext_vector_type(4)));
#endif

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

    using Thunk = void(*)(void);
    static const Thunk raster_pipeline_stages[] = {
        (Thunk)foo,
        (Thunk)bar,
    };
}

#endif//SkRasterPipeline_opts_DEFINED
