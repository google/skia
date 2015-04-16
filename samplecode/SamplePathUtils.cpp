/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"

#include "SkCanvas.h"
#include "SkCornerPathEffect.h"
#include "SkDashPathEffect.h"
#include "SkPathUtils.h"
#include "SkRandom.h"
#include "SkView.h"

typedef void (*BitsToPath)(SkPath*, const char*, int, int, int);

static const BitsToPath gBitsToPath_fns[] = {
    SkPathUtils::BitsToPath_Path,
    SkPathUtils::BitsToPath_Region,
};

// hardcoded bitmap patterns
static const uint8_t gBits[][16] = {
    { 0x18, 0x00, 0x3c, 0x00, 0x7e, 0x00, 0xdb, 0x00,
      0xff, 0x00, 0x24, 0x00, 0x5a, 0x00, 0xa5, 0x00 },

    { 0x20, 0x80, 0x91, 0x20, 0xbf, 0xa0, 0xee, 0xe0,
      0xff, 0xe0, 0x7f, 0xc0, 0x20, 0x80, 0x40, 0x40 },

    { 0x0f, 0x00, 0x7f, 0xe0, 0xff, 0xf0, 0xe6, 0x70,
      0xff, 0xf0, 0x19, 0x80, 0x36, 0xc0, 0xc0, 0x30 }
};


class SamplePathUtils : public SampleView {
public:
    static const int fNumBits = 3;
    static const int fH = 8, fW = 12;
    static const size_t fRowBytes = 2;
    static const int fNumChars = fH * fRowBytes;

    SkPaint fBmpPaint;
    SkScalar fPhase;

    SamplePathUtils() {
        fBmpPaint.setAntiAlias(true);  // Black paint for bitmap
        fBmpPaint.setStyle(SkPaint::kFill_Style);

        fPhase = 0.0f; // to animate the dashed path
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "PathUtils");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    /////////////////////////////////////////////////////////////

    virtual void onDrawContent(SkCanvas* canvas) {
        SkScalar intervals[8] = { .5f, .3f, .5f, .3f, .5f, .3f, .5f, .3f };
        SkAutoTUnref<SkDashPathEffect> dash(SkDashPathEffect::Create(intervals, 2, fPhase));
        SkAutoTUnref<SkCornerPathEffect> corner(SkCornerPathEffect::Create(.25f));
        SkAutoTUnref<SkComposePathEffect> compose(SkComposePathEffect::Create(dash, corner));

        SkPaint outlinePaint;
        outlinePaint.setAntiAlias(true);  // dashed paint for bitmap
        outlinePaint.setStyle(SkPaint::kStroke_Style);
        outlinePaint.setPathEffect(compose);

        canvas->scale(10.0f, 10.0f);  // scales up

        for (int i = 0; i < fNumBits; ++i) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(gBitsToPath_fns); ++j) {
                SkPath path;
                gBitsToPath_fns[j](&path, (char*) &gBits[i], fW, fH, fRowBytes);

                //draw skPath and outline
                canvas->drawPath(path, fBmpPaint);
                canvas->translate(1.5f * fW, 0); // translates past previous bitmap
                canvas->drawPath(path, outlinePaint);
                canvas->translate(1.5f * fW, 0); // translates past previous bitmap
            }
            canvas->restore();
            canvas->translate(0, 1.5f * fH); //translate to next row
        }

        // for animated pathEffect
        fPhase += .01f;
        this->inval(NULL);
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new SamplePathUtils; }
static SkViewRegister reg(MyFactory)
;
