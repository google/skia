/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "gm.h"

#include "Resources.h"
#include "SampleCode.h"
#include "SkBlurMaskFilter.h"
#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkRandom.h"
#include "SkStream.h"

// Intended to exercise pixel snapping observed with scaled images (and
// with non-scaled images, but for a different reason):  Bug 1145

class SubpixelTranslateView : public SampleView {
public:
    SubpixelTranslateView(const char imageFilename[],
                          float horizontalVelocity,
                          float verticalVelocity)
      : fHorizontalVelocity(horizontalVelocity),
        fVerticalVelocity(verticalVelocity) {
      SkString resourcePath = GetResourcePath(imageFilename);
      if (!decode_file(resourcePath.c_str(), &fBM)) {
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

    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "SubpixelTranslate");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onDrawContent(SkCanvas* canvas) override {

        static const SkFilterQuality gQualitys[] = {
            kNone_SkFilterQuality,
            kLow_SkFilterQuality,
            kMedium_SkFilterQuality,
            kHigh_SkFilterQuality
        };

        SkPaint paint;
        paint.setTextSize(48);
        paint.setSubpixelText(true);

        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            SkRect r = SkRect::MakeXYWH( fCurPos.fX + i * (fSize + 10), fCurPos.fY, fSize, fSize );
            canvas->drawBitmapRect( fBM, r, &paint );
        }

        canvas->drawString( "AA Scaled", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fSize + 10), fCurPos.fY + fSize/2, paint );

        paint.setAntiAlias(false);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            SkRect r = SkRect::MakeXYWH( fCurPos.fX + i * (fSize + 10), fCurPos.fY + fSize + 10, fSize, fSize );
            canvas->drawBitmapRect( fBM, r, &paint );
        }
        canvas->drawString( "Scaled", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fSize + 10), fCurPos.fY + fSize + 10 + fSize/2, paint );

        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            canvas->drawBitmap( fBM, fCurPos.fX + i * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10), &paint );
        }

        canvas->drawString( "AA No Scale", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10) + fSize/2, paint );

        paint.setAntiAlias(false);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gQualitys); ++i) {
            paint.setFilterQuality(gQualitys[i]);
            canvas->drawBitmap( fBM, fCurPos.fX + i * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10) + fBM.height() + 10, &paint );
        }

        canvas->drawString( "No Scale", fCurPos.fX + SK_ARRAY_COUNT(gQualitys) * (fBM.width() + 10), fCurPos.fY + 2*(fSize + 10) + fBM.height() + 10 + fSize/2, paint );


        fCurPos.fX += fHorizontalVelocity;
        fCurPos.fY += fVerticalVelocity;
        this->inval(nullptr);
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SubpixelTranslateView("mandrill_256.png", .05f, .05f); }
static SkViewRegister reg(MyFactory);
