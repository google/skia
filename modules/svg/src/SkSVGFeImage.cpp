/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/svg/include/SkSVGFeImage.h"

#include "include/core/SkImage.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/effects/SkImageFilters.h"
#include "modules/svg/include/SkSVGAttributeParser.h"
#include "modules/svg/include/SkSVGFilterContext.h"
#include "modules/svg/include/SkSVGImage.h"
#include "modules/svg/include/SkSVGRenderContext.h"

bool SkSVGFeImage::parseAndSetAttribute(const char* n, const char* v) {
    return INHERITED::parseAndSetAttribute(n, v) ||
           this->setHref(SkSVGAttributeParser::parse<SkSVGIRI>("xlink:href", n, v)) ||
           this->setPreserveAspectRatio(SkSVGAttributeParser::parse<SkSVGPreserveAspectRatio>(
                   "preserveAspectRatio", n, v));
}

sk_sp<SkImageFilter> SkSVGFeImage::onMakeImageFilter(const SkSVGRenderContext& ctx,
                                                     const SkSVGFilterContext& fctx) const {
    // Load image and map viewbox (image bounds) to viewport (filter effects subregion).
    const SkRect viewport = this->resolveFilterSubregion(ctx, fctx);
    const auto imgInfo =
            SkSVGImage::LoadImage(ctx.resourceProvider(), fHref, viewport, fPreserveAspectRatio);
    if (!imgInfo.fImage) {
        return nullptr;
    }

    // Create the image filter mapped according to aspect ratio
    const SkRect srcRect = SkRect::Make(imgInfo.fImage->bounds());
    const SkRect& dstRect = imgInfo.fDst;
    // TODO: image-rendering property
    auto imgfilt = SkImageFilters::Image(imgInfo.fImage, srcRect, dstRect,
                                         SkSamplingOptions(SkFilterMode::kLinear,
                                                           SkMipmapMode::kNearest));

    // Aspect ratio mapping may end up drawing content outside of the filter effects region,
    // so perform an explicit crop.
    return SkImageFilters::Merge(&imgfilt, 1, fctx.filterEffectsRegion());
}
