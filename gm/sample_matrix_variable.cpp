/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/effects/SkGradientShader.h"
#include "src/core/SkMatrixProvider.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrClip.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/Resources.h"

class SampleMatrixVariableEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 2;

    SampleMatrixVariableEffect(std::unique_ptr<GrFragmentProcessor> child, float xOffset,
                               float yOffset)
        : INHERITED(CLASS_ID, kNone_OptimizationFlags)
        , fXOffset(xOffset)
        , fYOffset(yOffset) {
        child->setSampleMatrix(SkSL::SampleMatrix(SkSL::SampleMatrix::Kind::kVariable));
        this->registerChildProcessor(std::move(child));
    }

    const char* name() const override { return "SampleMatrixVariableEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        SkASSERT(false);
        return nullptr;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {
    }

    bool onIsEqual(const GrFragmentProcessor& that) const override {
        return this == &that;
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    float fXOffset;
    float fYOffset;

    typedef GrFragmentProcessor INHERITED;

    friend class GLSLSampleMatrixVariableEffect;
};

class GLSLSampleMatrixVariableEffect : public GrGLSLFragmentProcessor {
    void emitCode(EmitArgs& args) override {
        auto& smve = args.fFp.cast<SampleMatrixVariableEffect>();
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString sample1 = this->invokeChildWithMatrix(0, args, "float3x3(1)");
        SkString sample2 = this->invokeChildWithMatrix(0, args,
                                                       SkStringPrintf("float3x3(1, -1, 0, 1, 0, 0, "
                                                                      "%g, %g, 1)",
                                                                      smve.fXOffset,
                                                                      smve.fYOffset).c_str());
        fragBuilder->codeAppendf("%s = (%s + %s) / 2;\n", args.fOutputColor, sample1.c_str(),
                                 sample2.c_str());
    }
};

GrGLSLFragmentProcessor* SampleMatrixVariableEffect::onCreateGLSLInstance() const {
    return new GLSLSampleMatrixVariableEffect();
}

DEF_SIMPLE_GPU_GM(sample_matrix_variable, ctx, rtCtx, canvas, 512, 256) {
    {
        SkRect bounds = SkRect::MakeIWH(256, 256);
        SkBitmap bmp;
        GetResourceAsBitmap("images/mandrill_256.png", &bmp);
        GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
        auto view = maker.view(GrMipMapped::kNo);
        std::unique_ptr<GrFragmentProcessor> imgFP =
                GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
        imgFP->setSampleMatrix(SkSL::SampleMatrix::Kind::kVariable);
        auto fp = std::unique_ptr<GrFragmentProcessor>(
                new SampleMatrixVariableEffect(std::move(imgFP), -128, 256));

        GrPaint paint;
        paint.addCoverageFragmentProcessor(std::move(fp));
        rtCtx->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), bounds);
    }

    {
        GrPaint paint;
        SkRect bounds = SkRect::MakeLTRB(256, 0, 512, 256);
        static constexpr SkColor colors[] = { 0xff00ff00, 0xffff00ff };
        static constexpr SkScalar   pos[] = {       0.0f,       1.0f };
        const SkPoint pts[] = {{ 256, 0 }, { 512, 0 }};

        auto shader = SkGradientShader::MakeLinear(pts, colors, pos,
                                                   SK_ARRAY_COUNT(colors),
                                                   SkTileMode::kRepeat);
        SkMatrix matrix;
        SkSimpleMatrixProvider matrixProvider(matrix);
        GrColorInfo colorInfo;
        GrFPArgs args(ctx, matrixProvider, kHigh_SkFilterQuality, &colorInfo);
        std::unique_ptr<GrFragmentProcessor> gradientFP = as_SB(shader)->asFragmentProcessor(args);
        gradientFP->setSampleMatrix(SkSL::SampleMatrix::Kind::kVariable);
        auto fp = std::unique_ptr<GrFragmentProcessor>(
                new SampleMatrixVariableEffect(std::move(gradientFP), -0.5, 1));
        paint.addCoverageFragmentProcessor(std::move(fp));
        rtCtx->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), bounds);
    }
}
