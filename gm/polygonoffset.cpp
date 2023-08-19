/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTDArray.h"
#include "src/utils/SkPolyUtils.h"
#include "tools/ToolUtils.h"

#include <functional>
#include <memory>

#if !defined(SK_ENABLE_OPTIMIZE_SIZE)
static void create_ngon(int n, SkPoint* pts, SkScalar w, SkScalar h, SkPathDirection dir) {
    float angleStep = 360.0f / n, angle = 0.0f;
    if ((n % 2) == 1) {
        angle = angleStep/2.0f;
    }
    if (SkPathDirection::kCCW == dir) {
        angle = -angle;
        angleStep = -angleStep;
    }

    for (int i = 0; i < n; ++i) {
        pts[i].fX = -SkScalarSin(SkDegreesToRadians(angle)) * w;
        pts[i].fY =  SkScalarCos(SkDegreesToRadians(angle)) * h;
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
// Quad with near coincident point
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
    { 10.0f, -20.0f },
    { 10.0f,   0.0f },
    { 10.0f,  35.0f },
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

// tab
const SkPoint gPoints11[] = {
    { -45, -25 },
    { 45, -25 },
    { 45, 25 },
    { 20, 25 },
    { 19.6157f, 25.f + 3.9018f },
    { 18.4776f, 25.f + 7.6537f },
    { 16.6294f, 25.f + 11.1114f },
    { 14.1421f, 25.f + 14.1421f },
    { 11.1114f, 25.f + 16.6294f },
    { 7.6537f, 25.f + 18.4776f },
    { 3.9018f, 25.f + 19.6157f },
    { 0, 45.f },
    { -3.9018f, 25.f + 19.6157f },
    { -7.6537f, 25.f + 18.4776f },
    { -11.1114f, 25.f + 16.6294f },
    { -14.1421f, 25.f + 14.1421f },
    { -16.6294f, 25.f + 11.1114f },
    { -18.4776f, 25.f + 7.6537f },
    { -19.6157f, 25.f + 3.9018f },
    { -20, 25 },
    { -45, 25 }
};

// star of david
const SkPoint gPoints12[] = {
    { 0.0f, -50.0f },
    { 14.43f, -25.0f },
    { 43.30f, -25.0f },
    { 28.86f, 0.0f },
    { 43.30f, 25.0f },
    { 14.43f, 25.0f },
    { 0.0f, 50.0f },
    { -14.43f, 25.0f },
    { -43.30f, 25.0f },
    { -28.86f, 0.0f },
    { -43.30f, -25.0f },
    { -14.43f, -25.0f },
};

// notch
const SkScalar kBottom = 25.f;
const SkPoint gPoints13[] = {
    { -50, kBottom - 50.f },
    { 50, kBottom - 50.f },
    { 50, kBottom },
    { 20, kBottom },
    { 19.6157f, kBottom - 3.9018f },
    { 18.4776f, kBottom - 7.6537f },
    { 16.6294f, kBottom - 11.1114f },
    { 14.1421f, kBottom - 14.1421f },
    { 11.1114f, kBottom - 16.6294f },
    { 7.6537f, kBottom - 18.4776f },
    { 3.9018f, kBottom - 19.6157f },
    { 0, kBottom - 20.f },
    { -3.9018f, kBottom - 19.6157f },
    { -7.6537f, kBottom - 18.4776f },
    { -11.1114f, kBottom - 16.6294f },
    { -14.1421f, kBottom - 14.1421f },
    { -16.6294f, kBottom - 11.1114f },
    { -18.4776f, kBottom - 7.6537f },
    { -19.6157f, kBottom - 3.9018f },
    { -20, kBottom },
    { -50, kBottom }
};

// crown
const SkPoint gPoints14[] = {
    { -40, -39 },
    { 40, -39 },
    { 40, -20 },
    { 30, 40 },
    { 20, -20 },
    { 10, 40 },
    { 0, -20 },
    { -10, 40 },
    { -20, -20 },
    { -30, 40 },
    { -40, -20 }
};

// dumbbell
const SkPoint gPoints15[] = {
    { -26, -3 },
    { -24, -6.2f },
    { -22.5f, -8 },
    { -20, -9.9f },
    { -17.5f, -10.3f },
    { -15, -10.9f },
    { -12.5f, -10.2f },
    { -10, -9.7f },
    { -7.5f, -8.1f },
    { -5, -7.7f },
    { -2.5f, -7.4f },
    { 0, -7.7f },
    { 3, -9 },
    { 6.5f, -11.5f },
    { 10.6f, -14 },
    { 14, -15.2f },
    { 17, -15.5f },
    { 20, -15.2f },
    { 23.4f, -14 },
    { 27.5f, -11.5f },
    { 30, -8 },
    { 32, -4 },
    { 32.5f, 0 },
    { 32, 4 },
    { 30, 8 },
    { 27.5f, 11.5f },
    { 23.4f, 14 },
    { 20, 15.2f },
    { 17, 15.5f },
    { 14, 15.2f },
    { 10.6f, 14 },
    { 6.5f, 11.5f },
    { 3, 9 },
    { 0, 7.7f },
    { -2.5f, 7.4f },
    { -5, 7.7f },
    { -7.5f, 8.1f },
    { -10, 9.7f },
    { -12.5f, 10.2f },
    { -15, 10.9f },
    { -17.5f, 10.3f },
    { -20, 9.9f },
    { -22.5f, 8 },
    { -24, 6.2f },
    { -26, 3 },
    { -26.5f, 0 }
};

// truncated dumbbell
// (checks winding computation in OffsetSimplePolygon)
const SkPoint gPoints16[] = {
    { -15 + 3, -9 },
    { -15 + 6.5f, -11.5f },
    { -15 + 10.6f, -14 },
    { -15 + 14, -15.2f },
    { -15 + 17, -15.5f },
    { -15 + 20, -15.2f },
    { -15 + 23.4f, -14 },
    { -15 + 27.5f, -11.5f },
    { -15 + 30, -8 },
    { -15 + 32, -4 },
    { -15 + 32.5f, 0 },
    { -15 + 32, 4 },
    { -15 + 30, 8 },
    { -15 + 27.5f, 11.5f },
    { -15 + 23.4f, 14 },
    { -15 + 20, 15.2f },
    { -15 + 17, 15.5f },
    { -15 + 14, 15.2f },
    { -15 + 10.6f, 14 },
    { -15 + 6.5f, 11.5f },
    { -15 + 3, 9 },
};

// square notch
// (to detect segment-segment intersection)
const SkPoint gPoints17[] = {
    { -50, kBottom - 50.f },
    { 50, kBottom - 50.f },
    { 50, kBottom },
    { 20, kBottom },
    { 20, kBottom - 20.f },
    { -20, kBottom - 20.f },
    { -20, kBottom },
    { -50, kBottom }
};

// box with Peano curve
const SkPoint gPoints18[] = {
    { 0, 0 },
    { 0, -12 },
    { -6, -12 },
    { -6, 0 },
    { -12, 0 },
    { -12, -12},
    { -18, -12},
    { -18, 18},
    { -12, 18},
    {-12, 6},
    {-6, 6},
    {-6, 36},
    {-12, 36},
    {-12, 24},
    {-18, 24},
    {-18, 36},
    {-24, 36},
    {-24, 24},
    {-30, 24},
    {-30, 36},
    {-36, 36},
    {-36, 6},
    {-30, 6},
    {-30, 18},
    {-24, 18},
    {-24, -12},
    {-30, -12},
    {-30, 0},
    {-36, 0},
    {-36, -36},
    {36, -36},
    {36, 36},
    {12, 36},
    {12, 24},
    {6, 24},
    {6, 36},
    {0, 36},
    {0, 6},
    {6, 6},
    {6, 18},
    {12, 18},
    {12, -12},
    {6, -12},
    {6, 0}
};


const SkPoint* gConvexPoints[] = {
    gPoints0, gPoints1, gPoints2, gPoints3, gPoints4, gPoints5, gPoints6,
    gPoints7, gPoints8, gPoints9, gPoints10,
};

const size_t gConvexSizes[] = {
    std::size(gPoints0),
    std::size(gPoints1),
    std::size(gPoints2),
    std::size(gPoints3),
    std::size(gPoints4),
    std::size(gPoints5),
    std::size(gPoints6),
    std::size(gPoints7),
    std::size(gPoints8),
    std::size(gPoints9),
    std::size(gPoints10),
};
static_assert(std::size(gConvexSizes) == std::size(gConvexPoints), "array_mismatch");

const SkPoint* gSimplePoints[] = {
    gPoints0, gPoints1, gPoints2, gPoints4, gPoints5, gPoints7,
    gPoints8, gPoints11, gPoints12, gPoints13, gPoints14, gPoints15,
    gPoints16, gPoints17, gPoints18,
};

const size_t gSimpleSizes[] = {
    std::size(gPoints0),
    std::size(gPoints1),
    std::size(gPoints2),
    std::size(gPoints4),
    std::size(gPoints5),
    std::size(gPoints7),
    std::size(gPoints8),
    std::size(gPoints11),
    std::size(gPoints12),
    std::size(gPoints13),
    std::size(gPoints14),
    std::size(gPoints15),
    std::size(gPoints16),
    std::size(gPoints17),
    std::size(gPoints18),
};
static_assert(std::size(gSimpleSizes) == std::size(gSimplePoints), "array_mismatch");

}  // namespace PolygonOffsetData

namespace skiagm {

// This GM is intended to exercise the offsetting of polygons
// When fVariableOffset is true it will skew the offset by x,
// to test perspective and other variable offset functions
class PolygonOffsetGM : public GM {
public:
    PolygonOffsetGM(bool convexOnly)
        : fConvexOnly(convexOnly) {
        this->setBGColor(0xFFFFFFFF);
    }

protected:
    SkString getName() const override {
        if (fConvexOnly) {
            return SkString("convex-polygon-inset");
        } else {
            return SkString("simple-polygon-offset");
        }
    }
    SkISize getISize() override { return SkISize::Make(kGMWidth, kGMHeight); }
    bool runAsBench() const override { return true; }

    static void GetConvexPolygon(int index, SkPathDirection dir,
                                 std::unique_ptr<SkPoint[]>* data, int* numPts) {
        if (index < (int)std::size(PolygonOffsetData::gConvexPoints)) {
            // manually specified
            *numPts = (int)PolygonOffsetData::gConvexSizes[index];
            *data = std::make_unique<SkPoint[]>(*numPts);
            if (SkPathDirection::kCW == dir) {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gConvexPoints[index][i];
                }
            } else {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gConvexPoints[index][*numPts - i - 1];
                }
            }
        } else {
            // procedurally generated
            SkScalar width = kMaxPathHeight / 2;
            SkScalar height = kMaxPathHeight / 2;
            int numPtsArray[] = { 3, 4, 5, 5, 6, 8, 8, 20, 100 };

            size_t arrayIndex = index - std::size(PolygonOffsetData::gConvexPoints);
            SkASSERT(arrayIndex < std::size(numPtsArray));
            *numPts = numPtsArray[arrayIndex];
            if (arrayIndex == 3 || arrayIndex == 6) {
                // squashed pentagon and octagon
                width = kMaxPathHeight / 5;
            }

            *data = std::make_unique<SkPoint[]>(*numPts);

            create_ngon(*numPts, data->get(), width, height, dir);
        }
    }

    static void GetSimplePolygon(int index, SkPathDirection dir,
                                 std::unique_ptr<SkPoint[]>* data, int* numPts) {
        if (index < (int)std::size(PolygonOffsetData::gSimplePoints)) {
            // manually specified
            *numPts = (int)PolygonOffsetData::gSimpleSizes[index];
            *data = std::make_unique<SkPoint[]>(*numPts);
            if (SkPathDirection::kCW == dir) {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gSimplePoints[index][i];
                }
            } else {
                for (int i = 0; i < *numPts; ++i) {
                    (*data)[i] = PolygonOffsetData::gSimplePoints[index][*numPts - i - 1];
                }
            }
        } else {
            // procedurally generated
            SkScalar width = kMaxPathHeight / 2;
            SkScalar height = kMaxPathHeight / 2;
            int numPtsArray[] = { 5, 7, 8, 20, 100 };

            size_t arrayIndex = index - std::size(PolygonOffsetData::gSimplePoints);
            arrayIndex = std::min(arrayIndex, std::size(numPtsArray) - 1);
            SkASSERT(arrayIndex < std::size(numPtsArray));
            *numPts = numPtsArray[arrayIndex];
            // squash horizontally
            width = kMaxPathHeight / 5;

            *data = std::make_unique<SkPoint[]>(*numPts);

            create_ngon(*numPts, data->get(), width, height, dir);
        }
    }
    // Draw a single polygon with insets and potentially outsets
    void drawPolygon(SkCanvas* canvas, int index, SkPoint* position) {

        SkPoint center;
        {
            std::unique_ptr<SkPoint[]> data(nullptr);
            int numPts;
            if (fConvexOnly) {
                GetConvexPolygon(index, SkPathDirection::kCW, &data, &numPts);
            } else {
                GetSimplePolygon(index, SkPathDirection::kCW, &data, &numPts);
            }
            SkRect bounds;
            bounds.setBounds(data.get(), numPts);
            if (!fConvexOnly) {
                bounds.outset(kMaxOutset, kMaxOutset);
            }
            if (position->fX + bounds.width() > kGMWidth) {
                position->fX = 0;
                position->fY += kMaxPathHeight;
            }
            center = { position->fX + SkScalarHalf(bounds.width()), position->fY };
            position->fX += bounds.width();
        }

        const SkPathDirection dirs[2] = { SkPathDirection::kCW, SkPathDirection::kCCW };
        const float insets[] = { 5, 10, 15, 20, 25, 30, 35, 40 };
        const float offsets[] = { 2, 5, 9, 14, 20, 27, 35, 44, -2, -5, -9 };
        const SkColor colors[] = { 0xFF901313, 0xFF8D6214, 0xFF698B14, 0xFF1C8914,
                                   0xFF148755, 0xFF146C84, 0xFF142482, 0xFF4A1480,
                                   0xFF901313, 0xFF8D6214, 0xFF698B14 };

        SkPaint paint;
        paint.setAntiAlias(true);
        paint.setStyle(SkPaint::kStroke_Style);
        paint.setStrokeWidth(1);

        std::unique_ptr<SkPoint[]> data(nullptr);
        int numPts;
        if (fConvexOnly) {
            GetConvexPolygon(index, dirs[index % 2], &data, &numPts);
        } else {
            GetSimplePolygon(index, dirs[index % 2], &data, &numPts);
        }

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

        SkTDArray<SkPoint> offsetPoly;
        size_t count = fConvexOnly ? std::size(insets) : std::size(offsets);
        for (size_t i = 0; i < count; ++i) {
            SkScalar offset = fConvexOnly ? insets[i] : offsets[i];
            std::function<SkScalar(const SkPoint&)> offsetFunc;

            bool result;
            if (fConvexOnly) {
                result = SkInsetConvexPolygon(data.get(), numPts, offset, &offsetPoly);
            } else {
                SkRect bounds;
                bounds.setBoundsCheck(data.get(), numPts);
                result = SkOffsetSimplePolygon(data.get(), numPts, bounds, offset, &offsetPoly);
            }
            if (result) {
                SkPath path;
                path.moveTo(offsetPoly[0]);
                for (int j = 1; j < offsetPoly.size(); ++j) {
                    path.lineTo(offsetPoly[j]);
                }
                path.close();

                paint.setColor(ToolUtils::color_to_565(colors[i]));
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
        if (!fConvexOnly) {
            offset.fY += kMaxOutset;
        }

        for (int i = 0; i < kNumPaths; ++i) {
            this->drawPolygon(canvas, i, &offset);
        }
    }

private:
    inline static constexpr int kNumPaths = 20;
    inline static constexpr int kMaxPathHeight = 100;
    inline static constexpr int kMaxOutset = 16;
    inline static constexpr int kGMWidth = 512;
    inline static constexpr int kGMHeight = 512;

    bool fConvexOnly;

    using INHERITED = GM;
};

//////////////////////////////////////////////////////////////////////////////

DEF_GM(return new PolygonOffsetGM(true);)
DEF_GM(return new PolygonOffsetGM(false);)
}  // namespace skiagm

#endif // !defined(SK_ENABLE_OPTIMIZE_SIZE)
