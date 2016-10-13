/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
#include "SkBlurMaskFilter.h"
#include "SkGaussianEdgeShader.h"
#include "SkRRectsGaussianEdgeShader.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkRRect.h"
#include "SkStroke.h"

constexpr int kNumCols = 2;
constexpr int kNumRows = 5;
constexpr int kCellSize = 128;
constexpr SkScalar kPad = 8.0f;
constexpr SkScalar kInitialBlurRadius = 8.0f;
constexpr SkScalar kPeriod = 8.0f;
constexpr int kClipOffset = 32;

///////////////////////////////////////////////////////////////////////////////////////////////////

class Object {
public:
    virtual ~Object() {}
    virtual bool asRRect(SkRRect* rr) const = 0;
    virtual SkPath asPath(SkScalar inset) const = 0;
    virtual void draw(SkCanvas* canvas, const SkPaint& paint) const = 0;
    virtual void clip(SkCanvas* canvas) const = 0;
    virtual bool contains(const SkRect& r) const = 0;
    virtual const SkRect& bounds() const = 0;
};

typedef Object* (*PFMakeMthd)(const SkRect& r);

class RRect : public Object {
public:
    RRect(const SkRect& r) {
        fRRect = SkRRect::MakeRectXY(r, 4*kPad, 4*kPad);
    }

    bool asRRect(SkRRect* rr) const override {
        *rr = fRRect;
        return true;
    }

    SkPath asPath(SkScalar inset) const override {
        SkRRect tmp = fRRect;
        tmp.inset(inset, inset);
        SkPath p;
        p.addRRect(tmp);
        return p;
    }

    void draw(SkCanvas* canvas, const SkPaint& paint) const override {
        canvas->drawRRect(fRRect, paint);
    }

    void clip(SkCanvas* canvas) const override {
        canvas->clipRRect(fRRect);
    }

    bool contains(const SkRect& r) const override {
        return fRRect.contains(r);
    }

    const SkRect& bounds() const override {
        return fRRect.getBounds();
    }

    static Object* Make(const SkRect& r) {
        return new RRect(r);
    }

private:
    SkRRect  fRRect;
};

class StrokedRRect : public Object {
public:
    StrokedRRect(const SkRect& r) {
        fRRect = SkRRect::MakeRectXY(r, 2*kPad, 2*kPad);
        fStrokedBounds = r.makeOutset(kPad, kPad);
    }

    bool asRRect(SkRRect* rr) const override {
        return false;
    }

    SkPath asPath(SkScalar inset) const override {
        SkRRect tmp = fRRect;
        tmp.inset(inset, inset);

        // In this case we want the outline of the stroked rrect
        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(kPad);

        SkPath p, stroked;
        p.addRRect(tmp);
        SkStroke stroke(paint);
        stroke.strokePath(p, &stroked);
        return stroked;
    }

    void draw(SkCanvas* canvas, const SkPaint& paint) const override {
        SkPaint stroke(paint);
        stroke.setStyle(SkPaint::kStroke_Style);
        stroke.setStrokeWidth(kPad);

        canvas->drawRRect(fRRect, stroke);
    }

    void clip(SkCanvas* canvas) const override {
        canvas->clipPath(this->asPath(0.0f));
    }

    bool contains(const SkRect& r) const override {
        return false;
    }

    const SkRect& bounds() const override {
        return fStrokedBounds;
    }

    static Object* Make(const SkRect& r) {
        return new StrokedRRect(r);
    }

private:
    SkRRect  fRRect;
    SkRect   fStrokedBounds;
};

class Oval : public Object {
public:
    Oval(const SkRect& r) {
        fRRect = SkRRect::MakeOval(r);
    }

    bool asRRect(SkRRect* rr) const override {
        *rr = fRRect;
        return true;
    }

    SkPath asPath(SkScalar inset) const override { 
        SkRRect tmp = fRRect;
        tmp.inset(inset, inset);

        SkPath p;
        p.addRRect(tmp);
        return p;
    }

    void draw(SkCanvas* canvas, const SkPaint& paint) const override {
        canvas->drawRRect(fRRect, paint);
    }

    void clip(SkCanvas* canvas) const override {
        canvas->clipRRect(fRRect);
    }

