/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

#include "include/effects/SkImageFilters.h"

#include "include/gpu/GrDirectContext.h"

#include "tools/Resources.h"
#include "tools/ToolUtils.h"

#include <utility>

///////////////////////////////////////////////////////////////////////////////

static void show_bounds(SkCanvas* canvas, const SkIRect* clip, const SkIRect* inSubset,
                        const SkIRect* outSubset) {
    const SkIRect* rects[] { clip, inSubset, outSubset };
    SkColor colors[] { SK_ColorBLUE, SK_ColorYELLOW, SK_ColorRED };

    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);

    for (size_t i = 0; i < SK_ARRAY_COUNT(rects); ++i) {
        // Skip null bounds rects, since not all methods have subsets
        if (rects[i]) {
            paint.setColor(colors[i]);
            canvas->drawRect(SkRect::Make(*(rects[i])), paint);
        }
    }
}

// Factories for creating image filters, either with or without a cropRect
// (this could go away if there was a SkImageFilter::makeWithCropRect() function, but that seems
//  less generally useful).
typedef sk_sp<SkImageFilter> (*FilterFactory)(sk_sp<SkImage> auxImage, const SkIRect* cropRect);

static sk_sp<SkImageFilter> color_filter_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    // The color filter uses kSrcIn so that it respects the transparency introduced by clamping;
    // using kSrc would just turn the entire out rect to green regardless.
    auto cf = SkColorFilters::Blend(SK_ColorGREEN, SkBlendMode::kSrcIn);
    return SkImageFilters::ColorFilter(std::move(cf), nullptr, cropRect);
}

static sk_sp<SkImageFilter> blur_filter_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    return SkImageFilters::Blur(2.0f, 2.0f, nullptr, cropRect);
}

static sk_sp<SkImageFilter> drop_shadow_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    return SkImageFilters::DropShadow(10.0f, 5.0f, 3.0f, 3.0f, SK_ColorBLUE, nullptr, cropRect);
}

static sk_sp<SkImageFilter> offset_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    return SkImageFilters::Offset(10.f, 5.f, nullptr, cropRect);
}

static sk_sp<SkImageFilter> dilate_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    return SkImageFilters::Dilate(10.f, 5.f, nullptr, cropRect);
}

static sk_sp<SkImageFilter> erode_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    return SkImageFilters::Erode(10.f, 5.f, nullptr, cropRect);
}

static sk_sp<SkImageFilter> displacement_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    sk_sp<SkImageFilter> displacement = SkImageFilters::Image(std::move(auxImage));
    return SkImageFilters::DisplacementMap(SkColorChannel::kR, SkColorChannel::kG, 40.f,
                                           std::move(displacement), nullptr, cropRect);
}

static sk_sp<SkImageFilter> arithmetic_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    sk_sp<SkImageFilter> background = SkImageFilters::Image(std::move(auxImage));
    return SkImageFilters::Arithmetic(0.0f, .6f, 1.f, 0.f, false, std::move(background),
                                      nullptr, cropRect);
}

static sk_sp<SkImageFilter> blend_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    sk_sp<SkImageFilter> background = SkImageFilters::Image(std::move(auxImage));
    return SkImageFilters::Blend(
            SkBlendMode::kModulate, std::move(background), nullptr, cropRect);
}

static sk_sp<SkImageFilter> convolution_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    SkISize kernelSize = SkISize::Make(3, 3);
    SkIPoint kernelOffset = SkIPoint::Make(1, 1);
    // A Laplacian edge detector, ee https://en.wikipedia.org/wiki/Kernel_(image_processing)
    SkScalar kernel[9] = {-1.f, -1.f, -1.f,
                          -1.f, 8.f, -1.f,
                          -1.f, -1.f, -1.f};
    return SkImageFilters::MatrixConvolution(kernelSize, kernel, 1.f, 0.f, kernelOffset,
                                             SkTileMode::kClamp, false, nullptr, cropRect);
}

static sk_sp<SkImageFilter> matrix_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    SkMatrix matrix = SkMatrix::I();
    matrix.setRotate(45.f, 50.f, 50.f);

    // This doesn't support a cropRect
    return SkImageFilters::MatrixTransform(matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr);
}

