/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"

// This should be on SkCanvas imho
static void rotateAbout(SkCanvas* canvas, SkScalar degrees,
                        SkScalar px, SkScalar py) {
    canvas->translate(px, py);
    canvas->rotate(degrees);
    canvas->translate(-px, -py);
}

class SaveLayerGM : public skiagm::GM {
    void drawStuff(SkCanvas* canvas, const SkRect& r) {
        SkPaint paint;
        paint.setAntiAlias(true);
        canvas->drawOval(r, paint);
    }

public:
    SaveLayerGM() {}

protected:
    SkString onShortName() {
        return SkString("savelayer");
    }

    virtual SkISize onISize() { return SkISize::Make(100, 100); }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint hairpaint;
        hairpaint.setAntiAlias(true);
        hairpaint.setStyle(SkPaint::kStroke_Style);
        hairpaint.setColor(SK_ColorRED);

        canvas->translate(50, 50);

        SkRect r = SkRect::MakeWH(100, 60);
        SkRect r2 = r;
        r2.inset(5, 5);

        this->drawStuff(canvas, r);
        canvas->drawRect(r, hairpaint);
        canvas->translate(r.width() * 5/4, 0);
    
        canvas->saveLayer(&r2, NULL);
        this->drawStuff(canvas, r);
        canvas->restore();
        canvas->drawRect(r, hairpaint);
        canvas->translate(r.width() * 5/4, 0);

        // We need to ensure that we still clip against r2 (after it is rotated)
        // even though the layer's bounds will be larger (since they are the
        // enclosing rect of rotated-r2).

        rotateAbout(canvas, 30, r.centerX(), r.centerY());
        canvas->saveLayer(&r2, NULL);
        this->drawStuff(canvas, r);
        canvas->restore();
        canvas->drawRect(r, hairpaint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new SaveLayerGM; }
static skiagm::GMRegistry reg(MyFactory);
