/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "../src/jumper/SkJumper.h"
#include "Benchmark.h"
#include "SkColor.h"
#include "SkColorSpaceXform.h"
#include "SkColorSpaceXformer.h"
#include "SkColorSpaceXformSteps.h"
#include "SkMakeUnique.h"
#include "SkPM4fPriv.h"
#include "SkRandom.h"
#include "SkRasterPipeline.h"

enum class Mode { xform, steps, pipeA, pipeB, xformer };

struct ColorSpaceXformBench : public Benchmark {
    ColorSpaceXformBench(Mode mode) : fMode(mode) {}

    const Mode fMode;

    std::unique_ptr<SkColorSpaceXform>                  fXform;
    std::unique_ptr<SkColorSpaceXformSteps>             fSteps;
    std::function<void(size_t, size_t, size_t, size_t)> fPipeA;

    SkJumper_MemoryCtx fPipeSrc = {nullptr,0},
                       fPipeDst = {nullptr,0};
    SkSTArenaAlloc<1024> fAlloc;

    std::unique_ptr<SkColorSpaceXformer> fXformer;

    const char* onGetName() override {
        switch (fMode) {
            case Mode::xform  : return "ColorSpaceXformBench_xform";
            case Mode::steps  : return "ColorSpaceXformBench_steps";
            case Mode::pipeA  : return "ColorSpaceXformBench_pipeA";
            case Mode::pipeB  : return "ColorSpaceXformBench_pipeB";
            case Mode::xformer: return "ColorSpaceXformBench_xformer";
        }
        return "";
    }

    bool isSuitableFor(Backend backend) override { return kNonRendering_Backend == backend; }

    void onDelayedSetup() override {
        sk_sp<SkColorSpace> src = SkColorSpace::MakeSRGB(),
                            dst = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                        SkColorSpace::kDCIP3_D65_Gamut);

        fXform = SkColorSpaceXform::New(src.get(), dst.get());
        fSteps = skstd::make_unique<SkColorSpaceXformSteps>(src.get(), kOpaque_SkAlphaType,
                                                            dst.get(), kPremul_SkAlphaType);

        SkRasterPipeline p(&fAlloc);
        p.append(SkRasterPipeline::load_bgra, &fPipeSrc);
        fSteps->apply(&p);
        p.append(SkRasterPipeline::store_bgra, &fPipeDst);

        fPipeA = p.compile();

        fXformer = SkColorSpaceXformer::Make(dst);  // src is implicitly sRGB, what we want anyway
    }

    void onDraw(int n, SkCanvas* canvas) override {
        volatile SkColor junk = 0;
        SkRandom rand;

        for (int i = 0; i < n; i++) {
            SkColor src = rand.nextU(),
                    dst;
            fPipeSrc.pixels = &src;
            fPipeDst.pixels = &dst;

            switch (fMode) {
                case Mode::xform: {
                    fXform->apply(SkColorSpaceXform::kBGRA_8888_ColorFormat, &dst,
                                  SkColorSpaceXform::kBGRA_8888_ColorFormat, &src,
                                  1, kUnpremul_SkAlphaType);
                } break;

                case Mode::steps: {
                    float rgba[4];
                    swizzle_rb(Sk4f_fromL32(src)).store(rgba);
                    fSteps->apply(rgba);
                    dst = Sk4f_toL32(swizzle_rb(Sk4f::Load(rgba)));
                } break;

                case Mode::pipeA: {
                    fPipeA(0,0,1,1);
                } break;

                case Mode::pipeB: {
                    SkSTArenaAlloc<1024> alloc;
                    SkRasterPipeline p(&alloc);
                    p.append(SkRasterPipeline::load_bgra, &fPipeSrc);
                    fSteps->apply(&p);
                    p.append(SkRasterPipeline::store_bgra, &fPipeDst);
                    p.run(0,0,1,1);
                } break;

                case Mode::xformer: {
                    dst = fXformer->apply(src);
                } break;
            }

            if (false && i == 0) {
                SkDebugf("%x ~~> %x\n", src, dst);
            }

            junk ^= dst;
        }
    }
};

DEF_BENCH(return new ColorSpaceXformBench{Mode::xform  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::steps  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::pipeA  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::pipeB  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::xformer};)
