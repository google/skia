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
    TestDrawable(const GrVkInterface* interface, int width, int height, bool requireFlush)
            : INHERITED(kVulkan_DrawSupportFlag)
            , fInterface(interface)
            , fDevice(VK_NULL_HANDLE)
            , fCmdPool(VK_NULL_HANDLE)
            , fWaitSemaphore(VK_NULL_HANDLE)
            , fWidth(width)
            , fHeight(height)
            , fRequireFlush(requireFlush) {}

    ~TestDrawable() {
        if (fCmdPool) {
            SkASSERT(fDevice);
            GR_VK_CALL(fInterface, DestroyCommandPool(fDevice, fCmdPool, nullptr));
            GR_VK_CALL(fInterface, DestroySemaphore(fDevice, fWaitSemaphore, nullptr));
        }
    }

    bool requiresFlushBeforeDraw() override { return fRequireFlush; }

    void drawVulkan(GrVkClientDrawableInfo* info) override {
        fDevice = info->fDevice;
        VkCommandBuffer cmdBuffer;
        if (fRequireFlush) {
            // Create Command Pool
            const VkCommandPoolCreateInfo poolInfo = {
                VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, //sType
                nullptr,                                    // pNext
                VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,       // flags
                info->fQueueFamilyIndex                     // queueFamilyIndex
            };
            VkResult err = GR_VK_CALL(fInterface, CreateCommandPool(fDevice,
                                                                    &poolInfo,
                                                                    nullptr,
                                                                    &fCmdPool));
            if (err) {
                return;
            }

            // Create Command Buffer
            const VkCommandBufferAllocateInfo cmdInfo = {
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,   // sType
                nullptr,                                          // pNext
                fCmdPool,                                         // commandPool
                VK_COMMAND_BUFFER_LEVEL_PRIMARY,                  // level
                1                                                 // bufferCount
            };
            err = GR_VK_CALL(fInterface, AllocateCommandBuffers(fDevice, &cmdInfo, &cmdBuffer));
            if (err) {
                return;
            }

            VkCommandBufferBeginInfo cmdBufferBeginInfo;
            memset(&cmdBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
            cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBufferBeginInfo.pNext = nullptr;
            cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            cmdBufferBeginInfo.pInheritanceInfo = nullptr;

            GR_VK_CALL_ERRCHECK(fInterface, BeginCommandBuffer(cmdBuffer,
                                                               &cmdBufferBeginInfo));

            // Insert memory barrier
            SkASSERT(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL == info->fLayout);

            VkAccessFlags srcAccessMask = GrVkMemory::LayoutToSrcAccessMask(info->fLayout);
            VkPipelineStageFlags srcStageMask =
                    GrVkMemory::LayoutToPipelineStageFlags(info->fLayout);
            VkImageMemoryBarrier imageMemoryBarrier = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
                nullptr,                                         // pNext
                srcAccessMask,                                   // outputMask
                VK_ACCESS_TRANSFER_WRITE_BIT,                    // inputMask
                info->fLayout,                                   // oldLayout
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,            // newLayout
                VK_QUEUE_FAMILY_IGNORED,                         // srcQueueFamilyIndex
                VK_QUEUE_FAMILY_IGNORED,                         // dstQueueFamilyIndex
                info->fImage,                                    // image
                { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }        // subresourceRange
            };

            GR_VK_CALL(fInterface, CmdPipelineBarrier(cmdBuffer,
                                                      srcStageMask,
                                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                      0,
                                                      0, nullptr,
                                                      0, nullptr,
                                                      1, &imageMemoryBarrier));
            // Clear to Purple
            VkClearColorValue vkColor;
            vkColor.float32[0] = 1.0f; // r
            vkColor.float32[1] = 0.0f; // g
            vkColor.float32[2] = 1.0f; // b
            vkColor.float32[3] = 1.0f; // a

            VkImageSubresourceRange subRange;
            subRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subRange.baseMipLevel = 0;
            subRange.levelCount = 1;
            subRange.baseArrayLayer = 0;
            subRange.layerCount = 1;

            GR_VK_CALL(fInterface, CmdClearColorImage(cmdBuffer,
                                                      info->fImage,
                                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                                      &vkColor,
                                                      1,
                                                      &subRange));

            // Insert memory barrier to go back to original
            VkImageMemoryBarrier imageMemoryBarrier2 = {
                VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,          // sType
                nullptr,                                         // pNext
                VK_ACCESS_TRANSFER_WRITE_BIT,                    // outputMask
                srcAccessMask,                                   // inputMask
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,            // oldLayout
                info->fLayout,                                   // newLayout
                VK_QUEUE_FAMILY_IGNORED,                         // srcQueueFamilyIndex
                VK_QUEUE_FAMILY_IGNORED,                         // dstQueueFamilyIndex
                info->fImage,                                    // image
                { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }        // subresourceRange
            };

            GR_VK_CALL(fInterface, CmdPipelineBarrier(cmdBuffer,
                                                      VK_PIPELINE_STAGE_TRANSFER_BIT,
                                                      srcStageMask,
                                                      0,
                                                      0, nullptr,
                                                      0, nullptr,
                                                      1, &imageMemoryBarrier2));

            GR_VK_CALL_ERRCHECK(fInterface, EndCommandBuffer(cmdBuffer));

            VkPipelineStageFlags waitStages = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;

            VkSubmitInfo submitInfo;
            memset(&submitInfo, 0, sizeof(VkSubmitInfo));
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.pNext = nullptr;
            submitInfo.waitSemaphoreCount = 1;
            submitInfo.pWaitSemaphores = &info->fWaitSemaphore;
            submitInfo.pWaitDstStageMask = &waitStages;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers = &cmdBuffer;
            submitInfo.signalSemaphoreCount = 1;
            submitInfo.pSignalSemaphores = &info->fSignalSemaphore;

            fWaitSemaphore = info->fWaitSemaphore;

            GR_VK_CALL_ERRCHECK(fInterface, QueueSubmit(info->fQueue, 1, &submitInfo, VK_NULL_HANDLE));
        } else {
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
            attachment.colorAttachment = info->fImageAttachmentIndex;
            attachment.clearValue.color = vkColor;

            GR_VK_CALL(fInterface, CmdClearAttachments(info->fCommandBuffer,
                                                       1,
                                                       &attachment,
                                                       1,
                                                       &clearRect));
            info->fBounds->iset(fWidth/2, 0, fWidth, fHeight);
        }
    }

    SkRect onGetBounds() override {
        return SkRect::MakeLTRB(fWidth / 2, 0, fWidth, fHeight);
    }

    void onDraw(SkCanvas*) override {
        SkASSERT(false);
    }

