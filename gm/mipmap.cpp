/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "SkCanvas.h"
#include "SkImage.h"
#include "SkRandom.h"
#include "SkSurface.h"

static SkImage* make_image() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(319, 52);
    SkAutoTUnref<SkSurface> surface(SkSurface::NewRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(sk_tool_utils::color_to_565(0xFFF8F8F8));

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 20; ++i) {
        canvas->drawCircle(-4, 25, 20, paint);
        canvas->translate(25, 0);
    }
    return surface->newImageSnapshot();
}

static void test_mip(SkCanvas* canvas) {
    SkAutoTUnref<SkImage> img(make_image());//SkImage::NewFromEncoded(data));

    SkPaint paint;
    const SkRect dst = SkRect::MakeWH(177, 15);

    paint.setTextSize(30);
    SkString str;
    str.printf("scale %g %g", dst.width() / img->width(), dst.height() / img->height());
//    canvas->drawText(str.c_str(), str.size(), 300, 100, paint);

    canvas->translate(20, 20);
    for (int i = 0; i < 4; ++i) {
        paint.setFilterQuality(SkFilterQuality(i));
        canvas->drawImageRect(img, dst, &paint);
        canvas->translate(0, 20);
    }
    canvas->drawImage(img, 20, 20, NULL);
}

class MipMapGM : public skiagm::GM {
public:
    MipMapGM() {}

protected:
    SkString onShortName() override { return SkString("mipmap"); }

    SkISize onISize() override { return SkISize::Make(400, 200); }

    void onDraw(SkCanvas* canvas) override {
        test_mip(canvas);
    }

private:
    typedef skiagm::GM INHERITED;
};
DEF_GM( return new MipMapGM; )

