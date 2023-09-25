/*
 * Copyright 2014 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkImageFilters.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

struct SkISize;

namespace {

class SkMatrixTransformImageFilter final : public SkImageFilter_Base {
public:
    // TODO(michaelludwig): Update this to use SkM44.
    SkMatrixTransformImageFilter(const SkMatrix& transform,
                                 const SkSamplingOptions& sampling,
                                 sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fTransform(transform)
            , fSampling(sampling) {
        // Pre-cache so future calls to fTransform.getType() are threadsafe.
        (void) static_cast<const SkMatrix&>(fTransform).getType();
    }

    SkRect computeFastBounds(const SkRect&) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMatrixTransformImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixTransformImageFilter)
    static sk_sp<SkFlattenable> LegacyOffsetCreateProc(SkReadBuffer& buffer);

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    skif::FilterResult onFilterImage(const skif::Context& context) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::LayerSpace<SkIRect> requiredInput(const skif::Mapping& mapping,
                                            const skif::LayerSpace<SkIRect>& desiredOutput) const;

    skif::ParameterSpace<SkMatrix> fTransform;
    SkSamplingOptions fSampling;
};

} // namespace

sk_sp<SkImageFilter> SkImageFilters::MatrixTransform(const SkMatrix& transform,
                                                     const SkSamplingOptions& sampling,
                                                     sk_sp<SkImageFilter> input) {
    return sk_sp<SkImageFilter>(new SkMatrixTransformImageFilter(transform,
                                                                 sampling,
                                                                 std::move(input)));
}

sk_sp<SkImageFilter> SkImageFilters::Offset(SkScalar dx, SkScalar dy,
                                            sk_sp<SkImageFilter> input,
                                            const CropRect& cropRect) {
    // The legacy ::Offset() implementation rounded its offset vector to layer-space pixels, which
    // is roughly equivalent to using nearest-neighbor sampling with the translation matrix.
    sk_sp<SkImageFilter> offset = SkImageFilters::MatrixTransform(
            SkMatrix::Translate(dx, dy),
            SkFilterMode::kNearest,
            std::move(input));
    // The legacy 'cropRect' applies only to the output of the offset filter.
    if (cropRect) {
        offset = SkImageFilters::Crop(*cropRect, std::move(offset));
    }
    return offset;
}

void SkRegisterMatrixTransformImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkMatrixTransformImageFilter);
    // TODO(michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMatrixImageFilter", SkMatrixTransformImageFilter::CreateProc);
    // TODO(michaelludwig): Remove after grace period for SKPs to stop using old serialization
    SkFlattenable::Register("SkOffsetImageFilter",
                            SkMatrixTransformImageFilter::LegacyOffsetCreateProc);
    SkFlattenable::Register("SkOffsetImageFilterImpl",
                            SkMatrixTransformImageFilter::LegacyOffsetCreateProc);
}

sk_sp<SkFlattenable> SkMatrixTransformImageFilter::LegacyOffsetCreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkPoint offset;
    buffer.readPoint(&offset);
    return SkImageFilters::Offset(offset.x(), offset.y(), common.getInput(0), common.cropRect());
}

sk_sp<SkFlattenable> SkMatrixTransformImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkMatrix matrix;
    buffer.readMatrix(&matrix);

    auto sampling = [&]() {
        if (buffer.isVersionLT(SkPicturePriv::kMatrixImageFilterSampling_Version)) {
            return SkSamplingPriv::FromFQ(buffer.read32LE(kLast_SkLegacyFQ), kLinear_SkMediumAs);
        } else {
            return buffer.readSampling();
        }
    }();
    return SkImageFilters::MatrixTransform(matrix, sampling, common.getInput(0));
}

void SkMatrixTransformImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeMatrix(SkMatrix(fTransform));
    buffer.writeSampling(fSampling);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkMatrixTransformImageFilter::onFilterImage(const skif::Context& context) const {
    skif::LayerSpace<SkIRect> requiredInput =
            this->requiredInput(context.mapping(), context.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, context.withNewDesiredOutput(requiredInput));

    skif::LayerSpace<SkMatrix> transform = context.mapping().paramToLayer(fTransform);
    return childOutput.applyTransform(context, transform, fSampling);
}

SkRect SkMatrixTransformImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    return static_cast<const SkMatrix&>(fTransform).mapRect(bounds);
}

skif::LayerSpace<SkIRect> SkMatrixTransformImageFilter::requiredInput(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput) const {
    // The required input for this filter to cover 'desiredOutput' is the smallest rectangle such
    // that after being transformed by the layer-space adjusted 'fTransform', it contains the output
    skif::LayerSpace<SkIRect> requiredInput;
    if (!mapping.paramToLayer(fTransform).inverseMapRect(desiredOutput, &requiredInput)) {
        return skif::LayerSpace<SkIRect>::Empty();
    }

    // Additionally if there is any filtering beyond nearest neighbor, we request an extra buffer of
    // pixels so that the content is available to the bilerp/bicubic kernel.
    if (fSampling != SkSamplingOptions()) {
        requiredInput.outset(skif::LayerSpace<SkISize>({1, 1}));
    }
    return requiredInput;
}


skif::LayerSpace<SkIRect> SkMatrixTransformImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Our required input is the desired output for our child image filter.
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(mapping, desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkMatrixTransformImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // The output of this filter is the transformed bounds of its child's output.
    auto childOutput = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (childOutput) {
        return mapping.paramToLayer(fTransform).mapRect(*childOutput);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}
