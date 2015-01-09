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
#include "SkStrokerPriv.h"

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
    bool onQuery(SkEvent* evt) SK_OVERRIDE {
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

    void onDrawContent(SkCanvas* canvas) SK_OVERRIDE {
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

struct StrokeTypeButton {
    SkRect fBounds;
    char fLabel;
    bool fEnabled;
};

class TestStrokeView : public SampleView {
    enum {
        SKELETON_COLOR = 0xFF0000FF,
        WIREFRAME_COLOR = 0x80FF0000
    };

    enum {
        kCount = 9
    };
    SkPoint fPts[kCount];
    SkRect fErrorControl;
    SkRect fWidthControl;
    StrokeTypeButton fCubicButton;
    StrokeTypeButton fQuadButton;
    StrokeTypeButton fRRectButton;
    SkScalar fWidth, fDWidth;
    bool fAnimate;
#if QUAD_STROKE_APPROXIMATION && defined(SK_DEBUG)
    #define kStrokerErrorMin 0.001f
    #define kStrokerErrorMax 5
#endif
    #define kWidthMin 1
    #define kWidthMax 100
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

        fCubicButton.fLabel = 'C';
        fCubicButton.fEnabled = true;
        fQuadButton.fLabel = 'Q';
        fQuadButton.fEnabled = true;
        fRRectButton.fLabel = 'R';
        fRRectButton.fEnabled = true;
        fAnimate = true;
    }

protected:
    bool onQuery(SkEvent* evt) SK_OVERRIDE {
        if (SampleCode::TitleQ(*evt)) {
            SampleCode::TitleR(evt, "RotateCircles3");
            return true;
        }
        return this->INHERITED::onQuery(evt);
    }

    void onSizeChange() SK_OVERRIDE {
        fErrorControl.setXYWH(this->width() - 100, 30, 30, 400);
        fWidthControl.setXYWH(this->width() -  50, 30, 30, 400);
        fCubicButton.fBounds.setXYWH(this->width() - 50, 450, 30, 30);
        fQuadButton.fBounds.setXYWH(this->width() - 50, 500, 30, 30);
        fRRectButton.fBounds.setXYWH(this->width() - 50, 550, 30, 30);
        this->INHERITED::onSizeChange();
    }

    void draw_points(SkCanvas* canvas, const SkPath& path, SkColor color,
                     bool show_lines) {
        SkPaint paint;
        paint.setColor(color);
        paint.setAlpha(0x80);
        paint.setAntiAlias(true);
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

    void draw_button(SkCanvas* canvas, const StrokeTypeButton& button) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(button.fEnabled ? 0xFF3F0000 : 0x6F3F0000);
        canvas->drawRect(button.fBounds, paint);
        paint.setTextSize(25.0f);
        paint.setColor(button.fEnabled ? 0xFF3F0000 : 0x6F3F0000);
        paint.setTextAlign(SkPaint::kCenter_Align);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawText(&button.fLabel, 1, button.fBounds.centerX(), button.fBounds.fBottom - 5,
                paint);
    }

    void draw_control(SkCanvas* canvas, const SkRect& bounds, SkScalar value,
            SkScalar min, SkScalar max, const char* name) {
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        canvas->drawRect(bounds, paint);
        SkScalar scale = max - min;
        SkScalar yPos = bounds.fTop + (value - min) * bounds.height() / scale;
        paint.setColor(0xFFFF0000);
        canvas->drawLine(bounds.fLeft - 5, yPos, bounds.fRight + 5, yPos, paint);
        SkString label;
        label.printf("%0.3g", value);
        paint.setColor(0xFF000000);
        paint.setTextSize(11.0f);
        paint.setStyle(SkPaint::kFill_Style);
        canvas->drawText(label.c_str(), label.size(), bounds.fLeft + 5, yPos - 5, paint);
        paint.setTextSize(13.0f);
        canvas->drawText(name, strlen(name), bounds.fLeft, bounds.bottom() + 11, paint);
    }

    void onDrawContent(SkCanvas* canvas) SK_OVERRIDE {
        SkPath path;
        SkScalar width = fWidth;

        if (fCubicButton.fEnabled) {
            path.moveTo(fPts[0]);
            path.cubicTo(fPts[1], fPts[2], fPts[3]);
            draw_stroke(canvas, path, width);
        }

        if (fQuadButton.fEnabled) {
            path.reset();
            path.moveTo(fPts[4]);
            path.quadTo(fPts[5], fPts[6]);
            draw_stroke(canvas, path, width);
        }

        if (fRRectButton.fEnabled) {
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
        }

        if (fAnimate) {
            fWidth += fDWidth;
            if (fDWidth > 0 && fWidth > kWidthMax) {
                fDWidth = -fDWidth;
            } else if (fDWidth < 0 && fWidth < kWidthMin) {
                fDWidth = -fDWidth;
            }
        }
#if QUAD_STROKE_APPROXIMATION && defined(SK_DEBUG)
        draw_control(canvas, fErrorControl, gDebugStrokerError, kStrokerErrorMin, kStrokerErrorMax,
                "error");
#endif
        draw_control(canvas, fWidthControl, fWidth, kWidthMin, kWidthMax, "width");
        draw_button(canvas, fQuadButton);
        draw_button(canvas, fCubicButton);
        draw_button(canvas, fRRectButton);
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
        const SkRect& rectPt = SkRect::MakeXYWH(x, y, 1, 1);
#if QUAD_STROKE_APPROXIMATION && defined(SK_DEBUG)
        if (fErrorControl.contains(rectPt)) {
            return new MyClick(this, (int) SK_ARRAY_COUNT(fPts) + 1);
        }
#endif
        if (fWidthControl.contains(rectPt)) {
            return new MyClick(this, (int) SK_ARRAY_COUNT(fPts) + 3);
        }
        if (fCubicButton.fBounds.contains(rectPt)) {
            fCubicButton.fEnabled ^= true;
            return new MyClick(this, (int) SK_ARRAY_COUNT(fPts) + 4);
        }
        if (fQuadButton.fBounds.contains(rectPt)) {
            fQuadButton.fEnabled ^= true;
            return new MyClick(this, (int) SK_ARRAY_COUNT(fPts) + 5);
        }
        if (fRRectButton.fBounds.contains(rectPt)) {
            fRRectButton.fEnabled ^= true;
            return new MyClick(this, (int) SK_ARRAY_COUNT(fPts) + 6);
        }
        return this->INHERITED::onFindClickHandler(x, y, modi);
    }

    static SkScalar MapScreenYtoValue(int y, const SkRect& control, SkScalar min,
            SkScalar max) {
        return (SkIntToScalar(y) - control.fTop) / control.height() * (max - min) + min;
    }

    bool onClick(Click* click) SK_OVERRIDE {
        int index = ((MyClick*)click)->fIndex;
        if (index < (int) SK_ARRAY_COUNT(fPts)) {
            fPts[index].offset(SkIntToScalar(click->fICurr.fX - click->fIPrev.fX),
                               SkIntToScalar(click->fICurr.fY - click->fIPrev.fY));
            this->inval(NULL);
        }
#if QUAD_STROKE_APPROXIMATION && defined(SK_DEBUG)
        else if (index == (int) SK_ARRAY_COUNT(fPts) + 1) {
            gDebugStrokerError = MapScreenYtoValue(click->fICurr.fY, fErrorControl,
                    kStrokerErrorMin, kStrokerErrorMax);
            gDebugStrokerErrorSet = true;
        }
#endif
        else if (index == (int) SK_ARRAY_COUNT(fPts) + 3) {
            fWidth = MapScreenYtoValue(click->fICurr.fY, fWidthControl, kWidthMin, kWidthMax);
            fAnimate = fWidth <= kWidthMin;
        }
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
