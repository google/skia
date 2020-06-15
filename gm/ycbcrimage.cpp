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
#include "tools/gpu/vk/VkTestHelper.h"
#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

class YCbCrStuff {
public:
    static void Release(void* releaseContext) {
        YCbCrStuff* foo = reinterpret_cast<YCbCrStuff*>(releaseContext);

        foo->fYCbCrHelper.destroyBackendTexture(foo->fTestHelper.grContext());
        delete foo;
    }

    YCbCrStuff() : fTestHelper(false) { }
    ~YCbCrStuff() {
    }

    skiagm::DrawResult init(int width, int height, SkString* errorMsg) {
        if (!fTestHelper.init()) {
            *errorMsg = "VkTestHelper initialization failed.";
            return skiagm::DrawResult::kSkip;
        }

        if (!VkYcbcrSamplerHelper::IsYCbCrSupported(fTestHelper.grContext())) {
            *errorMsg = "YCbCr sampling not supported.";
            return skiagm::DrawResult::kSkip;
        }

        if (!fYCbCrHelper.createBackendTexture(fTestHelper.grContext(), width, height)) {
            *errorMsg = "Failed to create I420 backend texture.";
            return skiagm::DrawResult::kFail;
        }

        return skiagm::DrawResult::kOk;
    }

    const GrBackendTexture& backendTexture() { return fYCbCrHelper.backendTexture(); }

private:
    VkTestHelper         fTestHelper;
    VkYcbcrSamplerHelper fYCbCrHelper;
};

namespace skiagm {

// This GM exercises the native YCbCr image format on Vulkan
class YCbCrImageGM : public GpuGM {
public:
    YCbCrImageGM() {
        this->setBGColor(0xFFCCCCCC);
    }

    ~YCbCrImageGM() {

    }

protected:

    SkString onShortName() override {
        return SkString("ycbcrimage");
    }

    SkISize onISize() override {
        return SkISize::Make(2*kPad+kImageSize, 2*kPad+kImageSize);
    }

    DrawResult init(GrContext* context, SkString* errorMsg) {
        std::unique_ptr<YCbCrStuff> foo(new YCbCrStuff);

        DrawResult result = foo->init(kImageSize, kImageSize, errorMsg);
        if (result != DrawResult::kOk) {
            return result;
        }

        fYCbCrImage = SkImage::MakeFromTexture(context, foo->backendTexture(),
                                               kTopLeft_GrSurfaceOrigin, kRGB_888x_SkColorType,
                                               kPremul_SkAlphaType, nullptr,
                                               YCbCrStuff::Release, foo.get());
        if (!fYCbCrImage) {
            *errorMsg = "Failed to create I420 image.";
            return DrawResult::kFail;
        }

        foo.release();
        return DrawResult::kOk;
    }

    DrawResult onHailMary(GrContext* context) override {
        return DrawResult::kOk;
    }

    DrawResult onDraw1(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (context->backend() != GrBackendApi::kVulkan) {
            *errorMsg = "This GM requires a Vulkan context.";
            return DrawResult::kSkip;
        }

        if (!fYCbCrImage) {
            DrawResult result = this->init(context, errorMsg);
            if (result != DrawResult::kOk) {
                return result;
            }
        }

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
