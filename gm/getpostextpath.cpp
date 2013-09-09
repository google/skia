/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkRandom.h"
#include "SkTemplates.h"

class GetPosTextPathGM : public skiagm::GM {
public:
    GetPosTextPathGM() {}

protected:
    SkString onShortName() {
        return SkString("getpostextpath");
    }

    SkISize onISize() { return skiagm::make_isize(480, 780); }

    static void strokePath(SkCanvas* canvas, const SkPath& path) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawPath(path, paint);
    }

    virtual void onDraw(SkCanvas* canvas) {
        // explicitly add spaces, to test a prev. bug
        const char* text = "Ham bur ge fons";
        size_t len = strlen(text);
        SkPath path;

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));

        canvas->translate(SkIntToScalar(10), SkIntToScalar(64));

        canvas->drawText(text, len, 0, 0, paint);
        paint.getTextPath(text, len, 0, 0, &path);
        strokePath(canvas, path);
        path.reset();

        SkAutoTArray<SkPoint>  pos(len);
        SkAutoTArray<SkScalar> widths(len);
        paint.getTextWidths(text, len, &widths[0]);

        SkLCGRandom rand;
        SkScalar x = SkIntToScalar(20);
        SkScalar y = SkIntToScalar(100);
        for (size_t i = 0; i < len; ++i) {
            pos[i].set(x, y + rand.nextSScalar1() * 24);
            x += widths[i];
        }

        canvas->translate(0, SkIntToScalar(64));

        canvas->drawPosText(text, len, &pos[0], paint);
        paint.getPosTextPath(text, len, &pos[0], &path);
        strokePath(canvas, path);
    }
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* F(void*) { return new GetPosTextPathGM; }
static skiagm::GMRegistry gR(F);
