/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkColorFilter.h"
#include "SkSurface.h"
#include "SkImage_Gpu.h"

static const int kNumMatrices = 6;
static const int kImageSize = 128;
static const int kLabelSize = 32;

static const int kLL = 0;
static const int kLR = 1;
static const int kUL = 2;
static const int kUR = 3;
static const int kNumLabels = 4;

static const SkPoint kPoints[kNumLabels] = {
    {          0, kImageSize },     // kLL
    { kImageSize, kImageSize },     // kLR
    {          0,          0 },     // kUL
    { kImageSize,          0 },     // kUR
};

static const SkColor kColors[kNumMatrices] = {
    SK_ColorRED,
    SK_ColorGREEN,
    SK_ColorBLUE,
    SK_ColorCYAN,
    SK_ColorMAGENTA,
    SK_ColorYELLOW,
};

#if 0
#include "SkImageEncoder.h"

static void write_bm(const char* name, const SkBitmap& bm) {

    static int foo = 0;

    char fullName[256];
    snprintf(fullName, 256, "d:\\src\\bugs\\%s%d.png", name, foo++);
    fullName[255] = '\0';

    SkFILEWStream file(fullName);
    SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100);
}
#endif

// Create a fixed size text label like "LL" or "LR".
static sk_sp<SkImage> make_text_image(GrContext* context, const char* text) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setTextSize(32);

    SkRect bounds;
    paint.measureText(text, strlen(text), &bounds);
    const SkMatrix mat = SkMatrix::MakeRectToRect(bounds, SkRect::MakeWH(kLabelSize, kLabelSize),
                                                  SkMatrix::kFill_ScaleToFit);

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(kLabelSize, kLabelSize);
    sk_sp<SkSurface> surf = SkSurface::MakeRaster(ii);

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorWHITE);
    canvas->concat(mat);
    canvas->drawText(text, strlen(text), 0, 0, paint);

    sk_sp<SkImage> image = surf->makeImageSnapshot();

    return image->makeTextureImage(context, nullptr);
}

// Create an image with each corner marked w/ "LL", "LR", etc.
static sk_sp<SkImage> make_image(GrContext* context, const SkTArray<sk_sp<SkImage>>& labels) {
    SkASSERT(kNumLabels == labels.count());

    SkImageInfo ii = SkImageInfo::MakeN32Premul(kImageSize, kImageSize);
    sk_sp<SkSurface> surf = SkSurface::MakeRaster(ii);

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorWHITE);
    for (int i = 0; i < kNumLabels; ++i) {
        canvas->drawImage(labels[i],
                          0.0 != kPoints[i].fX ? kPoints[i].fX-kLabelSize : 0,
                          0.0 != kPoints[i].fY ? kPoints[i].fY-kLabelSize : 0);
    }

    sk_sp<SkImage> image = surf->makeImageSnapshot();
    image = image->makeTextureImage(context, nullptr);

    if (image) {
        SkImage_Gpu* tmp = (SkImage_Gpu*) image.get();
        tmp->peekProxy()->fOrigin = kBottomLeft_GrSurfaceOrigin;
    }

    return image;
}

static void create_matrices(SkMatrix* mats) {
    mats[0].setAll( 0, -1, kImageSize,
                   -1,  0, kImageSize,
                    0,  0, 1);

    mats[1].setAll( 1,  0, 0,
                    0, -1, kImageSize,
                    0,  0, 1);

    // flip x
    mats[2].setAll(-1,  0, kImageSize,
                    0,  1, 0,
                    0,  0, 1);

    mats[3].setAll( 0,  1, 0,
                   -1,  0, kImageSize,
                    0,  0, 1);
//    mats[3].setRotate(-90, 0.5f, 0.5f);

    // flip both x & y == rotate 180
    mats[4].setAll(-1,  0, kImageSize,
                    0, -1, kImageSize,
                    0,  0, 1);
//    mats[4].setRotate(180, 0.5f, 0.5f);

    // identity
    mats[5].setAll( 1,  0, 0,
                    0,  1, 0,
                    0,  0, 1);
//    mats[5].setRotate(0, 0.5f, 0.5f);
}

