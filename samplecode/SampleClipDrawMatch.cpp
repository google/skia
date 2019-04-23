/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkRRect.h"
#include "include/core/SkTime.h"
#include "include/utils/SkInterpolator.h"
#include "samplecode/Sample.h"

// This slide tests out the match up between BW clipping and rendering. It can
// draw a large rect through some clip geometry and draw the same geometry
// normally. Which one is drawn first can be toggled. The pair of objects is translated
// fractionally (via an animator) to expose snapping bugs. The key bindings are:
//      1-9: the different geometries
//      t:   toggle which is drawn first the clip or the normal geometry
//      f:   flip-flops which corner the bottom AA clip rect occupies in the complex clip cases

// The possible geometric combinations to test
enum Geometry {
    kRect_Geometry,
    kRRect_Geometry,
    kCircle_Geometry,
    kConvexPath_Geometry,
    kConcavePath_Geometry,
    kRectAndRect_Geometry,
    kRectAndRRect_Geometry,
    kRectAndConvex_Geometry,
    kRectAndConcave_Geometry
};

// The basic rect used is [kMin,kMin]..[kMax,kMax]
static const float kMin = 100.5f;
static const float kMid = 200.0f;
static const float kMax = 299.5f;

// The translation applied to the base AA rect in the combination cases
// (i.e., kRectAndRect through kRectAndConcave)
static const float kXlate = 100.0f;

SkRect create_rect(const SkPoint& offset) {
    SkRect r = SkRect::MakeLTRB(kMin, kMin, kMax, kMax);
    r.offset(offset);
    return r;
}

SkRRect create_rrect(const SkPoint& offset) {
    SkRRect rrect;
    rrect.setRectXY(create_rect(offset), 10, 10);
    return rrect;
}

SkRRect create_circle(const SkPoint& offset) {
    SkRRect circle;
    circle.setOval(create_rect(offset));
    return circle;
}

SkPath create_convex_path(const SkPoint& offset) {
    SkPath convexPath;
    convexPath.moveTo(kMin, kMin);
    convexPath.lineTo(kMax, kMax);
    convexPath.lineTo(kMin, kMax);
    convexPath.close();
    convexPath.offset(offset.fX, offset.fY);
    return convexPath;
}

SkPath create_concave_path(const SkPoint& offset) {
    SkPath concavePath;
    concavePath.moveTo(kMin, kMin);
    concavePath.lineTo(kMid, 105.0f);
    concavePath.lineTo(kMax, kMin);
    concavePath.lineTo(295.0f, kMid);
    concavePath.lineTo(kMax, kMax);
    concavePath.lineTo(kMid, 295.0f);
    concavePath.lineTo(kMin, kMax);
    concavePath.lineTo(105.0f, kMid);
    concavePath.close();

    concavePath.offset(offset.fX, offset.fY);
    return concavePath;
}

static void draw_normal_geom(SkCanvas* canvas, const SkPoint& offset, int geom, bool useAA) {
    SkPaint p;
    p.setAntiAlias(useAA);
    p.setColor(SK_ColorBLACK);

    switch (geom) {
    case kRect_Geometry:                // fall thru
    case kRectAndRect_Geometry:
        canvas->drawRect(create_rect(offset), p);
        break;
    case kRRect_Geometry:               // fall thru
    case kRectAndRRect_Geometry:
        canvas->drawRRect(create_rrect(offset), p);
        break;
    case kCircle_Geometry:
        canvas->drawRRect(create_circle(offset), p);
        break;
    case kConvexPath_Geometry:          // fall thru
    case kRectAndConvex_Geometry:
        canvas->drawPath(create_convex_path(offset), p);
        break;
    case kConcavePath_Geometry:         // fall thru
    case kRectAndConcave_Geometry:
        canvas->drawPath(create_concave_path(offset), p);
        break;
    }
}

