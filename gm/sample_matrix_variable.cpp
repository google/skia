/*
 * Copyright 2019 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "src/gpu/GrBitmapTextureMaker.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrRenderTargetContextPriv.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/ops/GrFillRectOp.h"
#include "tools/Resources.h"

class SampleMatrixVariableEffect : public GrFragmentProcessor {
public:
    static constexpr GrProcessor::ClassID CLASS_ID = (GrProcessor::ClassID) 2;

    SampleMatrixVariableEffect(std::unique_ptr<GrFragmentProcessor> child)
        : INHERITED(CLASS_ID, kNone_OptimizationFlags) {
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

    bool onIsEqual(const GrFragmentProcessor&) const override {
        SkASSERT(false);
        return true;
    }

private:
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    typedef GrFragmentProcessor INHERITED;
};

class GLSLSampleMatrixVariableEffect : public GrGLSLFragmentProcessor {
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        SkString sample1 = this->invokeChildWithMatrix(0, args, "float3x3(1)");
        SkString sample2 = this->invokeChildWithMatrix(0, args, "float3x3(0.75, 0, 0, 0, 0.75, 0, "
                                                                "-0.2, -0.3, 0.75)");
        fragBuilder->codeAppendf("%s = (%s + %s) / 2;\n", args.fOutputColor, sample1.c_str(),
                                 sample2.c_str());
    }
};

GrGLSLFragmentProcessor* SampleMatrixVariableEffect::onCreateGLSLInstance() const {
    return new GLSLSampleMatrixVariableEffect();
}

DEF_SIMPLE_GPU_GM(sample_matrix_variable, ctx, rtCtx, canvas, 512, 512) {
    SkRect bounds = SkRect::MakeIWH(512, 512);

    SkBitmap bmp;
    GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bmp);
    GrBitmapTextureMaker maker(ctx, bmp, GrImageTexGenPolicy::kDraw);
    auto view = maker.view(GrMipMapped::kNo);
    std::unique_ptr<GrFragmentProcessor> imgFP =
            GrTextureEffect::Make(std::move(view), bmp.alphaType(), SkMatrix());
    imgFP->setSampleMatrix(SkSL::SampleMatrix::Kind::kVariable);
    auto fp = std::unique_ptr<GrFragmentProcessor>(
            new SampleMatrixVariableEffect(std::move(imgFP)));

    GrPaint grPaint;
    grPaint.addCoverageFragmentProcessor(std::move(fp));

    rtCtx->priv().testingOnly_addDrawOp(GrFillRectOp::MakeNonAARect(ctx,
                                                                    std::move(grPaint),
                                                                    SkMatrix::I(),
                                                                    bounds));
}
