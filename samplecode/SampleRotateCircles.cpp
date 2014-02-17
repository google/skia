/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SampleCode.h"
#include "SkView.h"
#include "SkCanvas.h"
#include "SkRandom.h"
#include "SkRRect.h"
#include "SkColorPriv.h"

static void rotateAbout(SkCanvas* canvas, SkScalar degrees,
                        SkScalar cx, SkScalar cy) {
    canvas->translate(cx, cy);
    canvas->rotate(degrees);
    canvas->translate(-cx, -cy);
}

class RotateCirclesView : public SampleView {
public:
    RotateCirclesView() {
        this->setBGColor(SK_ColorLTGRAY);

        fAngle = 0;
    }

protected:
    // overrides from SkEventSink
    virtual bool onQuery(SkEvent* evt) {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RotateCircles");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkRandom rand;
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStrokeWidth(20);

        SkScalar cx = 240;
        SkScalar cy = 240;
        SkScalar DX = 240 * 2;
        SkColor color = 0;

        float scale = 1;
        float sign = 0.3f;
        for (SkScalar rad = 200; rad >= 20; rad -= 15) {
            sign = -sign;
            scale += 0.2f;

            paint.setColor(rand.nextU());
            paint.setAlpha(0xFF);
            color = ~color;

            paint.setStyle(SkPaint::kFill_Style);

            canvas->save();
            rotateAbout(canvas, fAngle * scale * sign, cx, cy);
            canvas->drawCircle(cx, cy, rad, paint);
            canvas->restore();

            paint.setStyle(SkPaint::kStroke_Style);
            paint.setStrokeWidth(rad*2);

            canvas->save();
            rotateAbout(canvas, fAngle * scale * sign, cx + DX, cy);
            canvas->drawCircle(cx + DX, cy, 10, paint);
            canvas->restore();

            canvas->save();
            rotateAbout(canvas, fAngle * scale * sign, cx + DX, cy + DX);
            canvas->drawCircle(cx + DX, cy + DX, 10, paint);
            canvas->restore();

        }

        fAngle = (fAngle + 1) % 360;
        this->inval(NULL);
    }

private:
    int fAngle;
    typedef SkView INHERITED;
};

class TestCirclesView : public SampleView {
public:
    TestCirclesView() {
    }

protected:
    virtual bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RotateCircles2");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void draw_real_circle(SkCanvas* canvas, SkScalar radius) {
        int w = SkScalarCeilToInt(radius * 2);
        int h = w;

        SkBitmap bm;
        bm.allocN32Pixels(w, h);
        bm.eraseColor(0);

        SkAutoLockPixels alp(bm);

        SkScalar cx = radius;
        SkScalar cy = radius;
        for (int y = 0; y < h; y += 1) {
            for (int x = 0; x < w; x += 1) {
                float d = sqrtf((x - cx)*(x - cx) + (y - cy)*(y - cy));
                if (d <= radius) {
                    *bm.getAddr32(x, y) = SkPackARGB32(0xFF, 0, 0, 0);
                }
            }
        }

        canvas->drawBitmap(bm, 0, 0, NULL);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkScalar radius = 256;
        canvas->translate(10, 10);

        draw_real_circle(canvas, radius);

        SkPaint paint;
        paint.setAntiAlias(true);

        paint.setColor(0x80FF0000);
        canvas->drawCircle(radius, radius, radius, paint);

        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(radius);
        paint.setColor(0x8000FF00);
        canvas->drawCircle(radius, radius, radius/2, paint);
    }

private:
    typedef SkView INHERITED;
};

static bool hittest(const SkPoint& target, SkScalar x, SkScalar y) {
    const SkScalar TOL = 7;
    return SkPoint::Distance(target, SkPoint::Make(x, y)) <= TOL;
}

static int getOnCurvePoints(const SkPath& path, SkPoint storage[]) {
    SkPath::RawIter iter(path);
    SkPoint pts[4];
    SkPath::Verb verb;

    int count = 0;
    while ((verb = iter.next(pts)) != SkPath::kDone_Verb) {
        switch (verb) {
            case SkPath::kMove_Verb:
            case SkPath::kLine_Verb:
            case SkPath::kQuad_Verb:
            case SkPath::kCubic_Verb:
                storage[count++] = pts[0];
                break;
            default:
                break;
        }
    }
    return count;
}

#include "SkPathMeasure.h"

class TestStrokeView : public SampleView {
    enum {
        SKELETON_COLOR = 0xFF0000FF,
        WIREFRAME_COLOR = 0x80FF0000
    };

