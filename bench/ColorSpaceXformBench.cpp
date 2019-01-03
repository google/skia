/*
* Copyright 2017 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "Benchmark.h"
#include "SkColor.h"
#include "SkColorSpaceXformer.h"
#include "SkColorSpaceXformSteps.h"
#include "SkMakeUnique.h"
#include "SkRandom.h"

enum class Mode { steps, xformer };

struct ColorSpaceXformBench : public Benchmark {
    ColorSpaceXformBench(Mode mode) : fMode(mode) {}

    const Mode fMode;

    std::unique_ptr<SkColorSpaceXformSteps>  fSteps;
    std::unique_ptr<SkColorSpaceXformer>     fXformer;

    const char* onGetName() override {
        switch (fMode) {
            case Mode::steps  : return "ColorSpaceXformBench_steps";
            case Mode::xformer: return "ColorSpaceXformBench_xformer";
        }
        return "";
    }

    bool isSuitableFor(Backend backend) override { return kNonRendering_Backend == backend; }

    void onDelayedSetup() override {
        sk_sp<SkColorSpace> src = SkColorSpace::MakeSRGB(),
                            dst = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                        SkNamedGamut::kDCIP3);

        fSteps = skstd::make_unique<SkColorSpaceXformSteps>(src.get(), kOpaque_SkAlphaType,
                                                            dst.get(), kPremul_SkAlphaType);
        fXformer = SkColorSpaceXformer::Make(dst);  // src is implicitly sRGB, what we want anyway
    }

    void onDraw(int n, SkCanvas* canvas) override {
        volatile SkColor junk = 0;
        SkRandom rand;

        for (int i = 0; i < n; i++) {
            SkColor src = rand.nextU(),
                    dst;
            switch (fMode) {
                case Mode::steps: {
                    SkColor4f rgba = SkColor4f::FromColor(src);
                    fSteps->apply(rgba.vec());
                    dst = rgba.toSkColor();
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

DEF_BENCH(return new ColorSpaceXformBench{Mode::steps  };)
DEF_BENCH(return new ColorSpaceXformBench{Mode::xformer};)