    bool contains(const SkRect& r) const override {
        return fRRect.contains(r);
    }

    const SkRect& bounds() const override {
        return fRRect.getBounds();
    }

    static Object* Make(const SkRect& r) {
        return new Oval(r);
    }

private:
    SkRRect  fRRect;
};

class Rect : public Object {
public:
    Rect(const SkRect& r) : fRect(r) { }

    bool asRRect(SkRRect* rr) const override {
        *rr = SkRRect::MakeRect(fRect);
        return true;
    }

    SkPath asPath(SkScalar inset) const override { 
        SkRect tmp = fRect;
        tmp.inset(inset, inset);

        SkPath p;
        p.addRect(tmp);
        return p;
    }

    void draw(SkCanvas* canvas, const SkPaint& paint) const override {
        canvas->drawRect(fRect, paint);
    }

    void clip(SkCanvas* canvas) const override {
        canvas->clipRect(fRect);
    }

    bool contains(const SkRect& r) const override {
        return fRect.contains(r);
    }

    const SkRect& bounds() const override {
        return fRect;
    }

    static Object* Make(const SkRect& r) {
        return new Rect(r);
    }

private:
    SkRect  fRect;
};

class Pentagon : public Object {
public:
    Pentagon(const SkRect& r) {
        SkPoint points[5] = {
            {  0.000000f, -1.000000f },
            { -0.951056f, -0.309017f },
            { -0.587785f,  0.809017f },
            {  0.587785f,  0.809017f },
            {  0.951057f, -0.309017f },
        };

        SkScalar height = r.height()/2.0f;
        SkScalar width = r.width()/2.0f;

        fPath.moveTo(r.centerX() + points[0].fX * width, r.centerY() + points[0].fY * height);
        fPath.lineTo(r.centerX() + points[1].fX * width, r.centerY() + points[1].fY * height);
        fPath.lineTo(r.centerX() + points[2].fX * width, r.centerY() + points[2].fY * height);
        fPath.lineTo(r.centerX() + points[3].fX * width, r.centerY() + points[3].fY * height);
        fPath.lineTo(r.centerX() + points[4].fX * width, r.centerY() + points[4].fY * height);
        fPath.close();
    }

    bool asRRect(SkRRect* rr) const override {
        return false;
    }

    SkPath asPath(SkScalar inset) const override { return fPath; }

    void draw(SkCanvas* canvas, const SkPaint& paint) const override {
        canvas->drawPath(fPath, paint);
    }

    void clip(SkCanvas* canvas) const override {
        canvas->clipPath(this->asPath(0.0f));
    }

    bool contains(const SkRect& r) const override {
        return false;
    }

    const SkRect& bounds() const override {
        return fPath.getBounds();
    }

    static Object* Make(const SkRect& r) {
        return new Pentagon(r);
    }

private:
    SkPath fPath;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
namespace skiagm {

// This GM attempts to mimic Android's reveal animation
class RevealGM : public GM {
public:
    enum Mode {
        kGaussianEdge_Mode,
        kBlurMask_Mode,
        kRRectsGaussianEdge_Mode,

        kLast_Mode = kRRectsGaussianEdge_Mode
    };

    static const int kModeCount = kLast_Mode + 1;

    RevealGM()
        : fFraction(0.5f)
        , fMode(kRRectsGaussianEdge_Mode)
        , fPause(false)
        , fBlurRadius(kInitialBlurRadius) {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("reveal");
    }

    SkISize onISize() override {
        return SkISize::Make(kNumCols * kCellSize, kNumRows * kCellSize);
    }

