/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/effects/GrColorTableEffect.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkSurfaceProps.h"
#include "include/gpu/GpuTypes.h"
#include "include/private/SkSLSampleUsage.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTo.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/base/SkRandom.h"
#include "src/gpu/ganesh/GrColorInfo.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrFragmentProcessors.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSurfaceProxyView.h"
#include "src/gpu/ganesh/GrTestUtils.h"
#include "src/gpu/ganesh/SkGr.h"
#include "src/gpu/ganesh/effects/GrTextureEffect.h"
#include "src/gpu/ganesh/glsl/GrGLSLFragmentShaderBuilder.h"

#include <cstdint>
#include <tuple>
#include <utility>

class GrRecordingContext;

ColorTableEffect::ColorTableEffect(std::unique_ptr<GrFragmentProcessor> inputFP,
                                   GrSurfaceProxyView view)
        // Not bothering with table-specific optimizations.
        : GrFragmentProcessor(kColorTableEffect_ClassID, kNone_OptimizationFlags) {
    this->registerChild(GrTextureEffect::Make(std::move(view), kUnknown_SkAlphaType),
                        SkSL::SampleUsage::Explicit());
    this->registerChild(std::move(inputFP));
}

ColorTableEffect::ColorTableEffect(const ColorTableEffect& that) : GrFragmentProcessor(that) {}

std::unique_ptr<GrFragmentProcessor::ProgramImpl> ColorTableEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            SkString inputColor = this->invokeChild(kInputFPIndex, args);
            SkString a = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.a, 0.5)");
            SkString r = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.r, 1.5)");
            SkString g = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.g, 2.5)");
            SkString b = this->invokeChild(kTexEffectFPIndex, args, "half2(coord.b, 3.5)");
            fragBuilder->codeAppendf(
                    "half4 coord = 255 * unpremul(%s) + 0.5;\n"
                    "half4 color = half4(%s.a, %s.a, %s.a, 1);\n"
                    "return color * %s.a;\n",
                    inputColor.c_str(),
                    r.c_str(),
                    g.c_str(),
                    b.c_str(),
                    a.c_str());
        }
    };

    return std::make_unique<Impl>();
}

std::unique_ptr<GrFragmentProcessor> ColorTableEffect::Make(
        std::unique_ptr<GrFragmentProcessor> inputFP,
        GrRecordingContext* context,
        const SkBitmap& bitmap) {
    SkASSERT(kPremul_SkAlphaType == bitmap.alphaType());
    SkASSERT(bitmap.isImmutable());

    auto view = std::get<0>(GrMakeCachedBitmapProxyView(context,
                                                        bitmap,
                                                        /*label=*/"MakeColorTableEffect",
                                                        skgpu::Mipmapped::kNo));
    if (!view) {
        return nullptr;
    }

    return std::unique_ptr<GrFragmentProcessor>(
            new ColorTableEffect(std::move(inputFP), std::move(view)));
}

///////////////////////////////////////////////////////////////////////////////

GR_DEFINE_FRAGMENT_PROCESSOR_TEST(ColorTableEffect)

#if defined(GPU_TEST_UTILS)
std::unique_ptr<GrFragmentProcessor> ColorTableEffect::TestCreate(GrProcessorTestData* d) {
    int flags = 0;
    uint8_t luts[256][4];
    do {
        for (int i = 0; i < 4; ++i) {
            flags |= d->fRandom->nextBool() ? (1 << i) : 0;
        }
    } while (!flags);
    for (int i = 0; i < 4; ++i) {
        if (flags & (1 << i)) {
            for (int j = 0; j < 256; ++j) {
                luts[j][i] = SkToU8(d->fRandom->nextBits(8));
            }
        }
    }
    auto filter(SkColorFilters::TableARGB((flags & (1 << 0)) ? luts[0] : nullptr,
                                          (flags & (1 << 1)) ? luts[1] : nullptr,
                                          (flags & (1 << 2)) ? luts[2] : nullptr,
                                          (flags & (1 << 3)) ? luts[3] : nullptr));
    sk_sp<SkColorSpace> colorSpace = GrTest::TestColorSpace(d->fRandom);
    SkSurfaceProps props;  // default props for testing
    auto [success, fp] = GrFragmentProcessors::Make(
            d->surfaceDrawContext(),
            filter.get(),
            d->inputFP(),
            GrColorInfo(GrColorType::kRGBA_8888, kUnknown_SkAlphaType, std::move(colorSpace)),
            props);
    SkASSERT(success);
    return std::move(fp);
}
#endif
