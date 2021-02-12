/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "include/core/SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "include/gpu/vk/GrVkVulkan.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkDrawable.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendDrawableInfo.h"
#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/vk/GrVkGpu.h"
#include "src/gpu/vk/GrVkInterface.h"
#include "src/gpu/vk/GrVkMemory.h"
#include "src/gpu/vk/GrVkSecondaryCBDrawContext.h"
#include "src/gpu/vk/GrVkUtil.h"
#include "tests/Test.h"
#include "tools/gpu/GrContextFactory.h"

using sk_gpu_test::GrContextFactory;

static const int DEV_W = 16, DEV_H = 16;

class TestDrawable : public SkDrawable {
public:
    TestDrawable(const GrVkInterface* interface, GrDirectContext* dContext,
                 int32_t width, int32_t height)
            : INHERITED()
            , fInterface(interface)
            , fDContext(dContext)
            , fWidth(width)
            , fHeight(height) {}

    ~TestDrawable() override {}

    class DrawHandlerBasic : public GpuDrawHandler {
    public:
        DrawHandlerBasic(const GrVkInterface* interface, int32_t width, int32_t height)
            : INHERITED()
            , fInterface(interface)
            , fWidth(width)
            , fHeight(height) {}
        ~DrawHandlerBasic() override {}

        void draw(const GrBackendDrawableInfo& info) override {
            GrVkDrawableInfo vkInfo;
            SkAssertResult(info.getVkDrawableInfo(&vkInfo));

            // Clear to Red
            VkClearColorValue vkColor;
            vkColor.float32[0] = 1.0f; // r
            vkColor.float32[1] = 0.0f; // g
            vkColor.float32[2] = 0.0f; // b
            vkColor.float32[3] = 1.0f; // a

            // Clear right half of render target
            VkClearRect clearRect;
            clearRect.rect.offset = { fWidth / 2, 0 };
            clearRect.rect.extent = { (uint32_t)fWidth / 2, (uint32_t)fHeight };
            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = 1;

            VkClearAttachment attachment;
            attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            attachment.colorAttachment = vkInfo.fColorAttachmentIndex;
            attachment.clearValue.color = vkColor;

            GR_VK_CALL(fInterface, CmdClearAttachments(vkInfo.fSecondaryCommandBuffer,
                                                       1,
                                                       &attachment,
                                                       1,
                                                       &clearRect));
            vkInfo.fDrawBounds->offset = { fWidth / 2, 0 };
            vkInfo.fDrawBounds->extent = { (uint32_t)fWidth / 2, (uint32_t)fHeight };
        }
    private:
        const GrVkInterface* fInterface;
        int32_t              fWidth;
        int32_t              fHeight;

        using INHERITED = GpuDrawHandler;
    };

    typedef void (*DrawProc)(TestDrawable*, const SkMatrix&, const SkIRect&,
                             const SkImageInfo&, const GrVkDrawableInfo&);
    typedef void (*SubmitProc)(TestDrawable*);

    // Exercises the exporting of a secondary command buffer from one context and then importing
    // it into a second context. We then draw to the secondary command buffer from the second
    // context.
    class DrawHandlerImport : public GpuDrawHandler {
    public:
        DrawHandlerImport(TestDrawable* td, DrawProc drawProc, SubmitProc submitProc,
                          const SkMatrix& matrix,
                          const SkIRect& clipBounds,
                          const SkImageInfo& bufferInfo)
            : INHERITED()
            , fTestDrawable(td)
            , fDrawProc(drawProc)
            , fSubmitProc(submitProc)
            , fMatrix(matrix)
            , fClipBounds(clipBounds)
            , fBufferInfo(bufferInfo) {}
        ~DrawHandlerImport() override {
            fSubmitProc(fTestDrawable);
        }

        void draw(const GrBackendDrawableInfo& info) override {
            GrVkDrawableInfo vkInfo;
            SkAssertResult(info.getVkDrawableInfo(&vkInfo));

            fDrawProc(fTestDrawable, fMatrix, fClipBounds, fBufferInfo, vkInfo);
        }
    private:
        TestDrawable*     fTestDrawable;
        DrawProc          fDrawProc;
        SubmitProc        fSubmitProc;
        const SkMatrix    fMatrix;
        const SkIRect     fClipBounds;
        const SkImageInfo fBufferInfo;

        using INHERITED = GpuDrawHandler;
    };

    // Helper function to test drawing to a secondary command buffer that we imported into the
    // context using a GrVkSecondaryCBDrawContext.
    static void ImportDraw(TestDrawable* td, const SkMatrix& matrix, const SkIRect& clipBounds,
                           const SkImageInfo& bufferInfo, const GrVkDrawableInfo& info) {
        td->fDrawContext = GrVkSecondaryCBDrawContext::Make(td->fDContext, bufferInfo,
                                                            info, nullptr);
        if (!td->fDrawContext) {
            return;
        }

        SkCanvas* canvas = td->fDrawContext->getCanvas();
        canvas->clipRect(SkRect::Make(clipBounds));
        canvas->setMatrix(matrix);

        SkIRect rect = SkIRect::MakeXYWH(td->fWidth/2, 0, td->fWidth/4, td->fHeight);
        SkPaint paint;
        paint.setColor(SK_ColorRED);
        canvas->drawIRect(rect, paint);

        // Draw to an offscreen target so that we end up with a mix of "real" secondary command
        // buffers and the imported secondary command buffer.
        sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(td->fDContext, SkBudgeted::kYes,
                                                            bufferInfo);
        surf->getCanvas()->clear(SK_ColorRED);

        SkRect dstRect = SkRect::MakeXYWH(3*td->fWidth/4, 0, td->fWidth/4, td->fHeight);
        SkRect srcRect = SkRect::MakeIWH(td->fWidth/4, td->fHeight);
        canvas->drawImageRect(surf->makeImageSnapshot(), srcRect, dstRect, SkSamplingOptions(),
                              &paint, SkCanvas::kStrict_SrcRectConstraint);

        td->fDrawContext->flush();
    }

