/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "gm.h"
#include "sk_tool_utils.h"
#include "SkCanvas.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkImageEncoder.h"
#include "SkMatrix.h"

#include "SkSurface.h"

static const int kImageWidth = 128;
static const int kImageHeight = 128;

static sk_sp<SkImage> make_image(GrContext* context) {
    SkImageInfo ii = SkImageInfo::MakeN32Premul(kImageWidth, kImageHeight);
    sk_sp<SkSurface> surf = SkSurface::MakeRaster(ii);

    SkCanvas* canvas = surf->getCanvas();

    SkPaint paint;
    paint.setTextSize(128);
    paint.setAntiAlias(true);
    canvas->clear(SK_ColorWHITE);
    canvas->drawText("f", 1, kImageWidth/2.0f, kImageHeight, paint);

    sk_sp<SkImage> image = surf->makeImageSnapshot();

    return image->makeTextureImage(context, nullptr);
}

static void create_matrices(SkMatrix* mats) {
    mats[0].setAll( 0,  1, 0,
                   -1,  0, 1,
                    0,  0, 1);
//    mats[0].setRotate(-90, 0.5f, 0.5f);

    // flip both x & y == rotate 180
    mats[1].setAll(-1,  0, 1,
                    0, -1, 1,
                    0,  0, 1);
//    mats[1].setRotate(180, 0.5f, 0.5f);

    // identity
    mats[2].setAll( 1,  0, 0,
                    0,  1, 0,
                    0,  0, 1);
//    mats[2].setRotate(0, 0.5f, 0.5f);

    mats[3].setAll( 0, -1, 1,
                   -1,  0, 1,
                    0,  0, 1);

    mats[4].setAll( 1,  0, 0,
                    0, -1, 1,
                    0,  0, 1);
    mats[5].setAll(-1,  0, 1,
                    0,  1, 0,
                    0,  0, 1);

}

namespace skiagm {

#if 1
class ColorCubeGM : public GM {
public:
    ColorCubeGM() {}

protected:
    SkString onShortName() override {
        return SkString("jpg-color-cube");
    }

    SkISize onISize() override {
        return SkISize::Make(6*kImageWidth, kImageHeight);
    }

    void onOnceBeforeDraw() override {

        create_matrices(fMatrices);

        for (int i = 0; i < 6; ++i) {
            fMatrices[i].preScale(1.0f/kImageWidth, 1.0f/kImageHeight);
            fMatrices[i].postScale(kImageWidth, kImageHeight);

            SkMatrix tmp;
            SkAssertResult(fMatrices[i].invert(&tmp));
            //tmp = fMatrices[i];
            fMatrices[i] = tmp;
        }
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        const SkRect box = SkRect::MakeWH(kImageWidth, kImageHeight);

        SkPaint stroke;
        stroke.setColor(SK_ColorRED);
        stroke.setStrokeWidth(1.0f);
        stroke.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < 6; ++i) {
            canvas->save();

            canvas->translate(i*kImageWidth, 0);
            /**/canvas->concat(fMatrices[i]);

            sk_sp<SkImage> image = make_image(context);
            canvas->drawImage(std::move(image), 0, 0);

            canvas->drawRect(box, stroke);

            canvas->restore();
        }
    }

private:
    SkMatrix       fMatrices[6];

    typedef GM INHERITED;
};

DEF_GM( return new ColorCubeGM; )
#endif


#if 1
class ColorCubeGM_textureMatrix : public GM {
public:
  ColorCubeGM_textureMatrix() {}

protected:
    SkString onShortName() override {
        return SkString("jpg-color-cube-txt");
    }

    SkISize onISize() override {
        return SkISize::Make(6*kImageWidth, kImageHeight);
    }

    void onOnceBeforeDraw() override {
        create_matrices(fMatrices);
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        const SkRect box = SkRect::MakeWH(kImageWidth, kImageHeight);

        SkPaint stroke;
        stroke.setColor(SK_ColorRED);
        stroke.setStrokeWidth(1.0f);
        stroke.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < 6; ++i) {
            canvas->save();

            canvas->translate(i*kImageWidth, 0);

            sk_sp<SkImage> image = make_image(context);
            /**/image->mTextureMatrix = fMatrices[i];
            canvas->drawImage(std::move(image), 0, 0);

            canvas->drawRect(box, stroke);

            canvas->restore();
        }
    }

private:
    SkMatrix       fMatrices[6];

    typedef GM INHERITED;
};

DEF_GM( return new ColorCubeGM_textureMatrix; )
#endif

}
