/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkGradientBaseShader.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkShader.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "modules/skcms/skcms.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorData.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkEffectPriv.h"
#include "src/core/SkPicturePriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>

using namespace skia_private;

enum GradientSerializationFlags {
    // Bits 29:31 used for various boolean flags
    kHasPosition_GSF          = 0x80000000,
    kHasLegacyLocalMatrix_GSF = 0x40000000,
    kHasColorSpace_GSF        = 0x20000000,

    // Bits 12:28 unused

    // Bits 8:11 for fTileMode
    kTileModeShift_GSF  = 8,
    kTileModeMask_GSF   = 0xF,

    // Bits 4:7 for fInterpolation.fColorSpace
    kInterpolationColorSpaceShift_GSF = 4,
    kInterpolationColorSpaceMask_GSF  = 0xF,

    // Bits 1:3 for fInterpolation.fHueMethod
    kInterpolationHueMethodShift_GSF = 1,
    kInterpolationHueMethodMask_GSF  = 0x7,

    // Bit 0 for fInterpolation.fInPremul
    kInterpolationInPremul_GSF = 0x1,
};

SkGradientBaseShader::Descriptor::Descriptor() {
    sk_bzero(this, sizeof(*this));
    fTileMode = SkTileMode::kClamp;
}
SkGradientBaseShader::Descriptor::~Descriptor() = default;

void SkGradientBaseShader::flatten(SkWriteBuffer& buffer) const {
    uint32_t flags = 0;
    if (fPositions) {
        flags |= kHasPosition_GSF;
    }
    sk_sp<SkData> colorSpaceData = fColorSpace ? fColorSpace->serialize() : nullptr;
    if (colorSpaceData) {
        flags |= kHasColorSpace_GSF;
    }
    if (fInterpolation.fInPremul == Interpolation::InPremul::kYes) {
        flags |= kInterpolationInPremul_GSF;
    }
    SkASSERT(static_cast<uint32_t>(fTileMode) <= kTileModeMask_GSF);
    flags |= ((uint32_t)fTileMode << kTileModeShift_GSF);
    SkASSERT(static_cast<uint32_t>(fInterpolation.fColorSpace) <= kInterpolationColorSpaceMask_GSF);
    flags |= ((uint32_t)fInterpolation.fColorSpace << kInterpolationColorSpaceShift_GSF);
    SkASSERT(static_cast<uint32_t>(fInterpolation.fHueMethod) <= kInterpolationHueMethodMask_GSF);
    flags |= ((uint32_t)fInterpolation.fHueMethod << kInterpolationHueMethodShift_GSF);

    buffer.writeUInt(flags);

    // If we injected implicit first/last stops at construction time, omit those when serializing:
    int colorCount = fColorCount;
    const SkColor4f* colors = fColors;
    const SkScalar* positions = fPositions;
    if (fFirstStopIsImplicit) {
        colorCount--;
        colors++;
        if (positions) {
            positions++;
        }
    }
    if (fLastStopIsImplicit) {
        colorCount--;
    }

    buffer.writeColor4fArray({colors, colorCount});
    if (colorSpaceData) {
        buffer.writeDataAsByteArray(colorSpaceData.get());
    }
    if (positions) {
        buffer.writeScalarArray({positions, colorCount});
    }
}

template <int N, typename T, bool MEM_MOVE>
static bool validate_array(SkReadBuffer& buffer, size_t count, STArray<N, T, MEM_MOVE>* array) {
    if (!buffer.validateCanReadN<T>(count)) {
        return false;
    }

    array->resize_back(count);
    return true;
}

bool SkGradientBaseShader::DescriptorScope::unflatten(SkReadBuffer& buffer,
                                                      SkMatrix* legacyLocalMatrix) {
    // New gradient format. Includes floating point color, color space, densely packed flags
    uint32_t flags = buffer.readUInt();

    fTileMode = (SkTileMode)((flags >> kTileModeShift_GSF) & kTileModeMask_GSF);

    fInterpolation.fColorSpace = (Interpolation::ColorSpace)(
            (flags >> kInterpolationColorSpaceShift_GSF) & kInterpolationColorSpaceMask_GSF);
    fInterpolation.fHueMethod = (Interpolation::HueMethod)(
            (flags >> kInterpolationHueMethodShift_GSF) & kInterpolationHueMethodMask_GSF);
    fInterpolation.fInPremul = (flags & kInterpolationInPremul_GSF) ? Interpolation::InPremul::kYes
                                                                    : Interpolation::InPremul::kNo;

    fColorCount = buffer.getArrayCount();

    if (!(validate_array(buffer, fColorCount, &fColorStorage) &&
          buffer.readColor4fArray({fColorStorage.begin(), fColorCount}))) {
        return false;
    }
    fColors = fColorStorage.begin();

    if (SkToBool(flags & kHasColorSpace_GSF)) {
        sk_sp<SkData> data = buffer.readByteArrayAsData();
        fColorSpace = data ? SkColorSpace::Deserialize(data->data(), data->size()) : nullptr;
    } else {
        fColorSpace = nullptr;
    }
    if (SkToBool(flags & kHasPosition_GSF)) {
        if (!(validate_array(buffer, fColorCount, &fPositionStorage) &&
              buffer.readScalarArray({fPositionStorage.begin(), fColorCount}))) {
            return false;
        }
        fPositions = fPositionStorage.begin();
    } else {
        fPositions = nullptr;
    }
    if (SkToBool(flags & kHasLegacyLocalMatrix_GSF)) {
        SkASSERT(buffer.isVersionLT(SkPicturePriv::Version::kNoShaderLocalMatrix));
        buffer.readMatrix(legacyLocalMatrix);
    } else {
        *legacyLocalMatrix = SkMatrix::I();
    }
    return buffer.isValid();
}

////////////////////////////////////////////////////////////////////////////////////////////

