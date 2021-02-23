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

class SampleMatrixConstantEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 1;

    SampleMatrixConstantEffect(std::unique_ptr<GrFragmentProcessor> child)
            : INHERITED(CLASS_ID, kNone_OptimizationFlags) {
        this->registerChild(std::move(child),
                            SkSL::SampleUsage::UniformMatrix(
                                    "float3x3(float3(0.5, 0.0, 0.0), "
                                             "float3(0.0, 0.5, 0.0), "
                                             "float3(0.0, 0.0, 1.0))"));
    }

    const char* name() const override { return "SampleMatrixConstantEffect"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        SkASSERT(false);
        return nullptr;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}
    bool onIsEqual(const GrFragmentProcessor& that) const override { return this == &that; }

private:
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;
    using INHERITED = GrFragmentProcessor;
};

class GLSLSampleMatrixConstantEffect : public GrGLSLFragmentProcessor {
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString sample = this->invokeChildWithMatrix(0, args);
        fragBuilder->codeAppendf("return %s;\n", sample.c_str());
    }
};

std::unique_ptr<GrGLSLFragmentProcessor> SampleMatrixConstantEffect::onMakeProgramImpl() const {
    return std::make_unique<GLSLSampleMatrixConstantEffect>();
}

DEF_SIMPLE_GPU_GM(sample_matrix_constant, ctx, rtCtx, canvas, 1024, 256) {
    auto wrap = [](std::unique_ptr<GrFragmentProcessor> baseFP) {
      return std::unique_ptr<GrFragmentProcessor>(
              new SampleMatrixConstantEffect(std::move(baseFP)));
    };
    auto draw = [rtCtx, &wrap](std::unique_ptr<GrFragmentProcessor> baseFP, int tx, int ty) {
        auto fp = wrap(std::move(baseFP));
        GrPaint paint;
        paint.setColorFragmentProcessor(std::move(fp));
        rtCtx->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::Translate(tx, ty),
                        SkRect::MakeIWH(256, 256));
    };
    auto draw2 = [rtCtx, &wrap](std::unique_ptr<GrFragmentProcessor> baseFP, int tx, int ty) {
      auto fp = wrap(wrap(std::move(baseFP)));
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
        draw(std::move(imgFP), 0, 0);
        view = maker.view(GrMipmapped::kNo);
        imgFP =
                GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
        draw2(std::move(imgFP), 256, 0);
    }

    {
        static constexpr SkColor colors[] = { 0xff00ff00, 0xffff00ff };
        const SkPoint pts[] = {{ 0, 0 }, { 256, 0 }};

        auto shader = SkGradientShader::MakeLinear(pts, colors, nullptr,
                                                   SK_ARRAY_COUNT(colors),
                                                   SkTileMode::kClamp);
        SkMatrix matrix;
        SkSimpleMatrixProvider matrixProvider(matrix);
        GrColorInfo colorInfo;
        GrFPArgs args(ctx, matrixProvider, SkSamplingOptions(SkCubicResampler::Mitchell()),
                      &colorInfo);
        std::unique_ptr<GrFragmentProcessor> gradientFP = as_SB(shader)->asFragmentProcessor(args);
        draw(std::move(gradientFP), 512, 0);
        gradientFP = as_SB(shader)->asFragmentProcessor(args);
        draw2(std::move(gradientFP), 768, 0);
    }
}
