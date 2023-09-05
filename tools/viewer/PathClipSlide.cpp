/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypeface.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkTime.h"
#include "src/base/SkUTF.h"
#include "tools/viewer/ClickHandlerSlide.h"

#include <utility>

class PathClipSlide : public ClickHandlerSlide {
public:
    SkRect fOval;
    SkPoint fCenter;

    PathClipSlide() : fOval(SkRect::MakeWH(200, 50)), fCenter(SkPoint::Make(250, 250)) {
        fName = "PathClip";
    }

    void draw(SkCanvas* canvas) override {
        const SkRect oval = fOval.makeOffset(fCenter.fX - fOval.centerX(),
                                             fCenter.fY - fOval.centerY());

        SkPaint p;
        p.setAntiAlias(true);

        p.setStyle(SkPaint::kStroke_Style);
        canvas->drawOval(oval, p);

        const SkRect r = SkRect::MakeLTRB(200, 200, 300, 300);
        canvas->clipRect(r);

        p.setStyle(SkPaint::kFill_Style);
        p.setColor(SK_ColorRED);
        canvas->drawRect(r, p);

        p.setColor(0x800000FF);
        canvas->drawOval(oval, p);
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        return new Click([&](Click* c) {
            fCenter = c->fCurr;
            return false;
        });
    }

    bool onClick(ClickHandlerSlide::Click *) override { return false; }
};
DEF_SLIDE( return new PathClipSlide; )

//////////////////////////////////////////////////////////////////////////////

static int clip_line(const SkRect& bounds, SkPoint p0, SkPoint p1, SkPoint edges[]) {
    SkPoint* edgesStart = edges;

    if (p0.fY == p1.fY) {
        return 0;
    }

    if (p0.fY > p1.fY) {
        using std::swap;
        swap(p0, p1);
    }
    // now we're monotonic in Y: p0 <= p1
    if (p1.fY <= bounds.top() || p0.fY >= bounds.bottom()) {
        return 0;
    }

    double dxdy = (double)(p1.fX - p0.fX) / (p1.fY - p0.fY);
    if (p0.fY < bounds.top()) {
        p0.fX = SkDoubleToScalar(p0.fX + dxdy * (bounds.top() - p0.fY));
        p0.fY = bounds.top();
    }
    if (p1.fY > bounds.bottom()) {
        p1.fX = SkDoubleToScalar(p1.fX + dxdy * (bounds.bottom() - p1.fY));
        p1.fY = bounds.bottom();
    }

    // Now p0...p1 is strictly inside bounds vertically, so we just need to clip horizontally

    if (p0.fX > p1.fX) {
        using std::swap;
        swap(p0, p1);
    }
    // now we're left-to-right: p0 .. p1

    if (p1.fX <= bounds.left()) {   // entirely to the left
        p0.fX = p1.fX = bounds.left();
        *edges++ = p0;
        *edges++ = p1;
        return 2;
    }
    if (p0.fX >= bounds.right()) {  // entirely to the right
        p0.fX = p1.fX = bounds.right();
        *edges++ = p0;
        *edges++ = p1;
        return 2;
    }

    if (p0.fX < bounds.left()) {
        float y = SkDoubleToScalar(p0.fY + (bounds.left() - p0.fX) / dxdy);
        *edges++ = SkPoint::Make(bounds.left(), p0.fY);
        *edges++ = SkPoint::Make(bounds.left(), y);
        p0.set(bounds.left(), y);
    }
    if (p1.fX > bounds.right()) {
        float y = SkDoubleToScalar(p0.fY + (bounds.right() - p0.fX) / dxdy);
        *edges++ = p0;
        *edges++ = SkPoint::Make(bounds.right(), y);
        *edges++ = SkPoint::Make(bounds.right(), p1.fY);
    } else {
        *edges++ = p0;
        *edges++ = p1;
    }
    return SkToInt(edges - edgesStart);
}

static void draw_clipped_line(SkCanvas* canvas, const SkRect& bounds,
                              SkPoint p0, SkPoint p1, const SkPaint& paint) {
    SkPoint verts[6];
    int count = clip_line(bounds, p0, p1, verts);

    SkPath path;
    path.addPoly(verts, count, false);
    canvas->drawPath(path, paint);
}

// Demonstrate edge-clipping that is used in the scan converter
//
class EdgeClipSlide : public ClickHandlerSlide {
    enum {
        N = 3
    };
    SkPoint fPoly[N];
    SkRect  fClip;
    SkColor fEdgeColor[N];

public:
    EdgeClipSlide() : fClip(SkRect::MakeLTRB(150, 150, 550, 450)) {
        fPoly[0].set(300, 40);
        fPoly[1].set(550, 250);
        fPoly[2].set(40, 450);

        fEdgeColor[0] = 0xFFFF0000;
        fEdgeColor[1] = 0xFF00FF00;
        fEdgeColor[2] = 0xFF0000FF;
        fName = "EdgeClip";
    }