SkGradientBaseShader::SkGradientBaseShader(const Descriptor& desc, const SkMatrix& ptsToUnit)
        : fPtsToUnit(ptsToUnit)
        , fColorSpace(desc.fColorSpace ? desc.fColorSpace : SkColorSpace::MakeSRGB())
        , fFirstStopIsImplicit(false)
        , fLastStopIsImplicit(false)
        , fColorsAreOpaque(true) {
    fPtsToUnit.getType();  // Precache so reads are threadsafe.
    SkASSERT(desc.fColorCount > 1);

    fInterpolation = desc.fInterpolation;

    SkASSERT((unsigned)desc.fTileMode < kSkTileModeCount);
    fTileMode = desc.fTileMode;

    /*  Note: we let the caller skip the first and/or last position.
        i.e. pos[0] = 0.3, pos[1] = 0.7
        In these cases, we insert entries to ensure that the final data
        will be bracketed by [0, 1].
        i.e. our_pos[0] = 0, our_pos[1] = 0.3, our_pos[2] = 0.7, our_pos[3] = 1

        Thus colorCount (the caller's value, and fColorCount (our value) may
        differ by up to 2. In the above example:
            colorCount = 2
            fColorCount = 4
     */
    fColorCount = desc.fColorCount;

    // Check if we need to add in start and/or end position/colors
    if (desc.fPositions) {
        fFirstStopIsImplicit = desc.fPositions[0] > 0;
        fLastStopIsImplicit = desc.fPositions[desc.fColorCount - 1] != SK_Scalar1;
        fColorCount += fFirstStopIsImplicit + fLastStopIsImplicit;
    }

    size_t storageSize =
            fColorCount * (sizeof(SkColor4f) + (desc.fPositions ? sizeof(SkScalar) : 0));
    fColors = reinterpret_cast<SkColor4f*>(fStorage.reset(storageSize));
    fPositions = desc.fPositions ? reinterpret_cast<SkScalar*>(fColors + fColorCount) : nullptr;

    // Now copy over the colors, adding the duplicates at t=0 and t=1 as needed
    SkColor4f* colors = fColors;
    if (fFirstStopIsImplicit) {
        *colors++ = desc.fColors[0];
    }
    for (int i = 0; i < desc.fColorCount; ++i) {
        colors[i] = desc.fColors[i];
        fColorsAreOpaque = fColorsAreOpaque && (desc.fColors[i].fA == 1);
    }
    if (fLastStopIsImplicit) {
        colors += desc.fColorCount;
        *colors = desc.fColors[desc.fColorCount - 1];
    }

    if (desc.fPositions) {
        SkScalar prev = 0;
        SkScalar* positions = fPositions;
        *positions++ = prev;  // force the first pos to 0

        int startIndex = fFirstStopIsImplicit ? 0 : 1;
        int count = desc.fColorCount + fLastStopIsImplicit;

        bool uniformStops = true;
        const SkScalar uniformStep = desc.fPositions[startIndex] - prev;
        for (int i = startIndex; i < count; i++) {
            // Pin the last value to 1.0, and make sure pos is monotonic.
            float curr = 1.0f;
            if (i != desc.fColorCount) {
                curr = SkTPin(desc.fPositions[i], prev, 1.0f);

                // If a value is clamped to 1.0 before the last stop, the last stop
                // actually isn't implicit if we thought it was.
                if (curr == 1.0f && fLastStopIsImplicit) {
                    fLastStopIsImplicit = false;
                }
            }

            uniformStops &= SkScalarNearlyEqual(uniformStep, curr - prev);

            *positions++ = prev = curr;
        }

        if (uniformStops) {
            // If the stops are uniform, treat them as implicit.
            fPositions = nullptr;
        } else {
            // Remove duplicate stops with more than two of the same stop,
            // keeping the leftmost and rightmost stop colors.
            // i.e.       0, 0, 0,   0.2, 0.2, 0.3, 0.3, 0.3, 1, 1
            // w/  clamp  0,    0,   0.2, 0.2, 0.3,      0.3, 1, 1
            // w/o clamp        0,   0.2, 0.2, 0.3,      0.3, 1
            int i = 0;
            int dedupedColorCount = 0;
            for (int j = 1; j <= fColorCount; j++) {
                // We can compare the current positions at i and j since once these fPosition
                // values are overwritten, our i and j pointers will be past the overwritten values.
                if (j == fColorCount || fPositions[i] != fPositions[j]) {
                    bool dupStop = j - i > 1;

                    // Ignore the leftmost stop (i) if it is a non-clamp tilemode with
                    // a duplicate stop on t = 0.
                    bool ignoreLeftmost = dupStop && fTileMode != SkTileMode::kClamp
                                                    && fPositions[i] == 0;
                    if (!ignoreLeftmost) {
                        fPositions[dedupedColorCount] = fPositions[i];
                        fColors[dedupedColorCount] =  fColors[i];
                        dedupedColorCount++;
                    }

                    // Include the rightmost stop (j-1) only if the stop has a duplicate,
                    // ignoring the rightmost stop if it is a non-clamp tilemode with t = 1.
                    bool ignoreRightmost = fTileMode != SkTileMode::kClamp
                                                    && fPositions[j - 1] == 1;
                    if (dupStop && !ignoreRightmost) {
                        fPositions[dedupedColorCount] = fPositions[j - 1];
                        fColors[dedupedColorCount] = fColors[j - 1];
                        dedupedColorCount++;
                    }
                    i = j;
                }
            }
            fColorCount = dedupedColorCount;
        }
    }
}

SkGradientBaseShader::~SkGradientBaseShader() {}

static void add_stop_color(SkRasterPipelineContexts::GradientCtx* ctx,
                           size_t stop,
                           const SkPMColor4f& Fs,
                           const SkPMColor4f& Bs) {
    (ctx->factors[0])[stop] = Fs.fR;
    (ctx->factors[1])[stop] = Fs.fG;
    (ctx->factors[2])[stop] = Fs.fB;
    (ctx->factors[3])[stop] = Fs.fA;

    (ctx->biases[0])[stop] = Bs.fR;
    (ctx->biases[1])[stop] = Bs.fG;
    (ctx->biases[2])[stop] = Bs.fB;
    (ctx->biases[3])[stop] = Bs.fA;
}

