/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkAnimTimer.h"
#include "SkGaussianEdgeShader.h"
#include "SkPath.h"
#include "SkPathOps.h"
#include "SkRRect.h"
#include "SkStroke.h"

constexpr int kNumCols = 2;
constexpr int kNumRows = 5;
constexpr int kCellSize = 128;
constexpr SkScalar kPad = 8.0f;
constexpr SkScalar kPeriod = 8.0f;
constexpr int kClipOffset = 32;

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef SkPath (*PFDrawMthd)(SkCanvas*, const SkRect&, bool);

static SkPath draw_rrect(SkCanvas* canvas, const SkRect& r, bool stroked) {
    SkRRect rr = SkRRect::MakeRectXY(r, 2*kPad, 2*kPad);

    SkPaint paint;
    paint.setAntiAlias(true);
    if (stroked) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
    } else {
        // G channel is an F6.2 radius
        paint.setColor(SkColorSetARGB(255, 255, (unsigned char)(4*kPad), 0));
        paint.setShader(SkGaussianEdgeShader::Make());
    }
    canvas->drawRRect(rr, paint);

    SkPath p;
    p.addRoundRect(r, 2*kPad, 2*kPad);
    return p;
}

static SkPath draw_stroked_rrect(SkCanvas* canvas, const SkRect& r, bool stroked) {
    SkRect insetRect = r;
    insetRect.inset(kPad, kPad);
    SkRRect rr = SkRRect::MakeRectXY(insetRect, 2*kPad, 2*kPad);

    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(kPad);

    if (stroked) {
        // In this case we want to draw a stroked representation of the stroked rrect
        SkPath p, stroked;
        p.addRRect(rr);
        SkStroke stroke(paint);
        stroke.strokePath(p, &stroked);

        paint.setStrokeWidth(0);
        paint.setColor(SK_ColorRED);
        canvas->drawPath(stroked, paint);
    } else {
        // G channel is an F6.2 radius
        paint.setColor(SkColorSetARGB(255, 255, (unsigned char)(4*kPad), 0));
        paint.setShader(SkGaussianEdgeShader::Make());

        canvas->drawRRect(rr, paint);
    }

    SkPath p;
    insetRect.outset(kPad/2.0f, kPad/2.0f);
    p.addRoundRect(insetRect, 2*kPad, 2*kPad);
    return p;
}

static SkPath draw_oval(SkCanvas* canvas, const SkRect& r, bool stroked) {
    SkRRect rr = SkRRect::MakeOval(r);

    SkPaint paint;
    paint.setAntiAlias(true);
    if (stroked) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
    } else {
        // G channel is an F6.2 radius
        paint.setColor(SkColorSetARGB(255, 255, (unsigned char)(4*kPad), 0));
        paint.setShader(SkGaussianEdgeShader::Make());
    }
    canvas->drawRRect(rr, paint);

    SkPath p;
    p.addOval(r);
    return p;
}

static SkPath draw_square(SkCanvas* canvas, const SkRect& r, bool stroked) {
    SkPaint paint;
    paint.setAntiAlias(true);
    if (stroked) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
    } else {
        // G channel is an F6.2 radius
        paint.setColor(SkColorSetARGB(255, 255, (unsigned char)(4*kPad), 0));
        paint.setShader(SkGaussianEdgeShader::Make());
    }
    canvas->drawRect(r, paint);

    SkPath p;
    p.addRect(r);
    return p;
}

