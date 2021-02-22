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
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrCaps.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/effects/GrRRectEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

class SampleCoordEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 0;

    SampleCoordEffect(std::unique_ptr<GrFragmentProcessor> child)
        : INHERITED(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child), SkSL::SampleUsage::Explicit());
    }

    const char* name() const override { return "SampleCoordEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        SkASSERT(false);
        return nullptr;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {
    }

    bool onIsEqual(const GrFragmentProcessor&) const override {
        SkASSERT(false);
        return true;
    }

private:
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;
    using INHERITED = GrFragmentProcessor;
};

class GLSLSampleCoordEffect : public GrGLSLFragmentProcessor {
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString sample1 = this->invokeChild(0, args, "float2(sk_FragCoord.x, sk_FragCoord.y)");
        SkString sample2 = this->invokeChild(0, args, "float2(sk_FragCoord.x, 512-sk_FragCoord.y)");
        fragBuilder->codeAppendf("return (%s + %s) / 2;\n", sample1.c_str(), sample2.c_str());
    }
};

std::unique_ptr<GrGLSLFragmentProcessor> SampleCoordEffect::onMakeProgramImpl() const {
    return std::make_unique<GLSLSampleCoordEffect>();
}

DEF_SIMPLE_GPU_GM_BG(fpcoordinateoverride, ctx, rtCtx, canvas, 512, 512,
                     ToolUtils::color_to_565(0xFF66AA99)) {
    SkRect bounds = SkRect::MakeIWH(512, 512);

    SkBitmap bmp;
    GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
    auto view = maker.view(GrMipmapped::kNo);
    if (!view) {
        return;
    }
    std::unique_ptr<GrFragmentProcessor> imgFP =
            GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
    auto fp = std::unique_ptr<GrFragmentProcessor>(new SampleCoordEffect(std::move(imgFP)));

    GrPaint grPaint;
    grPaint.setCoverageFragmentProcessor(std::move(fp));

    rtCtx->addDrawOp(GrFillRectOp::MakeNonAARect(ctx, std::move(grPaint), SkMatrix::I(), bounds));
}
