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
            p.append(SkOpts::load_s_srgb_body, SkOpts::load_s_srgb_tail, src);
            p.append(SkOpts::scale_u8_body,    SkOpts::scale_u8_tail,    mask);
            p.append(SkOpts::load_d_srgb_body, SkOpts::load_d_srgb_tail, dst);
            p.append(SkOpts::srcover);
            p.append(SkOpts::store_srgb_body,  SkOpts::store_srgb_tail,  dst);
            p.run(N);
        }
    }
};
DEF_BENCH( return new SkRasterPipelineBench; )