    // Helper function to test waiting for the imported secondary command buffer to be submitted on
    // its original context and then cleaning up the GrVkSecondaryCBDrawContext from this context.
    static void ImportSubmitted(TestDrawable* td) {
        // Typical use case here would be to create a fence that we submit to the gpu and then wait
        // on before releasing the GrVkSecondaryCBDrawContext resources. To simulate that for this
        // test (and since we are running single threaded anyways), we will just force a sync of
        // the gpu and cpu here.
        td->fDContext->submit(true);

        td->fDrawContext->releaseResources();
        // We release the context here manually to test that we waited long enough before
        // releasing the GrVkSecondaryCBDrawContext. This simulates when a client is able to delete
        // the context it used to imported the secondary command buffer. If we had released the
        // context's resources earlier (before waiting on the gpu above), we would get vulkan
        // validation layer errors saying we freed some vulkan objects while they were still in use
        // on the GPU.
        td->fDContext->releaseResourcesAndAbandonContext();
    }


    std::unique_ptr<GpuDrawHandler> onSnapGpuDrawHandler(GrBackendApi backendApi,
                                                         const SkMatrix& matrix,
                                                         const SkIRect& clipBounds,
                                                         const SkImageInfo& bufferInfo) override {
        if (backendApi != GrBackendApi::kVulkan) {
            return nullptr;
        }
        std::unique_ptr<GpuDrawHandler> draw;
        if (fDContext) {
            draw = std::make_unique<DrawHandlerImport>(this, ImportDraw, ImportSubmitted, matrix,
                                                       clipBounds, bufferInfo);
        } else {
            draw = std::make_unique<DrawHandlerBasic>(fInterface, fWidth, fHeight);
        }
        return draw;
    }

    SkRect onGetBounds() override {
        return SkRect::MakeLTRB(fWidth / 2, 0, fWidth, fHeight);
    }

    void onDraw(SkCanvas*) override {
        SkASSERT(false);
    }

private:
    const GrVkInterface* fInterface;
    GrDirectContext*     fDContext;
    sk_sp<GrVkSecondaryCBDrawContext> fDrawContext;
    int32_t              fWidth;
    int32_t              fHeight;

    using INHERITED = SkDrawable;
};

void draw_drawable_test(skiatest::Reporter* reporter,
                        GrDirectContext* dContext,
                        GrDirectContext* childDContext) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(dContext->priv().getGpu());

    const SkImageInfo ii = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(dContext, SkBudgeted::kNo,
                                                         ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    sk_sp<TestDrawable> drawable(new TestDrawable(gpu->vkInterface(), childDContext, DEV_W, DEV_H));
    canvas->drawDrawable(drawable.get());

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    SkIRect rect = SkIRect::MakeLTRB(0, DEV_H/2, DEV_W, DEV_H);
    canvas->drawIRect(rect, paint);

    // read pixels
    SkBitmap bitmap;
    bitmap.allocPixels(ii);
    canvas->readPixels(bitmap, 0, 0);

    const uint32_t* canvasPixels = static_cast<const uint32_t*>(bitmap.getPixels());
    bool failureFound = false;
    SkPMColor expectedPixel;
    for (int cy = 0; cy < DEV_H && !failureFound; ++cy) {
        for (int cx = 0; cx < DEV_W && !failureFound; ++cx) {
            SkPMColor canvasPixel = canvasPixels[cy * DEV_W + cx];
            if (cy < DEV_H / 2) {
                if (cx < DEV_W / 2) {
                    expectedPixel = 0xFFFF0000; // Blue
                } else {
                    expectedPixel = 0xFF0000FF; // Red
                }
            } else {
                expectedPixel = 0xFF00FF00; // Green
            }
            if (expectedPixel != canvasPixel) {
                failureFound = true;
                ERRORF(reporter, "Wrong color at %d, %d. Got 0x%08x when we expected 0x%08x",
                       cx, cy, canvasPixel, expectedPixel);
            }
        }
    }
}

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkDrawableTest, reporter, ctxInfo) {
    draw_drawable_test(reporter, ctxInfo.directContext(), nullptr);
}

DEF_GPUTEST(VkDrawableImportTest, reporter, options) {
    for (int typeInt = 0; typeInt < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++typeInt) {
        sk_gpu_test::GrContextFactory::ContextType contextType =
                (sk_gpu_test::GrContextFactory::ContextType) typeInt;
        if (contextType != sk_gpu_test::GrContextFactory::kVulkan_ContextType) {
            continue;
        }
        sk_gpu_test::GrContextFactory factory(options);
        sk_gpu_test::ContextInfo ctxInfo = factory.getContextInfo(contextType);
        skiatest::ReporterContext ctx(
                   reporter, SkString(sk_gpu_test::GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.directContext()) {
            sk_gpu_test::ContextInfo child =
                    factory.getSharedContextInfo(ctxInfo.directContext(), 0);
            if (!child.directContext()) {
                continue;
            }

            draw_drawable_test(reporter, ctxInfo.directContext(), child.directContext());
        }
    }
}

#endif
