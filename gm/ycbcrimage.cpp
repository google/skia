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

    DrawResult createYCbCrImage(GrDirectContext* dContext, SkString* errorMsg) {
        std::unique_ptr<VkYcbcrSamplerHelper> ycbcrHelper(new VkYcbcrSamplerHelper(dContext));

        if (!ycbcrHelper->isYCbCrSupported()) {
            *errorMsg = "YCbCr sampling not supported.";
            return skiagm::DrawResult::kSkip;
        }

        if (!ycbcrHelper->createBackendTexture(kImageSize, kImageSize)) {
            *errorMsg = "Failed to create I420 backend texture.";
            return skiagm::DrawResult::kFail;
        }

        SkASSERT(!fYCbCrImage);
        fYCbCrImage = SkImages::BorrowTextureFrom(dContext,
                                                  ycbcrHelper->backendTexture(),
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

    DrawResult onGpuSetup(SkCanvas* canvas, SkString* errorMsg, GraphiteTestContext*) override {
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