static void add_const_color(SkRasterPipelineContexts::GradientCtx* ctx,
                            size_t stop,
                            const SkPMColor4f& color) {
    add_stop_color(ctx, stop, {0, 0, 0, 0}, color);
}

// Calculate a factor F and a bias B so that color = F*t + B when t is in range of
// the stop. Assume that all stops have width 1/gapCount and the stop parameter
// refers to the nth stop.
static void init_stop_evenly(SkRasterPipelineContexts::GradientCtx* ctx,
                             float gapCount,
                             size_t stop,
                             const SkPMColor4f& leftC,
                             const SkPMColor4f& rightC) {
    auto left = skvx::float4::Load(leftC.vec());
    auto right = skvx::float4::Load(rightC.vec());

    SkPMColor4f factor, bias;

    // We start with the following 2 linear equations and 2 unknowns (factor, bias)
    // left = factor * t + bias
    // right = factor * (t + gap) + bias
    // gap = 1/gapCount
    // t = gap * stop

    // right - left = factor * (t + gap) - factor * t
    // right - left = factor * gap
    // factor = (right - left) / gap  (and gap = 1/gapCount)
    auto factor4 = ((right - left) * gapCount);
    (left - (factor4 * (stop / gapCount))).store(bias.vec());
    factor4.store(factor.vec());

    add_stop_color(ctx, stop, factor, bias);
}

// Calculate a factor F and a bias B so that color = F*t + B when t is in range of
// the stop. Unlike init_stop_evenly, this handles stops
static void init_stop_pos(SkRasterPipelineContexts::GradientCtx* ctx,
                          size_t stop,
                          float t_l,
                          float gapReciprocal,
                          const SkPMColor4f& leftC,
                          const SkPMColor4f& rightC) {
    // gapReciprocal is 1/gapWidth. If two colors were on top of each other, we should
    // have skipped that as a "stop".
    SkASSERT(SkIsFinite(gapReciprocal));

    auto left = skvx::float4::Load(leftC.vec());
    auto right = skvx::float4::Load(rightC.vec());

    SkPMColor4f factor, bias;

    // See init_stop_evenly for this derivation, noting that gap = 1/gapReciprocal
    // and t = t_l
    auto factor4 = ((right - left) * gapReciprocal);
    (left - (factor4 * t_l)).store(bias.vec());
    factor4.store(factor.vec());

    ctx->ts[stop] = t_l;
    add_stop_color(ctx, stop, factor, bias);
}

void SkGradientBaseShader::AppendGradientFillStages(SkRasterPipeline* p,
                                                    SkArenaAlloc* alloc,
                                                    const SkPMColor4f* pmColors,
                                                    const SkScalar* positions,
                                                    int count) {
    // The two-stop case with stops at 0 and 1.
    if (count == 2 && positions == nullptr) {
        const SkPMColor4f c_l = pmColors[0], c_r = pmColors[1];

        auto ctx = alloc->make<SkRasterPipelineContexts::EvenlySpaced2StopGradientCtx>();
        (skvx::float4::Load(c_r.vec()) - skvx::float4::Load(c_l.vec())).store(ctx->factor);
        (skvx::float4::Load(c_l.vec())).store(ctx->bias);

        p->append(SkRasterPipelineOp::evenly_spaced_2_stop_gradient, ctx);
        return;
    }
    // Linear gradients with evenly spaced stops involve doing calculations to interpolate
    // between color n and color n+1 based on t (in range [0.0,1.0]).
    //   color_n * (t - t_n) / gap_n + color_{n+1} * (t_{n+1} - t) / gap_n
    // We could just stick the colors and the gaps calculation in RP and do this calculation,
    // but instead we can precompute things to make the RP calculation simpler and faster.
    // For each gap, we calculate four linear equations in the form y = m*x + b, or rather
    //  color_channel = factor * t + bias
    // We do this pre-computation in init_stop_evenly and init_stop_pos.

    auto* ctx = alloc->make<SkRasterPipelineContexts::GradientCtx>();

    // Allocate at least enough for the AVX2 gather from a YMM register.
    constexpr int kMaxRegisterSize = 8;

    // There are n - 1 gaps between n colors plus 2 regions to the left and right
    // of the gradient to account for colors. For evenly spaced gradients, we cheat
    // and skip the left gap, using one block of floats unused.
    const size_t factorBiasFloats = std::max(count + 1, kMaxRegisterSize);
    const size_t tsForArbitraryStops = count + 1;
    using SkRasterPipelineContexts::kRGBAChannels;

    // We need space for all factors and biases, and while we are at it, some space
    // if we need to include the arbitrary stops.
    const size_t toAlloc = 2 * kRGBAChannels * factorBiasFloats + tsForArbitraryStops;
    float* gradientCtxBuffer = alloc->makeArray<float>(toAlloc);
    for (size_t i = 0; i < kRGBAChannels; i++) {
        ctx->factors[i] = gradientCtxBuffer;
        gradientCtxBuffer += factorBiasFloats;
        ctx->biases[i] = gradientCtxBuffer;
        gradientCtxBuffer += factorBiasFloats;
    }

    if (positions == nullptr) {
        // Handle evenly distributed stops.

        size_t stopCount = count;
        float gapCount = stopCount - 1;

        SkPMColor4f c_l = pmColors[0];
        for (size_t i = 0; i < gapCount; i++) {
            SkPMColor4f c_r = pmColors[i + 1];
            init_stop_evenly(ctx, gapCount, i, c_l, c_r);
            c_l = c_r;
        }
        add_const_color(ctx, stopCount - 1, c_l);

        ctx->stopCount = stopCount;
        p->append(SkRasterPipelineOp::evenly_spaced_gradient, ctx);
        return;
    }

    // Handle arbitrary stops.
    ctx->ts = gradientCtxBuffer;

    // Remove the default stops inserted by SkGradientBaseShader::SkGradientBaseShader
    // because they are naturally handled by the search method.
    int firstStop;
    int lastStop;
    if (count > 2) {
        firstStop = pmColors[0] != pmColors[1] ? 0 : 1;
        lastStop = pmColors[count - 2] != pmColors[count - 1] ? count - 1 : count - 2;
    } else {
        firstStop = 0;
        lastStop = 1;
    }

    size_t stopCount = 0;
    float t_l = positions[firstStop];
    SkPMColor4f c_l = pmColors[firstStop];
    add_const_color(ctx, stopCount++, c_l);

    for (int i = firstStop; i < lastStop; i++) {
        float t_r = positions[i + 1];
        SkPMColor4f c_r = pmColors[i + 1];
        SkASSERT(t_l <= t_r);
        if (t_l < t_r) {
            float c_scale = sk_ieee_float_divide(1, t_r - t_l);
            if (SkIsFinite(c_scale)) {
                init_stop_pos(ctx, stopCount, t_l, c_scale, c_l, c_r);
                stopCount += 1;
            }
        }
        t_l = t_r;
        c_l = c_r;
    }

    ctx->ts[stopCount] = t_l;
    add_const_color(ctx, stopCount++, c_l);

    ctx->stopCount = stopCount;
    p->append(SkRasterPipelineOp::gradient, ctx);
}

