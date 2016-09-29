/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"

static const int N = 1023;

static uint32_t dst[N],
                src[N];
static uint8_t mask[N];

// We'll build up a somewhat realistic useful pipeline:
//   - load srgb src
//   - scale src by 8-bit mask
//   - load srgb dst
//   - src = srcover(dst, src)
//   - store src back as srgb

class SkRasterPipelineBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override { return "SkRasterPipeline"; }

    void onDraw(int loops, SkCanvas*) override {
        while (loops --> 0) {
            SkRasterPipeline p;
            p.append(SkRasterPipeline::load_s_srgb, src);
            p.append(SkRasterPipeline::   scale_u8, mask);
            p.append(SkRasterPipeline::load_d_srgb, dst);
            p.append(SkRasterPipeline::    srcover);
            p.append(SkRasterPipeline:: store_srgb, dst);
            p.run(N);
        }
    }
};
DEF_BENCH( return new SkRasterPipelineBench; )
