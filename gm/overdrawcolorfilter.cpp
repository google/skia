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
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkOverdrawColorFilter.h"

struct OverdrawColorFilter : public skiagm::GM {
    SkString getName() const override { return SkString{"overdrawcolorfilter"}; }

    SkISize getISize() override { return {200, 400}; }

    void onDraw(SkCanvas* canvas) override {
        static const SkColor colors[SkOverdrawColorFilter::kNumColors] = {
                0x80FF0000, 0x8000FF00, 0x800000FF, 0x80FFFF00, 0x8000FFFF, 0x80FF00FF,
        };

        SkPaint paint;
        paint.setColorFilter(SkOverdrawColorFilter::MakeWithSkColors(colors));
        SkSamplingOptions sampling;

        SkImageInfo info = SkImageInfo::MakeA8(100, 100);
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        bitmap.eraseARGB(0, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 0, 0, sampling, &paint);
        bitmap.eraseARGB(1, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 0, 100, sampling, &paint);
        bitmap.eraseARGB(2, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 0, 200, sampling, &paint);
        bitmap.eraseARGB(3, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 0, 300, sampling, &paint);
        bitmap.eraseARGB(4, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 100, 0, sampling, &paint);
        bitmap.eraseARGB(5, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 100, 100, sampling, &paint);
        bitmap.eraseARGB(6, 0, 0, 0);
        canvas->drawImage(bitmap.asImage(), 100, 200, sampling, &paint);
    }
};

DEF_GM(return new OverdrawColorFilter;)
