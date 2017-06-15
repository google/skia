/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

#if SK_SUPPORT_GPU

#include "GrClip.h"
#include "GrFragmentProcessor.h"
#include "GrRenderTargetContext.h"
#include "GrTexture.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ImageStorageLoad, reporter, ctxInfo) {
    class TestFP : public GrFragmentProcessor {
    public:
        static sk_sp<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> proxy,
                                               GrSLMemoryModel mm,
                                               GrSLRestrict restrict) {
            return sk_sp<GrFragmentProcessor>(new TestFP(std::move(proxy), mm, restrict));
        }

        const char* name() const override { return "Image Load Test FP"; }

    private:
        TestFP(sk_sp<GrTextureProxy> proxy, GrSLMemoryModel mm, GrSLRestrict restrict)
                : INHERITED(kNone_OptimizationFlags)
                , fImageStorageAccess(std::move(proxy), kRead_GrIOType, mm, restrict) {
            this->initClassID<TestFP>();
            this->addImageStorageAccess(&fImageStorageAccess);
        }

        void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override {}

        bool onIsEqual(const GrFragmentProcessor& that) const override { return true; }

        GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
            class GLSLProcessor : public GrGLSLFragmentProcessor {
            public:
                GLSLProcessor() = default;
                void emitCode(EmitArgs& args) override {
                    const TestFP& tfp = args.fFp.cast<TestFP>();
                    GrGLSLFPFragmentBuilder* fb = args.fFragBuilder;
                    SkString imageLoadStr;
                    fb->codeAppend("highp vec2 coord = sk_FragCoord.xy;");
                    fb->appendImageStorageLoad(&imageLoadStr, args.fImageStorages[0],
                                               "ivec2(coord)");
                    if (GrPixelConfigIsSint(tfp.fImageStorageAccess.peekTexture()->config())) {
                        // Map the signed bytes so that when then get read back as unorm values they
                        // will have their original bit pattern.
                        fb->codeAppendf("highp ivec4 ivals = %s;", imageLoadStr.c_str());
                        // NV gives a linker error for this:
                        // fb->codeAppend("ivals +=
                        //                "mix(ivec4(0), ivec4(256), lessThan(ivals, ivec4(0)));");
                        fb->codeAppend("if (ivals.r < 0) { ivals.r += 256; }");
                        fb->codeAppend("if (ivals.g < 0) { ivals.g += 256; }");
                        fb->codeAppend("if (ivals.b < 0) { ivals.b += 256; }");
                        fb->codeAppend("if (ivals.a < 0) { ivals.a += 256; }");
                        fb->codeAppendf("%s = vec4(ivals)/255;", args.fOutputColor);
                    } else {
                        fb->codeAppendf("%s = %s;", args.fOutputColor, imageLoadStr.c_str());
                    }
                }
            };
            return new GLSLProcessor;
        }

        ImageStorageAccess fImageStorageAccess;
        typedef GrFragmentProcessor INHERITED;
    };

    static constexpr int kS = 256;
    GrContext* context = ctxInfo.grContext();
    if (context->caps()->shaderCaps()->maxFragmentImageStorages() < 1) {
        return;
    }

    std::unique_ptr<uint32_t[]> data(new uint32_t[kS * kS]);
    for (int j = 0; j < kS; ++j) {
        for (int i = 0; i < kS; ++i) {
            data[i + kS * j] = GrColorPackRGBA(i, j, 0, 0);
        }
    }

    std::unique_ptr<uint32_t[]> idata(new uint32_t[kS * kS]);
    for (int j = 0; j < kS; ++j) {
        for (int i = 0; i < kS; ++i) {
            int8_t r = i - 128;
            int8_t g = j - 128;
            int8_t b = -128;
            int8_t a = -128;
            idata[i + kS * j] = ((uint8_t)a << 24) | ((uint8_t)b << 16) |
                                ((uint8_t)g << 8)  |  (uint8_t)r;
        }
    }

    // Currently image accesses always have "top left" semantics.
    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = kS;
    desc.fHeight = kS;
    struct {
        GrPixelConfig fConfig;
        std::unique_ptr<uint32_t[]> fData;
    } tests[] = {
        {
           kRGBA_8888_GrPixelConfig,
           std::move(data)
        },
        {
           kRGBA_8888_sint_GrPixelConfig,
           std::move(idata)
        },
    };
    for (const auto& test : tests) {
        // This test should work with any memory model and with or without restrict
        for (auto mm : {GrSLMemoryModel::kNone,
                        GrSLMemoryModel::kCoherent,
                        GrSLMemoryModel::kVolatile}) {
            for (auto restrict : {GrSLRestrict::kNo, GrSLRestrict::kYes}) {
                if (!context->caps()->canConfigBeImageStorage(test.fConfig)) {
                    continue;
                }
                desc.fConfig = test.fConfig;
                sk_sp<GrTextureProxy> imageStorageTexture =
                    GrSurfaceProxy::MakeDeferred(context->resourceProvider(), desc,
                                                 SkBudgeted::kYes, test.fData.get(), 0);

                sk_sp<GrRenderTargetContext> rtContext =
                    context->makeDeferredRenderTargetContext(SkBackingFit::kExact, kS, kS,
                                                             kRGBA_8888_GrPixelConfig, nullptr);
                GrPaint paint;
                paint.setPorterDuffXPFactory(SkBlendMode::kSrc);
                paint.addColorFragmentProcessor(TestFP::Make(imageStorageTexture, mm, restrict));
                rtContext->drawPaint(GrNoClip(), std::move(paint), SkMatrix::I());
                std::unique_ptr<uint32_t[]> readData(new uint32_t[kS * kS]);
                SkImageInfo info = SkImageInfo::Make(kS, kS, kRGBA_8888_SkColorType,
                                                     kPremul_SkAlphaType);
                rtContext->readPixels(info, readData.get(), 0, 0, 0);
                int failed = false;
                for (int j = 0; j < kS && !failed; ++j) {
                    for (int i = 0; i < kS && !failed; ++i) {
                        uint32_t d = test.fData[j * kS + i];
                        uint32_t rd = readData[j * kS + i];
                        if (d != rd) {
                            failed = true;
                            ERRORF(reporter, "Expected 0x%08x, got 0x%08x at %d, %d.", d, rd, i, j);
                        }
                    }
                }
            }
        }
    }
}

#endif
