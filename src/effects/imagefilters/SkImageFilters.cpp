/*
 * Copyright 2019 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkPaint.h"

// TODO (michaelludwig) - Once SkCanvas can draw the results of a filter with any transform, this
// filter can be moved out of core
#include "src/core/SkMatrixImageFilter.h"

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
