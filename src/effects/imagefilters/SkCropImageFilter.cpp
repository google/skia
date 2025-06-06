/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAssert.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"


#include <cstdint>
#include <optional>
#include <utility>

namespace {

class SkCropImageFilter final : public SkImageFilter_Base {
public:
    SkCropImageFilter(const SkRect& cropRect, SkTileMode tileMode, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fCropRect(cropRect)
            , fTileMode(tileMode) {
        SkASSERT(cropRect.isFinite());
        SkASSERT(cropRect.isSorted());
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterCropImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkCropImageFilter)
    static sk_sp<SkFlattenable> LegacyTileCreateProc(SkReadBuffer&);

    bool onAffectsTransparentBlack() const override { return fTileMode != SkTileMode::kDecal; }

    // Disable recursing in affectsTransparentBlack() if we hit a Crop.
    // TODO(skbug.com/40045513): Automatically infer this from the output bounds being finite.
    bool ignoreInputsAffectsTransparentBlack() const override { return true; }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    // The crop rect is specified in floating point to allow cropping to partial local pixels,
    // that could become whole pixels in the layer-space image if the canvas is scaled.
    // For now it's always rounded to integer pixels as if it were non-AA.
    //
    // The returned rect is intersected with 'outputBounds', which is either the desired or
    // actual bounds of the child filter.
    skif::LayerSpace<SkIRect> cropRect(const skif::Mapping& mapping) const {
        skif::LayerSpace<SkRect> crop = mapping.paramToLayer(fCropRect);
        // If 'crop' has fractional values, rounding out can mean that rendering of the input image
        // or (particularly) the source content will produce fractional coverage values in the
        // edge pixels. With decal tiling, this is the most accurate behavior and does not produce
        // any surprises. However, with any other mode, the fractional coverage introduces
        // transparency that can be greatly magnified (particularly from clamping). To avoid this
        // we round in on those modes to ensure any transparency on the edges truly came from the
        // content and not rasterization.
        return fTileMode == SkTileMode::kDecal ? crop.roundOut() : crop.roundIn();
    }

    // Calculates the required input to fill the crop rect, given the desired output that it will
    // be tiled across.
    skif::LayerSpace<SkIRect> requiredInput(const skif::Mapping& mapping,
                                            const skif::LayerSpace<SkIRect>& outputBounds) const {
        return  this->cropRect(mapping).relevantSubset(outputBounds, fTileMode);
    }

    skif::ParameterSpace<SkRect> fCropRect;
    SkTileMode fTileMode;
};

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Crop(const SkRect& rect,
                                          SkTileMode tileMode,
                                          sk_sp<SkImageFilter> input) {
    if (!SkIsValidRect(rect)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkCropImageFilter(rect, tileMode, std::move(input)));
}

// While a number of filter factories could handle "empty" cases (e.g. a null SkShader or SkPicture)
// just use a crop with an empty rect because its implementation gracefully handles empty rects.
sk_sp<SkImageFilter> SkImageFilters::Empty() {
    return SkImageFilters::Crop(SkRect::MakeEmpty(), SkTileMode::kDecal, nullptr);
}

sk_sp<SkImageFilter> SkImageFilters::Tile(const SkRect& src,
                                          const SkRect& dst,
                                          sk_sp<SkImageFilter> input) {
    // The Tile filter is simply a crop to 'src' with a kRepeat tile mode wrapped in a crop to 'dst'
    // with a kDecal tile mode.
    sk_sp<SkImageFilter> filter = SkImageFilters::Crop(src, SkTileMode::kRepeat, std::move(input));
    filter = SkImageFilters::Crop(dst, SkTileMode::kDecal, std::move(filter));
    return filter;
}

void SkRegisterCropImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkCropImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkTileImageFilter", SkCropImageFilter::LegacyTileCreateProc);
    SkFlattenable::Register("SkTileImageFilterImpl", SkCropImageFilter::LegacyTileCreateProc);
}

sk_sp<SkFlattenable> SkCropImageFilter::LegacyTileCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect src, dst;
    buffer.readRect(&src);
    buffer.readRect(&dst);
    return SkImageFilters::Tile(src, dst, common.getInput(0));
}

