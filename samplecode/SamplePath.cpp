/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkAnimTimer.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkGradientShader.h"
#include "SkGraphics.h"
#include "SkPath.h"
#include "SkRegion.h"
#include "SkShader.h"
#include "SkUtils.h"
#include "SkColorPriv.h"
#include "SkColorFilter.h"
#include "SkParsePath.h"
#include "SkTime.h"
#include "SkTypeface.h"

#include "SkGeometry.h"

#include <stdlib.h>

// http://code.google.com/p/skia/issues/detail?id=32
static void test_cubic() {
    SkPoint src[4] = {
        { 556.25000f, 523.03003f },
        { 556.23999f, 522.96002f },
        { 556.21997f, 522.89001f },
        { 556.21997f, 522.82001f }
    };
    SkPoint dst[11];
    dst[10].set(42, -42);   // one past the end, that we don't clobber these
    SkScalar tval[] = { 0.33333334f, 0.99999994f };

    SkChopCubicAt(src, dst, tval, 2);

#if 0
    for (int i = 0; i < 11; i++) {
        SkDebugf("--- %d [%g %g]\n", i, dst[i].fX, dst[i].fY);
    }
#endif
}

static void test_cubic2() {
    const char* str = "M2242 -590088L-377758 9.94099e+07L-377758 9.94099e+07L2242 -590088Z";
    SkPath path;
    SkParsePath::FromSVGString(str, &path);

    {
        SkRect r = path.getBounds();
        SkIRect ir;
        r.round(&ir);
        SkDebugf("[%g %g %g %g] [%x %x %x %x]\n",
                SkScalarToDouble(r.fLeft), SkScalarToDouble(r.fTop),
                SkScalarToDouble(r.fRight), SkScalarToDouble(r.fBottom),
                ir.fLeft, ir.fTop, ir.fRight, ir.fBottom);
    }

    SkBitmap bitmap;
    bitmap.allocN32Pixels(300, 200);

    SkCanvas canvas(bitmap);
    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawPath(path, paint);
}

class PathView : public SampleView {
    SkScalar fPrevSecs;
public:
    SkScalar fDStroke, fStroke, fMinStroke, fMaxStroke;
    SkPath fPath[6];
    bool fShowHairline;
    bool fOnce;

    PathView() {
        fPrevSecs = 0;
        fOnce = false;
    }

    void init() {
        if (fOnce) {
            return;
        }
        fOnce = true;

        test_cubic();
        test_cubic2();

        fShowHairline = false;

        fDStroke = 1;
        fStroke = 10;
        fMinStroke = 10;
        fMaxStroke = 180;

        const SkScalar V = 85;

        fPath[0].moveTo(40, 70);
        fPath[0].lineTo(70, 70 + SK_ScalarHalf);
        fPath[0].lineTo(110, 70);

        fPath[1].moveTo(40, 70);
        fPath[1].lineTo(70, 70 - SK_ScalarHalf);
        fPath[1].lineTo(110, 70);

        fPath[2].moveTo(V, V);
        fPath[2].lineTo(50, V);
        fPath[2].lineTo(50, 50);

        fPath[3].moveTo(50, 50);
        fPath[3].lineTo(50, V);
        fPath[3].lineTo(V, V);

        fPath[4].moveTo(50, 50);
        fPath[4].lineTo(50, V);
        fPath[4].lineTo(52, 50);

        fPath[5].moveTo(52, 50);
        fPath[5].lineTo(50, V);
        fPath[5].lineTo(50, 50);

        this->setBGColor(0xFFDDDDDD);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "Paths");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawPath(SkCanvas* canvas, const SkPath& path, SkPaint::Join j) {
        SkPaint paint;

        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeJoin(j);
        paint.setStrokeWidth(fStroke);

        if (fShowHairline) {
            SkPath  fill;

            paint.getFillPath(path, &fill);
            paint.setStrokeWidth(0);
            canvas->drawPath(fill, paint);
        } else {
            canvas->drawPath(path, paint);
        }

        paint.setColor(SK_ColorRED);
        paint.setStrokeWidth(0);
        canvas->drawPath(path, paint);
    }

