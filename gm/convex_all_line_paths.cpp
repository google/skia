/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkPathPriv.h"

#include <memory>

static void create_ngon(int n, SkPoint* pts, SkScalar width, SkScalar height) {
    float angleStep = 360.0f / n, angle = 0.0f;
    if ((n % 2) == 1) {
        angle = angleStep/2.0f;
    }

    for (int i = 0; i < n; ++i) {
        pts[i].fX = -SkScalarSin(SkDegreesToRadians(angle)) * width;
        pts[i].fY =  SkScalarCos(SkDegreesToRadians(angle)) * height;
        angle += angleStep;
    }
}

namespace ConvexLineOnlyData {
// narrow rect
const SkPoint gPoints0[] = {
    { -1.5f, -50.0f },
    { 1.5f, -50.0f },
    { 1.5f,  50.0f },
    { -1.5f,  50.0f }
};
// narrow rect on an angle
const SkPoint gPoints1[] = {
    { -50.0f, -49.0f },
    { -49.0f, -50.0f },
    { 50.0f,  49.0f },
    { 49.0f,  50.0f }
};
// trap - narrow on top - wide on bottom
const SkPoint gPoints2[] = {
    { -10.0f, -50.0f },
    { 10.0f, -50.0f },
    { 50.0f,  50.0f },
    { -50.0f,  50.0f }
};
// wide skewed rect
const SkPoint gPoints3[] = {
    { -50.0f, -50.0f },
    { 0.0f, -50.0f },
    { 50.0f,  50.0f },
    { 0.0f,  50.0f }
};
// thin rect with colinear-ish lines
const SkPoint gPoints4[] = {
    { -6.0f, -50.0f },
    { 4.0f, -50.0f },
    { 5.0f, -25.0f },  // remove if collinear diagonal points are not concave
    { 6.0f,   0.0f },
    { 5.0f,  25.0f },  // remove if collinear diagonal points are not concave
    { 4.0f,  50.0f },
    { -4.0f,  50.0f }
};
// degenerate
const SkPoint gPoints5[] = {
    { -0.025f, -0.025f },
    { 0.025f, -0.025f },
    { 0.025f,  0.025f },
    { -0.025f,  0.025f }
};
// Triangle in which the first point should fuse with last
const SkPoint gPoints6[] = {
    { -20.0f, -13.0f },
    { -20.0f, -13.05f },
    { 20.0f, -13.0f },
    { 20.0f,  27.0f }
};
// thin rect with colinear lines
const SkPoint gPoints7[] = {
    { -10.0f, -50.0f },
    { 10.0f, -50.0f },
    { 10.0f, -25.0f },
    { 10.0f,   0.0f },
    { 10.0f,  25.0f },
    { 10.0f,  50.0f },
    { -10.0f,  50.0f }
};
// capped teardrop
const SkPoint gPoints8[] = {
    { 50.00f,  50.00f },
    { 0.00f,  50.00f },
    { -15.45f,  47.55f },
    { -29.39f,  40.45f },
    { -40.45f,  29.39f },
    { -47.55f,  15.45f },
    { -50.00f,   0.00f },
    { -47.55f, -15.45f },
    { -40.45f, -29.39f },
    { -29.39f, -40.45f },
    { -15.45f, -47.55f },
    { 0.00f, -50.00f },
    { 50.00f, -50.00f }
};
// teardrop
const SkPoint gPoints9[] = {
    { 4.39f,  40.45f },
    { -9.55f,  47.55f },
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
    { -9.55f, -47.55f },
    { 4.39f, -40.45f },
    { 75.00f,   0.00f }
};
// clipped triangle
const SkPoint gPoints10[] = {
    { -10.0f, -50.0f },
    { 10.0f, -50.0f },
    { 50.0f,  31.0f },
    { 40.0f,  50.0f },
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
}  // namespace ConvexLineOnlyData

namespace skiagm {

// This GM is intended to exercise Ganesh's handling of convex line-only
// paths
class ConvexLineOnlyPathsGM : public GM {
public:
    ConvexLineOnlyPathsGM(bool doStrokeAndFill) : fDoStrokeAndFill(doStrokeAndFill) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        if (fDoStrokeAndFill) {
            return SkString("convex-lineonly-paths-stroke-and-fill");
        }
        return SkString("convex-lineonly-paths");
    }
    SkISize onISize() override { return SkISize::Make(kGMWidth, kGMHeight); }
    bool runAsBench() const override { return true; }

