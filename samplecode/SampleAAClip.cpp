
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkAAClip.h"

static void drawClip(SkCanvas* canvas, const SkAAClip& clip) {
    SkMask mask;
    SkBitmap bm;
    
    clip.copyToMask(&mask);
    SkAutoMaskFreeImage amfi(mask.fImage);

    bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(),
                 mask.fBounds.height(), mask.fRowBytes);
    bm.setPixels(mask.fImage);

    SkPaint paint;
    canvas->drawBitmap(bm, mask.fBounds.fLeft, mask.fBounds.fTop, &paint);
}

class AAClipView : public SampleView {
public:
    AAClipView() {
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AAClip");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
#if 1
        SkAAClip aaclip;
        SkPath path;
        SkRect bounds;
        
        bounds.set(0, 0, 20, 20);
        bounds.inset(SK_ScalarHalf, SK_ScalarHalf);

//        path.addRect(bounds);
//        path.addOval(bounds);
        path.addRoundRect(bounds, 4, 4);
        aaclip.setPath(path);
        canvas->translate(30, 30);
        drawClip(canvas, aaclip);

        SkAAClip aaclip2;
        path.offset(10, 10);
        aaclip2.setPath(path);
        canvas->translate(30, 0);
        drawClip(canvas, aaclip2);

        SkAAClip aaclip3;
        aaclip3.op(aaclip, aaclip2, SkRegion::kIntersect_Op);
        canvas->translate(30, 0);
        drawClip(canvas, aaclip3);
        
#endif
        
#if 0
        SkRect r;
        r.set(0, 0, this->width(), this->height());
        r.inset(20, 20);
        canvas->clipRect(r);
        
        SkPath path;
        path.addRect(r);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(path, paint);
#endif
    }

private:
    typedef SkView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AAClipView; }
static SkViewRegister reg(MyFactory);

