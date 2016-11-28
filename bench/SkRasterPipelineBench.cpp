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

static uint64_t dst[N];  // sRGB or F16
static uint32_t src[N];  // sRGB
static uint8_t mask[N];  // 8-bit linear

// We'll build up a somewhat realistic useful pipeline:
//   - load srgb src
//   - scale src by 8-bit mask
//   - load srgb/f16 dst
//   - src = srcover(dst, src)
//   - store src back as srgb/f16

template <bool kF16>
class SkRasterPipelineBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return kF16 ? "SkRasterPipeline_f16"
                    : "SkRasterPipeline_srgb";
    }

    void onDraw(int loops, SkCanvas*) override {
        void* mask_ctx = mask;
        void*  src_ctx = src;
        void*  dst_ctx = dst;

        SkRasterPipeline p;
        p.append(SkRasterPipeline::load_s_8888, &src_ctx);
        p.append(SkRasterPipeline::from_srgb_s);
        p.append(SkRasterPipeline::scale_u8, &mask_ctx);
        if (kF16) {
            p.append(SkRasterPipeline::load_d_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::load_d_8888, &dst_ctx);
            p.append(SkRasterPipeline::from_srgb_d);
        }
        p.append(SkRasterPipeline::srcover);
        if (kF16) {
            p.append(SkRasterPipeline::store_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::to_srgb);
            p.append(SkRasterPipeline::store_8888, &dst_ctx);
        }
        auto compiled = p.compile();

        while (loops --> 0) {
            compiled(0,0, N);
        }
    }
};
DEF_BENCH( return new SkRasterPipelineBench<true>; )
DEF_BENCH( return new SkRasterPipelineBench<false>; )
