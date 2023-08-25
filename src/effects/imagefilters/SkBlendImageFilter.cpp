/*
 * Copyright 2013 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/effects/SkImageFilters.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlender.h"
#include "include/core/SkColor.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkM44.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkShader.h"
#include "include/core/SkTypes.h"
#include "include/effects/SkBlenders.h"
#include "include/private/base/SkSpan_impl.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlenderBase.h"
#include "src/core/SkImageFilterTypes.h"
#include "src/core/SkImageFilter_Base.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkWriteBuffer.h"
#include "src/effects/imagefilters/SkCropImageFilter.h"

#include <cstdint>
#include <optional>
#include <utility>

namespace {

class SkBlendImageFilter : public SkImageFilter_Base {
    // Input image filter indices
    static constexpr int kBackground = 0;
    static constexpr int kForeground = 1;

public:
    SkBlendImageFilter(sk_sp<SkBlender> blender,
                       const std::optional<SkV4>& coefficients,
                       bool enforcePremul,
                       sk_sp<SkImageFilter> inputs[2])
            : SkImageFilter_Base(inputs, 2, nullptr)
            , fBlender(std::move(blender))
            , fArithmeticCoefficients(coefficients)
            , fEnforcePremul(enforcePremul) {
        // A null blender represents src-over, which should have been filled in by the factory
        SkASSERT(fBlender);
    }

    SkRect computeFastBounds(const SkRect& bounds) const override;

protected:
    void flatten(SkWriteBuffer&) const override;

private:
    static constexpr uint32_t kArithmetic_SkBlendMode = kCustom_SkBlendMode + 1;

    friend void ::SkRegisterBlendImageFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkBlendImageFilter)
    static sk_sp<SkFlattenable> LegacyArithmeticCreateProc(SkReadBuffer& buffer);

    MatrixCapability onGetCTMCapability() const override { return MatrixCapability::kComplex; }

    bool onAffectsTransparentBlack() const override {
        // An arbitrary runtime blender or an arithmetic runtime blender with k3 != 0 affects
        // transparent black.
        return !as_BB(fBlender)->asBlendMode().has_value() &&
               (!fArithmeticCoefficients.has_value() || (*fArithmeticCoefficients)[3] != 0.f);
    }

    skif::FilterResult onFilterImage(const skif::Context&) const override;

    skif::LayerSpace<SkIRect> onGetInputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& desiredOutput,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    skif::LayerSpace<SkIRect> onGetOutputLayerBounds(
            const skif::Mapping& mapping,
            const skif::LayerSpace<SkIRect>& contentBounds) const override;

    sk_sp<SkShader> makeBlendShader(sk_sp<SkShader> bg, sk_sp<SkShader> fg) const;

    sk_sp<SkBlender> fBlender;

    // Normally runtime SkBlenders are pessimistic about the bounds they affect. For Arithmetic,
    // we remember the coefficients so that bounds can be reasoned about.
    std::optional<SkV4> fArithmeticCoefficients;
    bool fEnforcePremul; // Remembered to serialize the Arithmetic variant correctly
};

sk_sp<SkImageFilter> make_blend(sk_sp<SkBlender> blender,
                                sk_sp<SkImageFilter> background,
                                sk_sp<SkImageFilter> foreground,
                                const SkImageFilters::CropRect& cropRect,
                                std::optional<SkV4> coefficients = {},
                                bool enforcePremul = false) {
    if (!blender) {
        blender = SkBlender::Mode(SkBlendMode::kSrcOver);
    }

    auto cropped = [cropRect](sk_sp<SkImageFilter> filter) {
        if (cropRect) {
            filter = SkMakeCropImageFilter(*cropRect, std::move(filter));
        }
        return filter;
    };

    if (auto bm = as_BB(blender)->asBlendMode()) {
        if (bm == SkBlendMode::kSrc) {
            return cropped(std::move(foreground));
        } else if (bm == SkBlendMode::kDst) {
            return cropped(std::move(background));
        } else if (bm == SkBlendMode::kClear) {
            return SkImageFilters::Empty();
        }
    }

    sk_sp<SkImageFilter> inputs[2] = { std::move(background), std::move(foreground) };
    sk_sp<SkImageFilter> filter{new SkBlendImageFilter(blender, coefficients,
                                                       enforcePremul, inputs)};
    return cropped(std::move(filter));
}

} // anonymous namespace

sk_sp<SkImageFilter> SkImageFilters::Blend(SkBlendMode mode,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(SkBlender::Mode(mode),
                      std::move(background),
                      std::move(foreground),
                      cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Blend(sk_sp<SkBlender> blender,
                                           sk_sp<SkImageFilter> background,
                                           sk_sp<SkImageFilter> foreground,
                                           const CropRect& cropRect) {
    return make_blend(std::move(blender), std::move(background), std::move(foreground), cropRect);
}

sk_sp<SkImageFilter> SkImageFilters::Arithmetic(SkScalar k1,
                                                SkScalar k2,
                                                SkScalar k3,
                                                SkScalar k4,
                                                bool enforcePMColor,
                                                sk_sp<SkImageFilter> background,
                                                sk_sp<SkImageFilter> foreground,
                                                const CropRect& cropRect) {
    auto blender = SkBlenders::Arithmetic(k1, k2, k3, k4, enforcePMColor);
    if (!blender) {
        // Arithmetic() returns null on an error, not to optimize src-over
        return nullptr;
    }
    return make_blend(std::move(blender),
                      std::move(background),
                      std::move(foreground),
                      cropRect,
                      // Carry arithmetic coefficients and premul behavior into image filter for
                      // serialization and bounds analysis
                      SkV4{k1, k2, k3, k4},
                      enforcePMColor);
}

void SkRegisterBlendImageFilterFlattenable() {
    SK_REGISTER_FLATTENABLE(SkBlendImageFilter);
    // TODO (michaelludwig) - Remove after grace period for SKPs to stop using old name
    SkFlattenable::Register("SkXfermodeImageFilter_Base", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("SkXfermodeImageFilterImpl", SkBlendImageFilter::CreateProc);
    SkFlattenable::Register("ArithmeticImageFilterImpl",
                            SkBlendImageFilter::LegacyArithmeticCreateProc);
    SkFlattenable::Register("SkArithmeticImageFilter",
                            SkBlendImageFilter::LegacyArithmeticCreateProc);
}

sk_sp<SkFlattenable> SkBlendImageFilter::LegacyArithmeticCreateProc(SkReadBuffer& buffer) {
    // Newer SKPs should be using the updated Blend CreateProc.
    if (!buffer.validate(buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
        SkASSERT(false); // debug-only, so release will just see a failed deserialization
        return nullptr;
    }

    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);
    float k[4];
    for (int i = 0; i < 4; ++i) {
        k[i] = buffer.readScalar();
    }
    const bool enforcePremul = buffer.readBool();
    return SkImageFilters::Arithmetic(k[0], k[1], k[2], k[3], enforcePremul,
                                      common.getInput(0), common.getInput(1), common.cropRect());
}

sk_sp<SkFlattenable> SkBlendImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 2);

    sk_sp<SkBlender> blender;
    std::optional<SkV4> coefficients;
    bool enforcePremul = false;

    const uint32_t mode = buffer.read32();
    if (mode == kArithmetic_SkBlendMode) {
        // Should only see this sentinel value in newer SKPs
        if (buffer.validate(!buffer.isVersionLT(SkPicturePriv::kCombineBlendArithmeticFilters))) {
            SkV4 k;
            for (int i = 0; i < 4; ++i) {
                k[i] = buffer.readScalar();
            }
            coefficients = k;
            enforcePremul = buffer.readBool();
            blender = SkBlenders::Arithmetic(k.x, k.y, k.z, k.w, enforcePremul);
            if (!buffer.validate(SkToBool(blender))) {
                return nullptr; // A null arithmetic blender is an error condition
            }
        }
    } else if (mode == kCustom_SkBlendMode) {
        blender = buffer.readBlender();
    } else {
        if (!buffer.validate(mode <= (unsigned) SkBlendMode::kLastMode)) {
            return nullptr;
        }
        blender = SkBlender::Mode((SkBlendMode)mode);
    }

    return make_blend(std::move(blender),
                      common.getInput(kBackground),
                      common.getInput(kForeground),
                      common.cropRect(),
                      coefficients,
                      enforcePremul);
}

void SkBlendImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->SkImageFilter_Base::flatten(buffer);
    if (fArithmeticCoefficients.has_value()) {
        buffer.write32(kArithmetic_SkBlendMode);

        const SkV4& k = *fArithmeticCoefficients;
        buffer.writeScalar(k[0]);
        buffer.writeScalar(k[1]);
        buffer.writeScalar(k[2]);
        buffer.writeScalar(k[3]);
        buffer.writeBool(fEnforcePremul);
    } else if (auto bm = as_BB(fBlender)->asBlendMode()) {
        buffer.write32((unsigned)bm.value());
    } else {
        buffer.write32(kCustom_SkBlendMode);
        buffer.writeFlattenable(fBlender.get());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

sk_sp<SkShader> SkBlendImageFilter::makeBlendShader(sk_sp<SkShader> bg, sk_sp<SkShader> fg) const {
    // A null input shader signifies transparent black when image filtering, but SkShaders::Blend
    // expects non-null shaders. So we have to do some clean up.
    if (!bg || !fg) {
        // If we don't affect transparent black and both inputs are null, then return a null
        // shader to skip any evaluation.
        if (!this->onAffectsTransparentBlack() && !bg && !fg) {
            return nullptr;
        }
        // Otherwise if only one input is null, we might be able to just return that one.
        if (auto bm = as_BB(fBlender)->asBlendMode()) {
            SkBlendModeCoeff src, dst;
            if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
                if (bg && (dst == SkBlendModeCoeff::kOne ||
                           dst == SkBlendModeCoeff::kISA ||
                           dst == SkBlendModeCoeff::kISC)) {
                    return bg;
                }
                if (fg && (src == SkBlendModeCoeff::kOne ||
                           src == SkBlendModeCoeff::kIDA)) {
                    return fg;
                }
            }
        }
        // If we made it this far, the blend has non-trivial behavior even when one of the
        // inputs is transparent black, so replace the null shaders with that color.
        if (!bg) { bg = SkShaders::Color(SK_ColorTRANSPARENT); }
        if (!fg) { fg = SkShaders::Color(SK_ColorTRANSPARENT); }
    }

    return SkShaders::Blend(fBlender, std::move(bg), std::move(fg));
}

skif::FilterResult SkBlendImageFilter::onFilterImage(const skif::Context& ctx) const {
    // We could just request 'desiredOutput' for the blend's required input size, since that's what
    // it is expected to fill. However, some blend modes restrict the output to something other
    // than the union of the foreground and background. To make this restriction available to both
    // children before evaluating them, we determine the maximum possible output the blend can
    // produce from the contentBounds and require that for both children to produce.
    skif::LayerSpace<SkIRect> requiredInput = this->onGetOutputLayerBounds(
            ctx.mapping(), ctx.source().layerBounds());
    if (!requiredInput.intersect(ctx.desiredOutput())) {
        return {};
    }
    skif::Context inputCtx = ctx.withNewDesiredOutput(requiredInput);

    skif::FilterResult::Builder builder{ctx};
    builder.add(this->getChildOutput(kBackground, inputCtx));
    builder.add(this->getChildOutput(kForeground, inputCtx));
    return builder.eval(
            [&](SkSpan<sk_sp<SkShader>> inputs) -> sk_sp<SkShader> {
                return this->makeBlendShader(inputs[kBackground], inputs[kForeground]);
            }, requiredInput);
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetInputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& desiredOutput,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // See comment in onFilterImage().
    skif::LayerSpace<SkIRect> requiredInput = this->onGetOutputLayerBounds(mapping, contentBounds);
    if (!requiredInput.intersect(desiredOutput)) {
        // Don't bother recursing if we know the blend will discard everything
        return skif::LayerSpace<SkIRect>::Empty();
    }

    // Return the union of both FG and BG required inputs to ensure both have all necessary pixels
    skif::LayerSpace<SkIRect> bgInput =
            this->getChildInputLayerBounds(kBackground, mapping, requiredInput, contentBounds);
    skif::LayerSpace<SkIRect> fgInput =
            this->getChildInputLayerBounds(kForeground, mapping, requiredInput, contentBounds);

    bgInput.join(fgInput);
    return bgInput;
}

skif::LayerSpace<SkIRect> SkBlendImageFilter::onGetOutputLayerBounds(
        const skif::Mapping& mapping,
        const skif::LayerSpace<SkIRect>& contentBounds) const {
    // Blending is (k0*FG*BG +       k1*FG +       k2*BG + k3) for arithmetic blenders OR
    //             ( 0*FG*BG + srcCoeff*FG + dstCoeff*BG + 0 ) for Porter-Duff blend modes OR
    //              un-inspectable(FG, BG) for advanced blend modes and other runtime blenders.
    //
    // There are six possible output bounds that can be produced:
    //   1. No output: K = (0,0,0,0) or (srcCoeff,dstCoeff) = (kZero,kZero)
    //   2. intersect(FG,BG): K = (non-zero, 0,0,0) or (srcCoeff,dstCoeff) = (kZero|kDA, kZero|kSA)
    //   3. FG-only: K = (0, non-zero, 0,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, kZero|kSA)
    //   4. BG-only: K = (0,0, non-zero, 0) or (srcCoeff,dstCoeff) = (kZero|kDA, !kZero&!kSA)
    //   5. union(FG,BG): K = (*,*,*,0) or (srcCoeff,dstCoeff) = (!kZero&!kDA, !kZero&!kSA)
    //        or an advanced blend mode.
    //   6. infinite: K = (*,*,*, non-zero) or a runtime blender other than SkBlenders::Arithmetic.
    bool transparentOutsideFG = false;
    bool transparentOutsideBG = false;
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        SkASSERT(*bm != SkBlendMode::kClear); // Should have been caught at creation time
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
            // If dst's coefficient is 0 then nothing can produce non-transparent content outside
            // of the foreground. When dst coefficient is SA, it will always be 0 outside the FG.
            // For purposes of transparency analysis, SC == SA.
            transparentOutsideFG = dst == SkBlendModeCoeff::kZero || dst == SkBlendModeCoeff::kSA
                                                                  || dst == SkBlendModeCoeff::kSC;
            // And the reverse is true for src and the background content.
            transparentOutsideBG = src == SkBlendModeCoeff::kZero || src == SkBlendModeCoeff::kDA;
        }
        // NOTE: advanced blends use src-over for their alpha channel, which should produce the
        // union of FG and BG. That is the outcome if we leave transparentOutsideFG/BG false.
    } else if (fArithmeticCoefficients.has_value()) {
        [[maybe_unused]] static constexpr SkV4 kClearCoeff = {0.f, 0.f, 0.f, 0.f};
        const SkV4& k = *fArithmeticCoefficients;
        SkASSERT(k != kClearCoeff); // Should have been converted to an empty filter

        if (k[3] != 0.f) {
            // The arithmetic equation produces non-transparent black everywhere
            return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
        } else {
            // Given the earlier assert and if, then (k[1] == k[2] == 0) implies k[0] != 0. If only
            // one of k[1] or k[2] are non-zero then, regardless of k[0], then only that bounds
            // has non-transparent content.
            transparentOutsideFG = k[2] == 0.f;
            transparentOutsideBG = k[1] == 0.f;
        }
    } else {
        // A non-arithmetic runtime blender, so pessimistically assume it can return non-transparent
        // black anywhere.
        return skif::LayerSpace<SkIRect>(SkRectPriv::MakeILarge());
    }

    skif::LayerSpace<SkIRect> foregroundBounds =
            this->getChildOutputLayerBounds(kForeground, mapping, contentBounds);
    skif::LayerSpace<SkIRect> backgroundBounds =
            this->getChildOutputLayerBounds(kBackground, mapping, contentBounds);
    if (transparentOutsideFG) {
        if (transparentOutsideBG) {
            // Output is the intersection of both
            if (!foregroundBounds.intersect(backgroundBounds)) {
                return skif::LayerSpace<SkIRect>::Empty();
            }
        }
        return foregroundBounds;
    } else {
        if (!transparentOutsideBG) {
            // Output is the union of both (infinite bounds were detected earlier).
            backgroundBounds.join(foregroundBounds);
        }
        return backgroundBounds;
    }
}

SkRect SkBlendImageFilter::computeFastBounds(const SkRect& bounds) const {
    // TODO: This is a prime example of why computeFastBounds() and onGetOutputLayerBounds() should
    // be combined into the same function.
    bool transparentOutsideFG = false;
    bool transparentOutsideBG = false;
    if (auto bm = as_BB(fBlender)->asBlendMode()) {
        SkASSERT(*bm != SkBlendMode::kClear); // Should have been caught at creation time
        SkBlendModeCoeff src, dst;
        if (SkBlendMode_AsCoeff(*bm, &src, &dst)) {
            // If dst's coefficient is 0 then nothing can produce non-transparent content outside
            // of the foreground. When dst coefficient is SA, it will always be 0 outside the FG.
            transparentOutsideFG = dst == SkBlendModeCoeff::kZero || dst == SkBlendModeCoeff::kSA;
            // And the reverse is true for src and the background content.
            transparentOutsideBG = src == SkBlendModeCoeff::kZero || src == SkBlendModeCoeff::kDA;
        }
    } else if (fArithmeticCoefficients.has_value()) {
        [[maybe_unused]] static constexpr SkV4 kClearCoeff = {0.f, 0.f, 0.f, 0.f};
        const SkV4& k = *fArithmeticCoefficients;
        SkASSERT(k != kClearCoeff); // Should have been converted to an empty image filter

        if (k[3] != 0.f) {
            // The arithmetic equation produces non-transparent black everywhere
            return SkRectPriv::MakeLargeS32();
        } else {
            // Given the earlier assert and if, then (k[1] == k[2] == 0) implies k[0] != 0. If only
            // one of k[1] or k[2] are non-zero then, regardless of k[0], then only that bounds
            // has non-transparent content.
            transparentOutsideFG = k[2] == 0.f;
            transparentOutsideBG = k[1] == 0.f;
        }
    } else {
        // A non-arithmetic runtime blender, so pessimistically assume it can return non-transparent
        // black anywhere.
        return SkRectPriv::MakeLargeS32();
    }

    SkRect foregroundBounds = this->getInput(kForeground) ?
            this->getInput(kForeground)->computeFastBounds(bounds) : bounds;
    SkRect backgroundBounds = this->getInput(kBackground) ?
            this->getInput(kBackground)->computeFastBounds(bounds) : bounds;
    if (transparentOutsideFG) {
        if (transparentOutsideBG) {
            // Output is the intersection of both
            if (!foregroundBounds.intersect(backgroundBounds)) {
                return SkRect::MakeEmpty();
            }
        }
        return foregroundBounds;
    } else {
        if (!transparentOutsideBG) {
            // Output is the union of both (infinite bounds were detected earlier).
            backgroundBounds.join(foregroundBounds);
        }
        return backgroundBounds;
    }
}
