/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/imagefilters/SkCropImageFilter.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkRect.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkValidationUtils.h"
#include "src/core/SkWriteBuffer.h"

#include <utility>

namespace {

class SkCropImageFilter final : public SkImageFilter_Base {
public:
    SkCropImageFilter(const SkRect& cropRect, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, /*cropRect=*/nullptr)
            , fCropRect(cropRect) {
        SkASSERT(cropRect.isFinite());
        SkASSERT(cropRect.isSorted());
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterCropImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkCropImageFilter)

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    // The crop rect is specified in floating point to allow cropping to partial local pixels,
    // that could become whole pixels in the layer-space image if the canvas is scaled.
    // For now it's always rounded to integer pixels as if it were non-AA.
    //
    // The returned rect is intersected with 'outputBounds', which is either the desired or
    // actual bounds of the child filter.
    skif::LayerSpace<SkIRect> cropRect(const skif::Mapping& mapping,
                                       const skif::LayerSpace<SkIRect>& outputBounds) const {
        auto crop = mapping.paramToLayer(fCropRect).roundOut();
        if (!crop.intersect(outputBounds)) {
            return skif::LayerSpace<SkIRect>::Empty();
        } else {
            return crop;
        }
    }

    skif::ParameterSpace<SkRect> fCropRect;
};

} // end namespace

sk_sp<SkImageFilter> SkMakeCropImageFilter(const SkRect& rect, sk_sp<SkImageFilter> input) {
    if (!rect.isFinite()) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkCropImageFilter(rect, std::move(input)));
}

void SkRegisterCropImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkCropImageFilter);
}

sk_sp<SkFlattenable> SkCropImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkRect cropRect = buffer.readRect();
    if (!buffer.isValid() || !buffer.validate(SkIsValidRect(cropRect))) {
        return nullptr;
    }
    return SkMakeCropImageFilter(cropRect, common.getInput(0));
}

void SkCropImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeRect(SkRect(fCropRect));
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkCropImageFilter::onFilterImage(const skif::Context& context) const {
    skif::LayerSpace<SkIRect> cropBounds =
            this->cropRect(context.mapping(), context.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, context.withNewDesiredOutput(cropBounds));

    // While the child filter may have exactly matched the requested 'cropBounds', it's not
    // necessarily the case, so applyCrop() ensures this is true while avoiding rendering a new
    // when possible.
    return childOutput.applyCrop(context, cropBounds);
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
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Assuming unbounded desired output, this filter only needs to process an image that's at most
    // sized to our crop rect, but we can restrict the crop rect to just what's requested since
    // anything in the crop but outside 'desiredOutput' won't be visible.
    skif::LayerSpace<SkIRect> requiredInput = this->cropRect(mapping, desiredOutput);

    // Our required input is the desired output for our child image filter.
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

skif::LayerSpace<SkIRect> SkCropImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Assuming unbounded child content, our output is a decal-tiled image sized to our crop rect.
    // But the child output image is drawn into our output surface with its own decal tiling, which
    // may allow the output dimensions to be reduced.
    skif::LayerSpace<SkIRect> childOutput =
            this->getChildOutputLayerBounds(0, mapping, contentBounds);
    return this->cropRect(mapping, childOutput);
}

SkRect SkCropImageFilter::computeFastBounds(const SkRect& bounds) const {
    // TODO(michaelludwig) - This is conceptually very similar to calling onGetOutputLayerBounds()
    // with an identity skif::Mapping (hence why fCropRect can be used directly), but it also does
    // not involve any rounding to pixels for both the content bounds or the output.
    // FIXME(michaelludwig) - There is a limitation in the current system for "fast bounds", since
    // there's no way for the crop image filter to hide the fact that a child affects transparent
    // black, so the entire DAG still is treated as if it cannot compute fast bounds. If we migrate
    // getOutputLayerBounds() to operate on float rects, and to report infinite bounds for
    // nodes that affect transparent black, then fastBounds() and onAffectsTransparentBlack() impls
    // can go away entirely. That's not feasible until everything else is migrated onto the new crop
    // rect filter and the new APIs.
    if (this->getInput(0) && !this->getInput(0)->canComputeFastBounds()) {
        // The input bounds to the crop are effectively infinite so the output fills the crop rect.
        return SkRect(fCropRect);
    }

    SkRect inputBounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(bounds) : bounds;
    if (!inputBounds.intersect(SkRect(fCropRect))) {
        return SkRect::MakeEmpty();
    }
    return inputBounds;
}
