/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"
#include "SkOpts.h"
#include "SkRasterPipeline.h"

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

        while (loops --> 0) {
            p.run(0,N);
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
        void*  src_ctx = src;
        void*  dst_ctx = dst;

        SkRasterPipeline p;
        p.append(SkRasterPipeline::load_8888, &dst_ctx);
        p.append(SkRasterPipeline::move_src_dst);
        p.append(SkRasterPipeline::load_8888, &src_ctx);
        p.append(SkRasterPipeline::srcover);
        p.append(SkRasterPipeline::store_8888, &dst_ctx);

        if (fCompile) {
            char buffer[1024];
            SkArenaAlloc alloc(buffer);
            auto fn = p.compile(&alloc);
            while (loops --> 0) {
                fn(0,N);
            }
        } else {
            while (loops --> 0) {
                p.run(0,N);
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
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return "SkRasterPipeline_2dot2";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkColor4f c = { 1.0f, 1.0f, 1.0f, 1.0f };

        SkColorSpaceTransferFn from_2dot2 = gamma(  2.2f),
                                 to_2dot2 = gamma(1/2.2f);
        SkRasterPipeline p;
        p.append(SkRasterPipeline::constant_color, &c);
        p.append(SkRasterPipeline::parametric_r, &from_2dot2);
        p.append(SkRasterPipeline::parametric_g, &from_2dot2);
        p.append(SkRasterPipeline::parametric_b, &from_2dot2);
        p.append(SkRasterPipeline::parametric_r, &  to_2dot2);
        p.append(SkRasterPipeline::parametric_g, &  to_2dot2);
        p.append(SkRasterPipeline::parametric_b, &  to_2dot2);

        while (loops --> 0) {
            p.run(0,N);
        }
    }
};
DEF_BENCH( return (new SkRasterPipeline_2dot2); )

class SkRasterPipelineToSRGB : public Benchmark {
public:
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }
    const char* onGetName() override {
        return "SkRasterPipeline_to_srgb";
    }

    void onDraw(int loops, SkCanvas*) override {
        SkRasterPipeline p;
        p.append(SkRasterPipeline::to_srgb);

        while (loops --> 0) {
            p.run(0,N);
        }
    }
};
DEF_BENCH( return (new SkRasterPipelineToSRGB); )

class SkRasterPipelineReuseBench : public Benchmark {
public:
    enum Mode { None, Some, Full };

    explicit SkRasterPipelineReuseBench(Mode mode) : fMode(mode), fName("SkRasterPipelineReuse") {
        switch(mode) {
            case None: fName.append("_none"); break;
            case Some: fName.append("_some"); break;
            case Full: fName.append("_full"); break;
        }
    }
    const char* onGetName() override { return fName.c_str(); }
    bool isSuitableFor(Backend backend) override { return backend == kNonRendering_Backend; }

    void onDraw(int loops, SkCanvas*) override {
        const int kStages = 20;
        const auto stage  = SkRasterPipeline::to_srgb;  // Any stage will do.  We won't call it.

        switch(fMode) {
            case None:
                while (loops --> 0) {
                    SkRasterPipeline p;
                    for (int i = 0; i < kStages; i++) {
                        p.append(stage);
                    }
                }
                break;

            case Some:
                while (loops --> 0) {
                    SkRasterPipeline p(kStages);
                    for (int i = 0; i < kStages; i++) {
                        p.append(stage);
                    }
                }
                break;

            case Full:
                SkRasterPipeline p(kStages);
                while (loops --> 0) {
                    p.rewind();
                    for (int i = 0; i < kStages; i++) {
                        p.append(stage);
                    }
                }
                break;
        }
    }

private:
    Mode     fMode;
    SkString fName;
};
DEF_BENCH( return (new SkRasterPipelineReuseBench(SkRasterPipelineReuseBench::None)); )
DEF_BENCH( return (new SkRasterPipelineReuseBench(SkRasterPipelineReuseBench::Some)); )
DEF_BENCH( return (new SkRasterPipelineReuseBench(SkRasterPipelineReuseBench::Full)); )