static sk_sp<SkImageFilter> alpha_threshold_factory(sk_sp<SkImage> auxImage,
                                                    const SkIRect* cropRect) {
    // Centered cross with higher opacity
    SkRegion region(SkIRect::MakeLTRB(30, 45, 70, 55));
    region.op(SkIRect::MakeLTRB(45, 30, 55, 70), SkRegion::kUnion_Op);

    return SkImageFilters::AlphaThreshold(region, 1.f, .2f, nullptr, cropRect);
}

static sk_sp<SkImageFilter> lighting_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    // Must convert the RGB values of the source to alpha, since that is what the lighting filters
    // use to estimate their normals. This color matrix changes the color to white and the alpha
    // to be equal to the approx. luminance of the original color.
    static const float kMatrix[20] = {
        0.f, 0.f, 0.f, 0.f, 1.f,
        0.f, 0.f, 0.f, 0.f, 1.f,
        0.f, 0.f, 0.f, 0.f, 1.f,
        0.2126f, 0.7152f, 0.0722f, 0.f, 0.f
    };
    sk_sp<SkImageFilter> srcToAlpha = SkImageFilters::ColorFilter(
            SkColorFilters::Matrix(kMatrix), nullptr);

    // Combine both specular and diffuse into a single DAG since they use separate internal filter
    // implementations.
    SkScalar sinAzimuth = SkScalarSin(SkDegreesToRadians(225.f)),
             cosAzimuth = SkScalarCos(SkDegreesToRadians(225.f));

    SkPoint3 spotTarget = SkPoint3::Make(SkIntToScalar(40), SkIntToScalar(40), 0);
    SkPoint3 diffLocation = SkPoint3::Make(spotTarget.fX + 50 * cosAzimuth,
                                           spotTarget.fY + 50 * sinAzimuth,
                                           SkIntToScalar(10));
    SkPoint3 specLocation = SkPoint3::Make(spotTarget.fX - 50 * sinAzimuth,
                                           spotTarget.fY + 50 * cosAzimuth,
                                           SkIntToScalar(10));
    sk_sp<SkImageFilter> diffuse = SkImageFilters::PointLitDiffuse(
            diffLocation, SK_ColorWHITE, /* scale */ 1.f, /* kd */ 2.f, srcToAlpha, cropRect);
    sk_sp<SkImageFilter> specular = SkImageFilters::PointLitSpecular(
            specLocation, SK_ColorRED, /* scale */ 1.f, /* ks */ 1.f, /* shine */ 8.f,
            srcToAlpha, cropRect);
    return SkImageFilters::Merge(std::move(diffuse), std::move(specular), cropRect);
}

static sk_sp<SkImageFilter> tile_factory(sk_sp<SkImage> auxImage, const SkIRect* cropRect) {
    // Tile the subset over a large region
    return SkImageFilters::Tile(SkRect::MakeLTRB(25, 25, 75, 75), SkRect::MakeWH(100, 100),
                                nullptr);
}

namespace {
    enum class Strategy {
        // Uses makeWithFilter, passing in subset and clip directly
        kMakeWithFilter,
        // Uses saveLayer after clipRect() to filter on the restore (i.e. reference image)
        kSaveLayer
    };
}  // namespace

// In this GM, we're going to feed the inner portion of a 100x100 mandrill (i.e., strip off a
// 25-wide border) through the makeWithFilter method. We'll then draw the appropriate subset of the
// result to the screen at the given offset. Some filters rely on a secondary image, which will be a
// 100x100 checkerboard. The original image is drawn in the background so that alignment is clear
// when drawing the result at its reported offset.
class ImageMakeWithFilterGM : public skiagm::GM {
public:
    ImageMakeWithFilterGM (Strategy strategy, bool filterWithCropRect = false)
            : fStrategy(strategy)
            , fFilterWithCropRect(filterWithCropRect)
            , fMainImage(nullptr)
            , fAuxImage(nullptr) {}

protected:
    SkString onShortName() override {
        SkString name = SkString("imagemakewithfilter");

        if (fFilterWithCropRect) {
            name.append("_crop");
        }
        if (fStrategy == Strategy::kSaveLayer) {
            name.append("_ref");
        }
        return name;
    }

