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

template <bool kF16, bool kCompiled>
class SkRasterPipelineBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        switch ((int)kCompiled << 1 | (int)kF16) {
            case 0: return "SkRasterPipeline_srgb_run";
            case 1: return "SkRasterPipeline_f16_run";
            case 2: return "SkRasterPipeline_srgb_compile";
            case 3: return "SkRasterPipeline_f16_compile";
        }
        return "whoops";
    }

    void onDraw(int loops, SkCanvas*) override {
        void* mask_ctx = mask;
        void*  src_ctx = src;
        void*  dst_ctx = dst;

        SkRasterPipeline p;
        p.append(SkRasterPipeline::load_8888, &src_ctx);
        p.append_from_srgb(kUnpremul_SkAlphaType);
        p.append(SkRasterPipeline::scale_u8, &mask_ctx);
        p.append(SkRasterPipeline::move_src_dst);
        if (kF16) {
            p.append(SkRasterPipeline::load_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::load_8888, &dst_ctx);
            p.append_from_srgb(kPremul_SkAlphaType);
        }
        p.append(SkRasterPipeline::dstover);
        if (kF16) {
            p.append(SkRasterPipeline::store_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::to_srgb);
            p.append(SkRasterPipeline::store_8888, &dst_ctx);
        }

        if (kCompiled) {
            auto compiled = p.compile();
            while (loops --> 0) {
                compiled(0,0, N);
            }
        } else {
            while (loops --> 0) {
                p.run(0,0, N);
            }
        }
    }
};
DEF_BENCH( return (new SkRasterPipelineBench< true,  true>); )
DEF_BENCH( return (new SkRasterPipelineBench<false,  true>); )
DEF_BENCH( return (new SkRasterPipelineBench< true, false>); )
DEF_BENCH( return (new SkRasterPipelineBench<false, false>); )

template <bool kCompiled>
class SkRasterPipelineLegacyBench : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return kCompiled ? "SkRasterPipeline_legacy_compile"
                         : "SkRasterPipeline_legacy_run";
    }

    void onDraw(int loops, SkCanvas*) override {
        void*  src_ctx = src;
        void*  dst_ctx = dst;

        SkRasterPipeline p;
        p.append(SkRasterPipeline::load_8888, &dst_ctx);
        p.append(SkRasterPipeline::move_src_dst);
        p.append(SkRasterPipeline::load_8888, &src_ctx);
        p.append(SkRasterPipeline::srcover);
        p.append(SkRasterPipeline::store_8888, &dst_ctx);

        if (kCompiled) {
            auto compiled = p.compile();
            while (loops --> 0) {
                compiled(0,0, N);
            }
        } else {
            while (loops --> 0) {
                p.run(0,0, N);
            }
        }
    }
};
DEF_BENCH( return (new SkRasterPipelineLegacyBench< true>); )
DEF_BENCH( return (new SkRasterPipelineLegacyBench<false>); )
