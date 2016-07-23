/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkPathPriv.h"

static void create_ngon(int n, SkPoint* pts, SkScalar width, SkScalar height) {
    float angleStep = 360.0f / n, angle = 0.0f, sin, cos;
    if ((n % 2) == 1) {
        angle = angleStep/2.0f;
    }

    for (int i = 0; i < n; ++i) {
        sin = SkScalarSinCos(SkDegreesToRadians(angle), &cos);
        pts[i].fX = -sin * width;
        pts[i].fY = cos * height;
        angle += angleStep;
    }
}

namespace skiagm {

// This GM is intended to exercise Ganesh's handling of convex line-only
// paths
class ConvexLineOnlyPathsGM : public GM {
public:
    ConvexLineOnlyPathsGM() {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override { return SkString("convex-lineonly-paths"); }
    SkISize onISize() override { return SkISize::Make(kGMWidth, kGMHeight); }
    bool runAsBench() const override { return true; }

    static SkPath GetPath(int index, int offset, SkPath::Direction dir) {
        // narrow rect
        const SkPoint gPoints0[] = {
            { -1.5f, -50.0f },
            {  1.5f, -50.0f },
            {  1.5f,  50.0f },
            { -1.5f,  50.0f }
        };
        // narrow rect on an angle
        const SkPoint gPoints1[] = {
            { -50.0f, -49.0f },
            { -49.0f, -50.0f },
            {  50.0f,  49.0f },
            {  49.0f,  50.0f }
        };
        // trap - narrow on top - wide on bottom
        const SkPoint gPoints2[] = {
            { -10.0f, -50.0f },
            {  10.0f, -50.0f },
            {  50.0f,  50.0f },
            { -50.0f,  50.0f }
        };
        // wide skewed rect
        const SkPoint gPoints3[] = {
            { -50.0f, -50.0f },
            {   0.0f, -50.0f },
            {  50.0f,  50.0f },
            {   0.0f,  50.0f }
        };
        // thin rect with colinear-ish lines
        const SkPoint gPoints4[] = {
            { -6.0f, -50.0f },
            {  4.0f, -50.0f },
            {  5.0f, -25.0f },
            {  6.0f,   0.0f },
            {  5.0f,  25.0f },
            {  4.0f,  50.0f },
            { -4.0f,  50.0f }
        };
        // degenerate
        const SkPoint gPoints5[] = {
            { -0.025f, -0.025f  },
            {  0.025f, -0.025f  },
            {  0.025f,  0.025f },
            { -0.025f,  0.025f }
        };
        // Triangle in which the first point should fuse with last
        const SkPoint gPoints6[] = {
            { -20.0f, -13.0f },
            { -20.0f, -13.05f },
            {  20.0f, -13.0f },
            {  20.0f,  27.0f }
        };
        // thin rect with colinear lines
        const SkPoint gPoints7[] = {
            { -10.0f, -50.0f },
            {  10.0f, -50.0f },
            {  10.0f, -25.0f },
            {  10.0f,   0.0f },
            {  10.0f,  25.0f },
            {  10.0f,  50.0f },
            { -10.0f,  50.0f }
        };
        // capped teardrop
        const SkPoint gPoints8[] = {
            {  50.00f,  50.00f },
            {   0.00f,  50.00f },
            { -15.45f,  47.55f },
            { -29.39f,  40.45f },
            { -40.45f,  29.39f },
            { -47.55f,  15.45f },
            { -50.00f,   0.00f },
            { -47.55f, -15.45f },
            { -40.45f, -29.39f },
            { -29.39f, -40.45f },
            { -15.45f, -47.55f },
            {   0.00f, -50.00f },
            {  50.00f, -50.00f }
        };
        // teardrop
        const SkPoint gPoints9[] = {
            {   4.39f,  40.45f },
            {  -9.55f,  47.55f },
            { -25.00f,  50.00f },
            { -40.45f,  47.55f },
            { -54.39f,  40.45f },
            { -65.45f,  29.39f },
            { -72.55f,  15.45f },
            { -75.00f,   0.00f },
            { -72.55f, -15.45f },
            { -65.45f, -29.39f },
            { -54.39f, -40.45f },
            { -40.45f, -47.55f },
            { -25.0f,  -50.0f },
            {  -9.55f, -47.55f },
            {   4.39f, -40.45f },
            {  75.00f,   0.00f }
        };
        // clipped triangle
        const SkPoint gPoints10[] = {
            { -10.0f, -50.0f },
            {  10.0f, -50.0f },
            {  50.0f,  31.0f },
            {  40.0f,  50.0f },
            { -40.0f,  50.0f },
            { -50.0f,  31.0f },
        };

        const SkPoint* gPoints[] = {
            gPoints0, gPoints1, gPoints2, gPoints3, gPoints4, gPoints5, gPoints6,
            gPoints7, gPoints8, gPoints9, gPoints10,
        };

        const size_t gSizes[] = {
            SK_ARRAY_COUNT(gPoints0),
            SK_ARRAY_COUNT(gPoints1),
            SK_ARRAY_COUNT(gPoints2),
            SK_ARRAY_COUNT(gPoints3),
            SK_ARRAY_COUNT(gPoints4),
            SK_ARRAY_COUNT(gPoints5),
            SK_ARRAY_COUNT(gPoints6),
            SK_ARRAY_COUNT(gPoints7),
            SK_ARRAY_COUNT(gPoints8),
            SK_ARRAY_COUNT(gPoints9),
            SK_ARRAY_COUNT(gPoints10),
        };
        static_assert(SK_ARRAY_COUNT(gSizes) == SK_ARRAY_COUNT(gPoints), "array_mismatch");

        SkAutoTDeleteArray<SkPoint> data(nullptr);
        const SkPoint* points;
        int numPts;
        if (index < (int) SK_ARRAY_COUNT(gPoints)) {
            // manually specified
            points = gPoints[index];
            numPts = (int) gSizes[index];
        } else {
            // procedurally generated
            SkScalar width = kMaxPathHeight/2;
            SkScalar height = kMaxPathHeight/2;
            switch (index-SK_ARRAY_COUNT(gPoints)) {
            case 0:
                numPts = 3;
                break;
            case 1:
                numPts = 4;
                break;
            case 2:
                numPts = 5;
                break;
            case 3:             // squashed pentagon
                numPts = 5;
                width = kMaxPathHeight/5;
                break;
            case 4:
                numPts = 6;
                break;
            case 5:
                numPts = 8;
                break;
            case 6:              // squashed octogon
                numPts = 8;
                width = kMaxPathHeight/5;
                break;
            case 7:
                numPts = 20;
                break;
            case 8:
                numPts = 100;
                break;
            default:
                numPts = 3;
                break;
            }

            data.reset(new SkPoint[numPts]);

            create_ngon(numPts, data.get(), width, height);
            points = data.get();
        }

        SkPath path;

        if (SkPath::kCW_Direction == dir) {
            path.moveTo(points[0]);
            for (int i = 1; i < numPts; ++i) {
                path.lineTo(points[i]);
            }
        } else {
            path.moveTo(points[numPts-1]);
            for (int i = numPts-2; i >= 0; --i) {
                path.lineTo(points[i]);
            }
        }

        path.close();
#ifdef SK_DEBUG
        // Each path this method returns should be convex, only composed of
        // lines, wound the right direction, and short enough to fit in one
        // of the GMs rows.
        SkASSERT(path.isConvex());
        SkASSERT(SkPath::kLine_SegmentMask == path.getSegmentMasks());
        SkPathPriv::FirstDirection actualDir;
        SkASSERT(SkPathPriv::CheapComputeFirstDirection(path, &actualDir));
        SkASSERT(SkPathPriv::AsFirstDirection(dir) == actualDir);
        SkRect bounds = path.getBounds();
        SkASSERT(SkScalarNearlyEqual(bounds.centerX(), 0.0f));
        SkASSERT(bounds.height() <= kMaxPathHeight);
#endif
        return path;
    }

    // Draw a single path several times, shrinking it, flipping its direction
    // and changing its start vertex each time.
    void drawPath(SkCanvas* canvas, int index, SkPoint* offset) {

        SkPoint center;
        {
            SkPath path = GetPath(index, 0, SkPath::kCW_Direction);
            if (offset->fX+path.getBounds().width() > kGMWidth) {
                offset->fX = 0;
                offset->fY += kMaxPathHeight;
            }
            center = { offset->fX + SkScalarHalf(path.getBounds().width()), offset->fY};
            offset->fX += path.getBounds().width();
        }

        const SkColor colors[2] = { SK_ColorBLACK, SK_ColorWHITE };
        const SkPath::Direction dirs[2] = { SkPath::kCW_Direction, SkPath::kCCW_Direction };
        const float scales[] = { 1.0f, 0.75f, 0.5f, 0.25f, 0.1f, 0.01f, 0.001f };

        SkPaint paint;
        paint.setAntiAlias(true);

        for (size_t i = 0; i < SK_ARRAY_COUNT(scales); ++i) {
            SkPath path = GetPath(index, (int) i, dirs[i%2]);

            canvas->save();
                canvas->translate(center.fX, center.fY);
                canvas->scale(scales[i], scales[i]);
                paint.setColor(colors[i%2]);
                canvas->drawPath(path, paint);
            canvas->restore();
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // the right edge of the last drawn path
        SkPoint offset = { 0, SkScalarHalf(kMaxPathHeight) };

        for (int i = 0; i < kNumPaths; ++i) {
            this->drawPath(canvas, i, &offset);
        }

        // Repro for crbug.com/472723 (Missing AA on portions of graphic with GPU rasterization)
        {
            canvas->translate(356.0f, 50.0f);

            SkPaint p;
            p.setAntiAlias(true);

            SkPath p1;
            p1.moveTo(60.8522949f, 364.671021f);
            p1.lineTo(59.4380493f, 364.671021f);
            p1.lineTo(385.414276f, 690.647217f);
            p1.lineTo(386.121399f, 689.940125f);
            canvas->drawPath(p1, p);
        }
    }

private:
    static const int kNumPaths      = 20;
    static const int kMaxPathHeight = 100;
    static const int kGMWidth       = 512;
    static const int kGMHeight      = 512;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ConvexLineOnlyPathsGM;)
}