    SkISize onISize() override { return SkISize::Make(1980, 860); }

    void onOnceBeforeDraw() override {
        SkImageInfo info = SkImageInfo::MakeN32(100, 100, kUnpremul_SkAlphaType);
        auto surface = SkSurface::MakeRaster(info, nullptr);

        sk_sp<SkImage> colorImage = GetResourceAsImage("images/mandrill_128.png");
        // Resize to 100x100
        surface->getCanvas()->drawImageRect(
                colorImage, SkRect::MakeWH(colorImage->width(), colorImage->height()),
                SkRect::MakeWH(info.width(), info.height()), SkSamplingOptions(), nullptr,
                                            SkCanvas::kStrict_SrcRectConstraint);
        fMainImage = surface->makeImageSnapshot();

        ToolUtils::draw_checkerboard(surface->getCanvas());
        fAuxImage = surface->makeImageSnapshot();
    }

    DrawResult onDraw(SkCanvas* canvas, SkString* errorMsg) override {
        FilterFactory filters[] = {
            color_filter_factory,
            blur_filter_factory,
            drop_shadow_factory,
            offset_factory,
            dilate_factory,
            erode_factory,
            displacement_factory,
            arithmetic_factory,
            blend_factory,
            convolution_factory,
            matrix_factory,
            alpha_threshold_factory,
            lighting_factory,
            tile_factory
        };
        const char* filterNames[] = {
            "Color",
            "Blur",
            "Drop Shadow",
            "Offset",
            "Dilate",
            "Erode",
            "Displacement",
            "Arithmetic",
            "Xfer Mode", // "blend"
            "Convolution",
            "Matrix Xform",
            "Alpha Threshold",
            "Lighting",
            "Tile"
        };
        static_assert(SK_ARRAY_COUNT(filters) == SK_ARRAY_COUNT(filterNames), "filter name length");

        SkIRect clipBounds[] {
            { -20, -20, 100, 100 },
            {   0,   0,  75,  75 },
            {  20,  20, 100, 100 },
            { -20, -20,  50,  50 },
            {  20,  20,  50,  50 },
            {  30,  30,  75,  75 }
        };

        // These need to be GPU-backed when on the GPU to ensure that the image filters use the GPU
        // code paths (otherwise they may choose to do CPU filtering then upload)
        sk_sp<SkImage> mainImage, auxImage;

        auto rContext = canvas->recordingContext();
        // In a DDL context, we can't use the GPU code paths and we will drop the work – skip.
        auto dContext = GrAsDirectContext(rContext);
        if (rContext) {
            if (!dContext) {
                *errorMsg = "Requires a direct context.";
                return DrawResult::kSkip;
            }
            if (dContext->abandoned()) {
                *errorMsg = "Direct context abandoned.";
                return DrawResult::kSkip;
            }
            mainImage = fMainImage->makeTextureImage(dContext);
            auxImage = fAuxImage->makeTextureImage(dContext);
        } else {
            mainImage = fMainImage;
            auxImage = fAuxImage;
        }
        if (!mainImage || !auxImage) {
            return DrawResult::kFail;
        }
        SkASSERT(mainImage && (mainImage->isTextureBacked() || !rContext));
        SkASSERT(auxImage && (auxImage->isTextureBacked() || !rContext));

        SkScalar MARGIN = SkIntToScalar(40);
        SkScalar DX = mainImage->width() + MARGIN;
        SkScalar DY = auxImage->height() + MARGIN;

        // Header hinting at what the filters do
        SkPaint textPaint;
        textPaint.setAntiAlias(true);
        SkFont font(nullptr, 12);
        for (size_t i = 0; i < SK_ARRAY_COUNT(filterNames); ++i) {
            canvas->drawString(filterNames[i], DX * i + MARGIN, 15, font, textPaint);
        }

        canvas->translate(MARGIN, MARGIN);

        for (auto clipBound : clipBounds) {
            canvas->save();
            for (size_t i = 0; i < SK_ARRAY_COUNT(filters); ++i) {
                SkIRect subset = SkIRect::MakeXYWH(25, 25, 50, 50);
                SkIRect outSubset;

                // Draw the original image faintly so that it aids in checking alignment of the
                // filtered result.
                SkPaint alpha;
                alpha.setAlphaf(0.3f);
                canvas->drawImage(mainImage, 0, 0, SkSamplingOptions(), &alpha);

                this->drawImageWithFilter(canvas, mainImage, auxImage, filters[i], clipBound,
                                          subset, &outSubset);

                // Draw outlines to highlight what was subset, what was cropped, and what was output
                // (no output subset is displayed for kSaveLayer since that information isn't avail)
                SkIRect* outSubsetBounds = nullptr;
                if (fStrategy != Strategy::kSaveLayer) {
                    outSubsetBounds = &outSubset;
                }
                show_bounds(canvas, &clipBound, &subset, outSubsetBounds);

                canvas->translate(DX, 0);
            }
            canvas->restore();
            canvas->translate(0, DY);
        }
        return DrawResult::kOk;
    }

private:
    Strategy fStrategy;
    bool fFilterWithCropRect;
    sk_sp<SkImage> fMainImage;
    sk_sp<SkImage> fAuxImage;

