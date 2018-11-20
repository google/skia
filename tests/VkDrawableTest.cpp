/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && defined(SK_VULKAN)

#include "vk/GrVkVulkan.h"

#include "GrBackendDrawableInfo.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "SkDrawable.h"
#include "SkSurface.h"
#include "Test.h"
#include "vk/GrVkGpu.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkMemory.h"
#include "vk/GrVkUtil.h"

using sk_gpu_test::GrContextFactory;

static const int DEV_W = 16, DEV_H = 16;

class TestDrawable : public SkDrawable {
public:
    TestDrawable(const GrVkInterface* interface, int32_t width, int32_t height)
            : INHERITED()
            , fInterface(interface)
            , fWidth(width)
            , fHeight(height) {}

    ~TestDrawable() override {}

    class DrawHandler : public GpuDrawHandler {
    public:
        DrawHandler(const GrVkInterface* interface, int32_t width, int32_t height)
            : INHERITED()
            , fInterface(interface)
            , fWidth(width)
            , fHeight(height) {}
        ~DrawHandler() override {}

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
            attachment.colorAttachment = vkInfo.fImageAttachmentIndex;
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

        typedef GpuDrawHandler INHERITED;
    };

    std::unique_ptr<GpuDrawHandler> onSnapGpuDrawHandler(GrBackendApi backendApi,
                                                         const SkMatrix& matrix) override {
        if (backendApi != GrBackendApi::kVulkan) {
            return nullptr;
        }
        std::unique_ptr<DrawHandler> draw(new DrawHandler(fInterface, fWidth, fHeight));
        return std::move(draw);
    }

    SkRect onGetBounds() override {
        return SkRect::MakeLTRB(fWidth / 2, 0, fWidth, fHeight);
    }

    void onDraw(SkCanvas*) override {
        SkASSERT(false);
    }

private:
    const GrVkInterface* fInterface;
    int32_t fWidth;
    int32_t fHeight;

    typedef SkDrawable INHERITED;
};

void draw_drawable_test(skiatest::Reporter* reporter, GrContext* context) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->contextPriv().getGpu());

    const SkImageInfo ii = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo,
                                                         ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    sk_sp<TestDrawable> drawable(new TestDrawable(gpu->vkInterface(), DEV_W, DEV_H));
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
    for (int cy = 0; cy < DEV_H || failureFound; ++cy) {
        for (int cx = 0; cx < DEV_W || failureFound; ++cx) {
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
    draw_drawable_test(reporter, ctxInfo.grContext());
}

#endif