void SkGradientBaseShader::AppendInterpolatedToDstStages(SkRasterPipeline* p,
                                                         SkArenaAlloc* alloc,
                                                         bool colorsAreOpaque,
                                                         const Interpolation& interpolation,
                                                         const SkColorSpace* intermediateColorSpace,
                                                         const SkColorSpace* dstColorSpace) {
    using ColorSpace = Interpolation::ColorSpace;
    bool colorIsPremul = static_cast<bool>(interpolation.fInPremul);

    // If we interpolated premul colors in any of the special color spaces, we need to unpremul
    if (colorIsPremul && !colorsAreOpaque) {
        switch (interpolation.fColorSpace) {
            case ColorSpace::kLab:
            case ColorSpace::kOKLab:
            case ColorSpace::kOKLabGamutMap:
                p->append(SkRasterPipelineOp::unpremul);
                colorIsPremul = false;
                break;
            case ColorSpace::kLCH:
            case ColorSpace::kOKLCH:
            case ColorSpace::kOKLCHGamutMap:
            case ColorSpace::kHSL:
            case ColorSpace::kHWB:
                p->append(SkRasterPipelineOp::unpremul_polar);
                colorIsPremul = false;
                break;
            default:
                break;
        }
    }

    // Convert colors in exotic spaces back to their intermediate SkColorSpace
    switch (interpolation.fColorSpace) {
        case ColorSpace::kLab:   p->append(SkRasterPipelineOp::css_lab_to_xyz);           break;
        case ColorSpace::kOKLab: p->append(SkRasterPipelineOp::css_oklab_to_linear_srgb); break;
        case ColorSpace::kOKLabGamutMap:
            p->append(SkRasterPipelineOp::css_oklab_gamut_map_to_linear_srgb);
            break;
        case ColorSpace::kLCH:   p->append(SkRasterPipelineOp::css_hcl_to_lab);
                                 p->append(SkRasterPipelineOp::css_lab_to_xyz);           break;
        case ColorSpace::kOKLCH: p->append(SkRasterPipelineOp::css_hcl_to_lab);
                                 p->append(SkRasterPipelineOp::css_oklab_to_linear_srgb); break;
        case ColorSpace::kOKLCHGamutMap:
            p->append(SkRasterPipelineOp::css_hcl_to_lab);
            p->append(SkRasterPipelineOp::css_oklab_gamut_map_to_linear_srgb);
            break;
        case ColorSpace::kHSL:   p->append(SkRasterPipelineOp::css_hsl_to_srgb);          break;
        case ColorSpace::kHWB:   p->append(SkRasterPipelineOp::css_hwb_to_srgb);          break;
        default: break;
    }

    // Now transform from intermediate to destination color space.
    // See comments in GrGradientShader.cpp about the decisions here.
    if (!dstColorSpace) {
        dstColorSpace = sk_srgb_singleton();
    }
    SkAlphaType intermediateAlphaType = colorIsPremul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;
    // TODO(skbug.com/40044213): Get dst alpha type correctly
    SkAlphaType dstAlphaType = kPremul_SkAlphaType;

    if (colorsAreOpaque) {
        intermediateAlphaType = dstAlphaType = kUnpremul_SkAlphaType;
    }

    alloc->make<SkColorSpaceXformSteps>(
                 intermediateColorSpace, intermediateAlphaType, dstColorSpace, dstAlphaType)
            ->apply(p);
}

bool SkGradientBaseShader::appendStages(const SkStageRec& rec,
                                        const SkShaders::MatrixRec& mRec) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;
    SkRasterPipelineContexts::DecalTileCtx* decal_ctx = nullptr;

    std::optional<SkShaders::MatrixRec> newMRec = mRec.apply(rec, fPtsToUnit);
    if (!newMRec.has_value()) {
        return false;
    }

    SkRasterPipeline_<256> postPipeline;

    this->appendGradientStages(alloc, p, &postPipeline);

    switch (fTileMode) {
        case SkTileMode::kMirror:
            p->append(SkRasterPipelineOp::mirror_x_1);
            break;
        case SkTileMode::kRepeat:
            p->append(SkRasterPipelineOp::repeat_x_1);
            break;
        case SkTileMode::kDecal:
            decal_ctx = alloc->make<SkRasterPipelineContexts::DecalTileCtx>();
            decal_ctx->limit_x = SkBits2Float(SkFloat2Bits(1.0f) + 1);
            // reuse mask + limit_x stage, or create a custom decal_1 that just stores the mask
            p->append(SkRasterPipelineOp::decal_x, decal_ctx);
            [[fallthrough]];

        case SkTileMode::kClamp:
            if (!fPositions) {
                // We clamp only when the stops are evenly spaced.
                // If not, there may be hard stops, and clamping ruins hard stops at 0 and/or 1.
                // In that case, we must make sure we're using the general "gradient" stage,
                // which is the only stage that will correctly handle unclamped t.
                p->append(SkRasterPipelineOp::clamp_x_1);
            }
            break;
    }

    // Transform all of the colors to destination color space, possibly premultiplied
    SkColor4fXformer xformedColors(this, rec.fDstCS);
    AppendGradientFillStages(p, alloc,
                             xformedColors.fColors.begin(),
                             xformedColors.fPositions,
                             xformedColors.fColors.size());
    AppendInterpolatedToDstStages(p, alloc, fColorsAreOpaque, fInterpolation,
                                  xformedColors.fIntermediateColorSpace.get(), rec.fDstCS);

    if (decal_ctx) {
        p->append(SkRasterPipelineOp::check_decal_mask, decal_ctx);
    }

    p->extend(postPipeline);

    return true;
}