class ClipDrawMatchView : public Sample {
public:
    ClipDrawMatchView() : fTrans(2, 5), fGeom(kRect_Geometry), fClipFirst(true), fSign(1) {
        SkScalar values[2];

        fTrans.setRepeatCount(999);
        values[0] = values[1] = 0;
        fTrans.setKeyFrame(0, GetMSecs() + 1000, values);
        values[1] = 1;
        fTrans.setKeyFrame(1, GetMSecs() + 2000, values);
        values[0] = values[1] = 1;
        fTrans.setKeyFrame(2, GetMSecs() + 3000, values);
        values[1] = 0;
        fTrans.setKeyFrame(3, GetMSecs() + 4000, values);
        values[0] = 0;
        fTrans.setKeyFrame(4, GetMSecs() + 5000, values);
    }

protected:
    bool onQuery(Sample::Event* evt) override {
        if (Sample::TitleQ(*evt)) {
            Sample::TitleR(evt, "ClipDrawMatch");
            return true;
        }
        SkUnichar uni;
        if (Sample::CharQ(*evt, &uni)) {
            switch (uni) {
                case '1': fGeom = kRect_Geometry; return true;
                case '2': fGeom = kRRect_Geometry; return true;
                case '3': fGeom = kCircle_Geometry; return true;
                case '4': fGeom = kConvexPath_Geometry; return true;
                case '5': fGeom = kConcavePath_Geometry; return true;
                case '6': fGeom = kRectAndRect_Geometry; return true;
                case '7': fGeom = kRectAndRRect_Geometry; return true;
                case '8': fGeom = kRectAndConvex_Geometry; return true;
                case '9': fGeom = kRectAndConcave_Geometry; return true;
                case 'f': fSign = -fSign; return true;
                case 't': fClipFirst = !fClipFirst; return true;
                default: break;
            }
        }
        return this->INHERITED::onQuery(evt);
    }

    void drawClippedGeom(SkCanvas* canvas, const SkPoint& offset, bool useAA) {

        int count = canvas->save();

        switch (fGeom) {
        case kRect_Geometry:
            canvas->clipRect(create_rect(offset), useAA);
            break;
        case kRRect_Geometry:
            canvas->clipRRect(create_rrect(offset), useAA);
            break;
        case kCircle_Geometry:
            canvas->clipRRect(create_circle(offset), useAA);
            break;
        case kConvexPath_Geometry:
            canvas->clipPath(create_convex_path(offset), useAA);
            break;
        case kConcavePath_Geometry:
            canvas->clipPath(create_concave_path(offset), useAA);
            break;
        case kRectAndRect_Geometry: {
            SkRect r = create_rect(offset);
            r.offset(fSign * kXlate, fSign * kXlate);
            canvas->clipRect(r, true); // AA here forces shader clips
            canvas->clipRect(create_rect(offset), useAA);
            } break;
        case kRectAndRRect_Geometry: {
            SkRect r = create_rect(offset);
            r.offset(fSign * kXlate, fSign * kXlate);
            canvas->clipRect(r, true); // AA here forces shader clips
            canvas->clipRRect(create_rrect(offset), useAA);
            } break;
        case kRectAndConvex_Geometry: {
            SkRect r = create_rect(offset);
            r.offset(fSign * kXlate, fSign * kXlate);
            canvas->clipRect(r, true); // AA here forces shader clips
            canvas->clipPath(create_convex_path(offset), useAA);
            } break;
        case kRectAndConcave_Geometry: {
            SkRect r = create_rect(offset);
            r.offset(fSign * kXlate, fSign * kXlate);
            canvas->clipRect(r, true); // AA here forces shader clips
            canvas->clipPath(create_concave_path(offset), useAA);
            } break;
        }

        SkISize size = canvas->getBaseLayerSize();
        SkRect bigR = SkRect::MakeWH(SkIntToScalar(size.width()), SkIntToScalar(size.height()));

        SkPaint p;
        p.setColor(SK_ColorRED);

        canvas->drawRect(bigR, p);
        canvas->restoreToCount(count);
    }

    // Draw a big red rect through some clip geometry and also draw that same
    // geometry in black. The order in which they are drawn can be swapped.
    // This tests whether the clip and normally drawn geometry match up.
    void drawGeometry(SkCanvas* canvas, const SkPoint& offset, bool useAA) {
        if (fClipFirst) {
            this->drawClippedGeom(canvas, offset, useAA);
        }

        draw_normal_geom(canvas, offset, fGeom, useAA);

        if (!fClipFirst) {
            this->drawClippedGeom(canvas, offset, useAA);
        }
    }

    void onDrawContent(SkCanvas* canvas) override {
        SkScalar trans[2];
        fTrans.timeToValues(GetMSecs(), trans);

        SkPoint offset;
        offset.set(trans[0], trans[1]);

        int saveCount = canvas->save();
        this->drawGeometry(canvas, offset, false);
        canvas->restoreToCount(saveCount);
    }

    SkMSec GetMSecs() const {
        return static_cast<SkMSec>(SkTime::GetMSecs() - fStart);
    }

private:
    SkInterpolator  fTrans;
    Geometry        fGeom;
    bool            fClipFirst;
    int             fSign;
    const double    fStart = SkTime::GetMSecs();

    typedef Sample INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_SAMPLE( return new ClipDrawMatchView(); )
