/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontTypes.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "include/gpu/GrContext.h"
#include "include/gpu/GrTypes.h"
#include "include/private/SkTArray.h"
#include "src/image/SkImage_Base.h"
#include "src/image/SkImage_Gpu.h"
#include "tools/ToolUtils.h"
#include "tools/gpu/ProxyUtils.h"

#include <string.h>
#include <utility>

class GrRenderTargetContext;

static const int kNumMatrices = 6;
static const int kImageSize = 128;
static const int kLabelSize = 32;
static const int kNumLabels = 4;
static const int kInset = 16;

static const int kCellSize = kImageSize+2*kLabelSize;
static const int kGMWidth  = kNumMatrices*kCellSize;
static const int kGMHeight = 4*kCellSize;

static const SkPoint kPoints[kNumLabels] = {
    {          0, kImageSize },     // LL
    { kImageSize, kImageSize },     // LR
    {          0,          0 },     // UL
    { kImageSize,          0 },     // UR
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
static sk_sp<SkImage> make_text_image(GrContext* context, const char* text, SkColor color) {
    SkPaint paint;
    paint.setAntiAlias(true);
    paint.setColor(color);

    SkFont font;
    font.setEdging(SkFont::Edging::kAntiAlias);
    font.setTypeface(ToolUtils::create_portable_typeface());
    font.setSize(32);

    SkRect bounds;
    font.measureText(text, strlen(text), SkTextEncoding::kUTF8, &bounds);
    const SkMatrix mat = SkMatrix::MakeRectToRect(bounds, SkRect::MakeWH(kLabelSize, kLabelSize),
                                                  SkMatrix::kFill_ScaleToFit);

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(kLabelSize, kLabelSize);
    sk_sp<SkSurface> surf = SkSurface::MakeRaster(ii);

    SkCanvas* canvas = surf->getCanvas();

    canvas->clear(SK_ColorWHITE);
    canvas->concat(mat);
    canvas->drawSimpleText(text, strlen(text), SkTextEncoding::kUTF8, 0, 0, font, paint);

    sk_sp<SkImage> image = surf->makeImageSnapshot();

    return image->makeTextureImage(context);
}

// Create an image with each corner marked w/ "LL", "LR", etc., with the origin either bottom-left
// or top-left.
static sk_sp<SkImage> make_reference_image(GrContext* context,
                                           const SkTArray<sk_sp<SkImage>>& labels,
                                           bool bottomLeftOrigin) {
    SkASSERT(kNumLabels == labels.count());

    SkImageInfo ii =
            SkImageInfo::Make(kImageSize, kImageSize, kRGBA_8888_SkColorType, kOpaque_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(ii);
    SkCanvas canvas(bm);

    canvas.clear(SK_ColorWHITE);
    for (int i = 0; i < kNumLabels; ++i) {
        canvas.drawImage(labels[i],
                         0.0 != kPoints[i].fX ? kPoints[i].fX-kLabelSize-kInset : kInset,
                         0.0 != kPoints[i].fY ? kPoints[i].fY-kLabelSize-kInset : kInset);
    }

    auto origin = bottomLeftOrigin ? kBottomLeft_GrSurfaceOrigin : kTopLeft_GrSurfaceOrigin;

    auto proxy = sk_gpu_test::MakeTextureProxyFromData(context, GrRenderable::kNo, kImageSize,
                                                       kImageSize, bm.colorType(), bm.alphaType(),
                                                       origin, bm.getPixels(), bm.rowBytes());
    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(sk_ref_sp(context), kNeedNewImageUniqueID, kOpaque_SkAlphaType,
                                   std::move(proxy), nullptr);
}

// Here we're converting from a matrix that is intended for UVs to a matrix that is intended
// for rect geometry used for a drawImage call. They are, in some sense, inverses of each
// other but we also need a scale to map from the [0..1] uv range to the actual size of
// image.
static bool UVMatToGeomMatForImage(SkMatrix* geomMat, const SkMatrix& uvMat) {

    const SkMatrix yFlip = SkMatrix::MakeAll(1, 0, 0, 0, -1, 1, 0, 0, 1);

    SkMatrix tmp = uvMat;
    tmp.preConcat(yFlip);
    tmp.preScale(1.0f/kImageSize, 1.0f/kImageSize);

    tmp.postConcat(yFlip);
    tmp.postScale(kImageSize, kImageSize);

    return tmp.invert(geomMat);
}

// This GM exercises drawImage with a set of matrices that use an unusual amount of flips and
// rotates.
class FlippityGM : public skiagm::GpuGM {
public:
    FlippityGM() {
        this->setBGColor(0xFFCCCCCC);
    }

private:
    SkString onShortName() override {
        return SkString("flippity");
    }

    SkISize onISize() override {
        return SkISize::Make(kGMWidth, kGMHeight);
    }

    // Draw the reference image and the four corner labels in the matrix's coordinate space
    void drawImageWithMatrixAndLabels(SkCanvas* canvas, SkImage* image, int matIndex,
                                      bool drawSubset, bool drawScaled) {
        static const SkRect kSubsets[kNumMatrices] = {
            SkRect::MakeXYWH(kInset, 0, kImageSize-kInset, kImageSize),
            SkRect::MakeXYWH(0, kInset, kImageSize, kImageSize-kInset),
            SkRect::MakeXYWH(0, 0, kImageSize-kInset, kImageSize),
            SkRect::MakeXYWH(0, 0, kImageSize, kImageSize-kInset),
            SkRect::MakeXYWH(kInset/2, kInset/2, kImageSize-kInset, kImageSize-kInset),
            SkRect::MakeXYWH(kInset, kInset, kImageSize-2*kInset, kImageSize-2*kInset),
        };

        SkMatrix imageGeomMat;
        SkAssertResult(UVMatToGeomMatForImage(&imageGeomMat, kUVMatrices[matIndex]));

        canvas->save();

            // draw the reference image
            canvas->concat(imageGeomMat);
            if (drawSubset) {
                canvas->drawImageRect(image, kSubsets[matIndex],
                                      drawScaled ? SkRect::MakeWH(kImageSize, kImageSize)
                                                 : kSubsets[matIndex],
                                      nullptr, SkCanvas::kFast_SrcRectConstraint);
            } else {
                canvas->drawImage(image, 0, 0);
            }

            // draw the labels
            for (int i = 0; i < kNumLabels; ++i) {
                canvas->drawImage(fLabels[i],
                                    0.0f == kPoints[i].fX ? -kLabelSize : kPoints[i].fX,
                                    0.0f == kPoints[i].fY ? -kLabelSize : kPoints[i].fY);
            }
        canvas->restore();
    }

    void drawRow(GrContext* context, SkCanvas* canvas,
                 bool bottomLeftImage, bool drawSubset, bool drawScaled) {

        sk_sp<SkImage> referenceImage = make_reference_image(context, fLabels, bottomLeftImage);

        canvas->save();
            canvas->translate(kLabelSize, kLabelSize);

            for (int i = 0; i < kNumMatrices; ++i) {
                this->drawImageWithMatrixAndLabels(canvas, referenceImage.get(), i,
                                                   drawSubset, drawScaled);
                canvas->translate(kCellSize, 0);
            }
        canvas->restore();
    }

    void makeLabels(GrContext* context) {
        if (fLabels.count()) {
            return;
        }

        static const char* kLabelText[kNumLabels] = { "LL", "LR", "UL", "UR" };

        static const SkColor kLabelColors[kNumLabels] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorCYAN
        };

        for (int i = 0; i < kNumLabels; ++i) {
            fLabels.push_back(make_text_image(context, kLabelText[i], kLabelColors[i]));
        }
        SkASSERT(kNumLabels == fLabels.count());
    }

    void onDraw(GrContext* context, GrRenderTargetContext*, SkCanvas* canvas) override {
        this->makeLabels(context);

        canvas->save();

        // Top row gets TL image
        this->drawRow(context, canvas, false, false, false);

        canvas->translate(0, kCellSize);

        // Bottom row gets BL image
        this->drawRow(context, canvas, true, false, false);

        canvas->translate(0, kCellSize);

        // Third row gets subsets of BL images
        this->drawRow(context, canvas, true, true, false);

        canvas->translate(0, kCellSize);

        // Fourth row gets scaled subsets of BL images
        this->drawRow(context, canvas, true, true, true);

        canvas->restore();

        // separator grid
        for (int i = 0; i < 4; ++i) {
            canvas->drawLine(0, i * kCellSize, kGMWidth, i * kCellSize, SkPaint());
        }
        for (int i = 0; i < kNumMatrices; ++i) {
            canvas->drawLine(i * kCellSize, 0, i * kCellSize, kGMHeight, SkPaint());
        }
    }

private:
    SkTArray<sk_sp<SkImage>> fLabels;

    typedef GM INHERITED;
};

DEF_GM(return new FlippityGM;)