bool SkGradientBaseShader::isOpaque() const {
    return fColorsAreOpaque && (this->getTileMode() != SkTileMode::kDecal);
}

bool SkGradientBaseShader::onAsLuminanceColor(SkColor4f* lum) const {
    // We just compute an average color. There are several things we could do better:
    // 1) We already have a different average_gradient_color helper later in this file, that weights
    //    contribution by the relative size of each band.
    // 2) Colors should be converted to some standard color space! These could be in any space.
    // 3) Do we want to average in the source space, sRGB, or some linear space?
    SkColor4f color{0, 0, 0, 1};
    for (int i = 0; i < fColorCount; ++i) {
        color.fR += fColors[i].fR;
        color.fG += fColors[i].fG;
        color.fB += fColors[i].fB;
    }
    const float scale = 1.0f / fColorCount;
    color.fR *= scale;
    color.fG *= scale;
    color.fB *= scale;
    *lum = color;
    return true;
}

static sk_sp<SkColorSpace> intermediate_color_space(SkGradientShader::Interpolation::ColorSpace cs,
                                                    SkColorSpace* dst) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    switch (cs) {
        case ColorSpace::kDestination:
            return sk_ref_sp(dst);

        // css-color-4 allows XYZD50 and XYZD65. For gradients, those are redundant. Interpolating
        // in any linear RGB space, (regardless of white point), gives the same answer.
        case ColorSpace::kSRGBLinear:
            return SkColorSpace::MakeSRGBLinear();

        case ColorSpace::kSRGB:
        case ColorSpace::kHSL:
        case ColorSpace::kHWB:
            return SkColorSpace::MakeSRGB();

        case ColorSpace::kLab:
        case ColorSpace::kLCH:
            // Conversion to Lab (and LCH) starts with XYZD50
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kXYZ);

        case ColorSpace::kOKLab:
        case ColorSpace::kOKLabGamutMap:
        case ColorSpace::kOKLCH:
        case ColorSpace::kOKLCHGamutMap:
            // The "standard" conversion to these spaces starts with XYZD65. That requires extra
            // effort to conjure. The author also has reference code for going directly from linear
            // sRGB, so we use that.
            // TODO(skbug.com/40044213): Even better would be to have an LMS color space, because the first
            // part of the conversion is a matrix multiply, which could be absorbed into the
            // color space xform.
            return SkColorSpace::MakeSRGBLinear();

        // These rectangular color spaces have their own transfer curves.
        case ColorSpace::kDisplayP3:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3);

        case ColorSpace::kRec2020:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kRec2020, SkNamedGamut::kRec2020);

        case ColorSpace::kProphotoRGB:
            static skcms_Matrix3x3 lin_proPhoto_to_XYZ_D50;
            SkNamedPrimaries::kProPhotoRGB.toXYZD50(&lin_proPhoto_to_XYZ_D50);
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kProPhotoRGB, lin_proPhoto_to_XYZ_D50);

        case ColorSpace::kA98RGB:
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kA98RGB, SkNamedGamut::kAdobeRGB);
    }
    SkUNREACHABLE;
}

using ConvertColorProc = SkPMColor4f(*)(SkPMColor4f, bool*);
using PremulColorProc = SkPMColor4f(*)(SkPMColor4f);

static SkPMColor4f srgb_to_hsl(SkPMColor4f rgb, bool* hueIsPowerless) {
    float mx = std::max({rgb.fR, rgb.fG, rgb.fB});
    float mn = std::min({rgb.fR, rgb.fG, rgb.fB});
    float hue = 0, sat = 0, light = (mn + mx) / 2;
    float d = mx - mn;

    if (d != 0) {
        sat = (light == 0 || light == 1) ? 0 : (mx - light) / std::min(light, 1 - light);
        if (mx == rgb.fR) {
            hue = (rgb.fG - rgb.fB) / d + (rgb.fG < rgb.fB ? 6 : 0);
        } else if (mx == rgb.fG) {
            hue = (rgb.fB - rgb.fR) / d + 2;
        } else {
            hue = (rgb.fR - rgb.fG) / d + 4;
        }

        hue *= 60;
    }
    if (sat == 0) {
        *hueIsPowerless = true;
    }
    return {hue, sat * 100, light * 100, rgb.fA};
}

static SkPMColor4f srgb_to_hwb(SkPMColor4f rgb, bool* hueIsPowerless) {
    SkPMColor4f hsl = srgb_to_hsl(rgb, hueIsPowerless);
    float white = std::min({rgb.fR, rgb.fG, rgb.fB});
    float black = 1 - std::max({rgb.fR, rgb.fG, rgb.fB});
    return {hsl.fR, white * 100, black * 100, rgb.fA};
}

static SkPMColor4f xyzd50_to_lab(SkPMColor4f xyz, bool* /*hueIsPowerless*/) {
    constexpr float D50[3] = {0.3457f / 0.3585f, 1.0f, (1.0f - 0.3457f - 0.3585f) / 0.3585f};

    constexpr float e = 216.0f / 24389;
    constexpr float k = 24389.0f / 27;

    SkPMColor4f f;
    for (int i = 0; i < 3; ++i) {
        float v = xyz[i] / D50[i];
        f[i] = (v > e) ? std::cbrtf(v) : (k * v + 16) / 116;
    }

    return {(116 * f[1]) - 16, 500 * (f[0] - f[1]), 200 * (f[1] - f[2]), xyz.fA};
}

