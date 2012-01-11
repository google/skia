/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkTypeface.h"

namespace skiagm {

class StrokeFillGM : public GM {
public:
    StrokeFillGM() {

    }

protected:
    virtual SkString onShortName() {
        return SkString("stroke-fill");
    }

    virtual SkISize onISize() {
        return make_isize(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPaint paint;
        const char text[] = "Hello"; // "Hello";
        const size_t len = sizeof(text) - 1;
        paint.setAntiAlias(true);
        paint.setTextSize(SkIntToScalar(100));
//        SkTypeface* hira = SkTypeface::CreateFromName("Hiragino Maru Gothic Pro", SkTypeface::kNormal);
        SkTypeface* hira = SkTypeface::CreateFromName("Papyrus", SkTypeface::kNormal);
        paint.setTypeface(hira);
        SkScalar x = SkIntToScalar(180);
        SkScalar y = SkIntToScalar(88);
        
        canvas->drawText(text, len, x, y, paint);
        paint.setFakeBoldText(true);
        canvas->drawText(text, len, x, y + SkIntToScalar(100), paint);
        paint.setStyle(SkPaint::kStrokeAndFill_Style);
        paint.setStrokeWidth(SkIntToScalar(5));
        
        SkPath path;
        path.setFillType(SkPath::kWinding_FillType);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCW_Direction);
        path.addCircle(x, y + SkIntToScalar(200), SkIntToScalar(40), SkPath::kCCW_Direction);
        canvas->drawPath(path, paint);
        
        SkPath path2;
        path2.setFillType(SkPath::kWinding_FillType);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCCW_Direction);
        path2.addCircle(x + SkIntToScalar(120), y + SkIntToScalar(200), SkIntToScalar(40), SkPath::kCW_Direction);
        canvas->drawPath(path2, paint);
        
        path2.reset();
        path2.addCircle(x + SkIntToScalar(240), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCCW_Direction);
        canvas->drawPath(path2, paint);
        SkASSERT(path2.cheapIsDirection(SkPath::kCCW_Direction));
        
        path2.reset();
        SkASSERT(!path2.cheapComputeDirection(NULL));
        path2.addCircle(x + SkIntToScalar(360), y + SkIntToScalar(200), SkIntToScalar(50), SkPath::kCW_Direction);
        SkASSERT(path2.cheapIsDirection(SkPath::kCW_Direction));
        canvas->drawPath(path2, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new StrokeFillGM; }
static GMRegistry reg(MyFactory);

}
