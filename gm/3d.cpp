/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkCanvas.h"
#include "include/utils/Sk3D.h"

struct Info {
    float   fNear = 0.05f;
    float   fFar = 4;
    float   fAngle = SK_ScalarPI / 4;

    SkPoint3    fEye { 0, 0, 1.0f/tan(fAngle/2) - 1 };
    SkPoint3    fCOA { 0, 0, 0 };
    SkPoint3    fUp  { 0, 1, 0 };
};

static SkMatrix44 inv(const SkMatrix44& m) {
    SkMatrix44 inverse;
    m.invert(&inverse);
    return inverse;
}

static SkMatrix44 make_ctm(const Info& info, const SkMatrix44& model, SkSize size) {
    SkMatrix44  camera,
                perspective,
                viewport;

    SkScalar w = size.width();
    SkScalar h = size.height();

    Sk3Perspective(&perspective, info.fNear, info.fFar, info.fAngle);
    Sk3LookAt(&camera, info.fEye, info.fCOA, info.fUp);
    viewport.setScale(w*0.5f, h*0.5f, 1);//.postTranslate(r.centerX(), r.centerY(), 0);

    return viewport * perspective * camera * model * inv(viewport);
}

#include "include/core/SkPicture.h"
#include "include/core/SkPictureRecorder.h"

static void do_draw(SkCanvas* canvas) {
    Info info;

    SkMatrix44 m;
    m.setRotateDegreesAbout(0, 1, 0, 30);

    m = make_ctm(info, m, {300, 300});
    m.dump();
 //   canvas->concat(m);

    canvas->translate(100, 100);
    SkPaint paint;
    paint.setColor(SK_ColorRED);
    canvas->drawRect({-100, -100, 100, 100}, paint);
}

/*
 *  Test calling drawables w/ translate and matrices
 */
DEF_SIMPLE_GM(sk3d_simple, real_canvas, 300, 300) {
    if (true) {
        do_draw(real_canvas);
        return;
    }

    SkPictureRecorder recorder;
    SkCanvas* canvas = recorder.beginRecording(300, 300);

    do_draw(canvas);

    auto pic = recorder.finishRecordingAsPicture();
    real_canvas->drawPicture(pic);
    return;

    auto data = pic->serialize();
    auto pic2 = SkPicture::MakeFromData(data.get());
    real_canvas->drawPicture(pic2);
}
