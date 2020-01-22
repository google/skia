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

// Adapted from https://codepen.io/adamdupuis/pen/qLYzqB
class CrBug224618GM : public skiagm::GM {
public:
    CrBug224618GM() : fTime(0.f) {}

protected:
    SkString onShortName() override {
        return SkString("crbug_224618");
    }

    SkISize onISize() override {
        return SkISize::Make(kMaxVW, kMaxVW);
    }

    // This animates the FOV in viewer, to ensure the panorama covering rects are stable across
    // a variety of perspective matrices
    bool onAnimate(double nanos) override {
        fTime = TimeUtils::Scaled(1e-9 * nanos, 0.5f);
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar viewportWidth = SkScalarMod(fTime, 10.f) / 10.f * (kMaxVW - kMinVW) + kMinVW;
        SkScalar radius = viewportWidth / 2.f; // round?
        // See https://developer.mozilla.org/en-US/docs/Web/CSS/transform-function/perspective
        SkMatrix44 proj(SkMatrix44::kIdentity_Constructor);
        proj.set(3, 2, -1.f / radius);
        SkMatrix44 zoom;
        zoom.setTranslate(0.f, 0.f, radius);
        SkMatrix44 postZoom;
        postZoom.setTranslate(0.f, 0.f, -radius - 1.f);
        SkMatrix44 rotateHorizontal;
        rotateHorizontal.setRotateAboutUnit(0.f, 1.f, 0.f, 2.356194490192345f); // already radians
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
            // t is set to 'zoom * rotateHorizontal * rotateSide * postZoom' and then projected by
            // 'proj'. First compute 'rotateSide' from 'axisAngles' and then apply the other
            // matrices.
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

            canvas->drawRect(SkRect::MakeWH(viewportWidth, viewportWidth), fillPaint);
            canvas->restore();
        }
    }
private:
    static const int kMaxVW = 800;
    static const int kMinVW = 300;
    SkScalar fTime;
};

DEF_GM(return new CrBug224618GM();)
