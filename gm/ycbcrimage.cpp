/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

// This test only works with the Vulkan backend.
#ifdef SK_VULKAN

#include "tools/gpu/vk/VkYcbcrSamplerHelper.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/utils/SkRandom.h"

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
        return SkISize::Make(1000, 1000);
    }

    DrawResult onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas,
                      SkString* errorMsg) override {

        return DrawResult::kOk;
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new YCbCrImageGM;)

} // skiagm

#endif // SK_VULKAN