    void onDraw(SkCanvas* canvas) override {
        PFMakeMthd clipMakes[kNumCols] = { Oval::Make, Rect::Make };
        PFMakeMthd drawMakes[kNumRows] = {
            RRect::Make, StrokedRRect::Make, Oval::Make, Rect::Make, Pentagon::Make
        };

        SkPaint strokePaint;
        strokePaint.setColor(SK_ColorGREEN);
        strokePaint.setStyle(SkPaint::kStroke_Style);
        strokePaint.setStrokeWidth(0.0f);

        for (int y = 0; y < kNumRows; ++y) {
            for (int x = 0; x < kNumCols; ++x) {
                SkRect cell = SkRect::MakeXYWH(SkIntToScalar(x*kCellSize),
                                               SkIntToScalar(y*kCellSize),
                                               SkIntToScalar(kCellSize),
                                               SkIntToScalar(kCellSize));

                canvas->save();
                canvas->clipRect(cell);

                cell.inset(kPad, kPad);
                SkPoint clipCenter = SkPoint::Make(cell.centerX() - kClipOffset,
                                                   cell.centerY() + kClipOffset);
                SkScalar curSize = kCellSize * fFraction;
                const SkRect clipRect = SkRect::MakeLTRB(clipCenter.fX - curSize,
                                                         clipCenter.fY - curSize,
                                                         clipCenter.fX + curSize,
                                                         clipCenter.fY + curSize);

                SkAutoTDelete<Object> clipObj((*clipMakes[x])(clipRect));
                SkAutoTDelete<Object> drawObj((*drawMakes[y])(cell));

                // The goal is to replace this clipped draw (which clips the 
                // shadow) with a draw using the geometric clip
                if (kGaussianEdge_Mode == fMode) {
                    canvas->save();
                        clipObj->clip(canvas);

                        // Draw with GaussianEdgeShader
                        SkPaint paint;
                        paint.setAntiAlias(true);
                        // G channel is an F6.2 radius
                        int iBlurRad = (int)(4.0f * fBlurRadius);
                        paint.setColor(SkColorSetARGB(255, iBlurRad >> 8, iBlurRad & 0xFF, 0));
                        paint.setShader(SkGaussianEdgeShader::Make());
                        drawObj->draw(canvas, paint);
                    canvas->restore();
                } else if (kBlurMask_Mode == fMode) {
                    SkPath clippedPath;

                    SkScalar sigma = fBlurRadius / 4.0f;

                    if (clipObj->contains(drawObj->bounds())) {
                        clippedPath = drawObj->asPath(2.0f*sigma);
                    } else {
                        SkPath drawnPath = drawObj->asPath(2.0f*sigma);
                        SkPath clipPath  = clipObj->asPath(2.0f*sigma);

                        SkAssertResult(Op(clipPath, drawnPath, kIntersect_SkPathOp, &clippedPath));
                    }

                    SkPaint blurPaint;
                    blurPaint.setAntiAlias(true);
                    blurPaint.setMaskFilter(SkBlurMaskFilter::Make(kNormal_SkBlurStyle, sigma));
                    canvas->drawPath(clippedPath, blurPaint);
                } else {
                    SkASSERT(kRRectsGaussianEdge_Mode == fMode);

                    SkRect cover = drawObj->bounds();
                    SkAssertResult(cover.intersect(clipObj->bounds()));

                    SkPaint paint;

                    SkRRect clipRR, drawnRR;

                    if (clipObj->asRRect(&clipRR) && drawObj->asRRect(&drawnRR)) {
                        paint.setShader(SkRRectsGaussianEdgeShader::Make(clipRR, drawnRR,
                                                                         fBlurRadius));
                    }

                    canvas->drawRect(cover, paint);
                }

                // Draw the clip and draw objects for reference
                SkPaint strokePaint;
                strokePaint.setStyle(SkPaint::kStroke_Style);
                strokePaint.setStrokeWidth(0);
                strokePaint.setColor(SK_ColorRED);
                canvas->drawPath(drawObj->asPath(0.0f), strokePaint);
                strokePaint.setColor(SK_ColorGREEN);
                canvas->drawPath(clipObj->asPath(0.0f), strokePaint);

                canvas->restore();
            }
        }
    }

    bool onHandleKey(SkUnichar uni) override {
        switch (uni) {
            case 'C':
                fMode = (Mode)((fMode + 1) % kModeCount);
                return true;
            case '+':
                fBlurRadius += 1.0f;
                return true;
            case '-':
                fBlurRadius = SkTMax(1.0f, fBlurRadius - 1.0f);
                return true;
            case 'p':
                fPause = !fPause;
                return true;
        }        
    
        return false;
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        if (!fPause) {
            fFraction = timer.pingPong(kPeriod, 0.0f, 0.0f, 1.0f);
        }
        return true;
    }

private:
    SkScalar fFraction;
    Mode     fMode;
    bool     fPause;
    float    fBlurRadius;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new RevealGM;)
}
