/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipeline_opts_DEFINED
#define SkRasterPipeline_opts_DEFINED

namespace SK_OPTS_NS {

    static void start_pipeline(size_t x, size_t y, size_t w, size_t h, void** program) {
        // TODO
    }

    static void start_pipeline_lowp(size_t x, size_t y, size_t w, size_t h, void** program) {
        // TODO
    }

    static void(*just_return     )() = nullptr;
    static void(*just_return_lowp)() = nullptr;

    #define M(st) static void(*st)() = nullptr; static void(*st##_lowp)() = nullptr;
        SK_RASTER_PIPELINE_STAGES(M)
    #undef M
}

#endif//SkRasterPipeline_opts_DEFINED

