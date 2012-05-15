
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkAAClip.h"
#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkImageDecoder.h"

#define USE_PATHS 1

#ifdef SK_BUILD_FOR_WIN
// windows doesn't have roundf
inline float roundf(float x) { return (x-floor(x))>0.5 ? ceil(x) : floor(x); }
#endif

static void drawClip(SkCanvas* canvas, const SkAAClip& clip) {
    SkMask mask;
    SkBitmap bm;
    
    clip.copyToMask(&mask);
    SkAutoMaskFreeImage amfi(mask.fImage);

    bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(),
                 mask.fBounds.height(), mask.fRowBytes);
    bm.setPixels(mask.fImage);
    
    SkPaint paint;
    canvas->drawBitmap(bm,
                       SK_Scalar1 * mask.fBounds.fLeft,
                       SK_Scalar1 * mask.fBounds.fTop,
                       &paint);
}

static void paint_rgn(SkCanvas* canvas, const SkAAClip& clip,
                      const SkPaint& paint) {
    SkMask mask;
    SkBitmap bm;
    
    clip.copyToMask(&mask);
    SkAutoMaskFreeImage amfi(mask.fImage);

    bm.setConfig(SkBitmap::kA8_Config, mask.fBounds.width(),
                 mask.fBounds.height(), mask.fRowBytes);
    bm.setPixels(mask.fImage);
    
    canvas->drawBitmap(bm,
                       SK_Scalar1 * mask.fBounds.fLeft,
                       SK_Scalar1 * mask.fBounds.fTop,
                       &paint);
}

class AAClipView2 : public SampleView {
public:
        AAClipView2() {
        fBase.set(100, 100, 150, 150);
        fRect = fBase;
        fRect.inset(5, 5);
        fRect.offset(25, 25);
        this->setBGColor(0xFFDDDDDD);
    }

    static void setAAClip(SkAAClip* clip, const SkIRect& rect) {
        SkRect r;
        r.set(rect);
        SkPath path;
        path.addRoundRect(r, SkIntToScalar(5), SkIntToScalar(5));
        clip->setPath(path, NULL, true);
    }

    void build_rgn(SkAAClip* clip, SkRegion::Op op) {
        setAAClip(clip, fBase);

        SkAAClip clip2;
        setAAClip(&clip2, fRect);
        clip->op(clip2, op);
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "AAClips");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void drawOrig(SkCanvas* canvas, bool bg) {
        SkRect      r;
        SkPaint     paint;
        
        paint.setStyle(SkPaint::kStroke_Style);
        if (bg)
            paint.setColor(0xFFBBBBBB);
        
        r.set(fBase);
        canvas->drawRect(r, paint);
        r.set(fRect);
        canvas->drawRect(r, paint);
    }
    
    static void outer_frame(SkCanvas* canvas, const SkIRect& rect) {
        SkRect r;
        r.set(rect);
        r.inset(-SK_ScalarHalf, -SK_ScalarHalf);
        
        SkPaint paint;
        paint.setColor(SK_ColorGRAY);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(r, paint);
    }

