/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkOverdrawColorFilter.h"

static inline void set_bitmap(SkBitmap* bitmap, uint8_t alpha) {
    for (int y = 0; y < bitmap->height(); y++) {
        for (int x = 0; x < bitmap->width(); x++) {
            uint8_t* addr = bitmap->getAddr8(x, y);
            *addr = alpha;
        }
    }

    bitmap->notifyPixelsChanged();
}

class OverdrawColorFilter : public skiagm::GM {
    SkString onShortName() override { return SkString("overdrawcolorfilter"); }

    SkISize onISize() override { return {200, 400}; }

    void onDraw(SkCanvas* canvas) override {
        static const SkColor colors[SkOverdrawColorFilter::kNumColors] = {
                0x80FF0000, 0x8000FF00, 0x800000FF, 0x80FFFF00, 0x8000FFFF, 0x80FF00FF,
        };

        SkPaint paint;
        sk_sp<SkColorFilter> colorFilter = SkOverdrawColorFilter::MakeWithSkColors(colors);
        paint.setColorFilter(colorFilter);

        SkImageInfo info = SkImageInfo::MakeA8(100, 100);
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        set_bitmap(&bitmap, 0);
        canvas->drawBitmap(bitmap, 0, 0, &paint);
        set_bitmap(&bitmap, 1);
        canvas->drawBitmap(bitmap, 0, 100, &paint);
        set_bitmap(&bitmap, 2);
        canvas->drawBitmap(bitmap, 0, 200, &paint);
        set_bitmap(&bitmap, 3);
        canvas->drawBitmap(bitmap, 0, 300, &paint);
        set_bitmap(&bitmap, 4);
        canvas->drawBitmap(bitmap, 100, 0, &paint);
        set_bitmap(&bitmap, 5);
        canvas->drawBitmap(bitmap, 100, 100, &paint);
        set_bitmap(&bitmap, 6);
        canvas->drawBitmap(bitmap, 100, 200, &paint);
    }
};

DEF_GM(return new OverdrawColorFilter;)
