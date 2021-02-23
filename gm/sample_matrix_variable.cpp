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
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/Resources.h"

class SampleMatrixVariableEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 2;

    SampleMatrixVariableEffect(std::unique_ptr<GrFragmentProcessor> child,
                               float xOffset,
                               float yOffset)
            : INHERITED(CLASS_ID, kNone_OptimizationFlags)
            , fXOffset(xOffset)
            , fYOffset(yOffset) {
        this->registerChild(std::move(child), SkSL::SampleUsage::VariableMatrix());
    }

    const char* name() const override { return "SampleMatrixVariableEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        SkASSERT(false);
        return nullptr;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }

private:
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

    float fXOffset;
    float fYOffset;

    using INHERITED = GrFragmentProcessor;
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
        fragBuilder->codeAppendf("return (%s + %s) / 2;\n", sample1.c_str(), sample2.c_str());
    }
};

std::unique_ptr<GrGLSLFragmentProcessor> SampleMatrixVariableEffect::onMakeProgramImpl() const {
    return std::make_unique<GLSLSampleMatrixVariableEffect>();
}

DEF_SIMPLE_GPU_GM(sample_matrix_variable, ctx, rtCtx, canvas, 512, 256) {
    auto draw = [rtCtx](std::unique_ptr<GrFragmentProcessor> baseFP, float ofsX, float ofsY,
                        int tx, int ty) {
        auto fp = std::unique_ptr<GrFragmentProcessor>(
                new SampleMatrixVariableEffect(std::move(baseFP), ofsX, ofsY));
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        rtCtx->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::Translate(tx, ty),
                        SkRect::MakeIWH(256, 256));
    };

    {
        SkBitmap bmp;
        GetResourceAsBitmap("images/mandrill_256.png", &bmp);
        GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
        auto view = maker.view(GrMipmapped::kNo);
        std::unique_ptr<GrFragmentProcessor> imgFP =
                GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
        draw(std::move(imgFP), -128, 256, 0, 0);
    }

    {
        static constexpr SkColor colors[] = { 0xff00ff00, 0xffff00ff };
        const SkPoint pts[] = {{ 256, 0 }, { 512, 0 }};

        auto shader = SkGradientShader::MakeLinear(pts, colors, nullptr,
                                                   SK_ARRAY_COUNT(colors),
                                                   SkTileMode::kRepeat);
        SkMatrix matrix;
        SkSimpleMatrixProvider matrixProvider(matrix);
        GrColorInfo colorInfo;
        GrFPArgs args(ctx, matrixProvider, SkSamplingOptions(SkCubicResampler::Mitchell()),
                      &colorInfo);
        std::unique_ptr<GrFragmentProcessor> gradientFP = as_SB(shader)->asFragmentProcessor(args);
        draw(std::move(gradientFP), -128, 256, 256, 0);
    }
}