    void drawImageWithFilter(SkCanvas* canvas, sk_sp<SkImage> mainImage, sk_sp<SkImage> auxImage,
                             FilterFactory filterFactory, const SkIRect& clip,
                             const SkIRect& subset, SkIRect* dstRect) {
        // When creating the filter with a crop rect equal to the clip, we should expect to see no
        // difference from a filter without a crop rect. However, if the CTM isn't managed properly
        // by makeWithFilter, then the final result will be the incorrect intersection of the clip
        // and the transformed crop rect.
        sk_sp<SkImageFilter> filter = filterFactory(auxImage,
                                                    fFilterWithCropRect ? &clip : nullptr);

        if (fStrategy == Strategy::kSaveLayer) {
            SkAutoCanvasRestore acr(canvas, true);

            // Clip before the saveLayer with the filter
            canvas->clipRect(SkRect::Make(clip));

            // Put the image filter on the layer
            SkPaint paint;
            paint.setImageFilter(filter);
            canvas->saveLayer(nullptr, &paint);

            // Draw the original subset of the image
            SkRect r = SkRect::Make(subset);
            canvas->drawImageRect(mainImage, r, r, SkSamplingOptions(),
                                  nullptr, SkCanvas::kStrict_SrcRectConstraint);

            *dstRect = subset;
        } else {
            sk_sp<SkImage> result;
            SkIRect outSubset;
            SkIPoint offset;

            auto rContext = canvas->recordingContext();
            result = mainImage->makeWithFilter(rContext, filter.get(), subset, clip,
                                               &outSubset, &offset);

            SkASSERT(result);
            SkASSERT(mainImage->isTextureBacked() == result->isTextureBacked());

            *dstRect = SkIRect::MakeXYWH(offset.x(), offset.y(),
                                         outSubset.width(), outSubset.height());
            canvas->drawImageRect(result, SkRect::Make(outSubset), SkRect::Make(*dstRect),
                                  SkSamplingOptions(), nullptr,
                                  SkCanvas::kStrict_SrcRectConstraint);
        }
    }

    using INHERITED = GM;
};
// The different strategies should all look the same, with the exception of filters that affect
// transparent black (i.e. the lighting filter). In the save layer case, the filter affects the
// transparent pixels outside of the drawn subset, whereas the makeWithFilter is restricted. This
// works as intended.
DEF_GM( return new ImageMakeWithFilterGM(Strategy::kMakeWithFilter); )
DEF_GM( return new ImageMakeWithFilterGM(Strategy::kSaveLayer); )
// Test with crop rects on the image filters; should look identical to above if working correctly
DEF_GM( return new ImageMakeWithFilterGM(Strategy::kMakeWithFilter, true); )
DEF_GM( return new ImageMakeWithFilterGM(Strategy::kSaveLayer, true); )
