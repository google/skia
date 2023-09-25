/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <utility>

namespace {

enum class MorphType {
    kErode,
    kDilate,
    kLastType = kDilate
};

enum class MorphDirection { kX, kY };

class SkMorphologyImageFilter final : public SkImageFilter_Base {
public:
    SkMorphologyImageFilter(MorphType type, SkSize radii, sk_sp<SkImageFilter> input)
            : SkImageFilter_Base(&input, 1)
            , fType(type)
            , fRadii(radii) {}

    SkRect computeFastBounds(const SkRect& src) const override;


protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterMorphologyImageFilterFlattenables();
    SK_FLATTENABLE_HOOKS(SkMorphologyImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::LayerSpace<SkISize> radii(const skif::Mapping& mapping) const {
        skif::LayerSpace<SkISize> radii = mapping.paramToLayer(fRadii).round();
        SkASSERT(radii.width() >= 0 && radii.height() >= 0);

        // We limit the radius to something small, to avoid slow draw calls: crbug.com/1123035
        static constexpr int kMaxRadii = 256;
        return skif::LayerSpace<SkISize>({std::min(radii.width(), kMaxRadii),
                                          std::min(radii.height(), kMaxRadii)});
    }

    skif::LayerSpace<SkIRect> requiredInput(const skif::Mapping& mapping,
                                            skif::LayerSpace<SkIRect> bounds) const {
        // The input for a morphology filter is always the kernel outset, regardless of morph type.
        bounds.outset(this->radii(mapping));
        return bounds;
    }

    skif::LayerSpace<SkIRect> kernelOutputBounds(const skif::Mapping& mapping,
                                                 skif::LayerSpace<SkIRect> bounds) const {
        skif::LayerSpace<SkISize> radii = this->radii(mapping);
        if (fType == MorphType::kDilate) {
            // Transparent pixels up to the kernel radius away will be overridden by kDilate's "max"
            // function and be set to the input's boundary pixel colors, thus expanding the output.
            bounds.outset(radii);
        } else {
            // Pixels closer than the kernel radius to the input image's edges are overridden by
            // kErode's "min" function and will be set to transparent black, contracting the output.
            bounds.inset(radii);
        }
        return bounds;
    }

    MorphType fType;
    skif::ParameterSpace<SkSize> fRadii;
};

sk_sp<SkImageFilter> make_morphology(MorphType type,
                                     SkSize radii,
                                     sk_sp<SkImageFilter> input,
                                     const SkImageFilters::CropRect& cropRect) {
    if (radii.width() < 0.f || radii.height() < 0.f) {
        return nullptr; // invalid
    }
    sk_sp<SkImageFilter> filter = std::move(input);
    if (radii.width() > 0.f || radii.height() > 0.f) {
        filter = sk_sp<SkImageFilter>(new SkMorphologyImageFilter(type, radii, std::move(filter)));
    }
    // otherwise both radii are 0, so the kernel is always the identity function, in which case
    // we just need to apply the 'cropRect' to the 'input'.

    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, std::move(filter));
    }
    return filter;
}

// The linear morphology kernel does (2R+1) texture samples per pixel, which we want to keep less
// than the maximum fragment samples allowed in DX9SM2 (32), so we choose R=14 to have some head
// room. The other tradeoff is that for R > kMaxLinearRadius, the sparse morphology kernel only
// requires 2 samples to double the accumulated kernel size, but at the cost of another render
// target.
static constexpr int kMaxLinearRadius = 14;
sk_sp<SkShader> make_linear_morphology(sk_sp<SkShader> input,
                                       MorphType type,
                                       MorphDirection direction,
                                       int radius) {
    SkASSERT(radius <= kMaxLinearRadius);
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
            "const int kMaxLinearRadius = 14;" // KEEP IN SYNC WITH ABOVE DEFINITION

            "uniform shader child;"
            "uniform half2 offset;"
            "uniform half flip;" // -1 converts the max() calls to min()
            "uniform int radius;"

            "half4 main(float2 coord) {"
                "half4 aggregate = flip*child.eval(coord);" // case 0 only samples once
                "for (int i = 1; i <= kMaxLinearRadius; ++i) {"
                    "if (i > radius) break;"
                    "half2 delta = half(i) * offset;"
                    "aggregate = max(aggregate, max(flip*child.eval(coord + delta),"
                                                   "flip*child.eval(coord - delta)));"
                "}"
                "return flip*aggregate;"
            "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("child") = std::move(input);
    builder.uniform("offset") = direction == MorphDirection::kX ? SkV2{1.f, 0.f} : SkV2{0.f, 1.f};
    builder.uniform("flip") = (type == MorphType::kDilate) ? 1.f : -1.f;
    builder.uniform("radius") = (int32_t) radius;

    return builder.makeShader();
}

