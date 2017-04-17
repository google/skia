/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "DecodeFile.h"
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkTime.h"

static const char* gNames[] = {
    "/skimages/background_01.png"
};

class Filter2View : public SampleView {
public:
    SkBitmap*   fBitmaps;
    int         fBitmapCount;
    int         fCurrIndex;

    Filter2View() {
        fBitmapCount = SK_ARRAY_COUNT(gNames)*2;
        fBitmaps = new SkBitmap[fBitmapCount];

        for (int i = 0; i < fBitmapCount/2; i++) {
            decode_file(gNames[i], &fBitmaps[i]);
        }
        for (int i = fBitmapCount/2; i < fBitmapCount; i++) {
            decode_file(gNames[i-fBitmapCount/2], &fBitmaps[i], kRGB_565_SkColorType);
        }
        fCurrIndex = 0;

        this->setBGColor(SK_ColorGRAY);
    }

    virtual ~Filter2View() {
        delete[] fBitmaps;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SkString str("Filter/Dither ");
            str.append(gNames[fCurrIndex]);
            SampleCode::TitleR(evt, str.c_str());
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
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
                    if (i == 0) {
                        SkPaint p;
                        p.setAntiAlias(true);
                        p.setTextAlign(SkPaint::kCenter_Align);
                        p.setTextSize(SkIntToScalar(18));
                        SkString s("dither=");
                        s.appendS32(paint.isDither());
                        s.append(" filter=");
                        s.appendS32(paint.getFilterQuality() != kNone_SkFilterQuality);
                        canvas->drawString(s, x + W/2,
                                         y - p.getTextSize(), p);
                    }
                    if (k+j == 2) {
                        SkPaint p;
                        p.setAntiAlias(true);
                        p.setTextSize(SkIntToScalar(18));
                        SkString s;
                        s.append(" depth=");
                        s.appendS32(fBitmaps[i].colorType() == kRGB_565_SkColorType ? 16 : 32);
                        canvas->drawString(s, x + W + SkIntToScalar(4),
                                         y + H/2, p);
                    }
                }
            }
        }
    }

private:
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new Filter2View; }
static SkViewRegister reg(MyFactory);
