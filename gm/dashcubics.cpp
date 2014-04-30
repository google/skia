/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkPath.h"
#include "SkParsePath.h"
#include "SkDashPathEffect.h"

/*
 *  Inspired by http://code.google.com/p/chromium/issues/detail?id=112145
 */

class DashCubicsGM : public skiagm::GM {
public:
    DashCubicsGM() {}

protected:
    virtual uint32_t onGetFlags() const SK_OVERRIDE {
        return kSkipTiled_Flag;
    }

    virtual SkString onShortName() {
        return SkString("dashcubics");
    }

    virtual SkISize onISize() {
        return SkISize::Make(640, 480);
    }

    virtual void onDraw(SkCanvas* canvas) {
        SkPath path;
        const char* d = "M 337,98 C 250,141 250,212 250,212 C 250,212 250,212 250,212"
        "C 250,212 250,212 250,212 C 250,212 250,141 163,98 C 156,195 217,231 217,231"
        "C 217,231 217,231 217,231 C 217,231 217,231 217,231 C 217,231 156,195 75,250"
        "C 156,305 217,269 217,269 C 217,269 217,269 217,269 C 217,269 217,269 217,269"
        "C 217,269 156,305 163,402 C 250,359 250,288 250,288 C 250,288 250,288 250,288"
        "C 250,288 250,288 250,288 C 250,288 250,359 338,402 C 345,305 283,269 283,269"
        "C 283,269 283,269 283,269 C 283,269 283,269 283,269 C 283,269 345,305 425,250"
        "C 344,195 283,231 283,231 C 283,231 283,231 283,231 C 283,231 283,231 283,231"
        "C 283,231 344,195 338,98";

        SkParsePath::FromSVGString(d, &path);

        SkScalar intervals[] = { 5, 10 };
        SkPathEffect* pe = SkDashPathEffect::Create(intervals, 2, 0);

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        paint.setStrokeWidth(42);
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(21);
        paint.setPathEffect(pe)->unref();
        canvas->drawPath(path, paint);

        paint.setColor(SK_ColorGREEN);
        paint.setPathEffect(NULL);
        paint.setStrokeWidth(0);
        canvas->drawPath(path, paint);
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new DashCubicsGM; }
static skiagm::GMRegistry reg(MyFactory);
