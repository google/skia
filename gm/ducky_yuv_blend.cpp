/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkYUVASizeInfo.h"
#include "include/core/SkYUVAIndex.h"
#include "src/core/SkCachedData.h"
#include "src/image/SkImage_Base.h"
#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

DEF_SIMPLE_GM_CAN_FAIL(ducky_yuv_blend, canvas, errorMsg, 1000, 1000) {
    auto duckyBG = GetResourceAsImage("ducky.png");
    auto duckyFG = GetResourceAsImage("ducky.jpg");
    if (!duckyFG || !duckyBG) {
        *errorMsg = "Image(s) failed to load.";
        return skiagm::DrawResult::kFail;
    }

    static constexpr int kNumPerRow = 4;
    static constexpr int kPad = 10;
    static constexpr auto kDstRect = SkRect::MakeWH(130, 130);

    ToolUtils::draw_checkerboard(canvas, SK_ColorDKGRAY, SK_ColorLTGRAY, 5 * kPad);
    canvas->translate(kPad, kPad);
    canvas->save();
    int rowCnt = 0;
    if (auto* context = canvas->getGrContext()) {
        SkYUVASizeInfo info;
        SkYUVAIndex indices[4];
        SkYUVColorSpace yuvColorSpace;
        const void* planes[4];
        auto data = as_IB(duckyFG)->getPlanes(&info, indices, &yuvColorSpace, planes);
        SkPixmap pixmaps[4];
        for (int i = 0; i < 4; ++i) {
            if (indices[i].fIndex >= 0) {
                pixmaps[i].reset(SkImageInfo::MakeA8(info.fSizes[i]), planes[i], info.fWidthBytes[i]);
            }
        }
        duckyFG = SkImage::MakeFromYUVAPixmaps(context, yuvColorSpace, pixmaps, indices, duckyFG->dimensions(), kTopLeft_GrSurfaceOrigin, true);
    }
    for (int bm = static_cast<int>(SkBlendMode::kLastCoeffMode) + 1; bm < static_cast<int>(SkBlendMode::kLastMode); ++bm) {
        auto mode = static_cast<SkBlendMode>(bm);
        SkPaint paint;
        //paint.setBlendMode(SkBlendMode::kSrc);
        paint.setFilterQuality(kMedium_SkFilterQuality);
        canvas->drawImageRect(duckyBG, kDstRect, &paint);
        paint.setBlendMode(mode);
        canvas->drawImageRect(duckyFG, kDstRect, &paint);
        canvas->translate(kDstRect.width() + kPad, 0);
        if (++rowCnt == kNumPerRow) {
            canvas->restore();
            canvas->translate(0, kDstRect.height() + kPad);
            canvas->save();
            rowCnt = 0;
        }
    }
    canvas->restore();
    return skiagm::DrawResult::kOk;
}