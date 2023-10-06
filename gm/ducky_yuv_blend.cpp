/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/gpu/GrRecordingContext.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/YUVUtils.h"

// Modeled on the layout test css3/blending/background-blend-mode-image-image.html to reproduce
// skbug.com/9619
DEF_SIMPLE_GM_CAN_FAIL(ducky_yuv_blend, canvas, errorMsg, 560, 1130) {
    sk_sp<SkImage> duckyBG = ToolUtils::GetResourceAsImage("images/ducky.png");
    sk_sp<SkImage> duckyFG[2] = {ToolUtils::GetResourceAsImage("images/ducky.jpg"), nullptr};
    if (!duckyFG[0] || !duckyBG) {
        *errorMsg = "Image(s) failed to load.";
        return skiagm::DrawResult::kFail;
    }

    // If we're on the GPU we do a second round of draws where the source image is YUV planes.
    // Otherwise we just draw the original again,
    if (auto* rContext = canvas->recordingContext(); rContext && !rContext->abandoned()) {
        auto lazyYUV = sk_gpu_test::LazyYUVImage::Make(GetResourceAsData("images/ducky.jpg"),
                                                       skgpu::Mipmapped::kYes);
        if (lazyYUV) {
            duckyFG[1] = lazyYUV->refImage(rContext, sk_gpu_test::LazyYUVImage::Type::kFromPixmaps);
        }
        if (!duckyFG[1]) {
            return skiagm::DrawResult::kFail;
        }
    } else {
        duckyFG[1] = duckyFG[0];
    }

    static constexpr int kNumPerRow = 4;
    static constexpr int kPad = 10;
    static constexpr auto kDstRect = SkRect::MakeWH(130, 130);
    int rowCnt = 0;
    canvas->translate(kPad, kPad);
    canvas->save();
    auto newRow = [&] {
        canvas->restore();
        canvas->translate(0, kDstRect.height() + kPad);
        canvas->save();
        rowCnt = 0;
    };
    SkSamplingOptions sampling(SkFilterMode::kLinear,
                               SkMipmapMode::kNearest);
    ToolUtils::draw_checkerboard(
            canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, (kDstRect.height() + kPad)/5);
    for (auto& fg : duckyFG) {
        for (int bm = static_cast<int>(SkBlendMode::kLastCoeffMode) + 1;
             bm < static_cast<int>(SkBlendMode::kLastMode);
             ++bm) {
            canvas->drawImageRect(duckyBG, kDstRect, sampling, nullptr);
            SkPaint paint;
            paint.setBlendMode(static_cast<SkBlendMode>(bm));
            canvas->drawImageRect(fg, kDstRect, sampling, &paint);
            canvas->translate(kDstRect.width() + kPad, 0);
            if (++rowCnt == kNumPerRow) {
                newRow();
            }
        }
        // Force a new row between the two foreground images
        newRow();
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}
