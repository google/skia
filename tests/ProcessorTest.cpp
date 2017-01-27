/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrGpuResource.h"
#include "GrRenderTargetContext.h"
#include "GrRenderTargetContextPriv.h"
#include "GrResourceProvider.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "ops/GrTestMeshDrawOp.h"

namespace {
class TestOp : public GrTestMeshDrawOp {
public:
    DEFINE_OP_CLASS_ID
    const char* name() const override { return "TestOp"; }

    static std::unique_ptr<GrDrawOp> Make() { return std::unique_ptr<GrDrawOp>(new TestOp); }

private:
    TestOp() : INHERITED(ClassID(), SkRect::MakeWH(100, 100), 0xFFFFFFFF) {}

    void onPrepareDraws(Target* target) const override { return; }

    typedef GrTestMeshDrawOp INHERITED;
};

/**
 * FP used to test ref/IO counts on owned GrGpuResources. Can also be a parent FP to test counts
 * of resources owned by child FPs.
 */
class TestFP : public GrFragmentProcessor {
public:
    struct Image {
        Image(sk_sp<GrTexture> texture, GrIOType ioType) : fTexture(texture), fIOType(ioType) {}
        sk_sp<GrTexture> fTexture;
        GrIOType fIOType;
    };
    static sk_sp<GrFragmentProcessor> Make(sk_sp<GrFragmentProcessor> child) {
        return sk_sp<GrFragmentProcessor>(new TestFP(std::move(child)));
    }
    static sk_sp<GrFragmentProcessor> Make(const SkTArray<sk_sp<GrTexture>>& textures,
                                           const SkTArray<sk_sp<GrBuffer>>& buffers,
                                           const SkTArray<Image>& images) {
        return sk_sp<GrFragmentProcessor>(new TestFP(textures, buffers, images));
    }

    const char* name() const override { return "test"; }

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        // We don't really care about reusing these.
        static int32_t gKey = 0;
        b->add32(sk_atomic_inc(&gKey));
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // We don't care about optimizing these processors.
        inout->setToUnknown();
    }

private:
    TestFP(const SkTArray<sk_sp<GrTexture>>& textures, const SkTArray<sk_sp<GrBuffer>>& buffers,
           const SkTArray<Image>& images)
            : fSamplers(4), fBuffers(4), fImages(4) {
        for (const auto& texture : textures) {
            this->addTextureSampler(&fSamplers.emplace_back(texture.get()));
        }
        for (const auto& buffer : buffers) {
            this->addBufferAccess(&fBuffers.emplace_back(kRGBA_8888_GrPixelConfig, buffer.get()));
        }
        for (const Image& image : images) {
            this->addImageStorageAccess(&fImages.emplace_back(
                    image.fTexture, image.fIOType, GrSLMemoryModel::kNone, GrSLRestrict::kNo));
        }
    }

    TestFP(sk_sp<GrFragmentProcessor> child) : fSamplers(4), fBuffers(4), fImages(4) {
        this->registerChildProcessor(std::move(child));
    }

    virtual GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        class TestGLSLFP : public GrGLSLFragmentProcessor {
        public:
            TestGLSLFP() {}
            void emitCode(EmitArgs& args) override {
                GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
                fragBuilder->codeAppendf("%s = %s;", args.fOutputColor, args.fInputColor);
            }

        private:
        };
        return new TestGLSLFP();
    }

    bool onIsEqual(const GrFragmentProcessor&) const override { return false; }

    GrTAllocator<TextureSampler> fSamplers;
    GrTAllocator<BufferAccess> fBuffers;
    GrTAllocator<ImageStorageAccess> fImages;
};
}

template <typename T>
inline void testingOnly_getIORefCnts(const T* resource, int* refCnt, int* readCnt, int* writeCnt) {
    *refCnt = resource->fRefCnt;
    *readCnt = resource->fPendingReads;
    *writeCnt = resource->fPendingWrites;
}

