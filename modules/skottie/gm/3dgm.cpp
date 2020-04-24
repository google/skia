/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkMatrix44.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/utils/Sk3D.h"
#include "modules/skottie/include/Skottie.h"
#include "tools/Resources.h"

#include <math.h>
#include <algorithm>
#include <memory>

class SkMetaData;

static SkMatrix operator*(const SkMatrix& a, const SkMatrix& b) {
    SkMatrix44 c;
    c.setConcat(a, b);
    return c;
}

class GM3d : public skiagm::GM {
    float   fNear = 0.5;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkPoint3    fEye { 0, 0, 4 };
    SkPoint3    fCOA {0,0,0};//{ 0.5f, 0.5f, 0.5f };
    SkPoint3    fUp  { 0, 1, 0 };

    SkPoint3    fP3[8];

    sk_sp<skottie::Animation> fAnim;
    SkScalar fAnimT = 0;

public:
    GM3d() {}
    ~GM3d() override {}

protected:
    void onOnceBeforeDraw() override {
        if (auto stream = GetResourceAsStream("skottie/skottie_sample_2.json")) {
            fAnim = skottie::Animation::Make(stream.get());
        }

        int index = 0;
        for (float x = 0; x <= 1; ++x) {
            for (float y = 0; y <= 1; ++y) {
                for (float z = 0; z <= 1; ++z) {
                    fP3[index++] = { x, y, z };
                }
            }
        }
    }

    static void draw_viewport(SkCanvas* canvas, const SkMatrix& viewport) {
        SkPaint p;
        p.setColor(0x10FF0000);

        canvas->save();
        canvas->concat(viewport);
        canvas->drawRect({-1, -1, 1, 1}, p);

        p.setColor(0x80FF0000);
        canvas->drawLine({-1, -1}, {1, 1}, p);
        canvas->drawLine({1, -1}, {-1, 1}, p);
        canvas->restore();
    }

    static void draw_skia(SkCanvas* canvas, const SkMatrix44& m4, const SkMatrix& vp,
                          skottie::Animation* anim) {
        auto proc = [canvas, vp, anim](SkColor c, const SkMatrix44& m4) {
            SkPaint p;
            p.setColor(c);
            SkRect r = { 0, 0, 1, 1 };
            canvas->save();
            canvas->concat(vp * SkMatrix(m4));
            anim->render(canvas, &r);
//            canvas->drawRect({0, 0, 1, 1}, p);
            canvas->restore();
        };

        SkMatrix44 tmp;

        proc(0x400000FF, m4);
        tmp.setTranslate(0, 0, 1);
        proc(0xC00000FF, m4 * tmp);
        tmp.setRotateAboutUnit(1, 0, 0, SK_ScalarPI/2);
        proc(0x4000FF00, m4 * tmp);
        tmp.postTranslate(0, 1, 0);
        proc(0xC000FF00, m4 * tmp);
        tmp.setRotateAboutUnit(0, 1, 0, -SK_ScalarPI/2);
        proc(0x40FF0000, m4 * tmp);
        tmp.postTranslate(1, 0, 0);
        proc(0xC0FF0000, m4 * tmp);
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        if (!fAnim) {
            *errorMsg = "No animation.";
            return DrawResult::kFail;
        }
        SkMatrix44  camera,
                    perspective,
                    mv;
        SkMatrix    viewport;

        {
            float w = this->width();
            float h = this->height();
            float s = std::min(w, h);
            viewport.setTranslate(1, -1);
            viewport.postScale(s/2, -s/2);

            draw_viewport(canvas, viewport);
        }

        Sk3Perspective(&perspective, fNear, fFar, fAngle);
        Sk3LookAt(&camera, fEye, fCOA, fUp);
        mv.postConcat(camera);
        mv.postConcat(perspective);
        SkPoint pts[8];
        Sk3MapPts(pts, mv, fP3, 8);
        viewport.mapPoints(pts, 8);

        SkPaint paint;
        paint.setStyle(SkPaint::kStroke_Style);
        SkFont font;
        font.setEdging(SkFont::Edging::kAlias);

        SkPath cube;

        cube.moveTo(pts[0]);
        cube.lineTo(pts[2]);
        cube.lineTo(pts[6]);
        cube.lineTo(pts[4]);
        cube.close();

        cube.moveTo(pts[1]);
        cube.lineTo(pts[3]);
        cube.lineTo(pts[7]);
        cube.lineTo(pts[5]);
        cube.close();

        cube.moveTo(pts[0]);    cube.lineTo(pts[1]);
        cube.moveTo(pts[2]);    cube.lineTo(pts[3]);
        cube.moveTo(pts[4]);    cube.lineTo(pts[5]);
        cube.moveTo(pts[6]);    cube.lineTo(pts[7]);

        canvas->drawPath(cube, paint);

        {
            SkPoint3 src[4] = {
                { 0, 0, 0 }, { 2, 0, 0 }, { 0, 2, 0 }, { 0, 0, 2 },
            };
            SkPoint dst[4];
            mv.setConcat(perspective, camera);
            Sk3MapPts(dst, mv, src, 4);
            viewport.mapPoints(dst, 4);
            const char* str[3] = { "X", "Y", "Z" };
            for (int i = 1; i <= 3; ++i) {
                canvas->drawLine(dst[0], dst[i], paint);
            }

            for (int i = 0; i < 3; ++i) {
                canvas->drawString(str[i], dst[i + 1].fX, dst[i + 1].fY, font, paint);
            }
        }

        fAnim->seek(fAnimT);
        draw_skia(canvas, mv, viewport, fAnim.get());
        return DrawResult::kOk;
    }

    SkISize onISize() override { return { 1024, 768 }; }

    SkString onShortName() override { return SkString("3dgm"); }

    bool onAnimate(double nanos) override {
        if (!fAnim) {
            return false;
        }
        SkScalar dur = fAnim->duration();
        fAnimT = fmod(1e-9 * nanos, dur) / dur;
        return true;
    }
    bool onChar(SkUnichar uni) override {
        switch (uni) {
            case 'a': fEye.fX += 0.125f; return true;
            case 'd': fEye.fX -= 0.125f; return true;
            case 'w': fEye.fY += 0.125f; return true;
            case 's': fEye.fY -= 0.125f; return true;
            case 'q': fEye.fZ += 0.125f; return true;
            case 'z': fEye.fZ -= 0.125f; return true;
            default: break;
        }
        return false;
    }

    bool onGetControls(SkMetaData*) override { return false; }
    void onSetControls(const SkMetaData&) override {

    }
};

DEF_GM(return new GM3d;)

