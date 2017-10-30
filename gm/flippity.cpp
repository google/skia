/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkSurface.h"
#include "SkImage_Gpu.h"

static const int kNumMatrices = 6;
static const int kImageSize = 128;
static const int kLabelSize = 32;
static const int kNumLabels = 4;

static const int kCellSize = kImageSize+2*kLabelSize;
static const int kGMWidth  = kNumMatrices*kCellSize;
static const int kGMHeight = 2*kCellSize;


static const SkPoint kPoints[kNumLabels] = {
    {          0, kImageSize },     // kLL
    { kImageSize, kImageSize },     // kLR
    {          0,          0 },     // kUL
    { kImageSize,          0 },     // kUR
};

static const SkMatrix kUVMatrices[kNumMatrices] = {
    SkMatrix::MakeAll( 0, -1, 1,
                      -1,  0, 1,
                       0,  0, 1),
    SkMatrix::MakeAll( 1,  0, 0,
                       0, -1, 1,
                       0,  0, 1),
    // flip x
    SkMatrix::MakeAll(-1,  0, 1,
                       0,  1, 0,
                       0,  0, 1),
    SkMatrix::MakeAll( 0,  1, 0,
                      -1,  0, 1,
                       0,  0, 1),
    // flip both x & y == rotate 180
    SkMatrix::MakeAll(-1,  0, 1,
                       0, -1, 1,
                       0,  0, 1),
    // identity
    SkMatrix::MakeAll(1,  0, 0,
                      0,  1, 0,
                      0,  0, 1)
};

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

static void swap_rows(SkBitmap* bm, int rowOffset) {
    SkASSERT(rowOffset >= 0 && rowOffset < bm->height());

    int otherRow = bm->height() - rowOffset - 1;
    SkASSERT(rowOffset != otherRow);

    uint32_t* r1 = bm->getAddr32(0, rowOffset);
    uint32_t* r2 = bm->getAddr32(0, otherRow);

    for (int x = 0; x < bm->width(); ++x) {
        uint32_t tmp = r1[x];
        r1[x] = r2[x];
        r2[x] = tmp;
    }

}

// Create an image with each corner marked w/ "LL", "LR", etc., with the origin either bottom-left
// or top-left.
static sk_sp<SkImage> make_image(GrContext* context, const SkTArray<sk_sp<SkImage>>& labels,
                                 bool bottomLeftOrigin) {
    SkASSERT(kNumLabels == labels.count());

    SkImageInfo ii = SkImageInfo::Make(kImageSize, kImageSize,
                                       kN32_SkColorType, kOpaque_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(ii);
    SkCanvas canvas(bm);

    canvas.clear(SK_ColorWHITE);
    for (int i = 0; i < kNumLabels; ++i) {
        canvas.drawImage(labels[i],
                         0.0 != kPoints[i].fX ? kPoints[i].fX-kLabelSize : 0,
                         0.0 != kPoints[i].fY ? kPoints[i].fY-kLabelSize : 0);
    }

    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = kImageSize;
    desc.fHeight = kImageSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (bottomLeftOrigin) {
        // flip the data & the origin
        for (int y = 0; y < kImageSize/2; ++y) {
            swap_rows(&bm, y);
        }
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    }

    sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                               desc, SkBudgeted::kYes,
                                                               bm.getPixels(), bm.rowBytes());

    return sk_make_sp<SkImage_Gpu>(context, kNeedNewImageUniqueID, kOpaque_SkAlphaType,
                                   std::move(proxy), nullptr, SkBudgeted::kYes);
}

// When drawing the labels, they have UL origins so they require an extra flip when the
// reference image is BL.
static void uv_mat_to_geom_mat_for_labels(SkMatrix* geomMat, const SkMatrix& uvMat,
                                          bool isBottomLeft) {
    SkMatrix tmp = uvMat;

    if (isBottomLeft) {
        SkMatrix yFlip = SkMatrix::MakeAll(1.0f,  0.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  0.0f, 1.0f);
        tmp.preConcat(yFlip);
    }

    tmp.preScale(1.0f/kImageSize, 1.0f/kImageSize);
    tmp.postScale(kImageSize, kImageSize);

    SkAssertResult(tmp.invert(geomMat));
}