// The color space is technically LCH, but we produce HCL, so that all polar spaces have hue in the
// first component. This simplifies the hue handling for HueMethod and premul/unpremul.
static SkPMColor4f xyzd50_to_hcl(SkPMColor4f xyz, bool* hueIsPowerless) {
    SkPMColor4f Lab = xyzd50_to_lab(xyz, hueIsPowerless);
    float hue = sk_float_radians_to_degrees(atan2f(Lab[2], Lab[1]));
    float chroma = sqrtf(Lab[1] * Lab[1] + Lab[2] * Lab[2]);
    // The LCH math produces small-ish (but not tiny) chroma values for achromatic colors:
    constexpr float kMaxChromaForPowerlessHue = 1e-2f;
    if (chroma <= kMaxChromaForPowerlessHue) {
        *hueIsPowerless = true;
    }
    return {hue >= 0 ? hue : hue + 360, chroma, Lab[0], xyz.fA};
}

// https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
static SkPMColor4f lin_srgb_to_oklab(SkPMColor4f rgb, bool* /*hueIsPowerless*/) {
    float l = 0.4122214708f * rgb.fR + 0.5363325363f * rgb.fG + 0.0514459929f * rgb.fB;
    float m = 0.2119034982f * rgb.fR + 0.6806995451f * rgb.fG + 0.1073969566f * rgb.fB;
    float s = 0.0883024619f * rgb.fR + 0.2817188376f * rgb.fG + 0.6299787005f * rgb.fB;
    l = std::cbrtf(l);
    m = std::cbrtf(m);
    s = std::cbrtf(s);
    return {0.2104542553f * l + 0.7936177850f * m - 0.0040720468f * s,
            1.9779984951f * l - 2.4285922050f * m + 0.4505937099f * s,
            0.0259040371f * l + 0.7827717662f * m - 0.8086757660f * s,
            rgb.fA};
}

// The color space is technically OkLCH, but we produce HCL, so that all polar spaces have hue in
// the first component. This simplifies the hue handling for HueMethod and premul/unpremul.
static SkPMColor4f lin_srgb_to_okhcl(SkPMColor4f rgb, bool* hueIsPowerless) {
    SkPMColor4f OKLab = lin_srgb_to_oklab(rgb, hueIsPowerless);
    float hue = sk_float_radians_to_degrees(atan2f(OKLab[2], OKLab[1]));
    float chroma = sqrtf(OKLab[1] * OKLab[1] + OKLab[2] * OKLab[2]);
    // The OKLCH math produces very small chroma values for achromatic colors:
    constexpr float kMaxChromaForPowerlessHue = 1e-6f;
    if (chroma <= kMaxChromaForPowerlessHue) {
        *hueIsPowerless = true;
    }
    return {hue >= 0 ? hue : hue + 360, chroma, OKLab[0], rgb.fA};
}

static SkPMColor4f premul_polar(SkPMColor4f hsl) {
    return {hsl.fR, hsl.fG * hsl.fA, hsl.fB * hsl.fA, hsl.fA};
}

static SkPMColor4f premul_rgb(SkPMColor4f rgb) {
    return {rgb.fR * rgb.fA, rgb.fG * rgb.fA, rgb.fB * rgb.fA, rgb.fA};
}

static bool color_space_is_polar(SkGradientShader::Interpolation::ColorSpace cs) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    switch (cs) {
        case ColorSpace::kLCH:
        case ColorSpace::kOKLCH:
        case ColorSpace::kHSL:
        case ColorSpace::kHWB:
            return true;
        default:
            return false;
    }
}

