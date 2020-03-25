/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkYUVAIndex.h"
#include "include/core/SkYUVASizeInfo.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

// Modeled on the layout test css3/blending/background-blend-mode-image-image.html to reproduce
// skbug.com/9619
DEF_SIMPLE_GM_CAN_FAIL(ducky_yuv_blend, canvas, errorMsg, 560, 1130) {
    sk_sp<SkImage> duckyBG = GetResourceAsImage("images/ducky.png");
    sk_sp<SkImage> duckyFG[2] = {GetResourceAsImage("images/ducky.jpg"), nullptr};
    if (!duckyFG[0] || !duckyBG) {
        *errorMsg = "Image(s) failed to load.";
        return skiagm::DrawResult::kFail;
    }

    // If we're on the GPU we do a second round of draws where the source image is YUV planes.
    // Otherwise we just draw the original again,
    if (auto* context = canvas->getGrContext()) {
        SkYUVASizeInfo info;
        SkYUVAIndex indices[4];
        SkYUVColorSpace yuvColorSpace;
        const void* planes[4];
        auto data = as_IB(duckyFG[0])->getPlanes(&info, indices, &yuvColorSpace, planes);
        SkPixmap pixmaps[4];
        for (int i = 0; i < 4; ++i) {
            if (indices[i].fIndex >= 0) {
                pixmaps[i].reset(
                        SkImageInfo::MakeA8(info.fSizes[i]), planes[i], info.fWidthBytes[i]);
            }
        }
        duckyFG[1] = SkImage::MakeFromYUVAPixmaps(context,
                                                  yuvColorSpace,
                                                  pixmaps,
                                                  indices,
                                                  duckyFG[0]->dimensions(),
                                                  kTopLeft_GrSurfaceOrigin,
                                                  true);
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
    ToolUtils::draw_checkerboard(
            canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, (kDstRect.height() + kPad)/5);
    for (auto& fg : duckyFG) {
        for (int bm = static_cast<int>(SkBlendMode::kLastCoeffMode) + 1;
             bm < static_cast<int>(SkBlendMode::kLastMode);
             ++bm) {
            auto mode = static_cast<SkBlendMode>(bm);
            SkPaint paint;
            paint.setFilterQuality(kMedium_SkFilterQuality);
            canvas->drawImageRect(duckyBG, kDstRect, &paint);
            paint.setBlendMode(mode);
            canvas->drawImageRect(fg, kDstRect, &paint);
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
