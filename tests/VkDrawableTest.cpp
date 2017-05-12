/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This is a GPU-backend specific test. It relies on static intializers to work

#include "SkTypes.h"

#if SK_SUPPORT_GPU && SK_ALLOW_STATIC_GLOBAL_INITIALIZERS && defined(SK_VULKAN)

#include "GrContextFactory.h"
#include "GrTest.h"
#include "SkDrawable.h"
#include "Test.h"
#include "vk/GrVkGpu.h"
#include "vk/GrVkInterface.h"
#include "vk/GrVkUtil.h"

using sk_gpu_test::GrContextFactory;

static const int DEV_W = 16, DEV_H = 16;

class TestDrawable : public SkDrawable {
public:
    TestDrawable(const GrVkInterface* interface, int width, int height, bool requireFlush)
            : INHERTED(kVulkan_DrawSupportFlag)
            , fInterface(interface)
            , fWidth(width)
            , fHeight(height)
            , fRequireFlush(requireFlush) {}

    void drawVulkan(GrVkClientDrawableInfo* info) override {
        if (fRequireFlush) {
            SkASSERT(false); // Currently not supported
        } else {
            VkClearColorValue vkColor;
            vkColor.float32[0] = 1.0f;
            vkColor.float32[1] = 0.0f;
            vkColor.float32[2] = 0.0f;
            vkColor.float32[3] = 1.0f;

            // Clear right half of render target
            VkClearRect clearRect;
            clearRect.rect.offset = { fWidth / 2, 0 };
            clearRect.rect.extent = { fWidth / 2, fHeight };
            clearRect.baseArrayLayer = 0;
            clearRect.layerCount = 1;

            VkClearAttachment attachment;
            attachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            attachment.colorAttachment = colorIndex;
            attachment.clearValue.color = vkColor;

            GR_VK_CALL(fInterface, CmdClearAttachments(info->fCommandBuffer,
                                                       1,
                                                       &attachment,
                                                       1,
                                                       &clearRect));
            info->fBounds->iset(fWidth/2, 0, fWidth, fHeight);
        }
    }

private:
    const GrVkInterface* fInterface;
    int fWidth;
    int fHeight;
    bool fRequireFlush;
};

void draw_drawable_test(skiatest::Reporter* reporter, GrContext* context, bool requireFlush) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo,
                                                         ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);
    TestDrawable drawable(gpu->vkInterface(), DEV_W, DEV_H, requireFlush);
    canvas->drawDrawable(&drawable);

    SkPaint paint;
    paint.setColor(SK_ColorGREEN);
    SkIRect rect = SkIRect::MakeLTRB(0, DEV_H/2, DEV_W, DEV_H);
    canvas->drawIRect(rect, paint);

    // read pixels
    SkBitmap bitmap;
    canvas->readPixels(&bitmap, 0, 0);

}


DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkDrawableTest, reporter, ctxInfo) {
    draw_drawable_test(reporter, ctxInfo.grContext(), false);
}

#endif
