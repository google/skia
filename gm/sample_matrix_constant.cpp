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

class SampleMatrixConstantEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 1;

    SampleMatrixConstantEffect(std::unique_ptr<GrFragmentProcessor> child)
        : INHERITED(CLASS_ID, kNone_OptimizationFlags) {
        child->setSampleMatrix(SkSL::SampleMatrix(SkSL::SampleMatrix::Kind::kVariable));
        this->registerChildProcessor(std::move(child));
    }

    const char* name() const override { return "SampleMatrixConstantEffect"; }

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
    typedef GrFragmentProcessor INHERITED;
};

class GLSLSampleMatrixConstantEffect : public GrGLSLFragmentProcessor {
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString sample = this->invokeChildWithMatrix(0, args, "float3x3(0.5)");
        fragBuilder->codeAppendf("%s = %s;\n", args.fOutputColor, sample.c_str());
    }
};

GrGLSLFragmentProcessor* SampleMatrixConstantEffect::onCreateGLSLInstance() const {
    return new GLSLSampleMatrixConstantEffect();
}


DEF_SIMPLE_GPU_GM(sample_matrix_constant, ctx, rtCtx, canvas, 512, 256) {
    {
        SkRect bounds = SkRect::MakeIWH(256, 256);
        SkBitmap bmp;
        GetResourceAsBitmap("images/mandrill_256.png", &bmp);
        GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
        auto view = maker.view(GrMipMapped::kNo);
        std::unique_ptr<GrFragmentProcessor> imgFP =
                GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
        auto fp = std::unique_ptr<GrFragmentProcessor>(
                new SampleMatrixConstantEffect(std::move(imgFP)));

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
        auto fp = std::unique_ptr<GrFragmentProcessor>(
                new SampleMatrixConstantEffect(std::move(gradientFP)));
        paint.addCoverageFragmentProcessor(std::move(fp));
        rtCtx->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(), bounds);
    }
}