sk_sp<SkFlattenable> SkCropImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect cropRect = buffer.readRect();
    if (!buffer.isValid() || !buffer.validate(SkIsValidRect(cropRect))) {
        return nullptr;
    }

    SkTileMode tileMode = SkTileMode::kDecal;
    if (!buffer.isVersionLT(SkPicturePriv::kCropImageFilterSupportsTiling)) {
        tileMode = buffer.read32LE(SkTileMode::kLastTileMode);
    }

    return SkImageFilters::Crop(cropRect, tileMode, common.getInput(0));
}

void SkCropImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeRect(SkRect(fCropRect));
    buffer.writeInt(static_cast<int32_t>(fTileMode));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkCropImageFilter::onFilterImage(const skif::Context& context) const {
    skif::LayerSpace<SkIRect> cropInput = this->requiredInput(context.mapping(),
                                                              context.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, context.withNewDesiredOutput(cropInput));

    // The 'cropInput' is the optimal input to satisfy the original crop rect, but we have to pass
    // the actual crop rect in order for the tile mode to be applied correctly to the FilterResult.
    return childOutput.applyCrop(context, this->cropRect(context.mapping()), fTileMode);
}

// TODO(michaelludwig) - onGetInputLayerBounds() and onGetOutputLayerBounds() are tightly coupled
// to both each other's behavior and to onFilterImage(). If onFilterImage() had a concept of a
// dry-run (e.g. FilterResult had null images but tracked the bounds the images would be) then
// onGetInputLayerBounds() is the union of all requested inputs at the leaf nodes of the DAG, and
// onGetOutputLayerBounds() is the bounds of the dry-run result. This might have more overhead, but
// would reduce the complexity of implementations by quite a bit.
skif::LayerSpace<SkIRect> SkCropImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Assuming unbounded desired output, this filter only needs to process an image that's at most
    // sized to our crop rect, but we can restrict the crop rect to just what's requested since
    // anything in the crop but outside 'desiredOutput' won't be visible.
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(mapping, desiredOutput);

    // Our required input is the desired output for our child image filter.
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkCropImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Assuming unbounded child content, our output is an image tiled around the crop rect.
    // But the child output image is drawn into our output surface with its own decal tiling, which
    // may allow the output dimensions to be reduced.
    auto childOutput = this->getChildOutputLayerBounds(0, mapping, contentBounds);

    skif::LayerSpace<SkIRect> crop = this->cropRect(mapping);
    if (childOutput && !crop.intersect(*childOutput)) {
        // Regardless of tile mode, the content within the crop rect is fully transparent, so
        // any tiling will maintain that transparency.
        return skif::LayerSpace<SkIRect>::Empty();
    } else {
        // The crop rect contains non-transparent content from the child filter; if not a decal
        // tile mode, the actual visual output is unbounded (even if the underlying data is smaller)
        if (fTileMode == SkTileMode::kDecal) {
            return crop;
        } else {
            return skif::LayerSpace<SkIRect>::Unbounded();
        }
    }
}

SkRect SkCropImageFilter::computeFastBounds(const SkRect& bounds) const {
    // TODO(michaelludwig) - This is conceptually very similar to calling onGetOutputLayerBounds()
    // with an identity skif::Mapping (hence why fCropRect can be used directly), but it also does
    // not involve any rounding to pixels for both the content bounds or the output.
    // NOTE: This relies on all image filters returning an infinite bounds when they affect
    // transparent black.
    SkRect inputBounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(bounds) : bounds;
    if (!inputBounds.intersect(SkRect(fCropRect))) {
        return SkRect::MakeEmpty();
    }
    return fTileMode == SkTileMode::kDecal ? inputBounds : SkRectPriv::MakeLargeS32();
}