// Here we're converting from a matrix that is intended for UVs to an matrix that is intended
// for rect geometry used for a drawImage call. They are, in some sense, inverses of each
// other but we also need a scale to map from the [0..1] uv range to the actualy size of
// image.
static bool UVMatToGeomMatForImage(SkMatrix* geomMat, const SkMatrix& uvMat) {
    SkMatrix tmp = uvMat;
    tmp.preScale(1.0f/kImageSize, 1.0f/kImageSize);
    tmp.postScale(kImageSize, kImageSize);

    return tmp.invert(geomMat);
}

// This GM exercises drawImage with a set of matrices that use an unusual amount of flips and
// rotates.
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
        return SkISize::Make(kGMWidth, kGMHeight);
    }

    // Draw the reference image and the four corner labels in the matrix's coordinate space
    void drawImageWithMatrixAndLabels(SkCanvas* canvas, SkImage* image, int matIndex,
                                      bool isBottomLeft) {
        canvas->save();

        {
            canvas->save();
                SkMatrix imageGeomMat;
                SkAssertResult(UVMatToGeomMatForImage(&imageGeomMat, kUVMatrices[matIndex]));

                canvas->concat(imageGeomMat);
                canvas->drawImage(image, 0, 0);
            canvas->restore();
        }

        {
            canvas->save();

            SkMatrix labelGeomMat;
            uv_mat_to_geom_mat_for_labels(&labelGeomMat, kUVMatrices[matIndex], isBottomLeft);

            canvas->concat(labelGeomMat);

            for (int i = 0; i < kNumLabels; ++i) {
                canvas->drawImage(fLabels[i],
                                  0.0f == kPoints[i].fX ? -kLabelSize : kPoints[i].fX,
                                  0.0f == kPoints[i].fY ? -kLabelSize : kPoints[i].fY);
            }
            canvas->restore();
        }

        canvas->restore();
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();

        SkASSERT(!fLabels.count());
        fLabels.push_back(make_text_image(context, "LL"));
        fLabels.push_back(make_text_image(context, "LR"));
        fLabels.push_back(make_text_image(context, "UL"));
        fLabels.push_back(make_text_image(context, "UR"));
        SkASSERT(kNumLabels == fLabels.count());

        // Top row gets TL image
        {
            sk_sp<SkImage> referenceImage = make_image(context, fLabels, false);

            canvas->save();
            canvas->translate(kLabelSize, kLabelSize);

            for (int i = 0; i < kNumMatrices; ++i) {
                this->drawImageWithMatrixAndLabels(canvas, referenceImage.get(), i, false);
                canvas->translate(kImageSize+2*kLabelSize, 0);
            }
            canvas->restore();
        }

        // Bottom row gets BL image
        {
            sk_sp<SkImage> referenceImage = make_image(context, fLabels, true);

            canvas->save();
            canvas->translate(kLabelSize, kImageSize+3*kLabelSize);

            for (int i = 0; i < kNumMatrices; ++i) {
                this->drawImageWithMatrixAndLabels(canvas, referenceImage.get(), i, true);
                canvas->translate(kCellSize, 0);
            }
            canvas->restore();
        }

        // separator grid
        canvas->drawLine(0, kCellSize, kGMWidth, kCellSize, SkPaint());
        for (int i = 0; i < kNumMatrices; ++i) {
            canvas->drawLine(i * kCellSize, 0, i * kCellSize, kGMHeight, SkPaint());
        }
    }

private:
    SkTArray<sk_sp<SkImage>> fLabels;

    typedef GM INHERITED;
};

DEF_GM(return new FlippityGM;)