    static SkPath GetPath(int index, SkPathDirection dir) {
        std::unique_ptr<SkPoint[]> data(nullptr);
        const SkPoint* points;
        int numPts;
        if (index < (int) SK_ARRAY_COUNT(ConvexLineOnlyData::gPoints)) {
            // manually specified
            points = ConvexLineOnlyData::gPoints[index];
            numPts = (int)ConvexLineOnlyData::gSizes[index];
        } else {
            // procedurally generated
            SkScalar width = kMaxPathHeight/2;
            SkScalar height = kMaxPathHeight/2;
            switch (index-SK_ARRAY_COUNT(ConvexLineOnlyData::gPoints)) {
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

            data = std::make_unique<SkPoint[]>(numPts);

            create_ngon(numPts, data.get(), width, height);
            points = data.get();
        }

        SkPathBuilder builder;

        if (SkPathDirection::kCW == dir) {
            builder.moveTo(points[0]);
            for (int i = 1; i < numPts; ++i) {
                builder.lineTo(points[i]);
            }
        } else {
            builder.moveTo(points[numPts-1]);
            for (int i = numPts-2; i >= 0; --i) {
                builder.lineTo(points[i]);
            }
        }

        builder.close();
        SkPath path = builder.detach();
#ifdef SK_DEBUG
        // Each path this method returns should be convex, only composed of
        // lines, wound the right direction, and short enough to fit in one
        // of the GMs rows.
        SkASSERT(path.isConvex());
        SkASSERT(SkPath::kLine_SegmentMask == path.getSegmentMasks());
        SkPathFirstDirection actualDir = SkPathPriv::ComputeFirstDirection(path);
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
            SkPath path = GetPath(index, SkPathDirection::kCW);
            if (offset->fX+path.getBounds().width() > kGMWidth) {
                offset->fX = 0;
                offset->fY += kMaxPathHeight;
                if (fDoStrokeAndFill) {
                    offset->fX += kStrokeWidth / 2.0f;
                    offset->fY += kStrokeWidth / 2.0f;
                }
            }
            center = { offset->fX + SkScalarHalf(path.getBounds().width()), offset->fY};
            offset->fX += path.getBounds().width();
            if (fDoStrokeAndFill) {
                offset->fX += kStrokeWidth;
            }
        }

        const SkColor colors[2] = { SK_ColorBLACK, SK_ColorWHITE };
        const SkPathDirection dirs[2] = { SkPathDirection::kCW, SkPathDirection::kCCW };
        const float scales[] = { 1.0f, 0.75f, 0.5f, 0.25f, 0.1f, 0.01f, 0.001f };
        const SkPaint::Join joins[3] = { SkPaint::kRound_Join,
                                         SkPaint::kBevel_Join,
                                         SkPaint::kMiter_Join };

        SkPaint paint;
        paint.setAntiAlias(true);

        for (size_t i = 0; i < SK_ARRAY_COUNT(scales); ++i) {
            SkPath path = GetPath(index, dirs[i%2]);
            if (fDoStrokeAndFill) {
                paint.setStyle(SkPaint::kStrokeAndFill_Style);
                paint.setStrokeJoin(joins[i%3]);
                paint.setStrokeWidth(SkIntToScalar(kStrokeWidth));
            }

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
        if (fDoStrokeAndFill) {
            offset.fX += kStrokeWidth / 2.0f;
            offset.fY += kStrokeWidth / 2.0f;
        }

        for (int i = 0; i < kNumPaths; ++i) {
            this->drawPath(canvas, i, &offset);
        }

        {
            // Repro for crbug.com/472723 (Missing AA on portions of graphic with GPU rasterization)

            SkPaint p;
            p.setAntiAlias(true);
            if (fDoStrokeAndFill) {
                p.setStyle(SkPaint::kStrokeAndFill_Style);
                p.setStrokeJoin(SkPaint::kMiter_Join);
                p.setStrokeWidth(SkIntToScalar(kStrokeWidth));
            }

            SkPath p1 = SkPath::Polygon({
                {60.8522949f, 364.671021f},
                {59.4380493f, 364.671021f},
                {385.414276f, 690.647217f},
                {386.121399f, 689.940125f},
            }, false);
            canvas->save();
            canvas->translate(356.0f, 50.0f);
            canvas->drawPath(p1, p);
            canvas->restore();

            // Repro for crbug.com/869172 (SVG path incorrectly simplified when using GPU
            // Rasterization). This will only draw anything in the stroke-and-fill version.
            SkPath p2 = SkPath::Polygon({
                {10.f, 0.f},
                {38.f, 0.f},
                {66.f, 0.f},
                {94.f, 0.f},
                {122.f, 0.f},
                {150.f, 0.f},
                {150.f, 0.f},
                {122.f, 0.f},
                {94.f, 0.f},
                {66.f, 0.f},
                {38.f, 0.f},
                {10.f, 0.f},
            }, true);
            canvas->save();
            canvas->translate(0.0f, 500.0f);
            canvas->drawPath(p2, p);
            canvas->restore();

            // Repro for crbug.com/856137. This path previously caused GrAAConvexTessellator to turn
            // inset rings into outsets when adjacent bisector angles converged outside the previous
            // ring due to accumulated error.
            SkPath p3 = SkPath::Polygon({
                {1184.96f, 982.557f},
                {1183.71f, 982.865f},
                {1180.99f, 982.734f},
                {1178.5f,  981.541f},
                {1176.35f, 979.367f},
                {1178.94f, 938.854f},
                {1181.35f, 936.038f},
                {1183.96f, 934.117f},
                {1186.67f, 933.195f},
                {1189.36f, 933.342f},
                {1191.58f, 934.38f},
            }, true, SkPathFillType::kEvenOdd);
            canvas->save();
            SkMatrix m;
            m.setAll(0.0893210843f, 0, 79.1197586f, 0, 0.0893210843f, 300, 0, 0, 1);
            canvas->concat(m);
            canvas->drawPath(p3, p);
            canvas->restore();
        }
    }

private:
    inline static constexpr int kStrokeWidth   = 10;
    inline static constexpr int kNumPaths      = 20;
    inline static constexpr int kMaxPathHeight = 100;
    inline static constexpr int kGMWidth       = 512;
    inline static constexpr int kGMHeight      = 512;

    bool fDoStrokeAndFill;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new ConvexLineOnlyPathsGM(false);)
DEF_GM(return new ConvexLineOnlyPathsGM(true);)
}  // namespace skiagm
