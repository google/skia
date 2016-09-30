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
    #define STAGE(stage, kCallNext) \
        stages_8  [SkRasterPipeline::stage] = stage_8  <SK_OPTS_NS::stage, kCallNext>; \
        stages_1_7[SkRasterPipeline::stage] = stage_1_7<SK_OPTS_NS::stage, kCallNext>

        STAGE(constant_color, true);
        STAGE(lerp_constant_float , true);
        STAGE(srcover , true);

        STAGE(scale_u8, true);

        STAGE(load_d_srgb, true);
        STAGE(load_s_srgb, true);

    #undef STAGE
    }
}

void SK_VECTORCALL SkRasterPipeline::JustReturn8(Stage*, size_t, size_t, Sk8f,Sk8f,Sk8f,Sk8f,
                                                                         Sk8f,Sk8f,Sk8f,Sk8f) {}

bool SkRasterPipeline::run8(size_t x, size_t n) {
    if (fBody8Start == nullptr || fTail8Start == nullptr) {
        return false;
    }
    printf("\n");
    printf("%p %p\n", fBody8Start, fTail8Start);
    for (int i = 0; i < fBody8.count(); i++) {
        if (fBody8[i].fNext == nullptr || fTail8[i].fNext == nullptr) {
            return false;
        }
        printf("%p %p\n", fBody8[i].fNext, fTail8[i].fNext);
    }
    printf("\n");

    Sk8f v;

    while (n >= 8) {
        ((Fn8)fBody8Start)(fBody8.begin(), x,0, v,v,v,v, v,v,v,v);
        x += 8;
        n -= 8;
    }
    if (n > 0) {
        ((Fn8)fTail8Start)(fTail8.begin(), x,n, v,v,v,v, v,v,v,v);
    }
    return true;
}