    void onDrawContent(SkCanvas* canvas) override {
        this->init();
        canvas->translate(50, 50);

        static const SkPaint::Join gJoins[] = {
            SkPaint::kBevel_Join,
            SkPaint::kMiter_Join,
            SkPaint::kRound_Join
        };

        for (size_t i = 0; i < SK_ARRAY_COUNT(gJoins); i++) {
            canvas->save();
            for (size_t j = 0; j < SK_ARRAY_COUNT(fPath); j++) {
                this->drawPath(canvas, fPath[j], gJoins[i]);
                canvas->translate(200, 0);
            }
            canvas->restore();

            canvas->translate(0, 200);
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        SkScalar currSecs = timer.scaled(100);
        SkScalar delta = currSecs - fPrevSecs;
        fPrevSecs = currSecs;

        fStroke += fDStroke * delta;
        if (fStroke > fMaxStroke || fStroke < fMinStroke) {
            fDStroke = -fDStroke;
        }
        return true;
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        fShowHairline = !fShowHairline;
        this->inval(nullptr);
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};
DEF_SAMPLE( return new PathView; )

//////////////////////////////////////////////////////////////////////////////

#include "SkArcToPathEffect.h"
#include "SkCornerPathEffect.h"
#include "SkRandom.h"

class ArcToView : public SampleView {
    bool fDoFrame, fDoArcTo, fDoCorner, fDoConic;
    SkPaint fPtsPaint, fArcToPaint, fSkeletonPaint, fCornerPaint;
public:
    enum {
        N = 4
    };
    SkPoint fPts[N];

    ArcToView()
        : fDoFrame(false), fDoArcTo(false), fDoCorner(false), fDoConic(false)
    {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPts[i].fX = 20 + rand.nextUScalar1() * 640;
            fPts[i].fY = 20 + rand.nextUScalar1() * 480;
        }

        const SkScalar rad = 50;

        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(15);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);

        fArcToPaint.setAntiAlias(true);
        fArcToPaint.setStyle(SkPaint::kStroke_Style);
        fArcToPaint.setStrokeWidth(9);
        fArcToPaint.setColor(0x800000FF);
        fArcToPaint.setPathEffect(SkArcToPathEffect::Make(rad));

        fCornerPaint.setAntiAlias(true);
        fCornerPaint.setStyle(SkPaint::kStroke_Style);
        fCornerPaint.setStrokeWidth(13);
        fCornerPaint.setColor(SK_ColorGREEN);
        fCornerPaint.setPathEffect(SkCornerPathEffect::Make(rad*2));

        fSkeletonPaint.setAntiAlias(true);
        fSkeletonPaint.setStyle(SkPaint::kStroke_Style);
        fSkeletonPaint.setColor(SK_ColorRED);
    }

    void toggle(bool& value) {
        value = !value;
        this->inval(nullptr);
    }

protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "ArcTo");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case '1': this->toggle(fDoFrame); return true;
                case '2': this->toggle(fDoArcTo); return true;
                case '3': this->toggle(fDoCorner); return true;
                case '4': this->toggle(fDoConic); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void makePath(SkPath* path) {
        path->moveTo(fPts[0]);
        for (int i = 1; i < N; ++i) {
            path->lineTo(fPts[i]);
        }
        if (!fDoFrame) {
            path->close();
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, fPts, fPtsPaint);

        SkPath path;
        this->makePath(&path);

        if (fDoCorner) {
            canvas->drawPath(path, fCornerPaint);
        }
        if (fDoArcTo) {
            canvas->drawPath(path, fArcToPaint);
        }

        canvas->drawPath(path, fSkeletonPaint);
    }

    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            this->inval(nullptr);
            return true;
        }
        return false;
    }

    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

private:
    typedef SampleView INHERITED;
};
DEF_SAMPLE( return new ArcToView; )

/////////////

class FatStroke : public SampleView {
    bool fClosed, fShowStroke, fShowHidden, fShowSkeleton;
    int  fJoinType, fCapType;
    float fWidth = 30;
    SkPaint fPtsPaint, fHiddenPaint, fSkeletonPaint, fStrokePaint;
public:
    enum {
        N = 4
    };
    SkPoint fPts[N];
    