// Assuming 'input' was created by a series of morphology passes, each texel holds the aggregate
// (min or max depending on type) of (i-R) to (i+R) for some R. If 'radius' <= R, then the returned
// shader produces a new aggregate at each texel, i, of (i-R-radius) to (i+R+radius) with only two
// texture samples, which can be used to double the kernel size of the morphology effect.
sk_sp<SkShader> make_sparse_morphology(sk_sp<SkShader> input,
                                       MorphType type,
                                       MorphDirection direction,
                                       int radius) {
    static const SkRuntimeEffect* effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader,
        "uniform shader child;"
        "uniform half2 offset;"
        "uniform half flip;"

        "half4 main(float2 coord) {"
            "half4 aggregate = max(flip*child.eval(coord + offset),"
                                  "flip*child.eval(coord - offset));"
            "return flip*aggregate;"
        "}");

    SkRuntimeShaderBuilder builder(sk_ref_sp(effect));
    builder.child("child") = std::move(input);
    builder.uniform("offset") = direction == MorphDirection::kX ? SkV2{(float)radius, 0.f}
                                                                : SkV2{0.f, (float)radius};
    builder.uniform("flip") = (type == MorphType::kDilate) ? 1.f : -1.f;

    return builder.makeShader();
}

skif::FilterResult morphology_pass(const skif::Context& ctx, const skif::FilterResult& input,
                                   MorphType type, MorphDirection dir, int radius) {
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    auto axisDelta = [dir](int step) {
        return skif::LayerSpace<SkISize>({
                dir == MorphDirection::kX ? step : 0,
                dir == MorphDirection::kY ? step : 0});
    };

    // The first iteration will sample a full kernel outset from the final output.
    skif::LayerSpace<SkIRect> sampleBounds = ctx.desiredOutput();
    sampleBounds.outset(axisDelta(radius));

    skif::FilterResult childOutput = input;
    int appliedRadius = 0;
    while (radius > appliedRadius) {
        if (!childOutput) {
            return {}; // Eroded or dilated transparent black is still transparent black
        }

        // The first iteration uses up to kMaxLinearRadius with a linear accumulation pass.
        // After that we double the radius each step until we can finish with the target radius.
        int stepRadius =
                appliedRadius == 0 ? std::min(kMaxLinearRadius, radius)
                                   : std::min(radius - appliedRadius, appliedRadius);

        skif::Context stepCtx = ctx;
        if (appliedRadius + stepRadius < radius) {
            // Intermediate steps need to output what will be sampled on the next iteration
            auto outputBounds = sampleBounds;
            outputBounds.inset(axisDelta(stepRadius));
            stepCtx = ctx.withNewDesiredOutput(outputBounds);
        } // else the last iteration should output what was originally requested

        skif::FilterResult::Builder builder{stepCtx};
        builder.add(childOutput, sampleBounds, ShaderFlags::kSampledRepeatedly);
        childOutput = builder.eval(
                [&](SkSpan<sk_sp<SkShader>> inputs) {
                    if (appliedRadius == 0) {
                        return make_linear_morphology(inputs[0], type, dir, stepRadius);
                    } else {
                        return make_sparse_morphology(inputs[0], type, dir, stepRadius);
                    }
                });

        sampleBounds = stepCtx.desiredOutput();
        appliedRadius += stepRadius;
        SkASSERT(appliedRadius <= radius); // Our last iteration should hit 'radius' exactly.
    }

    return childOutput;
}

} // end namespace