    static SkScalar snap(SkScalar x) {
        return SkScalarRoundToScalar(x * 0.5f) * 2;
    }
    static SkPoint snap(const SkPoint& pt) {
        return SkPoint::Make(snap(pt.x()), snap(pt.y()));
    }
    static void snap(SkPoint dst[], const SkPoint src[], int count) {
        for (int i = 0; i < count; ++i) {
            dst[i] = snap(src[i]);
        }
    }

    void draw(SkCanvas* canvas) override {
        SkPath path;
        path.addPoly(fPoly, N, true);

        // Draw the full triangle, stroked and filled
        SkPaint p;
        p.setAntiAlias(true);
        p.setColor(0xFFE0E0E0);
        canvas->drawPath(path, p);
        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(2);
        for (int i = 0; i < N; ++i) {
            const int j = (i + 1) % N;
            p.setColor(fEdgeColor[i]);
            p.setAlpha(0x88);
            canvas->drawLine(fPoly[i], fPoly[j], p);
        }
        p.setStyle(SkPaint::kFill_Style);

        // Draw the clip itself
        p.setColor(0xFF8888CC);
        canvas->drawRect(fClip, p);

        // Draw the filled triangle through the clip
        p.setColor(0xFF88CC88);
        canvas->save();
        canvas->clipRect(fClip);
        canvas->drawPath(path, p);
        canvas->restore();

        p.setStyle(SkPaint::kStroke_Style);
        p.setStrokeWidth(6);

        // Draw each of the "Edges" that survived the clipping
        // We use a layer, so we can PLUS the different edge-colors, showing where two edges
        // canceled each other out.
        canvas->saveLayer(nullptr, nullptr);
        p.setBlendMode(SkBlendMode::kPlus);
        for (int i = 0; i < N; ++i) {
            const int j = (i + 1) % N;
            p.setColor(fEdgeColor[i]);
            draw_clipped_line(canvas, fClip, fPoly[i], fPoly[j], p);
        }
        canvas->restore();
    }

    class MyClick : public Click {
    public:
        MyClick() {}
        virtual void handleMove() = 0;
    };

    class VertClick : public MyClick {
        SkPoint* fPt;
    public:
        VertClick(SkPoint* pt) : fPt(pt) {}
        void handleMove() override { *fPt = snap(fCurr); }
    };

    class DragRectClick : public MyClick {
        SkRect* fRect;
    public:
        DragRectClick(SkRect* rect) : fRect(rect) {}
        void handleMove() override { fRect->offset(fCurr.x() - fPrev.x(), fCurr.y() - fPrev.y()); }
    };

    class DragPolyClick : public MyClick {
        SkPoint fSrc[100];
        SkPoint* fPoly;
        int fCount;
    public:
        DragPolyClick(SkPoint poly[], int count) : fPoly(poly), fCount(count)
        {
            SkASSERT((size_t)count <= std::size(fSrc));
            memcpy(fSrc, poly, count * sizeof(SkPoint));
        }
        void handleMove() override {
            const SkScalar dx = fCurr.x() - fOrig.x();
            const SkScalar dy = fCurr.y() - fOrig.y();
            for (int i = 0; i < fCount; ++i) {
                fPoly[i].set(snap(fSrc[i].x() + dx), snap(fSrc[i].y() + dy));
            }
        }
    };

    class DoNothingClick : public MyClick {
    public:
        DoNothingClick() {}
        void handleMove() override {}
    };

    static bool hit_test(const SkPoint& pt, SkScalar x, SkScalar y) {
        const SkScalar rad = 8;
        const SkScalar dx = pt.x() - x;
        const SkScalar dy = pt.y() - y;
        return dx*dx + dy*dy <= rad*rad;
    }

protected:
    Click* onFindClickHandler(SkScalar x, SkScalar y, skui::ModifierKey) override {
        for (int i = 0; i < N; ++i) {
            if (hit_test(fPoly[i], x, y)) {
                return new VertClick(&fPoly[i]);
            }
        }

        SkPath path;
        path.addPoly(fPoly, N, true);
        if (path.contains(x, y)) {
            return new DragPolyClick(fPoly, N);
        }

        if (fClip.intersects(SkRect::MakeLTRB(x - 1, y - 1, x + 1, y + 1))) {
            return new DragRectClick(&fClip);
        }
        return new DoNothingClick();
    }

    bool onClick(Click* click) override {
        ((MyClick*)click)->handleMove();
        return true;
    }
};

DEF_SLIDE( return new EdgeClipSlide; )