// Given `colors` in `src` color space, an interpolation space, and a `dst` color space,
// we are doing several things. First, some definitions:
//
// The interpolation color space is "special" if it can't be represented as an SkColorSpace. This
// applies to any color space that isn't an RGB space, like Lab or HSL. These need special handling
// because we have to run bespoke code to do the conversion (before interpolation here, and after
// interpolation in the backend shader/pipeline).
//
// The interpolation color space is "polar" if it involves hue (HSL, HWB, LCH, Oklch). These need
// special handling, becuase hue is never premultiplied, and because HueMethod comes into play.
//
// 1) Pick an `intermediate` SkColorSpace. If the interpolation color space is not "special",
//    (kDestination, kSRGB, etc... ), then `intermediate` is exact. Otherwise, `intermediate` is the
//    RGB space that prepares us to do the final conversion. For example, conversion to Lab starts
//    with XYZD50, so `intermediate` will be XYZD50 if we're actually interpolating in Lab.
// 2) Transform all colors to the `intermediate` color space, leaving them unpremultiplied.
// 3) If the interpolation color space is "special", transform the colors to that space.
// 4) If the interpolation color space is "polar", adjust the angles to respect HueMethod.
// 5) If premul interpolation is requested, apply that. For "polar" interpolated colors, don't
//    premultiply hue, only the other two channels. Note that there are four polar spaces.
//    Two have hue as the first component, and two have it as the third component. To reduce
//    complexity, we always store hue in the first component, swapping it with luminance for
//    LCH and Oklch. The backend code (eg, shaders) needs to know about this.
SkColor4fXformer::SkColor4fXformer(const SkGradientBaseShader* shader,
                                   SkColorSpace* dst,
                                   bool forceExplicitPositions) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    using HueMethod = SkGradientShader::Interpolation::HueMethod;

    int colorCount = shader->fColorCount;
    const SkGradientShader::Interpolation interpolation = shader->fInterpolation;

    // 0) Copy the shader's position pointer. Certain interpolation modes might force us to add
    //    new stops, in which case we'll allocate & edit the positions.
    fPositions = shader->fPositions;

    // 1) Determine the color space of our intermediate colors.
    fIntermediateColorSpace = intermediate_color_space(interpolation.fColorSpace, dst);

    // 2) Convert all colors to the intermediate color space
    auto info = SkImageInfo::Make(colorCount, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType);

    auto dstInfo = info.makeColorSpace(fIntermediateColorSpace);
    auto srcInfo = info.makeColorSpace(shader->fColorSpace);

    fColors.reset(colorCount);
    SkAssertResult(SkConvertPixels(dstInfo,
                                   fColors.begin(),
                                   info.minRowBytes(),
                                   srcInfo,
                                   shader->fColors,
                                   info.minRowBytes()));

    // 3) Transform to the interpolation color space (if it's special)
    ConvertColorProc convertFn = nullptr;
    switch (interpolation.fColorSpace) {
        case ColorSpace::kHSL:           convertFn = srgb_to_hsl;       break;
        case ColorSpace::kHWB:           convertFn = srgb_to_hwb;       break;
        case ColorSpace::kLab:           convertFn = xyzd50_to_lab;     break;
        case ColorSpace::kLCH:           convertFn = xyzd50_to_hcl;     break;
        case ColorSpace::kOKLab:         convertFn = lin_srgb_to_oklab; break;
        case ColorSpace::kOKLabGamutMap: convertFn = lin_srgb_to_oklab; break;
        case ColorSpace::kOKLCH:         convertFn = lin_srgb_to_okhcl; break;
        case ColorSpace::kOKLCHGamutMap: convertFn = lin_srgb_to_okhcl; break;
        default: break;
    }

    skia_private::STArray<4, bool> hueIsPowerless;
    bool anyPowerlessHue = false;
    hueIsPowerless.push_back_n(colorCount, false);
    if (convertFn) {
        for (int i = 0; i < colorCount; ++i) {
            fColors[i] = convertFn(fColors[i], hueIsPowerless.data() + i);
            anyPowerlessHue = anyPowerlessHue || hueIsPowerless[i];
        }
    }

    if (anyPowerlessHue) {
        // In theory, if we knew we were just going to adjust the existing colors (without adding
        // new ones), we could do it all in-place. To keep things simple, we always generate the
        // new colors in separate storage.
        ColorStorage newColors;
        PositionStorage newPositions;

        for (int i = 0; i < colorCount; ++i) {
            const SkPMColor4f& curColor = fColors[i];
            float curPos = shader->getPos(i);

            if (!hueIsPowerless[i]) {
                newColors.push_back(curColor);
                newPositions.push_back(curPos);
                continue;
            }

            auto colorWithHueFrom = [](const SkPMColor4f& color, const SkPMColor4f& hueColor) {
                // If we have any powerless hue, then all colors are already in (some) polar space,
                // and they all store their hue in the red channel.
                return SkPMColor4f{hueColor.fR, color.fG, color.fB, color.fA};
            };

            // In each case, we might be copying a powerless (invalid) hue from the neighbor, but
            // that should be fine, as it will match that neighbor perfectly, and any hue is ok.
            if (i != 0) {
                newPositions.push_back(curPos);
                newColors.push_back(colorWithHueFrom(curColor, fColors[i - 1]));
            }
            if (i != colorCount - 1) {
                newPositions.push_back(curPos);
                newColors.push_back(colorWithHueFrom(curColor, fColors[i + 1]));
            }
        }

        fColors.swap(newColors);
        fPositionStorage.swap(newPositions);
        fPositions = fPositionStorage.data();
        colorCount = fColors.size();
    }

    // 4) For polar colors, adjust hue values to respect the hue method. We're using a trick here...
    //    The specification looks at adjacent colors, and adjusts one or the other. Because we store
    //    the stops in uniforms (and our backend conversions normalize the hue angle), we can
    //    instead always apply the adjustment to the *second* color. That lets us keep a running
    //    total, and do a single pass across all the colors to respect the requested hue method,
    //    without needing to do any extra work per-pixel.
    if (color_space_is_polar(interpolation.fColorSpace)) {
        float delta = 0;
        for (int i = 0; i < colorCount - 1; ++i) {
            float h1 = fColors[i].fR;
            float& h2 = fColors[i + 1].fR;
            h2 += delta;
            switch (interpolation.fHueMethod) {
                case HueMethod::kShorter:
                    if (h2 - h1 > 180) {
                        h2 -= 360;  // i.e. h1 += 360
                        delta -= 360;
                    } else if (h2 - h1 < -180) {
                        h2 += 360;
                        delta += 360;
                    }
                    break;
                case HueMethod::kLonger:
                    if ((i == 0 && shader->fFirstStopIsImplicit) ||
                        (i == colorCount - 2 && shader->fLastStopIsImplicit)) {
                        // Do nothing. We don't want to introduce a full revolution for these stops
                        // Full rationale at skbug.com/40044215
                    } else if (0 < h2 - h1 && h2 - h1 < 180) {
                        h2 -= 360;  // i.e. h1 += 360
                        delta -= 360;
                    } else if (-180 < h2 - h1 && h2 - h1 <= 0) {
                        h2 += 360;
                        delta += 360;
                    }
                    break;
                case HueMethod::kIncreasing:
                    if (h2 < h1) {
                        h2 += 360;
                        delta += 360;
                    }
                    break;
                case HueMethod::kDecreasing:
                    if (h1 < h2) {
                        h2 -= 360;  // i.e. h1 += 360;
                        delta -= 360;
                    }
                    break;
            }
        }
    }

    // 5) Apply premultiplication
    PremulColorProc premulFn = nullptr;
    if (static_cast<bool>(interpolation.fInPremul)) {
        switch (interpolation.fColorSpace) {
            case ColorSpace::kHSL:
            case ColorSpace::kHWB:
            case ColorSpace::kLCH:
            case ColorSpace::kOKLCH:
                premulFn = premul_polar;
                break;
            default:
                premulFn = premul_rgb;
                break;
        }
    }

    if (premulFn) {
        for (int i = 0; i < colorCount; ++i) {
            fColors[i] = premulFn(fColors[i]);
        }
    }

    // Ganesh requires that the positions be explicit (rather than implicitly evenly spaced)
    if (forceExplicitPositions && !fPositions) {
        fPositionStorage.reserve_exact(colorCount);
        float posScale = 1.0f / (colorCount - 1);
        for (int i = 0; i < colorCount; i++) {
            fPositionStorage.push_back(i * posScale);
        }
        fPositions = fPositionStorage.data();
    }
}

