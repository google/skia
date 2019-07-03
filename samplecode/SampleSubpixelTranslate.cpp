/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "samplecode/Sample.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkFont.h"
#include "include/core/SkStream.h"
#include "include/effects/SkBlurMaskFilter.h"
#include "include/utils/SkRandom.h"
#include "samplecode/DecodeFile.h"
#include "tools/Resources.h"

// Intended to exercise pixel snapping observed with scaled images (and
// with non-scaled images, but for a different reason):  Bug 1145

class SubpixelTranslateView : public Sample {
public:
    SubpixelTranslateView(const char imageFilename[],
                          float horizontalVelocity,
                          float verticalVelocity)
        : fHorizontalVelocity(horizontalVelocity)
        , fVerticalVelocity(verticalVelocity)
    {
        if (!DecodeDataToBitmap(GetResourceAsData(imageFilename), &fBM)) {
            fBM.allocN32Pixels(1, 1);
            *(fBM.getAddr32(0,0)) = 0xFF0000FF; // red == bad
        }
        fCurPos = SkPoint::Make(0,0);
        fSize = 200;
    }

protected:
    SkBitmap fBM;
    SkScalar fSize;
    float fHorizontalVelocity, fVerticalVelocity;

    SkPoint fCurPos;

    SkString name() override { return SkString("SubpixelTranslate"); }

    void onDrawContent(SkCanvas* canvas) override {

        static const SkFilterQuality gQualitys[] = {
            kNone_SkFilterQuality,
            kLow_SkFilterQuality,
            kMedium_SkFilterQuality,
            kHigh_SkFilterQuality
        };

        SkPaint paint;
        SkFont font(nullptr, 48);
        font.setSubpixel(true);

        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            SkRect r = SkRect::MakeXYWH( fCurPos.fX + i * (fSize + 10), fCurPos.fY, fSize, fSize );
            canvas->drawBitmapRect( fBM, r, &paint );
        }

        canvas->drawString("AA Scaled", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fSize + 10),
                           fCurPos.fY + fSize/2, font, paint);

        paint.setAntiAlias(false);
        font.setEdging(SkFont::Edging::kAlias);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            SkRect r = SkRect::MakeXYWH( fCurPos.fX + i * (fSize + 10), fCurPos.fY + fSize + 10, fSize, fSize );
            canvas->drawBitmapRect( fBM, r, &paint );
        }
        canvas->drawString("Scaled", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fSize + 10),
                           fCurPos.fY + fSize + 10 + fSize/2, font, paint);

        paint.setAntiAlias(true);
        font.setEdging(SkFont::Edging::kAntiAlias);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            canvas->drawBitmap( fBM, fCurPos.fX + i * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10), &paint );
        }

        canvas->drawString("AA No Scale",
                           fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fBM.width() + 10),
                           fCurPos.fY + 2*(fSize + 10) + fSize/2, font, paint);

        paint.setAntiAlias(false);
        font.setEdging(SkFont::Edging::kAlias);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            canvas->drawBitmap( fBM, fCurPos.fX + i * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10) + fBM.height() + 10, &paint );
        }

        canvas->drawString("No Scale", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fBM.width() + 10),
                           fCurPos.fY + 2*(fSize + 10) + fBM.height() + 10 + fSize/2, font, paint);


        fCurPos.fX += fHorizontalVelocity;
        fCurPos.fY += fVerticalVelocity;
    }

private:
    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new SubpixelTranslateView("images/mandrill_256.png", .05f, .05f); )
