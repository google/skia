/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"

#if SK_SUPPORT_GPU

#include "GrFragmentProcessor.h"
#include "GrTexture.h"
#include "GrInvariantOutput.h"
#include "GrRenderTargetContext.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "Test.h"


static void func(skiatest::Reporter* reporter, sk_gpu_test::ContextInfo ctxInfo);
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ShaderImage, reporter, ctxInfo) {
    func(reporter, ctxInfo);
}

static void func(skiatest::Reporter* reporter, sk_gpu_test::ContextInfo ctxInfo) {
    class TestFP : public GrFragmentProcessor {
    public:
        static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTexture> texture) {
            return sk_sp<GrFragmentProcessor>(new TestFP(std::move(texture)));
        }

        const char* name() const override { return "Image Load Test FP"; }

    private:
        TestFP(sk_sp<GrTexture> texture) : fImageAccess(std::move(texture), kRead_GrIOType) {
            this->initClassID<TestFP>();
            this->setWillReadFragmentPosition();
            this->addImageAccess(&fImageAccess);
        }

        void onGetGLSLProcessorKey(const GrGLSLCaps&, GrProcessorKeyBuilder*) const override {}

        void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
            inout->setToUnknown(GrInvariantOutput::kWillNot_ReadInput);
        }

        bool onIsEqual(const GrFragmentProcessor& that) const override {
            return this->hasSameSamplers(that);
        }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLSLProcessor : public GrGLSLFragmentProcessor {
            public:
                GLSLProcessor() = default;
                void emitCode(EmitArgs& args) override {
                    GrGLSLFPFragmentBuilder* fb = args.fFragBuilder;
                    SkString imageLoadStr;
                    fb->codeAppendf("vec2 coord = %s.xy;", args.fFragBuilder->fragmentPosition());
                    fb->appendImageLoad(&imageLoadStr, args.fImages[0], "ivec2(coord)");
                    fb->codeAppendf("%s = %s;", args.fOutputColor, imageLoadStr.c_str());
                }
            };
            return new GLSLProcessor;
        }

        GrImageAccess fImageAccess;
    };

    static constexpr int kS = 256;
    GrContext* context = ctxInfo.grContext();
    if (context->caps()->shaderCaps()->maxShaderImages(kFragment_GrShaderType) < 1) {
        return;
    }
    GrSurfaceDesc desc;
    desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    desc.fWidth = kS;
    desc.fHeight = kS;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    std::unique_ptr<uint32_t[]> data(new uint32_t[kS * kS]);
    for (int j = 0; j < kS; ++j) {
        for (int i = 0; i < kS; ++i) {
            data[i] = GrColorPackRGBA(i, j, 0, 0);
        }
    }

    sk_sp<GrTexture> imageTexture(context->textureProvider()->createTexture(desc,
                                                                            SkBudgeted::kYes,
                                                                            data.get(), 0));
    desc.fFlags = kRenderTarget_GrSurfaceFlag;
    //sk_sp<GrTexture> dst(context->textureProvider()->createTexture(desc, SkBudgeted::kYes));

    sk_sp<GrRenderTargetContext> rtContext =
        context->makeRenderTargetContext(SkBackingFit::kExact, kS, kS, kRGBA_8888_GrPixelConfig,
                                         nullptr);
    GrPaint paint;
    paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
    paint.addColorFragmentProcessor(TestFP::Make(imageTexture));
    rtContext->drawPaint(GrNoClip(), paint, SkMatrix::I());
    std::unique_ptr<uint32_t[]> readData(new uint32_t[kS * kS]);
    SkImageInfo info = SkImageInfo::Make(kS, kS, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    rtContext->readPixels(info, readData.get(), 0, 0, 0);
    int failed = false;
    for (int j = 0; j < kS && !failed; ++j) {
        for (int i = 0; i < kS && !failed; ++i) {
            uint32_t d = data[j * kS + i];
            uint32_t rd = readData[j * kS + i];
            if (d != rd) {
                failed = true;
                ERRORF(reporter, "Expected 0x%08x, got 0x%08x at %d, %d.", d, rd, i, j);
            }
        }
    }
}

#endif
