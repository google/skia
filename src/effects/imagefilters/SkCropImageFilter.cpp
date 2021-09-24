/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/effects/imagefilters/SkCropImageFilter.h"

#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkValidationUtils.h"

namespace {

class SkCropImageFilter final : public SkImageFilter_Base {
public:
    SkCropImageFilter(const SkRect& cropRect, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1, /*cropRect=*/nullptr)
            , fCropRect(cropRect) {
        SkASSERT(cropRect.isFinite());
        SkASSERT(cropRect.isSorted());
        SkASSERT(!cropRect.isEmpty());
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
            const skif::LayerSpace<SkIRect>& contentBounds,
            VisitChildren recurse) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    // The crop rect is specified in floating point to allow cropping to partial local pixels,
    // that could become whole pixels in the layer-space image if the canvas is scaled.
    // For now it's always rounded to integer pixels as if it were non-AA.
    skif::LayerSpace<SkIRect> cropRect(const skif::Mapping& mapping) const {
        // TODO(michaelludwig) legacy code used roundOut() in applyCropRect(). If image diffs are
        // incorrect when migrating to this filter, this may need to be adjusted.
        return mapping.paramToLayer(fCropRect).round();
    }

    skif::ParameterSpace<SkRect> fCropRect;
};

} // end namespace

sk_sp<SkImageFilter> SkMakeCropImageFilter(const SkRect& rect, sk_sp<SkImageFilter> input) {
    if (rect.isEmpty() || !rect.isFinite()) {
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
    skif::LayerSpace<SkIRect> cropBounds = this->cropRect(context.mapping());
    // Limit our crop to just what is necessary for the next stage in the filter pipeline.
    if (!cropBounds.intersect(context.desiredOutput())) {
        // The output is fully transparent so skip evaluating the child, although in most cases this
        // is detected earlier based on getInputLayerBounds() and the entire DAG can be skipped.
        // That's not always possible when a parent filter combines a dynamic layer with image
        // filters that produce fixed outputs (i.e. source filters).
        return {};
    }
    skif::FilterResult childOutput = this->filterInput(0, context);
    // While filterInput() adjusts the context passed to our child filter to account for the
    // crop rect and desired output, 'childOutput' does not necessarily fit that exactly. An
    // explicit resolve to these bounds ensures the crop is applied and the result is as small as
    // possible, and in most cases does not require rendering a new image.
    // NOTE - for now, with decal-only tiling, it actually NEVER requires rendering a new image.
    return childOutput.resolveToBounds(cropBounds);
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
        const skif::LayerSpace<SkIRect>& contentBounds,
        VisitChildren recurse) const {
    // Assuming unbounded desired output, this filter only needs to process an image that's at most
    // sized to our crop rect.
    skif::LayerSpace<SkIRect> requiredInput = this->cropRect(mapping);
    // But we can restrict the crop rect to just what's requested, since anything beyond that won't
    // be rendered.
    if (!requiredInput.intersect(desiredOutput)) {
        // We wouldn't draw anything when filtering, so return empty bounds now to skip a layer.
        return skif::LayerSpace<SkIRect>::Empty();
    }

    if (recurse == VisitChildren::kNo) {
        return requiredInput;
    } else {
        // Our required input is the desired output for our child image filter.
        return this->visitInputLayerBounds(mapping, requiredInput, contentBounds);
    }
}

skif::LayerSpace<SkIRect> SkCropImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Assuming unbounded child content, our output is a decal-tiled image sized to our crop rect.
    skif::LayerSpace<SkIRect> output = this->cropRect(mapping);
    // But the child output image is drawn into our output surface with its own decal tiling, which
    // may allow the output dimensions to be reduced.
    skif::LayerSpace<SkIRect> childOutput = this->visitOutputLayerBounds(mapping, contentBounds);

    if (output.intersect(childOutput)) {
        return output;
    } else {
        // Nothing would be drawn into our crop rect, so nothing would be output.
        return skif::LayerSpace<SkIRect>::Empty();
    }
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
