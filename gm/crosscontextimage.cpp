/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "tools/Resources.h"

#include "include/core/SkImage.h"
#include "include/gpu/GrContext.h"

DEF_SIMPLE_GPU_GM_CAN_FAIL(cross_context_image, context, rtc, canvas, errorMsg,
                           5 * 256 + 60, 256 + 128 + 30) {
    sk_sp<SkData> encodedData = GetResourceAsData("images/mandrill_256.png");
    if (!encodedData) {
        *errorMsg = "Could not load mandrill_256.png. Did you forget to set the resourcePath?";
        return skiagm::DrawResult::kFail;
    }

    sk_sp<SkImage> images[5];
    images[0] = SkImage::MakeFromEncoded(encodedData);

    SkBitmap bmp;
    SkPixmap pixmap;
    SkAssertResult(images[0]->asLegacyBitmap(&bmp) &&
                   bmp.peekPixels(&pixmap));

    images[1] = SkImage::MakeCrossContextFromEncoded(context, encodedData, false, nullptr);
    images[2] = SkImage::MakeCrossContextFromEncoded(context, encodedData, true, nullptr);
    images[3] = SkImage::MakeCrossContextFromPixmap(context, pixmap, false, nullptr);
    images[4] = SkImage::MakeCrossContextFromPixmap(context, pixmap, true, nullptr);

    canvas->translate(10, 10);

    for (size_t i = 0; i < SK_ARRAY_COUNT(images); ++i) {
        canvas->save();

        canvas->drawImage(images[i], 0, 0);
        canvas->translate(0, 256 + 10);

        canvas->drawImage(images[i]->makeSubset(SkIRect::MakeXYWH(64, 64, 128, 128)), 0, 0);
        canvas->translate(128, 0);

        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);
        canvas->drawImageRect(images[i], SkRect::MakeWH(128, 128), &paint);

        canvas->restore();
        canvas->translate(256 + 10, 0);
    }
    return skiagm::DrawResult::kOk;
}
