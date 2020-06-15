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
#include "include/core/SkImage.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "tools/gpu/vk/VkTestHelper.h"
#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

// A GrContext/VkYcbcrSamplerHelper pair that allows the YCbCr image's backing memory to live
// beyond the lifespan of the SkImage.
class YCbCrCallbackContext {
public:
    static void Release(void* releaseContext) {
        YCbCrCallbackContext* ycbcrContext = reinterpret_cast<YCbCrCallbackContext*>(releaseContext);

        ycbcrContext->fYCbCrHelper.destroyBackendTexture();
        delete ycbcrContext;
    }

    YCbCrCallbackContext(GrContext* context) : fYCbCrHelper(context) {}

    skiagm::DrawResult allocVkMemory(int width, int height, SkString* errorMsg) {
        if (!fYCbCrHelper.isYCbCrSupported()) {
            *errorMsg = "YCbCr sampling not supported.";
            return skiagm::DrawResult::kSkip;
        }

        if (!fYCbCrHelper.createBackendTexture(width, height)) {
            *errorMsg = "Failed to create I420 backend texture.";
            return skiagm::DrawResult::kFail;
        }

        return skiagm::DrawResult::kOk;
    }

    const GrBackendTexture& backendTexture() { return fYCbCrHelper.backendTexture(); }

private:
    VkYcbcrSamplerHelper fYCbCrHelper;
};

namespace skiagm {

// This GM exercises the native YCbCr image format on Vulkan
class YCbCrImageGM : public GpuGM {
public:
    YCbCrImageGM() {
        this->setBGColor(0xFFCCCCCC);
    }

protected:

    SkString onShortName() override {
        return SkString("ycbcrimage");
    }

    SkISize onISize() override {
        return SkISize::Make(2*kPad+kImageSize, 2*kPad+kImageSize);
    }

    DrawResult createYCbCrImage(GrContext* context, SkString* errorMsg) {
        std::unique_ptr<YCbCrCallbackContext> ycbcrContext(new YCbCrCallbackContext(context));

        DrawResult result = ycbcrContext->allocVkMemory(kImageSize, kImageSize, errorMsg);
        if (result != DrawResult::kOk) {
            return result;
        }

        SkASSERT(!fYCbCrImage);
        fYCbCrImage = SkImage::MakeFromTexture(context, ycbcrContext->backendTexture(),
                                               kTopLeft_GrSurfaceOrigin, kRGB_888x_SkColorType,
                                               kPremul_SkAlphaType, nullptr,
                                               YCbCrCallbackContext::Release, ycbcrContext.get());
        if (!fYCbCrImage) {
            *errorMsg = "Failed to create I420 image.";
            return DrawResult::kFail;
        }

        ycbcrContext.release();
        return DrawResult::kOk;
    }

    DrawResult onGpuSetup(GrContext* context, SkString* errorMsg) override {
        SkASSERT(context->priv().asDirectContext());

        if (context->backend() != GrBackendApi::kVulkan) {
            *errorMsg = "This GM requires a Vulkan context.";
            return DrawResult::kSkip;
        }

        DrawResult result = this->createYCbCrImage(context, errorMsg);
        if (result != DrawResult::kOk) {
            return result;
        }

        return DrawResult::kOk;
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (context->backend() != GrBackendApi::kVulkan) {
            *errorMsg = "This GM requires a Vulkan context.";
            return DrawResult::kSkip;
        }

        SkASSERT(fYCbCrImage);

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);

        canvas->drawImage(fYCbCrImage, kPad, kPad, &paint);
        return DrawResult::kOk;
    }

private:
    static const int kImageSize = 112;
    static const int kPad = 8;

    sk_sp<SkImage> fYCbCrImage;

    typedef GpuGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new YCbCrImageGM;)

} // skiagm

#endif // SK_VULKAN
