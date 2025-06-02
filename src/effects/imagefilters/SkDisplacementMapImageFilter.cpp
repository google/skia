/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkRuntimeEffect.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkKnownRuntimeEffects.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <optional>
#include <utility>

namespace {

class SkDisplacementMapImageFilter final : public SkImageFilter_Base {
    // Input image filter indices
    static constexpr int kDisplacement = 0;
    static constexpr int kColor = 1;

    // TODO(skbug.com/40045448): Use nearest to match historical behavior, but eventually this should
    // become a factory option.
    static constexpr SkSamplingOptions kDisplacementSampling{SkFilterMode::kNearest};

public:
    SkDisplacementMapImageFilter(SkColorChannel xChannel,
                                 SkColorChannel yChannel,
                                 SkScalar scale,
                                 sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2)
            , fXChannel(xChannel)
            , fYChannel(yChannel)
            , fScale(scale) {
        SkASSERT(SkIsFinite(fScale));
    }

    SkRect computeFastBounds(const SkRect& src) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    friend void ::SkRegisterDisplacementMapImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkDisplacementMapImageFilter)

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    std::optional<skif::LayerSpace<SkIRect>> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            std::optional<skif::LayerSpace<SkIRect>> contentBounds) const override;

    skif::LayerSpace<SkIRect> outsetByMaxDisplacement(const skif::Mapping& mapping,
                                                      skif::LayerSpace<SkIRect> bounds) const {
        // For max displacement, we treat 'scale' as a size instead of a vector. The vector offset
        // maps a [0,1] channel value to [-scale/2, scale/2], and treating it as a size
        // automatically accounts for the absolute magnitude when transforming from param to layer.
        skif::LayerSpace<SkSize> maxDisplacement = mapping.paramToLayer(
            skif::ParameterSpace<SkSize>({0.5f * fScale, 0.5f * fScale}));
        bounds.outset(maxDisplacement.ceil());
        return bounds;
    }

    SkColorChannel fXChannel;
    SkColorChannel fYChannel;
    // Scale is really a ParameterSpace<Vector> where width = height = fScale, but we store just the
    // float here for easier serialization and convert to a size in onFilterImage().
    SkScalar fScale;
};

bool channel_selector_type_is_valid(SkColorChannel cst) {
    switch (cst) {
        case SkColorChannel::kR:
        case SkColorChannel::kG:
        case SkColorChannel::kB:
        case SkColorChannel::kA:
            return true;
        default:
            break;
    }
    return false;
}

sk_sp<SkShader> make_displacement_shader(
        sk_sp<SkShader> displacement,
        sk_sp<SkShader> color,
        skif::LayerSpace<skif::Vector> scale,
        SkColorChannel xChannel,
        SkColorChannel yChannel) {
    if (!color) {
        // Color is fully transparent, so no point in displacing it
        return nullptr;
    }
    if (!displacement) {
        // Somehow we had a valid displacement image but failed to produce a shader
        // (e.g. an internal resolve to a new image failed). Treat the displacement as
        // transparent, but it's too late to switch to the applyTransform() optimization.
        displacement = SkShaders::Color(SK_ColorTRANSPARENT);
    }

    const SkRuntimeEffect* displacementEffect =
            GetKnownRuntimeEffect(SkKnownRuntimeEffects::StableKey::kDisplacement);

    auto channelSelector = [](SkColorChannel c) {
        return SkV4{c == SkColorChannel::kR ? 1.f : 0.f,
                    c == SkColorChannel::kG ? 1.f : 0.f,
                    c == SkColorChannel::kB ? 1.f : 0.f,
                    c == SkColorChannel::kA ? 1.f : 0.f};
    };

    SkRuntimeShaderBuilder builder(sk_ref_sp(displacementEffect));
    builder.child("displMap") = std::move(displacement);
    builder.child("colorMap") = std::move(color);
    builder.uniform("scale") = SkV2{scale.x(), scale.y()};
    builder.uniform("xSelect") = channelSelector(xChannel);
    builder.uniform("ySelect") = channelSelector(yChannel);

    return builder.makeShader();
}

}  // anonymous namespace

///////////////////////////////////////////////////////////////////////////////

sk_sp<SkImageFilter> SkImageFilters::DisplacementMap(
        SkColorChannel xChannelSelector, SkColorChannel yChannelSelector, SkScalar scale,
        sk_sp<SkImageFilter> displacement, sk_sp<SkImageFilter> color, const CropRect& cropRect) {
    if (!channel_selector_type_is_valid(xChannelSelector) ||
        !channel_selector_type_is_valid(yChannelSelector)) {
        return nullptr;
    }
    if (!SkIsFinite(scale)) {
        return nullptr;
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(displacement), std::move(color) };
    sk_sp<SkImageFilter> filter(new SkDisplacementMapImageFilter(xChannelSelector, yChannelSelector,
                                                                 scale, inputs));
    if (cropRect) {
        filter = SkImageFilters::Crop(*cropRect, std::move(filter));
    }
    return filter;
}

void SkRegisterDisplacementMapImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkDisplacementMapImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkDisplacementMapEffect", SkDisplacementMapImageFilter::CreateProc);
    SkFlattenable::Register("SkDisplacementMapEffectImpl",
                            SkDisplacementMapImageFilter::CreateProc);
}

