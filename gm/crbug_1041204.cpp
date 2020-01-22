/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "src/gpu/geometry/GrQuad.h"
#include "tools/timer/TimeUtils.h"

// #include "include/core/SkPath.h"

// crbug 1041204
class SkBug9779GMA : public skiagm::GM {
public:
    SkBug9779GMA() : fTime(0.f) {}

protected:

    SkString onShortName() override {
        return SkString("skbug_9779a");
    }

    SkISize onISize() override {
        return SkISize::Make(kMaxVW, kMaxVW);
    }

    bool onAnimate(double nanos) override {
        fTime = TimeUtils::Scaled(1e-9 * nanos, 0.5f);
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar viewportWidth = SkScalarMod(fTime, 10.f) / 10.f * (kMaxVW - kMinVW) + kMinVW;
        SkScalar radius = viewportWidth / 2.f; // round?

        // See https://developer.mozilla.org/en-US/docs/Web/CSS/transform-function/perspective
        // for projection matrix when --webkit-perspective: 800px is used.
        SkMatrix44 proj(SkMatrix44::kIdentity_Constructor);
        proj.set(3, 2, -1.f / radius);

        SkMatrix44 zoom;
        zoom.setTranslate(0.f, 0.f, radius - 5.f);
        SkMatrix44 postZoom;
        postZoom.setTranslate(0.f, 0.f, -radius - 5.f);

        SkMatrix44 rotateHorizontal;
        rotateHorizontal.setRotateAboutUnit(0.f, 1.f, 0.f, 2.356194490192345); // radians

        // w in degrees will need to be converted to radians
        SkVector4 axisAngles[6] = {
            {0.f, 1.f, 0.f, -90.f}, // rotateY(-90deg)
            {1.f, 0.f, 0.f, 0.f},   // <none>
            {0.f, 1.f, 0.f, 90.f},  // rotateY(90deg)
            {0.f, 1.f, 0.f, 180.f}, // rotateY(180deg)
            {1.f, 0.f, 0.f, -90.f}, // rotateX(-90deg)
            {1.f, 0.f, 0.f, 90.f},  // rotateX(90deg)
        };

        SkColor faceColors[6] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorYELLOW,
            SkColorSetARGB(0xFF, 0xFF, 0xA5, 0x00), // orange css
            SkColorSetARGB(0xFF, 0x80, 0x00, 0x80)  // purple css
        };

        for (int i = 0; i < 6; ++i) {
            // if (i != 4) continue;
            // t is set to 'zoom * rotateHorizontal * rotateSide * postZoom' and then projected by
            // 'proj'. First compute 'rotateSide' from 'axisAngles' and then apply the other
            // matrices (I think the original author got the right-to-left effect of the transform
            // sequence backwards, given how the positioned 'postZoom' in the CSS declaration).
            SkMatrix44 model;
            model.setRotateAboutUnit(
                    axisAngles[i].fData[0], axisAngles[i].fData[1], axisAngles[i].fData[2],
                    SkDegreesToRadians(axisAngles[i].fData[3]));
            model.preTranslate(-radius, -radius, 0.f); // persp origin
            model.preConcat(postZoom);
            model.postConcat(rotateHorizontal);
            model.postConcat(zoom);
            model.postConcat(proj);
            model.postTranslate(radius, radius, 0.f);  // persp origin

            canvas->save();
            canvas->concat(SkMatrix::MakeAll(
                            model.get(0, 0), model.get(0, 1), model.get(0, 3),
                            model.get(1, 0), model.get(1, 1), model.get(1, 3),
                            model.get(3, 0), model.get(3, 1), model.get(3, 3)));

            SkPaint fillPaint;
            fillPaint.setAntiAlias(true);
            fillPaint.setColor(faceColors[i]);
            fillPaint.setAlphaf(0.55f);

            SkMatrix ctm = canvas->getTotalMatrix();
            GrQuad q = GrQuad::MakeFromRect(SkRect::MakeWH(viewportWidth, viewportWidth), ctm);
            SkDebugf("Face %d:\n", i);
            SkDebugf("  0: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                     q.x(0), q.y(0), q.w(0), q.x(0) / q.w(0), q.y(0) / q.w(0));
            SkDebugf("  1: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                     q.x(1), q.y(1), q.w(1), q.x(1) / q.w(1), q.y(1) / q.w(1));
            SkDebugf("  2: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                     q.x(2), q.y(2), q.w(2), q.x(2) / q.w(2), q.y(2) / q.w(2));
            SkDebugf("  3: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                     q.x(3), q.y(3), q.w(3), q.x(3) / q.w(3), q.y(3) / q.w(3));
            SkRect b = q.bounds();
            SkDebugf("  b: [%.2f %.2f %.2f %.2f]\n", b.fLeft, b.fTop, b.fRight, b.fBottom);

            canvas->drawRect(SkRect::MakeWH(viewportWidth, viewportWidth), fillPaint);

            canvas->restore();
        }
    }

private:
    static const int kMaxVW = 800;
    static const int kMinVW = 300;

    SkScalar fTime;
};

// crbug 224618
class SkBug9779GMB : public skiagm::GM {
public:
    SkBug9779GMB() {}

protected:

    SkString onShortName() override {
        return SkString("skbug_9779b");
    }

    SkISize onISize() override {
        return SkISize::Make(512, 512);
    }

    void onDraw(SkCanvas* canvas) override {
        canvas->scale(2.f, 2.f);
        canvas->concat(SkMatrix::MakeAll(
                -0.0005550860255665798, -0.0030798374421905717, -0.014111959825129805,
                -0.07569627776417084, 232.00000000000017, 39.999999999999936,
                0, 0, 1));
        canvas->translate(-3040103.0493857153, 337502.1103282161);
        canvas->scale(9783.93962050256, -9783.93962050256);

        SkMatrix ctm = canvas->getTotalMatrix();
        GrQuad q = GrQuad::MakeFromRect(SkRect::MakeWH(512, 512), ctm);
        SkDebugf("canvas quad:\n");
        SkDebugf("  0: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                 q.x(0), q.y(0), q.w(0), q.x(0) / q.w(0), q.y(0) / q.w(0));
        SkDebugf("  1: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                 q.x(1), q.y(1), q.w(1), q.x(1) / q.w(1), q.y(1) / q.w(1));
        SkDebugf("  2: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                 q.x(2), q.y(2), q.w(2), q.x(2) / q.w(2), q.y(2) / q.w(2));
        SkDebugf("  3: (%.2f %.2f %.2f) -> (%.2f %.2f)\n",
                 q.x(3), q.y(3), q.w(3), q.x(3) / q.w(3), q.y(3) / q.w(3));

        SkPaint paint;
        paint.setColor(SK_ColorBLUE);
        paint.setAntiAlias(true);
        canvas->drawRect(SkRect::MakeWH(512, 512), paint);
    }
};

DEF_GM(return new SkBug9779GMA();)
DEF_GM(return new SkBug9779GMB();)
