/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm.h"
#include "sk_tool_utils.h"

#include "SkSurface.h"

#if SK_SUPPORT_GPU

#include "GrContextPriv.h"
#include "GrProxyProvider.h"
#include "SkImage_Gpu.h"

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
    sk_tool_utils::set_portable_typeface(&paint);
    paint.setAntiAlias(true);
    paint.setTextSize(32);
    paint.setColor(color);

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

static SkColor swap_red_and_blue(SkColor c) {
    return SkColorSetRGB(SkColorGetB(c), SkColorGetG(c), SkColorGetR(c));
}

// Create an image with each corner marked w/ "LL", "LR", etc., with the origin either bottom-left
// or top-left.
static sk_sp<SkImage> make_reference_image(GrContext* context,
                                           const SkTArray<sk_sp<SkImage>>& labels,
                                           bool bottomLeftOrigin) {
    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    SkASSERT(kNumLabels == labels.count());

    SkImageInfo ii = SkImageInfo::Make(kImageSize, kImageSize,
                                       kN32_SkColorType, kOpaque_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(ii);
    SkCanvas canvas(bm);

    canvas.clear(SK_ColorWHITE);
    for (int i = 0; i < kNumLabels; ++i) {
        canvas.drawImage(labels[i],
                         0.0 != kPoints[i].fX ? kPoints[i].fX-kLabelSize-kInset : kInset,
                         0.0 != kPoints[i].fY ? kPoints[i].fY-kLabelSize-kInset : kInset);
    }

    GrSurfaceDesc desc;
    desc.fOrigin = kTopLeft_GrSurfaceOrigin;
    desc.fWidth = kImageSize;
    desc.fHeight = kImageSize;
    desc.fConfig = kRGBA_8888_GrPixelConfig;

    if (bottomLeftOrigin) {
        // Note that Ganesh will flip the data when it is uploaded
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
    }

    if (kN32_SkColorType == kBGRA_8888_SkColorType) {
        // We're playing a game here and uploading N32 data into an RGB dest. We might have
        // to swap red & blue to compensate.
        for (int y = 0; y < bm.height(); ++y) {
            uint32_t *sl = bm.getAddr32(0, y);
            for (int x = 0; x < bm.width(); ++x) {
                sl[x] = swap_red_and_blue(sl[x]);
            }
        }
    }

    sk_sp<GrTextureProxy> proxy = proxyProvider->createTextureProxy(desc, SkBudgeted::kYes,
                                                                    bm.getPixels(), bm.rowBytes());
    if (!proxy) {
        return nullptr;
    }

    return sk_make_sp<SkImage_Gpu>(context, kNeedNewImageUniqueID, kOpaque_SkAlphaType,
                                   std::move(proxy), nullptr, SkBudgeted::kYes);
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
        static const char* kLabelText[kNumLabels] = { "LL", "LR", "UL", "UR" };

        static const SkColor kLabelColors[kNumLabels] = {
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorCYAN
        };

        SkASSERT(!fLabels.count());
        for (int i = 0; i < kNumLabels; ++i) {
            fLabels.push_back(make_text_image(context, kLabelText[i], kLabelColors[i]));
        }
        SkASSERT(kNumLabels == fLabels.count());
    }

    void onDraw(SkCanvas* canvas) override {
        GrContext* context = canvas->getGrContext();
        if (!context) {
            skiagm::GM::DrawGpuOnlyMessage(canvas);
            return;
        }

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

#endif