sk_sp<SkFlattenable> SkDisplacementMapImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    SkColorChannel xsel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkColorChannel ysel = buffer.read32LE(SkColorChannel::kLastEnum);
    SkScalar      scale = buffer.readScalar();

    return SkImageFilters::DisplacementMap(xsel, ysel, scale, common.getInput(0),
                                           common.getInput(1), common.cropRect());
}

void SkDisplacementMapImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    buffer.writeInt((int) fXChannel);
    buffer.writeInt((int) fYChannel);
    buffer.writeScalar(fScale);
}

///////////////////////////////////////////////////////////////////////////////

skif::FilterResult SkDisplacementMapImageFilter::onFilterImage(const skif::Context& ctx) const {
    skif::LayerSpace<SkIRect> requiredColorInput =
            this->outsetByMaxDisplacement(ctx.mapping(), ctx.desiredOutput());
    skif::FilterResult colorOutput =
            this->getChildOutput(kColor, ctx.withNewDesiredOutput(requiredColorInput));
    if (!colorOutput) {
        return {}; // No non-transparent black colors to displace
    }

    // When the color image filter is unrestricted, its output will be 'maxDisplacement' larger than
    // this filter's desired output. However, if it is cropped, we can restrict this filter's final
    // output. However it's not simply colorOutput intersected with desiredOutput since we have to
    // account for how the clipped colorOutput might still be displaced.
    skif::LayerSpace<SkIRect> outputBounds =
            this->outsetByMaxDisplacement(ctx.mapping(), colorOutput.layerBounds());
    // 'outputBounds' has double the max displacement for edges where colorOutput had not been
    // clipped, but that's fine since we intersect with 'desiredOutput'. For edges that were cropped
    // the second max displacement represents how far they can be displaced, which might be inside
    // the original 'desiredOutput'.
    if (!outputBounds.intersect(ctx.desiredOutput())) {
        // None of the non-transparent black colors can be displaced into the desired bounds.
        return {};
    }

    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    //
    //   User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    //   end up filtering the displacement map into that gamut, which has the effect of reducing
    //   the amount of displacement that it represents (as encoded values move away from the
    //   primaries).
    //
    //   With a more complex DAG attached to this input, it's not clear that working in ANY specific
    //   color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    //   ideal, but it's at least consistent and predictable.
    skif::FilterResult displacementOutput =
            this->getChildOutput(kDisplacement, ctx.withNewDesiredOutput(outputBounds)
                                                   .withNewColorSpace(/*cs=*/nullptr));

    // NOTE: The scale is a "vector" not a "size" since we want to preserve negations on the final
    // displacement vector.
    const skif::LayerSpace<skif::Vector> scale =
            ctx.mapping().paramToLayer(skif::ParameterSpace<skif::Vector>({fScale, fScale}));
    if (!displacementOutput) {
        // A null displacement map means its transparent black, but (0,0,0,0) becomes the vector
        // (-scale/2, -scale/2) applied to the color image, so represent the displacement as a
        // simple transform.
        skif::LayerSpace<SkMatrix> constantDisplacement{SkMatrix::Translate(-0.5f * scale.x(),
                                                                            -0.5f * scale.y())};
        return colorOutput.applyTransform(ctx, constantDisplacement, kDisplacementSampling);
    }

    // If we made it this far, then we actually have per-pixel displacement affecting the color
    // image. We need to evaluate each pixel within 'outputBounds'.
    using ShaderFlags = skif::FilterResult::ShaderFlags;

    skif::FilterResult::Builder builder{ctx};
    builder.add(displacementOutput, /*sampleBounds=*/outputBounds);
    builder.add(colorOutput,
                /*sampleBounds=*/requiredColorInput,
                ShaderFlags::kNonTrivialSampling,
                kDisplacementSampling);
    return builder.eval(
            [&](SkSpan<sk_sp<SkShader>> inputs) {
                return make_displacement_shader(inputs[kDisplacement], inputs[kColor],
                                                scale, fXChannel, fYChannel);
            }, outputBounds);
}

skif::LayerSpace<SkIRect> SkDisplacementMapImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    // Pixels up to the maximum displacement away from 'desiredOutput' can be moved into those
    // bounds, depending on how the displacement map renders. To ensure those colors are defined,
    // we require that outset buffer around 'desiredOutput' from the color map.
    skif::LayerSpace<SkIRect> requiredInput = this->outsetByMaxDisplacement(mapping, desiredOutput);
    requiredInput = this->getChildInputLayerBounds(kColor, mapping, requiredInput, contentBounds);

    // Accumulate the required input for the displacement filter to cover the original desired out
    requiredInput.join(this->getChildInputLayerBounds(
            kDisplacement, mapping, desiredOutput, contentBounds));
    return requiredInput;
}

std::optional<skif::LayerSpace<SkIRect>> SkDisplacementMapImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        std::optional<skif::LayerSpace<SkIRect>> contentBounds) const {
    auto colorOutput = this->getChildOutputLayerBounds(kColor, mapping, contentBounds);
    if (colorOutput) {
        return this->outsetByMaxDisplacement(mapping, *colorOutput);
    } else {
        return skif::LayerSpace<SkIRect>::Unbounded();
    }
}

SkRect SkDisplacementMapImageFilter::computeFastBounds(const SkRect& src) const {
    SkRect colorBounds = this->getInput(kColor) ? this->getInput(kColor)->computeFastBounds(src)
                                                : src;
    float maxDisplacement = 0.5f * SkScalarAbs(fScale);
    return colorBounds.makeOutset(maxDisplacement, maxDisplacement);
}
