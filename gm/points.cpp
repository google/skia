/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "SkRandom.h"
#include "SkTextUtils.h"
#include "sk_tool_utils.h"
#include "SkPath.h"
#include "SkPathMeasure.h"
#include "SkVertices.h"

namespace skiagm {

class PointsGM : public GM {
public:
    PointsGM() {}

protected:

    SkString onShortName() override {
        return SkString("points");
    }

    SkISize onISize() override {
        return SkISize::Make(640, 490);
    }

    static void fill_pts(SkPoint pts[], size_t n, SkRandom* rand) {
        for (size_t i = 0; i < n; i++) {
            // Compute these independently and store in variables, rather
            // than in the parameter-passing expression, to get consistent
            // evaluation order across compilers.
            SkScalar y = rand->nextUScalar1() * 480;
            SkScalar x = rand->nextUScalar1() * 640;
            pts[i].set(x, y);
        }
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->translate(SK_Scalar1, SK_Scalar1);

        SkRandom rand;
        SkPaint  p0, p1, p2, p3;
        const size_t n = 99;

        p0.setColor(SK_ColorRED);
        p1.setColor(SK_ColorGREEN);
        p2.setColor(SK_ColorBLUE);
        p3.setColor(SK_ColorWHITE);

        p0.setStrokeWidth(SkIntToScalar(4));
        p2.setStrokeCap(SkPaint::kRound_Cap);
        p2.setStrokeWidth(SkIntToScalar(6));

        SkPoint* pts = new SkPoint[n];
        fill_pts(pts, n, &rand);

        canvas->drawPoints(SkCanvas::kPolygon_PointMode, n, pts, p0);
        canvas->drawPoints(SkCanvas::kLines_PointMode, n, pts, p1);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p2);
        canvas->drawPoints(SkCanvas::kPoints_PointMode, n, pts, p3);

        delete[] pts;
    }

private:
    typedef GM INHERITED;
};

//////////////////////////////////////////////////////////////////////////////

static GM* MyFactory(void*) { return new PointsGM; }
static GMRegistry reg(MyFactory);

}

// Particle:
// Position (x, y)
// Color
namespace {

static constexpr int kMaxParticles = 4096;
SkPoint pos[kMaxParticles];
SkColor clr[kMaxParticles];
int life[kMaxParticles];
int nextFree[kMaxParticles];
int firstFree = 0;
}

DEF_SIMPLE_GM_BG(particles, canvas, 500, 500, SK_ColorWHITE) {
    static bool initialized = false;
    static SkScalar totalLength = 0;
    static SkTArray<SkPath> contours;
    static SkTArray<SkScalar> lengths;
    static SkRandom rand;

    if (!initialized) {
        initialized = true;

        for (int i = 0; i < kMaxParticles; ++i) {
            pos[i] = SkPoint::Make(SK_ScalarInfinity, SK_ScalarInfinity);
            life[i] = 0;
            nextFree[i] = i + 1;
            clr[i] = SK_ColorBLUE;
        }
        nextFree[kMaxParticles - 1] = -1;

        const char* msg = "Hello World";
        SkFont font(sk_tool_utils::create_portable_typeface());
        font.setSize(36);

        SkPath path;
        SkTextUtils::GetPath(msg, strlen(msg), kUTF8_SkTextEncoding, 0, 0, font, &path);

        SkPathMeasure pm(path, false);
        do {
            SkPath contour;
            if (pm.getSegment(0, SK_ScalarInfinity, &contour, true)) {
                contours.push_back(contour);
                totalLength += pm.getLength();
                lengths.push_back(pm.getLength());
            }
        } while (pm.nextContour());
    }

    const int kNewPerFrame = 180;
    const int kMinLifetime = 10;
    const int kMaxLifetime = 20;

    // Age old particles
    for (int i = 0; i < kMaxParticles; ++i) {
        if (life[i] == 1) {
            pos[i] = SkPoint::Make(SK_ScalarInfinity, SK_ScalarInfinity);
            life[i] = 0;
            nextFree[i] = firstFree;
            firstFree = i;
        } else if (life[i]) {
            --life[i];
            pos[i].fY -= rand.nextRangeScalar(0.5f, 1.5f);
            pos[i].fX += rand.nextRangeScalar(-0.5f, 0.5f);
        }
    }

    // Spawn new particles
    for (int i = 0; i < kNewPerFrame && firstFree >= 0; ++i) {
        int idx = firstFree;
        firstFree = nextFree[idx];

        life[idx] = rand.nextRangeU(kMinLifetime, kMaxLifetime);
        SkScalar len = rand.nextRangeScalar(0, totalLength);
        int c = 0;
        while (c < contours.count() && len > lengths[c]) {
            len -= lengths[c++];
        }
        SkPathMeasure cm(contours[c], false);
        cm.getPosTan(len, pos + idx, nullptr);
    }

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setStrokeCap(SkPaint::kRound_Cap);

    canvas->drawPoints(SkCanvas::kPoints_PointMode, kMaxParticles, pos, paint);
}