    enum {
        kCount = 9
    };
    SkPoint fPts[kCount];
    SkScalar fWidth, fDWidth;
public:
    TestStrokeView() {
        this->setBGColor(SK_ColorLTGRAY);

        fPts[0].set(50, 200);
        fPts[1].set(50, 100);
        fPts[2].set(150, 50);
        fPts[3].set(300, 50);

        fPts[4].set(350, 200);
        fPts[5].set(350, 100);
        fPts[6].set(450, 50);

        fPts[7].set(200, 200);
        fPts[8].set(400, 400);

        fWidth = 50;
        fDWidth = 0.25f;
    }

protected:
    virtual bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RotateCircles3");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void draw_points(SkCanvas* canvas, const SkPath& path, SkColor color,
                     bool show_lines) {
        SkPaint paint;
        paint.setColor(color);
        paint.setAlpha(0x80);

        int n = path.countPoints();
        SkAutoSTArray<32, SkPoint> pts(n);
        if (show_lines) {
            path.getPoints(pts.get(), n);
            canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts.get(), paint);
        } else {
            n = getOnCurvePoints(path, pts.get());
        }
        paint.setStrokeWidth(5);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts.get(), paint);
    }

    void draw_ribs(SkCanvas* canvas, const SkPath& path, SkScalar width,
                   SkColor color) {
        const SkScalar radius = width / 2;

        SkPathMeasure meas(path, false);
        SkScalar total = meas.getLength();

        SkScalar delta = 8;
        SkPaint paint;
        paint.setColor(color);

        SkPoint pos, tan;
        for (SkScalar dist = 0; dist <= total; dist += delta) {
            if (meas.getPosTan(dist, &pos, &tan)) {
                tan.scale(radius);
                tan.rotateCCW();
                canvas->drawLine(pos.x() + tan.x(), pos.y() + tan.y(),
                                 pos.x() - tan.x(), pos.y() - tan.y(), paint);
            }
        }
    }

    void draw_stroke(SkCanvas* canvas, const SkPath& path, SkScalar width) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);

        paint.setColor(SKELETON_COLOR);
        canvas->drawPath(path, paint);
        draw_points(canvas, path, SKELETON_COLOR, true);

        draw_ribs(canvas, path, width, 0xFF00FF00);

        SkPath fill;

        SkPaint p;
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(width);
        p.getFillPath(path, &fill);

        paint.setColor(WIREFRAME_COLOR);
        canvas->drawPath(fill, paint);
        draw_points(canvas, fill, WIREFRAME_COLOR, false);
    }

    virtual void onDrawContent(SkCanvas* canvas) {
        SkPath path;
        SkScalar width = fWidth;

        path.moveTo(fPts[0]);
        path.cubicTo(fPts[1], fPts[2], fPts[3]);
        draw_stroke(canvas, path, width);

        path.reset();
        path.moveTo(fPts[4]);
        path.quadTo(fPts[5], fPts[6]);
        draw_stroke(canvas, path, width);

        SkScalar rad = 32;
        SkRect r;
        r.set(&fPts[7], 2);
        path.reset();
        SkRRect rr;
        rr.setRectXY(r, rad, rad);
        path.addRRect(rr);
        draw_stroke(canvas, path, width);

        path.reset();
        SkRRect rr2;
        rr.inset(width/2, width/2, &rr2);
        path.addRRect(rr2, SkPath::kCCW_Direction);
        rr.inset(-width/2, -width/2, &rr2);
        path.addRRect(rr2, SkPath::kCW_Direction);
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setColor(0x40FF8844);
        canvas->drawPath(path, paint);

        fWidth += fDWidth;
        if (fDWidth > 0 && fWidth > 100) {
            fDWidth = -fDWidth;
        } else if (fDWidth < 0 && fWidth < 10) {
            fDWidth = -fDWidth;
        }
        this->inval(NULL);
    }

    class MyClick : public Click {
    public:
        int fIndex;
        MyClick(SkView* target, int index) : Click(target), fIndex(index) {}
    };

    virtual SkView::Click* onFindClickHandler(SkScalar x, SkScalar y,
                                              unsigned modi) SK_OVERRIDE {
        for (size_t i = 0; i < SK_ARRAY_COUNT(fPts); ++i) {
            if (hittest(fPts[i], x, y)) {
                return new MyClick(this, (int)i);
            }
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    virtual bool onClick(Click* click) {
        int index = ((MyClick*)click)->fIndex;
        fPts[index].offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                           SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
        this->inval(NULL);
        return true;
    }

private:
    typedef SkView INHERITED;
};

///////////////////////////////////////////////////////////////////////////////

static SkView* F0() { return new RotateCirclesView; }
static SkViewRegister gR0(F0);
static SkView* F1() { return new TestCirclesView; }
static SkViewRegister gR1(F1);
static SkView* F2() { return new TestStrokeView; }
static SkViewRegister gR2(F2);
