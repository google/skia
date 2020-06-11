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
#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"


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

    DrawResult onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas,
                      SkString* errorMsg) override {
        if (context->backend() != GrBackendApi::kVulkan) {
            *errorMsg = "This GM requires a Vulkan context.";
            return DrawResult::kSkip;
        }

        VkYcbcrSamplerHelper ycbcrHelper(context);
        if (!ycbcrHelper.isYCbCrSupported()) {
            *errorMsg = "YCbCr sampling not supported.";
            return DrawResult::kSkip;
        }

        sk_sp<SkImage> ycbcrImage = ycbcrHelper.createI420Image(kImageSize, kImageSize);
        if (!ycbcrImage) {
            *errorMsg = "Failed to create I420 image.";
            return DrawResult::kFail;
        }

        SkPaint paint;
        paint.setFilterQuality(kLow_SkFilterQuality);

        canvas->drawImage(ycbcrImage, kPad, kPad, &paint);

        // The VkYcbcrSamplerHelper holds the actual memory for 'ycbcrImage' so nothing can
        // be allowed to exist beyond this method.
        GrFlushInfo flushInfo;
        flushInfo.fFlags = kSyncCpu_GrFlushFlag;
        context->flush(flushInfo);
        context->submit(true);

        return DrawResult::kOk;
    }

private:
    static const int kImageSize = 112;
    static const int kPad = 8;

    typedef GpuGM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new YCbCrImageGM;)

} // skiagm

#endif // SK_VULKAN
