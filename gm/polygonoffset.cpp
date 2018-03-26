/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkOffsetPolygon.h"
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

namespace PolygonOffsetData {
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
    { 5.0f, -25.0f },
    { 6.0f,   0.0f },
    { 5.0f,  25.0f },
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
}

namespace skiagm {

// This GM is intended to exercise the offsetting of polygons
class PolygonOffsetGM : public GM {
public:
    PolygonOffsetGM(bool convexOnly) : fConvexOnly(convexOnly) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString onShortName() override {
        if (fConvexOnly) {
            return SkString("convex-polygon-inset");
        } else {
            return SkString("simple-polygon-offset");
        }
    }
    SkISize onISize() override { return SkISize::Make(kGMWidth, kGMHeight); }
    bool runAsBench() const override { return true; }

    static void GetPath(int index, SkPath::Direction dir,
                        std::unique_ptr<SkPoint[]>* data, int* numPts) {
        if (index < (int)SK_ARRAY_COUNT(PolygonOffsetData::gPoints)) {
            // manually specified
            *numPts = (int)PolygonOffsetData::gSizes[index];
            data->reset(new SkPoint[*numPts]);
            if (SkPath::kCW_Direction == dir) {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gPoints[index][i];
                }
            } else {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gPoints[index][*numPts - i - 1];
                }
            }
        } else {
            // procedurally generated
            SkScalar width = kMaxPathHeight / 2;
            SkScalar height = kMaxPathHeight / 2;
            switch (index - SK_ARRAY_COUNT(PolygonOffsetData::gPoints)) {
                case 0:
                    *numPts = 3;
                    break;
                case 1:
                    *numPts = 4;
                    break;
                case 2:
                    *numPts = 5;
                    break;
                case 3:             // squashed pentagon
                    *numPts = 5;
                    width = kMaxPathHeight / 5;
                    break;
                case 4:
                    *numPts = 6;
                    break;
                case 5:
                    *numPts = 8;
                    break;
                case 6:              // squashed octogon
                    *numPts = 8;
                    width = kMaxPathHeight / 5;
                    break;
                case 7:
                    *numPts = 20;
                    break;
                case 8:
                    *numPts = 100;
                    break;
                default:
                    *numPts = 3;
                    break;
            }

            data->reset(new SkPoint[*numPts]);

            create_ngon(*numPts, data->get(), width, height);
            if (SkPath::kCCW_Direction == dir) {
                // reverse it
                for (int i = 0; i < *numPts/2; ++i) {
                    SkPoint tmp = (*data)[i];
                    (*data)[i] = (*data)[*numPts - i - 1];
                    (*data)[*numPts - i - 1] = tmp;
                }
            }
        }
    }

    // Draw a single path several times, shrinking it, flipping its direction
    // and changing its start vertex each time.
    void drawPath(SkCanvas* canvas, int index, SkPoint* offset) {

        SkPoint center;
        {
            std::unique_ptr<SkPoint[]> data(nullptr);
            int numPts;
            GetPath(index, SkPath::kCW_Direction, &data, &numPts);
            SkRect bounds;
            bounds.set(data.get(), numPts);
            if (offset->fX + bounds.width() > kGMWidth) {
                offset->fX = 0;
                offset->fY += kMaxPathHeight;
            }
            center = { offset->fX + SkScalarHalf(bounds.width()), offset->fY };
            offset->fX += bounds.width();
        }

        const SkPath::Direction dirs[2] = { SkPath::kCW_Direction, SkPath::kCCW_Direction };
        const float insets[] = { 5, 10, 15, 20, 25, 30, 35, 40 };
        const SkColor colors[] = { 0xFF901313, 0xFF8D6214, 0xFF698B14, 0xFF1C8914,
                                   0xFF148755, 0xFF146C84, 0xFF142482, 0xFF4A1480 };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1);

        std::unique_ptr<SkPoint[]> data(nullptr);
        int numPts;
        GetPath(index, dirs[index % 2], &data, &numPts);
        {
            SkPath path;
            path.moveTo(data.get()[0]);
            for (int i = 1; i < numPts; ++i) {
                path.lineTo(data.get()[i]);
            }
            path.close();
            canvas->save();
            canvas->translate(center.fX, center.fY);
            canvas->drawPath(path, paint);
            canvas->restore();
        }

        SkTDArray<SkPoint> insetPoly;
        for (size_t i = 0; i < SK_ARRAY_COUNT(insets); ++i) {
            bool result;
            if (fConvexOnly) {
                result = SkInsetConvexPolygon(data.get(), numPts, insets[i], &insetPoly);
            } else {
                result = SkOffsetSimplePolygon(data.get(), numPts, insets[i], &insetPoly);
            }
            if (result) {
                SkPath path;
                path.moveTo(insetPoly[0]);
                for (int i = 1; i < insetPoly.count(); ++i) {
                    path.lineTo(insetPoly[i]);
                }
                path.close();

                paint.setColor(colors[i]);
                canvas->save();
                canvas->translate(center.fX, center.fY);
                canvas->drawPath(path, paint);
                canvas->restore();
            }
        }
    }

    void onDraw(SkCanvas* canvas) override {
        // the right edge of the last drawn path
        SkPoint offset = { 0, SkScalarHalf(kMaxPathHeight) };

        for (int i = 0; i < kNumPaths; ++i) {
            this->drawPath(canvas, i, &offset);
        }
    }

private:
    static constexpr int kNumPaths = 20;
    static constexpr int kMaxPathHeight = 100;
    static constexpr int kGMWidth = 512;
    static constexpr int kGMHeight = 512;

    bool fConvexOnly;

    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PolygonOffsetGM(true);)
DEF_GM(return new PolygonOffsetGM(false);)
}
