/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTileMode.h"
#include "include/core/SkTypes.h"
#include "src/core/SkCanvasPriv.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/SurfaceFillContext.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

class SampleCoordEffect : public GrFragmentProcessor {
public:
    inline static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 0;

    SampleCoordEffect(std::unique_ptr<GrFragmentProcessor> child)
        : INHERITED(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
    }

    const char* name() const override { return "SampleCoordEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        SkASSERT(false);
        return nullptr;
    }

    void onAddToKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

    bool onIsEqual(const GrFragmentProcessor&) const override {
        SkASSERT(false);
        return true;
    }

private:
    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;
    using INHERITED = GrFragmentProcessor;
};

std::unique_ptr<GrFragmentProcessor::ProgramImpl> SampleCoordEffect::onMakeProgramImpl() const {
    class Impl : public ProgramImpl {
    public:
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            SkString s1 = this->invokeChild(0, args, "float2(sk_FragCoord.x, sk_FragCoord.y)");
            SkString s2 = this->invokeChild(0, args, "float2(sk_FragCoord.x, 512-sk_FragCoord.y)");
            fragBuilder->codeAppendf("return (%s + %s) / 2;\n", s1.c_str(), s2.c_str());
        }
    };

    return std::make_unique<Impl>();
}

DEF_SIMPLE_GPU_GM_BG(fpcoordinateoverride, rContext, canvas, 512, 512,
                     ToolUtils::color_to_565(0xFF66AA99)) {

    auto sfc = SkCanvasPriv::TopDeviceSurfaceFillContext(canvas);
    if (!sfc) {
        return;
    }

    SkBitmap bmp;
    GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    auto view = std::get<0>(GrMakeCachedBitmapProxyView(rContext, bmp, GrMipmapped::kNo));
    if (!view) {
        return;
    }
    std::unique_ptr<GrFragmentProcessor> imgFP =
            GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
    auto fp = std::unique_ptr<GrFragmentProcessor>(new SampleCoordEffect(std::move(imgFP)));

    sfc->fillWithFP(std::move(fp));
}
