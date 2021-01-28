// Copyright 2020 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#include "tools/fiddle/examples.h"
REG_FIDDLE(example3ps, 320, 256, false, 0) {
static SkMatrix setpsmatrix(float sx, float kx, float ky, float sy, float tx, float ty, int h) {
    SkMatrix m;

    m.setAll(sx, -kx, -tx * sx - (ty - h) * kx, -ky, sy, (ty - h) * sy + tx * ky, 0, 0, 1);
    return m;
}

void draw(SkCanvas* canvas) {
    canvas->save();
    canvas->scale(15, -15);
    canvas->translate(0, -28);
    SkPath path;
    path.moveTo(2, 2);
    path.lineTo(3, 3);
    path.lineTo(3, 4);
    path.lineTo(2, 4);
    path.lineTo(1, 5);
    path.close();
    SkPaint p;
    p.setAntiAlias(true);
    p.setStrokeWidth(0.1f);

    canvas->save();
    canvas->translate(0, 18);
    p.setColor(SK_ColorGRAY);
    canvas->drawPath(path, p);
    canvas->restore();

    p.setStyle(SkPaint::kStroke_Style);
    canvas->save();
    canvas->translate(8, 19);
    canvas->rotate(90);
    p.setColor(SK_ColorRED);
    canvas->drawPath(path, p);
    canvas->restore();

    canvas->save();
    canvas->translate(5, 23);
    canvas->rotate(-90);
    p.setColor(SK_ColorBLUE);
    canvas->drawPath(path, p);
    canvas->restore();

    canvas->save();
    canvas->translate(14, 18);
    canvas->scale(-1, 1);
    p.setColor(0xFF007F00);
    canvas->drawPath(path, p);
    canvas->restore();
    canvas->restore();

    canvas->scale(15, 15);
    canvas->translate(0, 24);

    SkMatrix m;
    unsigned char d[] = {0x00, 0x00, 0x00, 0x00, 0xff, 0xff};
    sk_sp<SkData> data = SkData::MakeWithoutCopy((unsigned char*)d, sizeof(d));
    SkImageInfo info =
            SkImageInfo::Make(3, 2, SkColorType::kGray_8_SkColorType, kOpaque_SkAlphaType);
    sk_sp<SkImage> image = SkImage::MakeRasterData(info, data, 3);

    canvas->save();
    m = setpsmatrix(1, 0, 0, 1, -1, -24, 2);
    //  m.setAll(1, 0, -1, 0, 1, -24, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();

    canvas->save();
    m = setpsmatrix(0, 1, 1, 0, -24, -5, 2);
    //  m.setAll(0, -1, 5, -1, 0, -22, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();

    canvas->save();
    m = setpsmatrix(0, 1, -1, 0, 27, -8, 2);
    //  m.setAll(0, -1, 8, 1, 0, -25, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();

    canvas->save();
    m = setpsmatrix(0, -1, -1, 0, -24, 13, 2);
    //  m.setAll(0, 1, 9, -1, 0, -22, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();

    canvas->save();
    m = setpsmatrix(-1, 0, 0, -1, 17, 26, 2);
    //  m.setAll(-1, 0, 15, 0, -1, -22, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();

    canvas->save();
    m = setpsmatrix(-1, 0, 0, 1, 21, -24, 2);
    //  m.setAll(-1, 0, 19, 0, 1, -24, 0, 0, 1);
    canvas->concat(m);
    canvas->drawImage(image, 0, 0);
    canvas->restore();
}
}  // END FIDDLE
