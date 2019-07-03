/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkTime.h"
#include "include/effects/SkGradientShader.h"
#include "include/utils/SkTextUtils.h"
#include "samplecode/DecodeFile.h"
#include "samplecode/Sample.h"
#include "src/utils/SkUTF.h"

static const char* gNames[] = {
    "/skimages/background_01.png"
};

class Filter2View : public Sample {
public:
    std::unique_ptr<SkBitmap[]> fBitmaps;
    int fBitmapCount;
    int fCurrIndex;

    Filter2View() {
        fBitmapCount = SK_ARRAY_COUNT(gNames)*2;
        fBitmaps.reset(new SkBitmap[fBitmapCount]);

        for (int i = 0; i < fBitmapCount/2; i++) {
            decode_file(gNames[i], &fBitmaps[i]);
        }
        for (int i = fBitmapCount/2; i < fBitmapCount; i++) {
            decode_file(gNames[i-fBitmapCount/2], &fBitmaps[i], kRGB_565_SkColorType);
        }
        fCurrIndex = 0;

        this->setBGColor(SK_ColorGRAY);
    }

protected:
    SkString name() override { return SkStringPrintf("Filter/Dither %s", gNames[fCurrIndex]); }

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
                for (int i = 0; i < fBitmapCount; i++) {
                    SkScalar x = (k * fBitmapCount + j) * W;
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

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new Filter2View(); )