DEF_GPUTEST_FOR_ALL_CONTEXTS(ProcessorRefTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    GrTextureDesc desc;
    desc.fConfig = kRGBA_8888_GrPixelConfig;
    desc.fWidth = 10;
    desc.fHeight = 10;

    for (int parentCnt = 0; parentCnt < 2; parentCnt++) {
        sk_sp<GrRenderTargetContext> renderTargetContext(context->makeRenderTargetContext(
                SkBackingFit::kApprox, 1, 1, kRGBA_8888_GrPixelConfig, nullptr));
        {
            bool texelBufferSupport = context->caps()->shaderCaps()->texelBufferSupport();
            bool imageLoadStoreSupport = context->caps()->shaderCaps()->imageLoadStoreSupport();
            sk_sp<GrTexture> texture1(
                    context->resourceProvider()->createTexture(desc, SkBudgeted::kYes));
            sk_sp<GrTexture> texture2(
                    context->resourceProvider()->createTexture(desc, SkBudgeted::kYes));
            sk_sp<GrTexture> texture3(
                    context->resourceProvider()->createTexture(desc, SkBudgeted::kYes));
            sk_sp<GrTexture> texture4(
                    context->resourceProvider()->createTexture(desc, SkBudgeted::kYes));
            sk_sp<GrBuffer> buffer(texelBufferSupport
                                           ? context->resourceProvider()->createBuffer(
                                                     1024, GrBufferType::kTexel_GrBufferType,
                                                     GrAccessPattern::kStatic_GrAccessPattern, 0)
                                           : nullptr);
            {
                SkTArray<sk_sp<GrTexture>> textures;
                SkTArray<sk_sp<GrBuffer>> buffers;
                SkTArray<TestFP::Image> images;
                textures.push_back(texture1);
                if (texelBufferSupport) {
                    buffers.push_back(buffer);
                }
                if (imageLoadStoreSupport) {
                    images.emplace_back(texture2, GrIOType::kRead_GrIOType);
                    images.emplace_back(texture3, GrIOType::kWrite_GrIOType);
                    images.emplace_back(texture4, GrIOType::kRW_GrIOType);
                }
                std::unique_ptr<GrDrawOp> op(TestOp::Make());
                GrPaint paint;
                auto fp = TestFP::Make(std::move(textures), std::move(buffers), std::move(images));
                for (int i = 0; i < parentCnt; ++i) {
                    fp = TestFP::Make(std::move(fp));
                }
                paint.addColorFragmentProcessor(std::move(fp));
                renderTargetContext->priv().testingOnly_addDrawOp(std::move(paint), GrAAType::kNone,
                                                                  std::move(op));
            }
            int refCnt, readCnt, writeCnt;

            testingOnly_getIORefCnts(texture1.get(), &refCnt, &readCnt, &writeCnt);
            REPORTER_ASSERT(reporter, 1 == refCnt);
            REPORTER_ASSERT(reporter, 1 == readCnt);
            REPORTER_ASSERT(reporter, 0 == writeCnt);

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }

            if (imageLoadStoreSupport) {
                testingOnly_getIORefCnts(texture2.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(texture3.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 1 == writeCnt);

                testingOnly_getIORefCnts(texture4.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 1 == readCnt);
                REPORTER_ASSERT(reporter, 1 == writeCnt);
            }

            context->flush();

            testingOnly_getIORefCnts(texture1.get(), &refCnt, &readCnt, &writeCnt);
            REPORTER_ASSERT(reporter, 1 == refCnt);
            REPORTER_ASSERT(reporter, 0 == readCnt);
            REPORTER_ASSERT(reporter, 0 == writeCnt);

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(buffer.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }

            if (texelBufferSupport) {
                testingOnly_getIORefCnts(texture2.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(texture3.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);

                testingOnly_getIORefCnts(texture4.get(), &refCnt, &readCnt, &writeCnt);
                REPORTER_ASSERT(reporter, 1 == refCnt);
                REPORTER_ASSERT(reporter, 0 == readCnt);
                REPORTER_ASSERT(reporter, 0 == writeCnt);
            }
        }
    }
}
#endif