    FatStroke() : fClosed(false), fShowStroke(true), fShowHidden(false), fShowSkeleton(true),
                  fJoinType(0), fCapType(0)
    {
        SkRandom rand;
        for (int i = 0; i < N; ++i) {
            fPts[i].fX = 20 + rand.nextUScalar1() * 640;
            fPts[i].fY = 20 + rand.nextUScalar1() * 480;
        }
        
        fPtsPaint.setAntiAlias(true);
        fPtsPaint.setStrokeWidth(10);
        fPtsPaint.setStrokeCap(SkPaint::kRound_Cap);
        
        fHiddenPaint.setAntiAlias(true);
        fHiddenPaint.setStyle(SkPaint::kStroke_Style);
        fHiddenPaint.setColor(0xFF0000FF);
        
        fStrokePaint.setAntiAlias(true);
        fStrokePaint.setStyle(SkPaint::kStroke_Style);
        fStrokePaint.setStrokeWidth(50);
        fStrokePaint.setColor(0x8000FF00);
        
        fSkeletonPaint.setAntiAlias(true);
        fSkeletonPaint.setStyle(SkPaint::kStroke_Style);
        fSkeletonPaint.setColor(SK_ColorRED);
    }
    
    void toggle(bool& value) {
        value = !value;
        this->inval(nullptr);
    }
    
    void toggle3(int& value) {
        value = (value + 1) % 3;
        this->inval(nullptr);
    }
    
protected:
    // overrides from SkEventSink
    bool onQuery(SkEvent* evt) override {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "FatStroke");
            return true;
        }
        SkUnichar uni;
        if (SampleCode::CharQ(*evt, &uni)) {
            switch (uni) {
                case '1': this->toggle(fShowSkeleton); return true;
                case '2': this->toggle(fShowStroke); return true;
                case '3': this->toggle(fShowHidden); return true;
                case '4': this->toggle3(fJoinType); return true;
                case '5': this->toggle3(fCapType); return true;
                case '6': this->toggle(fClosed); return true;
                case '-': fWidth -= 5; this->inval(nullptr); return true;
                case '=': fWidth += 5; this->inval(nullptr); return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }
    
    void makePath(SkPath* path) {
        path->moveTo(fPts[0]);
        for (int i = 1; i < N; ++i) {
            path->lineTo(fPts[i]);
        }
        if (fClosed) {
            path->close();
        }
    }
    
    void onDrawContent(SkCanvas* canvas) override {
        canvas->drawColor(0xFFEEEEEE);

        SkPath path;
        this->makePath(&path);

        fStrokePaint.setStrokeWidth(fWidth);
        fStrokePaint.setStrokeJoin((SkPaint::Join)fJoinType);
        fStrokePaint.setStrokeCap((SkPaint::Cap)fCapType);

        if (fShowStroke) {
            canvas->drawPath(path, fStrokePaint);
        }
        if (fShowHidden) {
            SkPath hidden;
            fStrokePaint.getFillPath(path, &hidden);
            canvas->drawPath(hidden, fHiddenPaint);
        }
        if (fShowSkeleton) {
            canvas->drawPath(path, fSkeletonPaint);
        }
        canvas->drawPoints(SkCanvas::kPoints_PointMode, N, fPts, fPtsPaint);
    }
    
    bool onClick(Click* click) override {
        int32_t index;
        if (click->fMeta.findS32("index", &index)) {
            SkASSERT((unsigned)index < N);
            fPts[index] = click->fCurr;
            this->inval(nullptr);
            return true;
        }
        return false;
    }
    
    SkView::Click* onFindClickHandler(SkScalar x, SkScalar y, unsigned modi) override {
        const SkScalar tol = 4;
        const SkRect r = SkRect::MakeXYWH(x - tol, y - tol, tol * 2, tol * 2);
        for (int i = 0; i < N; ++i) {
            if (r.intersects(SkRect::MakeXYWH(fPts[i].fX, fPts[i].fY, 1, 1))) {
                Click* click = new Click(this);
                click->fMeta.setS32("index", i);
                return click;
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }
    
private:
    typedef SampleView INHERITED;
};
DEF_SAMPLE( return new FatStroke; )
