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

class GetPosTextPathGM : public skiagm::GM {
public:
    GetPosTextPathGM() {}

protected:
    SkString onShortName() {
        return SkString("getpostextpath");
    }

    SkISize onISize() { return skiagm::make_isize(480, 780); }

    virtual void onDraw(SkCanvas* canvas) {
        const char* text = "Hamburgefons";
        size_t len = strlen(text);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(48));
        
        SkAutoTArray<SkPoint>  pos(len);
        SkAutoTArray<SkScalar> widths(len);
        paint.getTextWidths(text, len, &widths[0]);
        
        SkRandom rand;
        SkScalar x = SkIntToScalar(20);
        SkScalar y = SkIntToScalar(100);
        for (size_t i = 0; i < len; ++i) {
            pos[i].set(x, y + rand.nextSScalar1() * 24);
            x += widths[i];
        }
        
        canvas->drawPosText(text, len, &pos[0], paint);
        
        SkPath path;
        paint.setColor(SK_ColorRED);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.getPosTextPath(text, len, &pos[0], &path);
        canvas->drawPath(path, paint);
    }
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* F(void*) { return new GetPosTextPathGM; }

static skiagm::GMRegistry gR(F);

