/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"


#define SK_OPTS_NS hsw
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_hsw() {

// The 32-bit MSVC __vectorcall ABI mangles type information into the names of
// SkOpts::body, SkOpts::tail, and SkOpts::run_pipeline, so that this code will
// not link as written: they're all defined in a file where SkRasterPipeline::V
// is Sk4f, but here we're seeing it as Sk8f.
//
// We can work around this by storing those pointers as some generic function
// pointer type like void(*)(), but it's even simpler to just not do any of this
// when targeting 32-bit Windows.
#if !defined(_M_IX86)

        run_pipeline = SK_OPTS_NS::run_pipeline;

    #define STAGE(stage)                                        \
        body[SkRasterPipeline::stage] = SK_OPTS_NS::stage;      \
        tail[SkRasterPipeline::stage] = SK_OPTS_NS::stage##_tail

        STAGE(store_565);
        STAGE(store_srgb);
        STAGE(store_f16);

        STAGE(load_s_565);
        STAGE(load_s_srgb);
        STAGE(load_s_f16);

        STAGE(load_d_565);
        STAGE(load_d_srgb);
        STAGE(load_d_f16);

        STAGE(scale_u8);

        STAGE(lerp_u8);
        STAGE(lerp_565);
    #undef STAGE

    #define STAGE(stage) \
        body[SkRasterPipeline::stage] = SK_OPTS_NS::stage; \
        tail[SkRasterPipeline::stage] = SK_OPTS_NS::stage

        STAGE(lerp_constant_float);
        STAGE(constant_color);

        STAGE(dst);
        STAGE(dstatop);
        STAGE(dstin);
        STAGE(dstout);
        STAGE(dstover);
        STAGE(srcatop);
        STAGE(srcin);
        STAGE(srcout);
        STAGE(srcover);
        STAGE(clear);
        STAGE(modulate);
        STAGE(multiply);
        STAGE(plus_);
        STAGE(screen);
        STAGE(xor_);
        STAGE(colorburn);
        STAGE(colordodge);
        STAGE(darken);
        STAGE(difference);
        STAGE(exclusion);
        STAGE(hardlight);
        STAGE(lighten);
        STAGE(overlay);
        STAGE(softlight);
    #undef STAGE

#endif // !defined(_M_IX86)
    }
}

