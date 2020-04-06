/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkData.h"

struct Info {
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkV3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkV3    fCOA { 0, 0, 0 };
    SkV3    fUp  { 0, 1, 0 };
};

static SkM44 inv(const SkM44& m) {
    SkM44 inverse(SkM44::kUninitialized_Constructor);
    if (!m.invert(&inverse)) {
        inverse.setIdentity();
    }
    return inverse;
}

static SkM44 make_ctm(const Info& info, const SkM44& model, SkSize size) {
    SkM44  camera, perspective, viewport;

    SkScalar w = size.width();
    SkScalar h = size.height();

    perspective = Sk3Perspective(info.fNear, info.fFar, info.fAngle);
    camera = Sk3LookAt(info.fEye, info.fCOA, info.fUp);
    viewport.setScale(w*0.5f, h*0.5f, 1);//.postTranslate(r.centerX(), r.centerY(), 0);

    return viewport * perspective * camera * model * inv(viewport);
}

#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"

static void do_draw(SkCanvas* canvas, SkColor color) {
    SkAutoCanvasRestore acr(canvas, true);

    Info info;

    SkM44 m = SkM44::Rotate({0, 1, 0}, SK_ScalarPI/6);

    canvas->concat(make_ctm(info, m, {300, 300}));

    canvas->translate(150, 150);
    SkPaint paint;
    paint.setColor(color);
    canvas->drawRect({-100, -100, 100, 100}, paint);
}

/*
 *  Test calling drawables w/ translate and matrices
 */
DEF_SIMPLE_GM(sk3d_simple, real_canvas, 300, 300) {
    do_draw(real_canvas, 0xFFFF0000);

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(300, 300);

    do_draw(canvas, 0x880000FF);

    auto pic = recorder.finishRecordingAsPicture();
    if (false) {
        real_canvas->drawPicture(pic);
    } else {
        auto data = pic->serialize();
        auto pic2 = SkPicture::MakeFromData(data.get());
        real_canvas->drawPicture(pic2);
    }
}

DEF_SIMPLE_GM(sk3d_camera, canvas, 300, 300) {
    SkScalar w = 300, h = 300;

    Info info;
    auto projection = Sk3Perspective(info.fNear, info.fFar, info.fAngle);
    auto camera = Sk3LookAt(info.fEye, info.fCOA, info.fUp);
    auto viewport = SkM44::Scale(w*0.5f, h*0.5f, 1);
    auto model = SkM44::Rotate({0, 1, 0}, SK_ScalarPI/6);

#if 0
    canvas->concat( viewport * projection * camera * model * inv(viewport) );
#else
    canvas->concat(viewport);
    canvas->saveCamera(projection, camera);
    canvas->concat(model * inv(viewport));
#endif

    canvas->translate(150, 150);
    SkPaint paint;
    paint.setColor(0xFFFF0000);
    canvas->drawRect({-100, -100, 100, 100}, paint);

    canvas->restore();
}