private:
    const GrVkInterface* fInterface;
    VkDevice fDevice;
    VkCommandPool fCmdPool;
    VkSemaphore fWaitSemaphore;
    int32_t fWidth;
    int32_t fHeight;
    bool fRequireFlush;

    typedef SkDrawable INHERITED;
};

void draw_drawable_test(skiatest::Reporter* reporter, GrContext* context, bool requireFlush) {
    GrVkGpu* gpu = static_cast<GrVkGpu*>(context->getGpu());

    const SkImageInfo ii = SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType,
                                             kPremul_SkAlphaType);
    sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(context, SkBudgeted::kNo,
                                                         ii, 0, kTopLeft_GrSurfaceOrigin, nullptr));
    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorBLUE);

    sk_sp<TestDrawable> drawable(new TestDrawable(gpu->vkInterface(), DEV_W, DEV_H, requireFlush));
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
                if (requireFlush) {
                    // In the flush mode we don't set up a render pass in order to do a partial
                    // like the no flush case. Thus we just clear the whole image and get one color
                    // for the top half of the surface.
                    expectedPixel = 0xFFFF00FF; // Purple
                } else {
                    if (cx < DEV_W / 2) {
                        expectedPixel = 0xFFFF0000; // Blue
                    } else {
                        expectedPixel = 0xFF0000FF; // Red
                    }
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
    draw_drawable_test(reporter, ctxInfo.grContext(), false);
    draw_drawable_test(reporter, ctxInfo.grContext(), true);
}

#endif
