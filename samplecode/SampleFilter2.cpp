/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkFont.h"
#include "include/core/SkString.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "tools/Resources.h"

#include <vector>

static const char* gNames[] = {
    "images/mandrill_512_q075.jpg",
    "images/dog.jpg",
};

struct Filter2View : public Sample {
    std::vector<SkBitmap> fBitmaps;

    void onOnceBeforeDraw() override {
        SkASSERT(fBitmaps.empty());
        fBitmaps.reserve(SK_ARRAY_COUNT(gNames) * 2);
        for (const char* name : gNames) {
            SkBitmap bitmap;
            (void)decode_file(GetResourceAsData(name), &bitmap);
            fBitmaps.push_back(std::move(bitmap));
        }
        for (const char* name : gNames) {
            SkBitmap bitmap;
            (void)decode_file(GetResourceAsData(name), &bitmap, kRGB_565_SkColorType);
            fBitmaps.push_back(std::move(bitmap));
        }
        this->setBGColor(SK_ColorGRAY);
    }

    SkString name() override { return SkString("Filter/Dither"); }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->translate(SkIntToScalar(10), SkIntToScalar(50));

        const SkScalar W = SkIntToScalar(fBitmaps[0].width() + 1);
        const SkScalar H = SkIntToScalar(fBitmaps[0].height() + 1);
        SkPaint paint;

        const SkScalar scale = 0.897917f;
        canvas->scale(SK_Scalar1, scale);

        for (int k = 0; k < 2; k++) {
            paint.setFilterQuality(k == 1 ? kLow_SkFilterQuality : kNone_SkFilterQuality);
            for (int j = 0; j < 2; j++) {
                paint.setDither(j == 1);
                for (int i = 0; i < (int)fBitmaps.size(); i++) {
                    SkScalar x = (k * (int)fBitmaps.size() + j) * W;
                    SkScalar y = i * H;
                    x = SkScalarRoundToScalar(x);
                    y = SkScalarRoundToScalar(y);
                    canvas->drawBitmap(fBitmaps[i], x, y, &paint);
                    SkFont font;
                    font.setSize(SkIntToScalar(18));
                    if (i == 0) {
                        SkString s("dither=");
                        s.appendS32(paint.isDither());
                        s.append(" filter=");
                        s.appendS32(paint.getFilterQuality() != kNone_SkFilterQuality);
                        SkTextUtils::DrawString(canvas, s.c_str(), x + W/2, y - font.getSize(), font, SkPaint(),
                                                SkTextUtils::kCenter_Align);
                    }
                    if (k+j == 2) {
                        SkString s;
                        s.append(" depth=");
                        s.appendS32(fBitmaps[i].colorType() == kRGB_565_SkColorType ? 16 : 32);
                        SkTextUtils::DrawString(canvas, s.c_str(), x + W + SkIntToScalar(4), y + H/2, font, SkPaint());
                    }
                }
            }
        }
    }
};
DEF_SAMPLE( return new Filter2View(); )
