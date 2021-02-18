/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkPaint.h"

#include "src/effects/imagefilters/SkMergeImageFilter.h"
#include "src/effects/imagefilters/SkMorphologyImageFilter.h"
#include "src/effects/imagefilters/SkOffsetImageFilter.h"
#include "src/effects/imagefilters/SkPaintImageFilter.h"
#include "src/effects/imagefilters/SkPictureImageFilter.h"
#include "src/effects/imagefilters/SkTileImageFilter.h"

// TODO (michaelludwig) - Once SkCanvas can draw the results of a filter with any transform, this
// filter can be moved out of core
#include "src/core/SkMatrixImageFilter.h"

void SkImageFilters::RegisterFlattenables() {
    SkDilateImageFilter::RegisterFlattenables();
    SkMergeImageFilter::RegisterFlattenables();
    SkOffsetImageFilter::RegisterFlattenables();
    SkPaintImageFilter::RegisterFlattenables();
    SkPictureImageFilter::RegisterFlattenables();
    SkTileImageFilter::RegisterFlattenables();
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, const SkSamplingOptions& sampling, sk_sp<SkImageFilter> input) {
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}

#ifdef SK_SUPPORT_LEGACY_MATRIX_IMAGEFILTER
sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(
        const SkMatrix& transform, SkFilterQuality filterQuality, sk_sp<SkImageFilter> input) {
    auto sampling = SkSamplingOptions(filterQuality,
                                      SkSamplingOptions::kMedium_asMipmapLinear);
    return SkMatrixImageFilter::Make(transform, sampling, std::move(input));
}
#endif

sk_sp<SkImageFilter> SkImageFilters::Merge(
        sk_sp<SkImageFilter>* const filters, int count, const CropRect& cropRect) {
    return SkMergeImageFilter::Make(filters, count, cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Offset(
        SkScalar dx, SkScalar dy, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkOffsetImageFilter::Make(dx, dy, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Paint(const SkPaint& paint, const CropRect& cropRect) {
    return SkPaintImageFilter::Make(paint, cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Picture(sk_sp<SkPicture> pic, const SkRect& targetRect) {
    return SkPictureImageFilter::Make(std::move(pic), targetRect);
}

sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader, Dither dither,
                                            const CropRect& cropRect) {
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setDither((bool) dither);
    return SkPaintImageFilter::Make(paint, cropRect);
}

#ifdef SK_SUPPORT_LEGACY_IMPLICIT_FILTERQUALITY
sk_sp<SkImageFilter> SkImageFilters::Shader(sk_sp<SkShader> shader, Dither dither,
                                            SkFilterQuality filterQuality,
                                            const CropRect& cropRect) {
    SkPaint paint;
    paint.setShader(std::move(shader));
    paint.setDither((bool) dither);
    // For SkImage::makeShader() shaders using SkImageShader::kInheritFromPaint sampling options
    paint.setFilterQuality(filterQuality);
    return SkPaintImageFilter::Make(paint, cropRect);
}
#endif

sk_sp<SkImageFilter> SkImageFilters::Tile(
        const SkRect& src, const SkRect& dst, sk_sp<SkImageFilter> input) {
    return SkTileImageFilter::Make(src, dst, std::move(input));
}

// Morphology filter effects

sk_sp<SkImageFilter> SkImageFilters::Dilate(
        SkScalar radiusX, SkScalar radiusY, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkDilateImageFilter::Make(radiusX, radiusY, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Erode(
        SkScalar radiusX, SkScalar radiusY, sk_sp<SkImageFilter> input, const CropRect& cropRect) {
    return SkErodeImageFilter::Make(radiusX, radiusY, std::move(input), cropRect);
}