sk_sp<SkImageFilter> SkImageFilters::Dilate(SkScalar radiusX, SkScalar radiusY,
                                            sk_sp<SkImageFilter> input,
                                            const CropRect& cropRect) {
    return make_morphology(MorphType::kDilate, {radiusX, radiusY}, std::move(input), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Erode(SkScalar radiusX, SkScalar radiusY,
                                           sk_sp<SkImageFilter> input,
                                           const CropRect& cropRect) {
    return make_morphology(MorphType::kErode, {radiusX, radiusY}, std::move(input), cropRect);
}

void SkRegisterMorphologyImageFilterFlattenables() {
    SK_REGISTER_FLATTENABLE(SkMorphologyImageFilter);
    // TODO (michaelludwig): Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkMorphologyImageFilterImpl", SkMorphologyImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkMorphologyImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    SkScalar width = buffer.readScalar();
    SkScalar height = buffer.readScalar();
    MorphType filterType = buffer.read32LE(MorphType::kLastType);

    if (filterType == MorphType::kDilate) {
        return SkImageFilters::Dilate(width, height, common.getInput(0), common.cropRect());
    } else if (filterType == MorphType::kErode) {
        return SkImageFilters::Erode(width, height, common.getInput(0), common.cropRect());
    } else {
        return nullptr;
    }
}

void SkMorphologyImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeScalar(SkSize(fRadii).width());
    buffer.writeScalar(SkSize(fRadii).height());
    buffer.writeInt(static_cast<int>(fType));
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkMorphologyImageFilter::onFilterImage(const skif::Context& ctx) const {
    skif::LayerSpace<SkIRect> requiredInput =
            this->requiredInput(ctx.mapping(), ctx.desiredOutput());
    skif::FilterResult childOutput =
            this->getChildOutput(0, ctx.withNewDesiredOutput(requiredInput));

    // If childOutput completely fulfilled requiredInput, maxOutput will match the context's
    // desired output, but if the output image is smaller, this will restrict the morphology output
    // to what is actual produceable.
    skif::LayerSpace<SkIRect> maxOutput =
        this->kernelOutputBounds(ctx.mapping(), childOutput.layerBounds());
    if (!maxOutput.intersect(ctx.desiredOutput())) {
        return {};
    }

    // The X pass has to preserve the extra rows to later be consumed by the Y pass.
    skif::LayerSpace<SkISize> radii = this->radii(ctx.mapping());
    skif::LayerSpace<SkIRect> maxOutputX = maxOutput;
    maxOutputX.outset(skif::LayerSpace<SkISize>({0, radii.height()}));
    childOutput = morphology_pass(ctx.withNewDesiredOutput(maxOutputX), childOutput, fType,
                                  MorphDirection::kX, radii.width());
    childOutput = morphology_pass(ctx.withNewDesiredOutput(maxOutput), childOutput, fType,
                                  MorphDirection::kY, radii.height());
    return childOutput;
}

skif::LayerSpace<SkIRect> SkMorphologyImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    skif::LayerSpace<SkIRect> requiredInput = this->requiredInput(mapping, desiredOutput);
    return this->getChildInputLayerBounds(0, mapping, requiredInput, contentBounds);
}

std::optional<skif::LayerSpace<SkIRect>> SkMorphologyImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    auto childOutput = this->getChildOutputLayerBounds(0, mapping, contentBounds);
    if (childOutput) {
        return this->kernelOutputBounds(mapping, *childOutput);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}

SkRect SkMorphologyImageFilter::computeFastBounds(const SkRect& src) const {
    // See kernelOutputBounds() for rationale
    SkRect bounds = this->getInput(0) ? this->getInput(0)->computeFastBounds(src) : src;
    if (fType == MorphType::kDilate) {
        bounds.outset(SkSize(fRadii).width(), SkSize(fRadii).height());
    } else {
        bounds.inset(SkSize(fRadii).width(), SkSize(fRadii).height());
    }
    return bounds;
}
