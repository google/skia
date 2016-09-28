/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS avx
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_avx() {
    #define STAGE(stage, kCallNext) \
        stage = body<SK_OPTS_NS::stage, kCallNext>

        STAGE(srcover, true);
        STAGE(constant_color, true);
        STAGE(lerp_constant_float, true);
    #undef STAGE

    #define STAGE(stage, kCallNext) \
        stage##_body = body<SK_OPTS_NS::stage, kCallNext>; \
        stage##_tail = tail<SK_OPTS_NS::stage, kCallNext>

        STAGE(load_d_srgb, true);
        STAGE(load_s_srgb, true);
        STAGE( store_srgb, false);

        STAGE(load_d_f16, true);
        STAGE(load_s_f16, true);
        STAGE( store_f16, false);

        STAGE(load_d_565, true);
        STAGE(load_s_565, true);
        STAGE( store_565, false);

        STAGE(scale_u8, true);

        STAGE(lerp_u8,  true);
        STAGE(lerp_565, true);
    #undef STAGE
    }
}
