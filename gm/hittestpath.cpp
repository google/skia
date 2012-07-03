/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkCullPoints.h"
#include "SkRandom.h"

static void test_hittest(SkCanvas* canvas, const SkPath& path, bool hires) {
    SkPaint paint;
    SkRect r = path.getBounds();
    
    paint.setColor(SK_ColorRED);
    canvas->drawPath(path, paint);
    
    const SkScalar MARGIN = SkIntToScalar(4);
    
    paint.setColor(0x800000FF);
    for (SkScalar y = r.fTop + SK_ScalarHalf - MARGIN; y < r.fBottom + MARGIN; y += SK_Scalar1) {
        for (SkScalar x = r.fLeft + SK_ScalarHalf - MARGIN; x < r.fRight + MARGIN; x += SK_Scalar1) {
            if (hires) {
                if (SkHitTestPathEx(path, x, y)) {
                    canvas->drawPoint(x, y, paint);
                }
            } else {
                if (SkHitTestPath(path, x, y, false)) {
                    canvas->drawPoint(x, y, paint);
                }
            }
        }
    }
}

class HitTestPathGM : public skiagm::GM {
public:
    HitTestPathGM () {}
    
protected:
    virtual SkString onShortName() {
        return SkString("hittestpath");
    }
    
    virtual SkISize onISize() { return SkISize::Make(700, 460); }
    
    virtual void onDraw(SkCanvas* canvas) {
        SkPath path;
        SkRandom rand;
        
        for (int i = 0; i < 5; ++i) {
            path.lineTo(rand.nextUScalar1() * 150, rand.nextUScalar1() * 150);
            path.quadTo(rand.nextUScalar1() * 150, rand.nextUScalar1() * 150,
                        rand.nextUScalar1() * 150, rand.nextUScalar1() * 150);
        }
        
        path.setFillType(SkPath::kEvenOdd_FillType);
        path.offset(SkIntToScalar(20), SkIntToScalar(20));
        
        test_hittest(canvas, path, false);
        canvas->translate(SkIntToScalar(200), 0);
        test_hittest(canvas, path, true);
        
        canvas->translate(-SkIntToScalar(200), SkIntToScalar(200));
        path.setFillType(SkPath::kWinding_FillType);
        
        test_hittest(canvas, path, false);
        canvas->translate(SkIntToScalar(200), 0);
        test_hittest(canvas, path, true);
    }
    
private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static skiagm::GM* MyFactory(void*) { return new HitTestPathGM; }
static skiagm::GMRegistry reg(MyFactory);


