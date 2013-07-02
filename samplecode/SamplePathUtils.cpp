/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkCanvas.h"
#include "SkPathUtils.h"
#include "SkView.h"
//#include "SkPathOps.h" // loads fine here, won't in PathUtils src files
#include "SkRandom.h"
//#include "SkTime.h"

class samplePathUtils : public SampleView {
public:
    samplePathUtils() {
        bmp_paint.setAntiAlias(true);  // Black paint for bitmap
    bmp_paint.setStyle(SkPaint::kFill_Style);
        bmp_paint.setColor(SK_ColorBLACK);
    }

protected:
    static const int numModes = 3;
    static const int h=8, w=12, stride=2; // stride is in bytes
    static const int numChars = h * stride; // number of chars in entire array

    SkPaint bmp_paint;

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
        // bitmap definitions
        const uint8_t bits[numModes][numChars] = {
            { 0x18, 0x00, 0x3c, 0x00, 0x7e, 0x00, 0xdb, 0x00,
               0xff, 0x00, 0x24, 0x00, 0x5a, 0x00, 0xa5, 0x00 },

            { 0x20, 0x80, 0x91, 0x20, 0xbf, 0xa0, 0xee, 0xe0,
              0xff, 0xe0, 0x7f, 0xc0, 0x20, 0x80, 0x40, 0x40 },

            { 0x0f, 0x00, 0x7f, 0xe0, 0xff, 0xf0, 0xe6, 0x70,
              0xff, 0xf0, 0x19, 0x80, 0x36, 0xc0, 0xc0, 0x30 }
        };

        static const SkScalar kScale = 10.0f;

        for (int i = 0; i < numModes; ++i) {
            SkPath path; // generate and simplify each path
            SkPathUtils::BitsToPath_Path(&path, (char*) &bits[i], h, w, stride);

            canvas->save(); // DRAWING
            canvas->scale(kScale, kScale);  // scales up each bitmap
            canvas->translate(0, 1.5f * h * i);
            canvas->drawPath(path, bmp_paint); // draw bitmap
            canvas->restore();

            // use the SkRegion method
            SkPath pathR;
            SkPathUtils::BitsToPath_Region(&pathR, (char*) &bits[i], h, w, stride);

            canvas->save();

            canvas->scale(kScale, kScale);  // scales up each bitmap
            canvas->translate(1.5f * w, 1.5f * h * i); // translates past previous bitmap
            canvas->drawPath(pathR, bmp_paint); // draw bitmap
            canvas->restore();
        }
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new samplePathUtils; }
static SkViewRegister reg(MyFactory)
;
