/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

// This test only works with the Vulkan backend.
#ifdef SK_VULKAN

#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkImageGanesh.h"
#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Image.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/gpu/vk/VulkanBackendContext.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/vk/VulkanSharedContext.h"
#include "tools/graphite/vk/GraphiteVulkanTestContext.h"

using VulkanTestContext = skiatest::graphite::VulkanTestContext;
using SharedContext = skgpu::graphite::SharedContext;
using VulkanSharedContext = skgpu::graphite::VulkanSharedContext;
#endif

static void release_ycbcrhelper(void* releaseContext) {
    VkYcbcrSamplerHelper* ycbcrHelper = reinterpret_cast<VkYcbcrSamplerHelper*>(releaseContext);
    delete ycbcrHelper;
}

namespace skiagm {

// This GM exercises the native YCbCr image format on Vulkan
class YCbCrImageGM : public GM {
public:
    YCbCrImageGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:
    SkString getName() const override { return SkString("ycbcrimage"); }

    SkISize getISize() override {
        return SkISize::Make(2*kPad+kImageSize, 2*kPad+kImageSize);
    }

#if defined(SK_GRAPHITE)
    DrawResult createYCbCrImage(skgpu::graphite::Recorder* recorder,
                                skiatest::graphite::GraphiteTestContext* graphiteTestContext,
                                SkString* errorMsg) {
        if (!graphiteTestContext || !recorder) {
            *errorMsg = "Cannot generate a YCbCr image without a valid GraphiteTestContext and "
                        "recorder.";
            return skiagm::DrawResult::kSkip;
        }

        SkASSERT_RELEASE(recorder->backend() == skgpu::BackendApi::kVulkan);

        VulkanTestContext* vkTestCtxt = static_cast<VulkanTestContext*>(graphiteTestContext);

        const VulkanSharedContext* vulkanSharedCtxt =
                static_cast<const VulkanSharedContext*>(recorder->priv().sharedContext());
        SkASSERT(vulkanSharedCtxt);

        std::unique_ptr<VkYcbcrSamplerHelper> ycbcrHelper(
                new VkYcbcrSamplerHelper(vulkanSharedCtxt,
                                         vkTestCtxt->getBackendContext().fPhysicalDevice));
        if (!ycbcrHelper) {
            *errorMsg = "Failed to create VkYcbcrSamplerHelper.";
            return skiagm::DrawResult::kFail;
        }
        if (!ycbcrHelper->isYCbCrSupported()) {
            *errorMsg = "YCbCr sampling is not supported.";
            return skiagm::DrawResult::kSkip;
        }
        if (!ycbcrHelper->createBackendTexture(kImageSize, kImageSize)) {
            *errorMsg = "Failed to create I420 backend texture.";
            return skiagm::DrawResult::kFail;
        }

        SkASSERT(!fYCbCrImage);

        // TODO(b/311392779): Once graphite supports YCbCr sampling, actually create the image and
        // return either DrawResult::Ok or DrawResult::kFail. For now, clean up the helper and
        // texture manually.
        recorder->deleteBackendTexture(ycbcrHelper->backendTexture());
        ycbcrHelper.release();
        return skiagm::DrawResult::kSkip;
        // fYCbCrImage = SkImages::WrapTexture(recorder,
        //                                     ycbcrHelper->backendTexture(),
        //                                     kRGB_888x_SkColorType,
        //                                     kPremul_SkAlphaType,
        //                                     /*colorSpace=*/nullptr,
        //                                     release_ycbcrhelper,
        //                                     ycbcrHelper.get());
        // SkASSERT(fYCbCrImage);
        // ycbcrHelper.release();
        // if (!fYCbCrImage) {
        //     *errorMsg = "Failed to create I420 SkImage.";
        //     return DrawResult::kFail;
        // }
        // return DrawResult::kOk;
    }
#endif // SK_GRAPHITE

    DrawResult createYCbCrImage(GrDirectContext* dContext, SkString* errorMsg) {
        std::unique_ptr<VkYcbcrSamplerHelper> ycbcrHelper(new VkYcbcrSamplerHelper(dContext));

        if (!ycbcrHelper->isYCbCrSupported()) {
            *errorMsg = "YCbCr sampling not supported.";
            return skiagm::DrawResult::kSkip;
        }

        if (!ycbcrHelper->createGrBackendTexture(kImageSize, kImageSize)) {
            *errorMsg = "Failed to create I420 backend texture.";
            return skiagm::DrawResult::kFail;
        }

        SkASSERT(!fYCbCrImage);
        fYCbCrImage = SkImages::BorrowTextureFrom(dContext,
                                                  ycbcrHelper->grBackendTexture(),
                                                  kTopLeft_GrSurfaceOrigin,
                                                  kRGB_888x_SkColorType,
                                                  kPremul_SkAlphaType,
                                                  nullptr,
                                                  release_ycbcrhelper,
                                                  ycbcrHelper.get());
        ycbcrHelper.release();
        if (!fYCbCrImage) {
            *errorMsg = "Failed to create I420 image.";
            return DrawResult::kFail;
        }

        return DrawResult::kOk;
    }

    DrawResult onGpuSetup(SkCanvas* canvas,
                          SkString* errorMsg,
                          GraphiteTestContext* graphiteTestContext) override {
#if defined(SK_GRAPHITE)
        skgpu::graphite::Recorder* recorder = canvas->recorder();

        if (recorder) {
            if (recorder->backend() != skgpu::BackendApi::kVulkan) {
                *errorMsg = "This GM requires using Vulkan.";
                return DrawResult::kSkip;
            }

            return this->createYCbCrImage(recorder, graphiteTestContext, errorMsg);
        } else
#endif
        {
            GrDirectContext* dContext = GrAsDirectContext(canvas->recordingContext());

            if (!dContext || dContext->abandoned()) {
                return DrawResult::kSkip;
            }

            if (dContext->backend() != GrBackendApi::kVulkan) {
                *errorMsg = "This GM requires a Vulkan context.";
                return DrawResult::kSkip;
            }

            DrawResult result = this->createYCbCrImage(dContext, errorMsg);
            if (result != DrawResult::kOk) {
                return result;
            }

            return DrawResult::kOk;
        }
    }

    void onGpuTeardown() override {
        fYCbCrImage = nullptr;
    }

    DrawResult onDraw(SkCanvas* canvas, SkString*) override {
        SkASSERT(fYCbCrImage);
        canvas->drawImage(fYCbCrImage, kPad, kPad, SkSamplingOptions(SkFilterMode::kLinear));
        return DrawResult::kOk;
    }

private:
    static const int kImageSize = 112;
    static const int kPad = 8;

    sk_sp<SkImage> fYCbCrImage;

    using INHERITED = GpuGM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new YCbCrImageGM;)

}  // namespace skiagm

#endif // SK_VULKAN
