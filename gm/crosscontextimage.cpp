/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "Resources.h"

#include "GrContext.h"
#include "SkImage.h"

const int kPad = 10;
const int kLargeSize = 256;
const int kSmallSize = 128;

DEF_SIMPLE_GM(cross_context_image, canvas,
              5 * kLargeSize + 6 * kPad, kLargeSize + kSmallSize + 3 * kPad) {
    GrContext* context = canvas->getGrContext();
    if (!context) {
        skiagm::GM::DrawGpuOnlyMessage(canvas);
        return;
    }

    sk_sp<SkData> encodedData = GetResourceAsData("images/mandrill_256.png");
    if (!encodedData) {
        skiagm::GM::DrawFailureMessage(canvas, "Could not load mandrill_256.png. "
                                               "Did you forget to set the resourcePath?");
        return;
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

    canvas->translate(kPad, kPad);

    for (int i = 0; i < SK_ARRAY_COUNT(images); ++i) {
        canvas->drawImage(images[i], 0, 0);

        sk_sp<SkImage> subset = images[i]->makeSubset(SkIRect::MakeXYWH(64, 64, 128, 128));
        canvas->drawImage(subset, 0, kLargeSize + kPad);

        SkPaint paint;
        paint.setFilterQuality(kMedium_SkFilterQuality);
        canvas->drawImageRect(images[i], SkRect::MakeXYWH(kSmallSize, kLargeSize + kPad, 128, 128),
                              &paint);

        canvas->translate(kLargeSize + kPad, 0);
    }
}
