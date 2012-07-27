
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkString.h"
#include "SkTypeface.h"
#include "SkTypes.h"

namespace skiagm {

const char* gFaces[] = {
    "Times Roman",
    "Hiragino Maru Gothic Pro",
    "Papyrus",
    "Helvetica",
    "Courier New"
};

class TypefaceGM : public GM {
public:
    TypefaceGM() {
        fFaces = new SkTypeface*[SK_ARRAY_COUNT(gFaces)];
        for (size_t i = 0; i < SK_ARRAY_COUNT(gFaces); i++) {
            fFaces[i] = SkTypeface::CreateFromName(gFaces[i], SkTypeface::kNormal);
        }
    }

    virtual ~TypefaceGM() {
        for (size_t i = 0; i < SK_ARRAY_COUNT(gFaces); i++) {
            fFaces[i]->unref();
        }
        delete fFaces;
    }

protected:
    virtual SkString onShortName() SK_OVERRIDE {
        return SkString("typeface");
    }

    virtual SkISize onISize() SK_OVERRIDE {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE {
        SkString text("Typefaces are fun!");
        SkScalar y = 0;

        SkPaint paint;
        paint.setAntiAlias(true);
        for (size_t i = 0; i < SK_ARRAY_COUNT(gFaces); i++) {
            this->drawWithFace(text, i, y, paint, canvas);
        }
        // Now go backwards
        for (int i = SK_ARRAY_COUNT(gFaces) - 1; i >= 0; i--) {
            this->drawWithFace(text, i, y, paint, canvas);
        }
    }

private:
    void drawWithFace(const SkString& text, int i, SkScalar& y, SkPaint& paint,
                      SkCanvas* canvas) {
        paint.setTypeface(fFaces[i]);
        y += paint.getFontMetrics(NULL);
        canvas->drawText(text.c_str(), text.size(), 0, y, paint);
    }

    SkTypeface** fFaces;
    
    typedef GM INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new TypefaceGM; }
static GMRegistry reg(MyFactory);

}   // skiagm