static SkPath draw_pentagon(SkCanvas* canvas, const SkRect& r, bool stroked) {
    SkPath p;

    SkPoint points[5] = {
        {  0.000000f, -1.000000f },
        { -0.951056f, -0.309017f },
        { -0.587785f,  0.809017f },
        {  0.587785f,  0.809017f },
        {  0.951057f, -0.309017f },
    };

    SkScalar height = r.height()/2.0f;
    SkScalar width = r.width()/2.0f;

    p.moveTo(r.centerX() + points[0].fX * width, r.centerY() + points[0].fY * height);
    p.lineTo(r.centerX() + points[1].fX * width, r.centerY() + points[1].fY * height);
    p.lineTo(r.centerX() + points[2].fX * width, r.centerY() + points[2].fY * height);
    p.lineTo(r.centerX() + points[3].fX * width, r.centerY() + points[3].fY * height);
    p.lineTo(r.centerX() + points[4].fX * width, r.centerY() + points[4].fY * height);
    p.close();

    SkPaint paint;
    paint.setAntiAlias(true);
    if (stroked) {
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setColor(SK_ColorRED);
    } else {
        // G channel is an F6.2 radius
        paint.setColor(SkColorSetARGB(255, 255, (unsigned char)(4*kPad), 0));
        // This currently goes through the GrAAConvexPathRenderer and produces a
        // AAConvexPathBatch (i.e., it doesn't have a analytic distance)
        // paint.setShader(SkGaussianEdgeShader::Make());
    }
    canvas->drawPath(p, paint);

    return p;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
typedef void (*PFClipMthd)(SkCanvas* canvas, const SkPoint&, SkScalar);

static void circle_clip(SkCanvas* canvas, const SkPoint& center, SkScalar rad) {
    SkRect r = SkRect::MakeLTRB(center.fX - rad, center.fY - rad, center.fX + rad, center.fY + rad);
    SkRRect rr = SkRRect::MakeOval(r);

    canvas->clipRRect(rr);
}

static void square_clip(SkCanvas* canvas, const SkPoint& center, SkScalar size) {
    SkScalar newSize = SK_ScalarRoot2Over2 * size;
    SkRect r = SkRect::MakeLTRB(center.fX - newSize, center.fY - newSize,
                                center.fX + newSize, center.fY + newSize);

    canvas->clipRect(r);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// These are stand alone methods (rather than just, say, returning the SkPath for the clip 
// object) so that we can catch the clip-contains-victim case.
typedef SkPath (*PFGeometricClipMthd)(const SkPoint&, SkScalar, const SkPath&);

static SkPath circle_geometric_clip(const SkPoint& center, SkScalar rad, const SkPath& victim) {
    const SkRect bound = victim.getBounds();
    SkPoint pts[4];
    bound.toQuad(pts);

    bool clipContainsVictim = true;
    for (int i = 0; i < 4; ++i) {
        SkScalar distSq = (pts[i].fX - center.fX) * (pts[i].fX - center.fX) +
                          (pts[i].fY - center.fY) * (pts[i].fY - center.fY);
        if (distSq >= rad*rad) {
            clipContainsVictim = false;
        }
    }

    if (clipContainsVictim) {
        return victim;
    }

    // Add victim contains clip test?

    SkPath clipPath;
    clipPath.addCircle(center.fX, center.fY, rad);

    SkPath result;
    SkAssertResult(Op(clipPath, victim, kIntersect_SkPathOp, &result));

    return result;
}

static SkPath square_geometric_clip(const SkPoint& center, SkScalar size, const SkPath& victim) {
    SkScalar newSize = SK_ScalarRoot2Over2 * size;
    SkRect r = SkRect::MakeLTRB(center.fX - newSize, center.fY - newSize,
                                center.fX + newSize, center.fY + newSize);

    const SkRect bound = victim.getBounds();

    if (r.contains(bound)) {
        return victim;
    }

    // Add victim contains clip test?

    SkPath clipPath;
    clipPath.addRect(r);

    SkPath result;
    SkAssertResult(Op(clipPath, victim, kIntersect_SkPathOp, &result));

    return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
namespace skiagm {

// This GM attempts to mimic Android's reveal animation
class RevealGM : public GM {
public:
    RevealGM() : fFraction(0.5f) {
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
        PFClipMthd clips[kNumCols] = { circle_clip, square_clip };
        PFGeometricClipMthd geometricClips[kNumCols] = {
            circle_geometric_clip, 
            square_geometric_clip
        };
        PFDrawMthd draws[kNumRows] = {
            draw_rrect,
            draw_stroked_rrect,
            draw_oval,
            draw_square,
            draw_pentagon
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

                cell.inset(kPad, kPad);
                SkPoint clipCenter = SkPoint::Make(cell.centerX() - kClipOffset,
                                                   cell.centerY() + kClipOffset);

                SkScalar curSize = kCellSize * fFraction;

                // The goal is to replace this clipped draw (which clips the 
                // shadow) with a draw using the geometric clip
                canvas->save();
                      (*clips[x])(canvas, clipCenter, curSize);
                      (*draws[y])(canvas, cell, false);
                canvas->restore();

                SkPath drawnPath = (*draws[y])(canvas, cell, true);
                SkPath clippedPath = (*geometricClips[x])(clipCenter, curSize, drawnPath);
                canvas->drawPath(clippedPath, strokePaint);
            }
        }
    }

    bool onAnimate(const SkAnimTimer& timer) override {
        fFraction = timer.pingPong(kPeriod, 0.0f, 0.0f, 1.0f);
        return true;
    }

private:
    SkScalar fFraction;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new RevealGM;)
}
