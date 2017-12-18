/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"
#include "../src/jumper/SkJumper.h"

static const int N = 15;

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
        switch ((int)kF16) {
            case 0: return "SkRasterPipeline_srgb";
            case 1: return "SkRasterPipeline_f16";
        }
        return "whoops";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkJumper_MemoryCtx mask_ctx = {mask, 0},
                            src_ctx = {src,  0},
                            dst_ctx = {dst,  0};

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_8888, &src_ctx);
        p.append(SkRasterPipeline::from_srgb);
        p.append(SkRasterPipeline::scale_u8, &mask_ctx);
        p.append(SkRasterPipeline::move_src_dst);
        if (kF16) {
            p.append(SkRasterPipeline::load_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::load_8888, &dst_ctx);
            p.append(SkRasterPipeline::from_srgb);
        }
        p.append(SkRasterPipeline::dstover);
        if (kF16) {
            p.append(SkRasterPipeline::store_f16, &dst_ctx);
        } else {
            p.append(SkRasterPipeline::to_srgb);
            p.append(SkRasterPipeline::store_8888, &dst_ctx);
        }

        while (loops --> 0) {
            p.run(0,0,N,1);
        }
    }
};
DEF_BENCH( return (new SkRasterPipelineBench< true>); )
DEF_BENCH( return (new SkRasterPipelineBench<false>); )

class SkRasterPipelineCompileVsRunBench : public Benchmark {
public:
    explicit SkRasterPipelineCompileVsRunBench(bool compile) : fCompile(compile) {}
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return fCompile ? "SkRasterPipeline_compile"
                        : "SkRasterPipeline_run";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkJumper_MemoryCtx src_ctx = {src, 0},
                           dst_ctx = {dst, 0};

        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::load_8888, &dst_ctx);
        p.append(SkRasterPipeline::move_src_dst);
        p.append(SkRasterPipeline::load_8888, &src_ctx);
        p.append(SkRasterPipeline::srcover);
        p.append(SkRasterPipeline::store_8888, &dst_ctx);

        if (fCompile) {
            auto fn = p.compile();
            while (loops --> 0) {
                fn(0,0,N,1);
            }
        } else {
            while (loops --> 0) {
                p.run(0,0,N,1);
            }
        }
    }
private:
    bool fCompile;
};
DEF_BENCH( return (new SkRasterPipelineCompileVsRunBench(true )); )
DEF_BENCH( return (new SkRasterPipelineCompileVsRunBench(false)); )

static SkColorSpaceTransferFn gamma(float g) {
    SkColorSpaceTransferFn fn = {0,0,0,0,0,0,0};
    fn.fG = g;
    fn.fA = 1;
    return fn;
}

class SkRasterPipeline_2dot2 : public Benchmark {
public:
    SkRasterPipeline_2dot2(bool parametric) : fParametric(parametric) {}

    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return fParametric ? "SkRasterPipeline_2dot2_parametric"
                           : "SkRasterPipeline_2dot2_gamma";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkColor4f c = { 1.0f, 1.0f, 1.0f, 1.0f };

        SkColorSpaceTransferFn from_2dot2 = gamma(  2.2f),
                                 to_2dot2 = gamma(1/2.2f);
        SkSTArenaAlloc<256> alloc;
        SkRasterPipeline p(&alloc);
        p.append_constant_color(&alloc, c);
        if (fParametric) {
            p.append(SkRasterPipeline::parametric_r, &from_2dot2);
            p.append(SkRasterPipeline::parametric_g, &from_2dot2);
            p.append(SkRasterPipeline::parametric_b, &from_2dot2);
            p.append(SkRasterPipeline::parametric_r, &  to_2dot2);
            p.append(SkRasterPipeline::parametric_g, &  to_2dot2);
            p.append(SkRasterPipeline::parametric_b, &  to_2dot2);
        } else {
            p.append(SkRasterPipeline::gamma, &from_2dot2.fG);
            p.append(SkRasterPipeline::gamma, &  to_2dot2.fG);
        }

        while (loops --> 0) {
            p.run(0,0,N,1);
        }
    }
private:
    bool fParametric;
};
DEF_BENCH( return (new SkRasterPipeline_2dot2( true)); )
DEF_BENCH( return (new SkRasterPipeline_2dot2(false)); )

class SkRasterPipelineToSRGB : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return "SkRasterPipeline_to_srgb";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkRasterPipeline_<256> p;
        p.append(SkRasterPipeline::to_srgb);

        while (loops --> 0) {
            p.run(0,0,N,1);
        }
    }
};
DEF_BENCH( return (new SkRasterPipelineToSRGB); )
