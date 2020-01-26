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
#include "include/private/SkM44.h"
#include "tools/timer/TimeUtils.h"

static SkM44 rotate_axis_angle(SkScalar x, SkScalar y, SkScalar z, SkScalar radians) {
    // SkM44 doesn't expose any rotation factories yet
    SkMatrix44 m;
    m.setRotateAboutUnit(x, y, z, radians);

    float a[16];
    m.asColMajorf(a);
    SkM44 m4;
    m4.setColMajor(a);
    return m4;
}

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
        fTime = TimeUtils::Scaled(1e-9f * nanos, 0.5f);
        return true;
    }

    void onDraw(SkCanvas* canvas) override {
        SkScalar viewportWidth = SkScalarMod(fTime, 10.f) / 10.f * (kMaxVW - kMinVW) + kMinVW;
        SkScalar radius = viewportWidth / 2.f; // round?
        // See https://developer.mozilla.org/en-US/docs/Web/CSS/transform-function/perspective
        SkM44 proj{1.f, 0.f, 0.f, 0.f,
                   0.f, 1.f, 0.f, 0.f,
                   0.f, 0.f, 1.f, 0.f,
                   0.f, 0.f, -1.f / radius, 1.f};
        SkM44 zoom             = SkM44::Translate(0.f, 0.f, radius);
        SkM44 postZoom         = SkM44::Translate(0.f, 0.f, -radius - 1.f);
        SkM44 rotateHorizontal = rotate_axis_angle(0.f, 1.f, 0.f, 2.356194490192345f);

        // w in degrees will need to be converted to radians
        SkV4 axisAngles[6] = {
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
            SkM44 model = rotate_axis_angle(axisAngles[i].x, axisAngles[i].y, axisAngles[i].z,
                                            SkDegreesToRadians(axisAngles[i].w));
            model = SkM44::Translate(radius, radius) * proj *    // project and place content
                    zoom * rotateHorizontal * model * postZoom * // main model matrix
                    SkM44::Translate(-radius, -radius);          // center content

            canvas->save();
            canvas->experimental_concat44(model);

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