    void drawRgnOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {

        SkAAClip clip;

        this->build_rgn(&clip, op);
        this->drawOrig(canvas, true);

        SkPaint paint;
        paint.setColor((color & ~(0xFF << 24)) | (0x44 << 24));
        paint_rgn(canvas, clip, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(color);
        paint_rgn(canvas, clip, paint);
        SkAAClip clip2(clip);
        clip2.translate(0, 80);
        outer_frame(canvas, clip2.getBounds());
        paint_rgn(canvas, clip2, paint);
    }
    
    static void createPath(SkPath *path, const SkIRect& rect) {
        SkRect r;
        r.set(rect);
        path->addRoundRect(r, SkIntToScalar(5), SkIntToScalar(5));
    }

    void drawPathsOped(SkCanvas* canvas, SkRegion::Op op, SkColor color) {

        this->drawOrig(canvas, true);

        canvas->save();

        // create the clip mask with the supplied boolean op

#if USE_PATHS
        // path-based case
        SkPath base;
        createPath(&base, fBase);

        canvas->clipPath(base, SkRegion::kReplace_Op, true);
#else
        // rect-based case
        SkRect base;
        base.set(fBase);
        // offset the rects so we get a bit of anti-aliasing
        base.offset(SkFloatToScalar(0.5f), SkFloatToScalar(0.5f));
        canvas->clipRect(base, SkRegion::kReplace_Op, true);

#endif

#if USE_PATHS
        // path-based case
        SkPath rect;
        createPath(&rect, fRect);

        canvas->clipPath(rect, op, true);
#else
        // rect-based case
        SkRect rect;
        rect.set(fRect);
        // offset the rects so we get a bit of anti-aliasing
        rect.offset(SkFloatToScalar(0.5f), SkFloatToScalar(0.5f));
        canvas->clipRect(rect, op, true);
#endif

        // draw a rect that will entirely cover the clip mask area
        SkPaint paint;
        paint.setColor(color);

        SkRect r = SkRect::MakeLTRB(SkIntToScalar(90),  SkIntToScalar(90),
                                    SkIntToScalar(180), SkIntToScalar(180));

        canvas->drawRect(r, paint);

        canvas->restore();
    }

    virtual void onDrawContent(SkCanvas* canvas) {

        static const struct {
            SkColor         fColor;
            const char*     fName;
            SkRegion::Op    fOp;
        } gOps[] = {
            { SK_ColorBLACK,    "Difference", SkRegion::kDifference_Op    },
            { SK_ColorRED,      "Intersect",  SkRegion::kIntersect_Op     },
            { 0xFF008800,       "Union",      SkRegion::kUnion_Op         },
            { SK_ColorBLUE,     "XOR",        SkRegion::kXOR_Op           },
            { SK_ColorGREEN,    "Rev Diff",   SkRegion::kReverseDifference_Op },
            { SK_ColorYELLOW,   "Replace",    SkRegion::kReplace_Op       }
        };

        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        textPaint.setTextSize(SK_Scalar1*24);

        this->drawOrig(canvas, false);

        canvas->translate(0, SkIntToScalar(200));

        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); op++) {
            canvas->drawText(gOps[op].fName, strlen(gOps[op].fName),
                             SkIntToScalar(75), SkIntToScalar(50), textPaint);

            this->drawRgnOped(canvas, gOps[op].fOp, gOps[op].fColor);

            if (op && !(op % 3)) {
                canvas->translate(SkIntToScalar(-600), SkIntToScalar(250));
            } else {
                canvas->translate(SkIntToScalar(200), 0);
            }
        }

        canvas->translate(SkIntToScalar(int(SK_ARRAY_COUNT(gOps) / 3) * -200), 
                          SkIntToScalar(250));

        for (size_t op = 0; op < SK_ARRAY_COUNT(gOps); op++) {
            canvas->drawText(gOps[op].fName, strlen(gOps[op].fName),
                             SkIntToScalar(75), SkIntToScalar(50), textPaint);

            this->drawPathsOped(canvas, gOps[op].fOp, gOps[op].fColor);

            if (op && !(op % 3)) {
                canvas->translate(SkIntToScalar(-600), SkIntToScalar(250));
            } else {
                canvas->translate(SkIntToScalar(200), 0);
            }
        }

    }

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y) {
        return fRect.contains(SkScalarRound(x), SkScalarRound(y)) ? new Click(this) : NULL;
    }
    
    virtual bool onClick(Click* click) {
        fRect.offset(click->fICurr.fX - click->fIPrev.fX,
                     click->fICurr.fY - click->fIPrev.fY);
        this->inval(NULL);
        return true;
    }
    
private:
    SkIRect    fBase, fRect;
    
    typedef SampleView INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static SkView* MyFactory() { return new AAClipView2; }
static SkViewRegister reg(MyFactory);