class FlippityGM : public skiagm::GM {
public:
    FlippityGM() {
        this->setBGColor(sk_tool_utils::color_to_565(0xFFCCCCCC));
    }

protected:

    SkString onShortName() override {
        return SkString("flippity");
    }

    SkISize onISize() override {
        return SkISize::Make(kNumMatrices*(kImageSize+2*kLabelSize), kImageSize+2*kLabelSize);
    }

    void onOnceBeforeDraw() override {

        create_matrices(fMatrices);

#if 0
        SkMatrix yFlip;
        yFlip.setAll(1.0f,  0.0f, 0.0f,
                     0.0f, -1.0f, 1.0f,
                     0.0f,  0.0f, 1.0f);

        for (int i = 0; i < kNumMatrices; ++i) {
            fMatrices[i].preConcat(yFlip);
            fMatrices[i].preScale(1.0f/kImageSize, 1.0f/kImageSize);
            fMatrices[i].postScale(kImageSize, kImageSize);

            SkMatrix tmp;
            SkAssertResult(fMatrices[i].invert(&tmp));
            fMatrices[i] = tmp;
        }
#endif
    }

    // Draw the reference image and the four corner labels in the matrix' coordinate space
    void drawWithMatrix(SkCanvas* canvas, const SkMatrix& mat, SkColor color) {
        SkPaint paint;
        paint.setColorFilter(SkColorFilter::MakeModeFilter(color, SkBlendMode::kModulate));

        canvas->save();
            canvas->concat(mat);

            canvas->drawImage(fReferenceImage, 0, 0, &paint);

            for (int i = 0; i < kNumLabels; ++i) {
                canvas->drawImage(fLabels[i],
                                  0.0f == kPoints[i].fX ? -kLabelSize : kPoints[i].fX,
                                  0.0f == kPoints[i].fY ? -kLabelSize : kPoints[i].fY,
                                  &paint);
            }

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        const SkRect divider = SkRect::MakeXYWH(kImageSize+kLabelSize, 0, 1, kImageSize+2*kLabelSize);

        SkASSERT(!fLabels.count());
        fLabels.push_back(make_text_image(context, "LL"));
        fLabels.push_back(make_text_image(context, "LR"));
        fLabels.push_back(make_text_image(context, "UL"));
        fLabels.push_back(make_text_image(context, "UR"));
        SkASSERT(kNumLabels == fLabels.count());

        fReferenceImage = make_image(context, fLabels);

        canvas->translate(kLabelSize, kLabelSize);

        for (int i = 0; i < kNumMatrices; ++i) {
            this->drawWithMatrix(canvas, fMatrices[i], kColors[i]);

            canvas->drawRect(divider, SkPaint());

            canvas->translate(kImageSize+2*kLabelSize, 0);
        }

#if 0
        const SkRect box = SkRect::MakeWH(kImageWidth, kImageHeight);

        SkPaint stroke;
        stroke.setColor(SK_ColorRED);
        stroke.setStrokeWidth(1.0f);
        stroke.setStyle(SkPaint::kStroke_Style);

        for (int i = 0; i < 6; ++i) {
            canvas->save();

            canvas->translate(i*kImageWidth, 0);
            /**/canvas->concat(fMatrices[i]);

            sk_sp<SkImage> image = make_image(context, false);
            canvas->drawImage(std::move(image), 0, 0);

            canvas->drawRect(box, stroke);

            canvas->restore();
        }
#endif
    }

private:
    SkMatrix                 fMatrices[kNumMatrices];
    SkTArray<sk_sp<SkImage>> fLabels;
    sk_sp<SkImage>           fReferenceImage;

    typedef GM INHERITED;
};

DEF_GM(return new FlippityGM;)