SkColorConverter::SkColorConverter(const SkColor* colors, int count) {
    constexpr float ONE_OVER_255 = 1.f / 255;
    for (int i = 0; i < count; ++i) {
        fColors4f.push_back({SkColorGetR(colors[i]) * ONE_OVER_255,
                             SkColorGetG(colors[i]) * ONE_OVER_255,
                             SkColorGetB(colors[i]) * ONE_OVER_255,
                             SkColorGetA(colors[i]) * ONE_OVER_255});
    }
}

void SkGradientBaseShader::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColors[i] = fColors[i];
                }
            }
            if (info->fColorOffsets) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColorOffsets[i] = this->getPos(i);
                }
            }
        }
        info->fColorCount = fColorCount;
        info->fTileMode = fTileMode;

        info->fGradientFlags =
                this->interpolateInPremul() ? SkGradientShader::kInterpolateColorsInPremul_Flag : 0;
    }
}

// Return true if these parameters are valid/legal/safe to construct a gradient
//
bool SkGradientBaseShader::ValidGradient(const SkColor4f colors[],
                                         int count,
                                         SkTileMode tileMode,
                                         const Interpolation& interpolation) {
    return nullptr != colors && count >= 1 && (unsigned)tileMode < kSkTileModeCount &&
           (unsigned)interpolation.fColorSpace < Interpolation::kColorSpaceCount &&
           (unsigned)interpolation.fHueMethod < Interpolation::kHueMethodCount;
}

SkGradientBaseShader::Descriptor::Descriptor(const SkColor4f colors[],
                                             sk_sp<SkColorSpace> colorSpace,
                                             const SkScalar positions[],
                                             int colorCount,
                                             SkTileMode mode,
                                             const Interpolation& interpolation)
        : fColors(colors)
        , fColorSpace(std::move(colorSpace))
        , fPositions(positions)
        , fColorCount(colorCount)
        , fTileMode(mode)
        , fInterpolation(interpolation) {
    SkASSERT(fColorCount > 1);
}

static SkColor4f average_gradient_color(const SkColor4f colors[],
                                        const SkScalar pos[],
                                        int colorCount) {
    // The gradient is a piecewise linear interpolation between colors. For a given interval,
    // the integral between the two endpoints is 0.5 * (ci + cj) * (pj - pi), which provides that
    // intervals average color. The overall average color is thus the sum of each piece. The thing
    // to keep in mind is that the provided gradient definition may implicitly use p=0 and p=1.
    skvx::float4 blend(0.0f);
    for (int i = 0; i < colorCount - 1; ++i) {
        // Calculate the average color for the interval between pos(i) and pos(i+1)
        auto c0 = skvx::float4::Load(&colors[i]);
        auto c1 = skvx::float4::Load(&colors[i + 1]);

        // when pos == null, there are colorCount uniformly distributed stops, going from 0 to 1,
        // so pos[i + 1] - pos[i] = 1/(colorCount-1)
        SkScalar w;
        if (pos) {
            // Match position fixing in SkGradientShader's constructor, clamping positions outside
            // [0, 1] and forcing the sequence to be monotonic
            SkScalar p0 = SkTPin(pos[i], 0.f, 1.f);
            SkScalar p1 = SkTPin(pos[i + 1], p0, 1.f);
            w = p1 - p0;

            // And account for any implicit intervals at the start or end of the positions
            if (i == 0) {
                if (p0 > 0.0f) {
                    // The first color is fixed between p = 0 to pos[0], so 0.5*(ci + cj)*(pj - pi)
                    // becomes 0.5*(c + c)*(pj - 0) = c * pj
                    auto c = skvx::float4::Load(&colors[0]);
                    blend += p0 * c;
                }
            }
            if (i == colorCount - 2) {
                if (p1 < 1.f) {
                    // The last color is fixed between pos[n-1] to p = 1, so 0.5*(ci + cj)*(pj - pi)
                    // becomes 0.5*(c + c)*(1 - pi) = c * (1 - pi)
                    auto c = skvx::float4::Load(&colors[colorCount - 1]);
                    blend += (1.f - p1) * c;
                }
            }
        } else {
            w = 1.f / (colorCount - 1);
        }

        blend += 0.5f * w * (c1 + c0);
    }

    SkColor4f avg;
    blend.store(&avg);
    return avg;
}

// Except for special circumstances of clamped gradients, every gradient shape--when degenerate--
// can be mapped to the same fallbacks. The specific shape factories must account for special
// clamped conditions separately, this will always return the last color for clamped gradients.
sk_sp<SkShader> SkGradientBaseShader::MakeDegenerateGradient(const SkColor4f colors[],
                                                             const SkScalar pos[],
                                                             int colorCount,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             SkTileMode mode) {
    switch (mode) {
        case SkTileMode::kDecal:
            // normally this would reject the area outside of the interpolation region, so since
            // inside region is empty when the radii are equal, the entire draw region is empty
            return SkShaders::Empty();
        case SkTileMode::kRepeat:
        case SkTileMode::kMirror:
            // repeat and mirror are treated the same: the border colors are never visible,
            // but approximate the final color as infinite repetitions of the colors, so
            // it can be represented as the average color of the gradient.
            return SkShaders::Color(average_gradient_color(colors, pos, colorCount),
                                    std::move(colorSpace));
        case SkTileMode::kClamp:
            // Depending on how the gradient shape degenerates, there may be a more specialized
            // fallback representation for the factories to use, but this is a reasonable default.
            return SkShaders::Color(colors[colorCount - 1], std::move(colorSpace));
    }
    SkDEBUGFAIL("Should not be reached");
    return nullptr;
}
