/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkOverdrawColorFilter.h"

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
public:
    OverdrawColorFilter() {}

protected:
    virtual SkString onShortName() override {
        return SkString("overdrawcolorfilter");;
    }

    virtual SkISize onISize() override {
        return SkISize::Make(200, 400);
    }

    void onDraw(SkCanvas* canvas) override {
        static const SkPMColor colors[SkOverdrawColorFilter::kNumColors] = {
                0x80800000, 0x80008000, 0x80000080, 0x80808000, 0x80008080, 0x80800080,
        };

        SkPaint paint;
        sk_sp<SkColorFilter> colorFilter = SkOverdrawColorFilter::Make(colors);
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

private:
    typedef GM INHERITED;
};

DEF_GM(return new OverdrawColorFilter;)
