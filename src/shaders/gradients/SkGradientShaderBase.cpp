/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/shaders/gradients/SkGradientShaderBase.h"

#include "include/core/SkColorSpace.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorSpacePriv.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkVM.h"
#include "src/core/SkWriteBuffer.h"

#if defined(SK_GRAPHITE)
#include "src/core/SkColorSpacePriv.h"
#include "src/gpu/graphite/KeyContext.h"
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif

#include <cmath>

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

SkGradientShaderBase::Descriptor::Descriptor() {
    sk_bzero(this, sizeof(*this));
    fTileMode = SkTileMode::kClamp;
}
SkGradientShaderBase::Descriptor::~Descriptor() = default;

void SkGradientShaderBase::flatten(SkWriteBuffer& buffer) const {
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

    buffer.writeColor4fArray(colors, colorCount);
    if (colorSpaceData) {
        buffer.writeDataAsByteArray(colorSpaceData.get());
    }
    if (positions) {
        buffer.writeScalarArray(positions, colorCount);
    }
}

template <int N, typename T, bool MEM_MOVE>
static bool validate_array(SkReadBuffer& buffer, size_t count, SkSTArray<N, T, MEM_MOVE>* array) {
    if (!buffer.validateCanReadN<T>(count)) {
        return false;
    }

    array->resize_back(count);
    return true;
}

bool SkGradientShaderBase::DescriptorScope::unflatten(SkReadBuffer& buffer,
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
          buffer.readColor4fArray(fColorStorage.begin(), fColorCount))) {
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
              buffer.readScalarArray(fPositionStorage.begin(), fColorCount))) {
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

SkGradientShaderBase::SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit)
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
    // check if we need to add in start and/or end position/colors
    if (desc.fPositions) {
        fFirstStopIsImplicit = desc.fPositions[0] != 0;
        fLastStopIsImplicit = desc.fPositions[desc.fColorCount - 1] != SK_Scalar1;
        fColorCount += fFirstStopIsImplicit + fLastStopIsImplicit;
    }

    size_t storageSize =
            fColorCount * (sizeof(SkColor4f) + (desc.fPositions ? sizeof(SkScalar) : 0));
    fColors    = reinterpret_cast<SkColor4f*>(fStorage.reset(storageSize));
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
        *positions++ = prev; // force the first pos to 0

        int startIndex = fFirstStopIsImplicit ? 0 : 1;
        int count = desc.fColorCount + fLastStopIsImplicit;

        bool uniformStops = true;
        const SkScalar uniformStep = desc.fPositions[startIndex] - prev;
        for (int i = startIndex; i < count; i++) {
            // Pin the last value to 1.0, and make sure pos is monotonic.
            auto curr = (i == desc.fColorCount) ? 1 : SkTPin(desc.fPositions[i], prev, 1.0f);
            uniformStops &= SkScalarNearlyEqual(uniformStep, curr - prev);

            *positions++ = prev = curr;
        }

        // If the stops are uniform, treat them as implicit.
        if (uniformStops) {
            fPositions = nullptr;
        }
    }
}

SkGradientShaderBase::~SkGradientShaderBase() {}

static void add_stop_color(SkRasterPipeline_GradientCtx* ctx, size_t stop,
                           SkPMColor4f Fs, SkPMColor4f Bs) {
    (ctx->fs[0])[stop] = Fs.fR;
    (ctx->fs[1])[stop] = Fs.fG;
    (ctx->fs[2])[stop] = Fs.fB;
    (ctx->fs[3])[stop] = Fs.fA;

    (ctx->bs[0])[stop] = Bs.fR;
    (ctx->bs[1])[stop] = Bs.fG;
    (ctx->bs[2])[stop] = Bs.fB;
    (ctx->bs[3])[stop] = Bs.fA;
}

static void add_const_color(SkRasterPipeline_GradientCtx* ctx, size_t stop, SkPMColor4f color) {
    add_stop_color(ctx, stop, { 0, 0, 0, 0 }, color);
}

// Calculate a factor F and a bias B so that color = F*t + B when t is in range of
// the stop. Assume that the distance between stops is 1/gapCount.
static void init_stop_evenly(SkRasterPipeline_GradientCtx* ctx, float gapCount, size_t stop,
                             SkPMColor4f c_l, SkPMColor4f c_r) {
    // Clankium's GCC 4.9 targeting ARMv7 is barfing when we use Sk4f math here, so go scalar...
    SkPMColor4f Fs = {
        (c_r.fR - c_l.fR) * gapCount,
        (c_r.fG - c_l.fG) * gapCount,
        (c_r.fB - c_l.fB) * gapCount,
        (c_r.fA - c_l.fA) * gapCount,
    };
    SkPMColor4f Bs = {
        c_l.fR - Fs.fR*(stop/gapCount),
        c_l.fG - Fs.fG*(stop/gapCount),
        c_l.fB - Fs.fB*(stop/gapCount),
        c_l.fA - Fs.fA*(stop/gapCount),
    };
    add_stop_color(ctx, stop, Fs, Bs);
}

// For each stop we calculate a bias B and a scale factor F, such that
// for any t between stops n and n+1, the color we want is B[n] + F[n]*t.
static void init_stop_pos(SkRasterPipeline_GradientCtx* ctx, size_t stop, float t_l, float t_r,
                          SkPMColor4f c_l, SkPMColor4f c_r) {
    // See note about Clankium's old compiler in init_stop_evenly().
    SkPMColor4f Fs = {
        (c_r.fR - c_l.fR) / (t_r - t_l),
        (c_r.fG - c_l.fG) / (t_r - t_l),
        (c_r.fB - c_l.fB) / (t_r - t_l),
        (c_r.fA - c_l.fA) / (t_r - t_l),
    };
    SkPMColor4f Bs = {
        c_l.fR - Fs.fR*t_l,
        c_l.fG - Fs.fG*t_l,
        c_l.fB - Fs.fB*t_l,
        c_l.fA - Fs.fA*t_l,
    };
    ctx->ts[stop] = t_l;
    add_stop_color(ctx, stop, Fs, Bs);
}

void SkGradientShaderBase::AppendGradientFillStages(SkRasterPipeline* p,
                                                    SkArenaAlloc* alloc,
                                                    const SkPMColor4f* pmColors,
                                                    const SkScalar* positions,
                                                    int count) {
    // The two-stop case with stops at 0 and 1.
    if (count == 2 && positions == nullptr) {
        const SkPMColor4f c_l = pmColors[0],
                          c_r = pmColors[1];

        // See F and B below.
        auto ctx = alloc->make<SkRasterPipeline_EvenlySpaced2StopGradientCtx>();
        (skvx::float4::Load(c_r.vec()) - skvx::float4::Load(c_l.vec())).store(ctx->f);
        (                                skvx::float4::Load(c_l.vec())).store(ctx->b);

        p->append(SkRasterPipelineOp::evenly_spaced_2_stop_gradient, ctx);
    } else {
        auto* ctx = alloc->make<SkRasterPipeline_GradientCtx>();

        // Note: In order to handle clamps in search, the search assumes a stop conceptully placed
        // at -inf. Therefore, the max number of stops is fColorCount+1.
        for (int i = 0; i < 4; i++) {
            // Allocate at least at for the AVX2 gather from a YMM register.
            ctx->fs[i] = alloc->makeArray<float>(std::max(count + 1, 8));
            ctx->bs[i] = alloc->makeArray<float>(std::max(count + 1, 8));
        }

        if (positions == nullptr) {
            // Handle evenly distributed stops.

            size_t stopCount = count;
            float gapCount = stopCount - 1;

            SkPMColor4f c_l = pmColors[0];
            for (size_t i = 0; i < stopCount - 1; i++) {
                SkPMColor4f c_r = pmColors[i + 1];
                init_stop_evenly(ctx, gapCount, i, c_l, c_r);
                c_l = c_r;
            }
            add_const_color(ctx, stopCount - 1, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipelineOp::evenly_spaced_gradient, ctx);
        } else {
            // Handle arbitrary stops.

            ctx->ts = alloc->makeArray<float>(count + 1);

            // Remove the default stops inserted by SkGradientShaderBase::SkGradientShaderBase
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
            float  t_l = positions[firstStop];
            SkPMColor4f c_l = pmColors[firstStop];
            add_const_color(ctx, stopCount++, c_l);
            // N.B. lastStop is the index of the last stop, not one after.
            for (int i = firstStop; i < lastStop; i++) {
                float  t_r = positions[i + 1];
                SkPMColor4f c_r = pmColors[i + 1];
                SkASSERT(t_l <= t_r);
                if (t_l < t_r) {
                    init_stop_pos(ctx, stopCount, t_l, t_r, c_l, c_r);
                    stopCount += 1;
                }
                t_l = t_r;
                c_l = c_r;
            }

            ctx->ts[stopCount] = t_l;
            add_const_color(ctx, stopCount++, c_l);

            ctx->stopCount = stopCount;
            p->append(SkRasterPipelineOp::gradient, ctx);
        }
    }
}

bool SkGradientShaderBase::appendStages(const SkStageRec& rec, const MatrixRec& mRec) const {
    SkRasterPipeline* p = rec.fPipeline;
    SkArenaAlloc* alloc = rec.fAlloc;
    SkRasterPipeline_DecalTileCtx* decal_ctx = nullptr;

    std::optional<MatrixRec> newMRec = mRec.apply(rec, fPtsToUnit);
    if (!newMRec.has_value()) {
        return false;
    }

    SkRasterPipeline_<256> postPipeline;

    this->appendGradientStages(alloc, p, &postPipeline);

    switch(fTileMode) {
        case SkTileMode::kMirror: p->append(SkRasterPipelineOp::mirror_x_1); break;
        case SkTileMode::kRepeat: p->append(SkRasterPipelineOp::repeat_x_1); break;
        case SkTileMode::kDecal:
            decal_ctx = alloc->make<SkRasterPipeline_DecalTileCtx>();
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
    AppendGradientFillStages(p, alloc, xformedColors.fColors.begin(), fPositions, fColorCount);

    using ColorSpace = Interpolation::ColorSpace;
    bool colorIsPremul = this->interpolateInPremul();

    // If we interpolated premul colors in any of the special color spaces, we need to unpremul
    if (colorIsPremul && !fColorsAreOpaque) {
        switch (fInterpolation.fColorSpace) {
            case ColorSpace::kLab:
            case ColorSpace::kOKLab:
                p->append(SkRasterPipelineOp::unpremul);
                colorIsPremul = false;
                break;
            case ColorSpace::kLCH:
            case ColorSpace::kOKLCH:
            case ColorSpace::kHSL:
            case ColorSpace::kHWB:
                p->append(SkRasterPipelineOp::unpremul_polar);
                colorIsPremul = false;
                break;
            default: break;
        }
    }

    // Convert colors in exotic spaces back to their intermediate SkColorSpace
    switch (fInterpolation.fColorSpace) {
        case ColorSpace::kLab:   p->append(SkRasterPipelineOp::css_lab_to_xyz);           break;
        case ColorSpace::kOKLab: p->append(SkRasterPipelineOp::css_oklab_to_linear_srgb); break;
        case ColorSpace::kLCH:   p->append(SkRasterPipelineOp::css_hcl_to_lab);
                                 p->append(SkRasterPipelineOp::css_lab_to_xyz);           break;
        case ColorSpace::kOKLCH: p->append(SkRasterPipelineOp::css_hcl_to_lab);
                                 p->append(SkRasterPipelineOp::css_oklab_to_linear_srgb); break;
        case ColorSpace::kHSL:   p->append(SkRasterPipelineOp::css_hsl_to_srgb);          break;
        case ColorSpace::kHWB:   p->append(SkRasterPipelineOp::css_hwb_to_srgb);          break;
        default: break;
    }

    // Now transform from intermediate to destination color space.
    // See comments in GrGradientShader.cpp about the decisions here.
    SkColorSpace* dstColorSpace = rec.fDstCS ? rec.fDstCS : sk_srgb_singleton();
    SkAlphaType intermediateAlphaType = colorIsPremul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;
    // TODO(skia:13108): Get dst alpha type correctly
    SkAlphaType dstAlphaType = kPremul_SkAlphaType;

    if (fColorsAreOpaque) {
        intermediateAlphaType = dstAlphaType = kUnpremul_SkAlphaType;
    }

    alloc->make<SkColorSpaceXformSteps>(xformedColors.fIntermediateColorSpace.get(),
                                        intermediateAlphaType,
                                        dstColorSpace,
                                        dstAlphaType)
            ->apply(p);

    if (decal_ctx) {
        p->append(SkRasterPipelineOp::check_decal_mask, decal_ctx);
    }

    p->extend(postPipeline);

    return true;
}

// Color conversion functions used in gradient interpolation, based on
// https://www.w3.org/TR/css-color-4/#color-conversion-code
static skvm::Color css_lab_to_xyz(skvm::Color lab) {
    constexpr float k = 24389 / 27.0f;
    constexpr float e = 216 / 24389.0f;

    skvm::F32 f[3];
    f[1] = (lab.r + 16) * (1 / 116.0f);
    f[0] = (lab.g * (1 / 500.0f)) + f[1];
    f[2] = f[1] - (lab.b * (1 / 200.0f));

    skvm::F32 f_cubed[3] = { f[0]*f[0]*f[0], f[1]*f[1]*f[1], f[2]*f[2]*f[2] };

    skvm::F32 xyz[3] = {
        skvm::select(f_cubed[0] > e, f_cubed[0], (116 * f[0] - 16) * (1 / k)),
        skvm::select(lab.r > k * e , f_cubed[1], lab.r * (1 / k)),
        skvm::select(f_cubed[2] > e, f_cubed[2], (116 * f[2] - 16) * (1 / k))
    };

    constexpr float D50[3] = { 0.3457f / 0.3585f, 1.0f, (1.0f - 0.3457f - 0.3585f) / 0.3585f };
    return skvm::Color { xyz[0]*D50[0], xyz[1]*D50[1], xyz[2]*D50[2], lab.a };
}

// Skia stores all polar colors with hue in the first component, so this "LCH -> Lab" transform
// actually takes "HCL". This is also used to do the same polar transform for OkHCL to OkLAB.
static skvm::Color css_hcl_to_lab(skvm::Color hcl) {
    skvm::F32 hueRadians = hcl.r * (SK_FloatPI / 180);
    return skvm::Color {
        hcl.b,
        hcl.g * approx_cos(hueRadians),
        hcl.g * approx_sin(hueRadians),
        hcl.a
    };
}

static skvm::Color css_hcl_to_xyz(skvm::Color hcl) {
    return css_lab_to_xyz(css_hcl_to_lab(hcl));
}

static skvm::Color css_oklab_to_linear_srgb(skvm::Color oklab) {
    skvm::F32 l_ = oklab.r + 0.3963377774f * oklab.g + 0.2158037573f * oklab.b,
              m_ = oklab.r - 0.1055613458f * oklab.g - 0.0638541728f * oklab.b,
              s_ = oklab.r - 0.0894841775f * oklab.g - 1.2914855480f * oklab.b;

    skvm::F32 l = l_*l_*l_,
              m = m_*m_*m_,
              s = s_*s_*s_;

    return skvm::Color {
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
        oklab.a
    };

}

static skvm::Color css_okhcl_to_linear_srgb(skvm::Color okhcl) {
    return css_oklab_to_linear_srgb(css_hcl_to_lab(okhcl));
}

static skvm::F32 mod_f(skvm::F32 x, float y) {
    return x - y * skvm::floor(x * (1 / y));
}

static skvm::Color css_hsl_to_srgb(skvm::Color hsl) {
    hsl.r = mod_f(hsl.r, 360);
    hsl.r = skvm::select(hsl.r < 0, hsl.r + 360, hsl.r);

    hsl.g *= 0.01f;
    hsl.b *= 0.01f;

    skvm::F32 k[3] = {
        mod_f(0 + hsl.r * (1 / 30.0f), 12),
        mod_f(8 + hsl.r * (1 / 30.0f), 12),
        mod_f(4 + hsl.r * (1 / 30.0f), 12),
    };
    skvm::F32 a = hsl.g * min(hsl.b, 1 - hsl.b);
    return skvm::Color {
        hsl.b - a * clamp(min(k[0] - 3, 9 - k[0]), -1, 1),
        hsl.b - a * clamp(min(k[1] - 3, 9 - k[1]), -1, 1),
        hsl.b - a * clamp(min(k[2] - 3, 9 - k[2]), -1, 1),
        hsl.a
    };
}

static skvm::Color css_hwb_to_srgb(skvm::Color hwb, skvm::Builder* p) {
    hwb.g *= 0.01f;
    hwb.b *= 0.01f;

    skvm::F32 gray = hwb.g / (hwb.g + hwb.b);

    skvm::Color rgb = css_hsl_to_srgb(skvm::Color{hwb.r, p->splat(100.0f), p->splat(50.0f), hwb.a});
    rgb.r = rgb.r * (1 - hwb.g - hwb.b) + hwb.g;
    rgb.g = rgb.g * (1 - hwb.g - hwb.b) + hwb.g;
    rgb.b = rgb.b * (1 - hwb.g - hwb.b) + hwb.g;

    skvm::I32 isGray = (hwb.g + hwb.b) >= 1;

    return skvm::Color {
        select(isGray, gray, rgb.r),
        select(isGray, gray, rgb.g),
        select(isGray, gray, rgb.b),
        hwb.a
    };
}

skvm::Color SkGradientShaderBase::program(skvm::Builder* p,
                                          skvm::Coord device,
                                          skvm::Coord local,
                                          skvm::Color /*paint*/,
                                          const MatrixRec& mRec,
                                          const SkColorInfo& dstInfo,
                                          skvm::Uniforms* uniforms,
                                          SkArenaAlloc* alloc) const {
    if (!mRec.apply(p, &local, uniforms, fPtsToUnit).has_value()) {
        return {};
    }

    skvm::I32 mask = p->splat(~0);
    skvm::F32 t = this->transformT(p,uniforms, local, &mask);

    // Perhaps unexpectedly, clamping is handled naturally by our search, so we
    // don't explicitly clamp t to [0,1].  That clamp would break hard stops
    // right at 0 or 1 boundaries in kClamp mode.  (kRepeat and kMirror always
    // produce values in [0,1].)
    switch(fTileMode) {
        case SkTileMode::kClamp:
            break;

        case SkTileMode::kDecal:
            mask &= (t == clamp01(t));
            break;

        case SkTileMode::kRepeat:
            t = fract(t);
            break;

        case SkTileMode::kMirror: {
            // t = | (t-1) - 2*(floor( (t-1)*0.5 )) - 1 |
            //       {-A-}      {--------B-------}
            skvm::F32 A = t - 1.0f,
                      B = floor(A * 0.5f);
            t = abs(A - (B + B) - 1.0f);
        } break;
    }

    // Transform our colors as we want them interpolated, in dst color space, possibly premul.
    SkColor4fXformer xformedColors(this, dstInfo.colorSpace());
    const SkPMColor4f* rgba = xformedColors.fColors.begin();

    // Transform our colors into a scale factor f and bias b such that for
    // any t between stops i and i+1, the color we want is mad(t, f[i], b[i]).
    using F4 = skvx::Vec<4,float>;
    struct FB { F4 f,b; };
    skvm::Color color;

    auto uniformF = [&](float x) { return p->uniformF(uniforms->pushF(x)); };

    if (fColorCount == 2) {
        // 2-stop gradients have colors at 0 and 1, and so must be evenly spaced.
        SkASSERT(fPositions == nullptr);

        // With 2 stops, we upload the single FB as uniforms and interpolate directly with t.
        F4 lo = F4::Load(rgba + 0),
           hi = F4::Load(rgba + 1);
        F4 F = hi - lo,
           B = lo;

        auto T = clamp01(t);
        color = {
            T * uniformF(F[0]) + uniformF(B[0]),
            T * uniformF(F[1]) + uniformF(B[1]),
            T * uniformF(F[2]) + uniformF(B[2]),
            T * uniformF(F[3]) + uniformF(B[3]),
        };
    } else {
        // To handle clamps in search we add a conceptual stop at t=-inf, so we
        // may need up to fColorCount+1 FBs and fColorCount t stops between them:
        //
        //   FBs:         [color 0]  [color 0->1]  [color 1->2]  [color 2->3]  ...
        //   stops:  (-inf)        t0            t1            t2  ...
        //
        // Both these arrays could end up shorter if any hard stops share the same t.
        FB* fb = alloc->makeArrayDefault<FB>(fColorCount+1);
        std::vector<float> stops;  // TODO: SkSTArray?
        stops.reserve(fColorCount);

        // Here's our conceptual stop at t=-inf covering all t<=0, clamping to our first color.
        float  t_lo = this->getPos(0);
        F4 color_lo = F4::Load(rgba);
        fb[0] = { 0.0f, color_lo };
        // N.B. No stops[] entry for this implicit -inf.

        // Now the non-edge cases, calculating scale and bias between adjacent normal stops.
        for (int i = 1; i < fColorCount; i++) {
            float  t_hi = this->getPos(i);
            F4 color_hi = F4::Load(rgba + i);

            // If t_lo == t_hi, we're on a hard stop, and transition immediately to the next color.
            SkASSERT(t_lo <= t_hi);
            if (t_lo < t_hi) {
                F4 f = (color_hi - color_lo) / (t_hi - t_lo),
                   b = color_lo - f*t_lo;
                stops.push_back(t_lo);
                fb[stops.size()] = {f,b};
            }

            t_lo = t_hi;
            color_lo = color_hi;
        }
        // Anything >= our final t clamps to our final color.
        stops.push_back(t_lo);
        fb[stops.size()] = { 0.0f, color_lo };

        // We'll gather FBs from that array we just created.
        skvm::Uniform fbs = uniforms->pushPtr(fb);

        // Find the two stops we need to interpolate.
        skvm::I32 ix;
        if (fPositions == nullptr) {
            // Evenly spaced stops... we can calculate ix directly.
            ix = trunc(clamp(t * uniformF(stops.size() - 1) + 1.0f, 0.0f, uniformF(stops.size())));
        } else {
            // Starting ix at 0 bakes in our conceptual first stop at -inf.
            // TODO: good place to experiment with a loop in skvm.... stops.size() can be huge.
            ix = p->splat(0);
            for (float stop : stops) {
                // ix += (t >= stop) ? +1 : 0 ~~>
                // ix -= (t >= stop) ? -1 : 0
                ix -= (t >= uniformF(stop));
            }
            // TODO: we could skip any of the default stops GradientShaderBase's ctor added
            // to ensure the full [0,1] span is covered.  This linear search doesn't need
            // them for correctness, and it'd be up to two fewer stops to check.
            // N.B. we do still need those stops for the fPositions == nullptr direct math path.
        }

        // A scale factor and bias for each lane, 8 total.
        // TODO: simpler, faster, tidier to push 8 uniform pointers, one for each struct lane?
        ix = shl(ix, 3);
        skvm::F32 Fr = gatherF(fbs, ix + 0);
        skvm::F32 Fg = gatherF(fbs, ix + 1);
        skvm::F32 Fb = gatherF(fbs, ix + 2);
        skvm::F32 Fa = gatherF(fbs, ix + 3);

        skvm::F32 Br = gatherF(fbs, ix + 4);
        skvm::F32 Bg = gatherF(fbs, ix + 5);
        skvm::F32 Bb = gatherF(fbs, ix + 6);
        skvm::F32 Ba = gatherF(fbs, ix + 7);

        // This is what we've been building towards!
        color = {
            t * Fr + Br,
            t * Fg + Bg,
            t * Fb + Bb,
            t * Fa + Ba,
        };
    }

    using ColorSpace = Interpolation::ColorSpace;
    bool colorIsPremul = this->interpolateInPremul();

    // If we interpolated premul colors in any of the special color spaces, we need to unpremul
    if (colorIsPremul) {
        switch (fInterpolation.fColorSpace) {
            case ColorSpace::kLab:
            case ColorSpace::kOKLab:
                color = unpremul(color);
                colorIsPremul = false;
                break;
            case ColorSpace::kLCH:
            case ColorSpace::kOKLCH:
            case ColorSpace::kHSL:
            case ColorSpace::kHWB: {
                // Avoid unpremuling hue
                skvm::F32 hue = color.r;
                color = unpremul(color);
                color.r = hue;
                colorIsPremul = false;
            } break;
            default: break;
        }
    }

    // Convert colors in exotic spaces back to their intermediate SkColorSpace
    switch (fInterpolation.fColorSpace) {
            case ColorSpace::kLab:   color = css_lab_to_xyz(color);           break;
            case ColorSpace::kOKLab: color = css_oklab_to_linear_srgb(color); break;
            case ColorSpace::kLCH:   color = css_hcl_to_xyz(color);           break;
            case ColorSpace::kOKLCH: color = css_okhcl_to_linear_srgb(color); break;
            case ColorSpace::kHSL:   color = css_hsl_to_srgb(color);          break;
            case ColorSpace::kHWB:   color = css_hwb_to_srgb(color, p);       break;
            default: break;
    }

    // Now transform from intermediate to destination color space.
    // See comments in GrGradientShader.cpp about the decisions here.
    SkColorSpace* dstColorSpace = dstInfo.colorSpace() ? dstInfo.colorSpace() : sk_srgb_singleton();
    SkAlphaType intermediateAlphaType = colorIsPremul ? kPremul_SkAlphaType : kUnpremul_SkAlphaType;
    SkAlphaType dstAlphaType = dstInfo.alphaType();

    if (fColorsAreOpaque) {
        intermediateAlphaType = dstAlphaType = kUnpremul_SkAlphaType;
    }

    color = SkColorSpaceXformSteps{xformedColors.fIntermediateColorSpace.get(),
                                   intermediateAlphaType,
                                   dstColorSpace,
                                   dstAlphaType}
                    .program(p, uniforms, color);

    return {
        pun_to_F32(mask & pun_to_I32(color.r)),
        pun_to_F32(mask & pun_to_I32(color.g)),
        pun_to_F32(mask & pun_to_I32(color.b)),
        pun_to_F32(mask & pun_to_I32(color.a)),
    };
}

bool SkGradientShaderBase::isOpaque() const {
    return fColorsAreOpaque && (this->getTileMode() != SkTileMode::kDecal);
}

static unsigned rounded_divide(unsigned numer, unsigned denom) {
    return (numer + (denom >> 1)) / denom;
}

bool SkGradientShaderBase::onAsLuminanceColor(SkColor* lum) const {
    // we just compute an average color.
    // possibly we could weight this based on the proportional width for each color
    //   assuming they are not evenly distributed in the fPos array.
    int r = 0;
    int g = 0;
    int b = 0;
    const int n = fColorCount;
    // TODO: use linear colors?
    for (int i = 0; i < n; ++i) {
        SkColor c = this->getLegacyColor(i);
        r += SkColorGetR(c);
        g += SkColorGetG(c);
        b += SkColorGetB(c);
    }
    *lum = SkColorSetRGB(rounded_divide(r, n), rounded_divide(g, n), rounded_divide(b, n));
    return true;
}

static sk_sp<SkColorSpace> intermediate_color_space(SkGradientShader::Interpolation::ColorSpace cs,
                                                    SkColorSpace* dst) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    switch (cs) {
        case ColorSpace::kDestination: return sk_ref_sp(dst);

        // css-color-4 allows XYZD50 and XYZD65. For gradients, those are redundant. Interpolating
        // in any linear RGB space, (regardless of white point), gives the same answer.
        case ColorSpace::kSRGBLinear: return SkColorSpace::MakeSRGBLinear();

        case ColorSpace::kSRGB:
        case ColorSpace::kHSL:
        case ColorSpace::kHWB: return SkColorSpace::MakeSRGB();

        case ColorSpace::kLab:
        case ColorSpace::kLCH:
            // Conversion to Lab (and LCH) starts with XYZD50
            return SkColorSpace::MakeRGB(SkNamedTransferFn::kLinear, SkNamedGamut::kXYZ);

        case ColorSpace::kOKLab:
        case ColorSpace::kOKLCH:
            // The "standard" conversion to these spaces starts with XYZD65. That requires extra
            // effort to conjure. The author also has reference code for going directly from linear
            // sRGB, so we use that.
            // TODO(skia:13108): Even better would be to have an LMS color space, because the first
            // part of the conversion is a matrix multiply, which could be absorbed into the
            // color space xform.
            return SkColorSpace::MakeSRGBLinear();
    }
    SkUNREACHABLE;
}

typedef SkPMColor4f (*ConvertColorProc)(SkPMColor4f);

static SkPMColor4f srgb_to_hsl(SkPMColor4f rgb) {
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
    return { hue, sat * 100, light * 100, rgb.fA };
}

static SkPMColor4f srgb_to_hwb(SkPMColor4f rgb) {
    SkPMColor4f hsl = srgb_to_hsl(rgb);
    float white =     std::min({rgb.fR, rgb.fG, rgb.fB});
    float black = 1 - std::max({rgb.fR, rgb.fG, rgb.fB});
    return { hsl.fR, white * 100, black * 100, rgb.fA };
}

static SkPMColor4f xyzd50_to_lab(SkPMColor4f xyz) {
    constexpr float D50[3] = { 0.3457f / 0.3585f, 1.0f, (1.0f - 0.3457f - 0.3585f) / 0.3585f };

    constexpr float e = 216.0f / 24389;
    constexpr float k = 24389.0f / 27;

    SkPMColor4f f;
    for (int i = 0; i < 3; ++i) {
        float v = xyz[i] / D50[i];
        f[i] = (v > e) ? std::cbrtf(v) : (k * v + 16) / 116;
    }

    return { (116 * f[1]) - 16, 500 * (f[0] - f[1]), 200 * (f[1] - f[2]), xyz.fA };
}

// The color space is technically LCH, but we produce HCL, so that all polar spaces have hue in the
// first component. This simplifies the hue handling for HueMethod and premul/unpremul.
static SkPMColor4f xyzd50_to_hcl(SkPMColor4f xyz) {
    SkPMColor4f Lab = xyzd50_to_lab(xyz);
    float hue = sk_float_radians_to_degrees(atan2f(Lab[2], Lab[1]));
    return {hue >= 0 ? hue : hue + 360,
            sqrtf(Lab[1] * Lab[1] + Lab[2] * Lab[2]),
            Lab[0],
            xyz.fA};
}

// https://bottosson.github.io/posts/oklab/#converting-from-linear-srgb-to-oklab
static SkPMColor4f lin_srgb_to_oklab(SkPMColor4f rgb) {
    float l = 0.4122214708f * rgb.fR + 0.5363325363f * rgb.fG + 0.0514459929f * rgb.fB;
    float m = 0.2119034982f * rgb.fR + 0.6806995451f * rgb.fG + 0.1073969566f * rgb.fB;
    float s = 0.0883024619f * rgb.fR + 0.2817188376f * rgb.fG + 0.6299787005f * rgb.fB;
    l = std::cbrtf(l);
    m = std::cbrtf(m);
    s = std::cbrtf(s);
    return {
        0.2104542553f*l + 0.7936177850f*m - 0.0040720468f*s,
        1.9779984951f*l - 2.4285922050f*m + 0.4505937099f*s,
        0.0259040371f*l + 0.7827717662f*m - 0.8086757660f*s,
        rgb.fA
    };
}

// The color space is technically OkLCH, but we produce HCL, so that all polar spaces have hue in
// the first component. This simplifies the hue handling for HueMethod and premul/unpremul.
static SkPMColor4f lin_srgb_to_okhcl(SkPMColor4f rgb) {
    SkPMColor4f OKLab = lin_srgb_to_oklab(rgb);
    float hue = sk_float_radians_to_degrees(atan2f(OKLab[2], OKLab[1]));
    return {hue >= 0 ? hue : hue + 360,
            sqrtf(OKLab[1] * OKLab[1] + OKLab[2] * OKLab[2]),
            OKLab[0],
            rgb.fA};
}

static SkPMColor4f premul_polar(SkPMColor4f hsl) {
    return { hsl.fR, hsl.fG * hsl.fA, hsl.fB * hsl.fA, hsl.fA };
}

static SkPMColor4f premul_rgb(SkPMColor4f rgb) {
    return { rgb.fR * rgb.fA, rgb.fG * rgb.fA, rgb.fB * rgb.fA, rgb.fA };
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
SkColor4fXformer::SkColor4fXformer(const SkGradientShaderBase* shader, SkColorSpace* dst) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    using HueMethod = SkGradientShader::Interpolation::HueMethod;

    const int colorCount = shader->fColorCount;
    const SkGradientShader::Interpolation interpolation = shader->fInterpolation;

    // 1) Determine the color space of our intermediate colors
    fIntermediateColorSpace = intermediate_color_space(interpolation.fColorSpace, dst);

    // 2) Convert all colors to the intermediate color space
    auto info = SkImageInfo::Make(colorCount, 1, kRGBA_F32_SkColorType, kUnpremul_SkAlphaType);

    auto dstInfo = info.makeColorSpace(fIntermediateColorSpace);
    auto srcInfo = info.makeColorSpace(shader->fColorSpace);

    fColors.reset(colorCount);
    SkAssertResult(SkConvertPixels(dstInfo, fColors.begin(), info.minRowBytes(),
                                   srcInfo, shader->fColors, info.minRowBytes()));

    // 3) Transform to the interpolation color space (if it's special)
    ConvertColorProc convertFn = nullptr;
    switch (interpolation.fColorSpace) {
        case ColorSpace::kHSL:   convertFn = srgb_to_hsl;       break;
        case ColorSpace::kHWB:   convertFn = srgb_to_hwb;       break;
        case ColorSpace::kLab:   convertFn = xyzd50_to_lab;     break;
        case ColorSpace::kLCH:   convertFn = xyzd50_to_hcl;     break;
        case ColorSpace::kOKLab: convertFn = lin_srgb_to_oklab; break;
        case ColorSpace::kOKLCH: convertFn = lin_srgb_to_okhcl; break;
        default: break;
    }

    if (convertFn) {
        for (int i = 0; i < colorCount; ++i) {
            fColors[i] = convertFn(fColors[i]);
        }
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
            float  h1 = fColors[i].fR;
            float& h2 = fColors[i+1].fR;
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
                        // Full rationale at skbug.com/13941
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
    ConvertColorProc premulFn = nullptr;
    if (static_cast<bool>(interpolation.fInPremul)) {
        switch (interpolation.fColorSpace) {
            case ColorSpace::kHSL:
            case ColorSpace::kHWB:
            case ColorSpace::kLCH:
            case ColorSpace::kOKLCH: premulFn = premul_polar; break;
            default:                 premulFn = premul_rgb;   break;
        }
    }

    if (premulFn) {
        for (int i = 0; i < colorCount; ++i) {
            fColors[i] = premulFn(fColors[i]);
        }
    }
}

SkColorConverter::SkColorConverter(const SkColor* colors, int count) {
    const float ONE_OVER_255 = 1.f / 255;
    for (int i = 0; i < count; ++i) {
        fColors4f.push_back({ SkColorGetR(colors[i]) * ONE_OVER_255,
                              SkColorGetG(colors[i]) * ONE_OVER_255,
                              SkColorGetB(colors[i]) * ONE_OVER_255,
                              SkColorGetA(colors[i]) * ONE_OVER_255 });
    }
}

void SkGradientShaderBase::commonAsAGradient(GradientInfo* info) const {
    if (info) {
        if (info->fColorCount >= fColorCount) {
            if (info->fColors) {
                for (int i = 0; i < fColorCount; ++i) {
                    info->fColors[i] = this->getLegacyColor(i);
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
bool SkGradientShaderBase::ValidGradient(const SkColor4f colors[], int count, SkTileMode tileMode,
                                         const Interpolation& interpolation) {
    return nullptr != colors && count >= 1 && (unsigned)tileMode < kSkTileModeCount &&
           (unsigned)interpolation.fColorSpace < Interpolation::kColorSpaceCount &&
           (unsigned)interpolation.fHueMethod < Interpolation::kHueMethodCount;
}

SkGradientShaderBase::Descriptor::Descriptor(const SkColor4f colors[],
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

static SkColor4f average_gradient_color(const SkColor4f colors[], const SkScalar pos[],
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
sk_sp<SkShader> SkGradientShaderBase::MakeDegenerateGradient(const SkColor4f colors[],
                                                             const SkScalar pos[],
                                                             int colorCount,
                                                             sk_sp<SkColorSpace> colorSpace,
                                                             SkTileMode mode) {
    switch(mode) {
        case SkTileMode::kDecal:
            // normally this would reject the area outside of the interpolation region, so since
            // inside region is empty when the radii are equal, the entire draw region is empty
            return SkShaders::Empty();
        case SkTileMode::kRepeat:
        case SkTileMode::kMirror:
            // repeat and mirror are treated the same: the border colors are never visible,
            // but approximate the final color as infinite repetitions of the colors, so
            // it can be represented as the average color of the gradient.
            return SkShaders::Color(
                    average_gradient_color(colors, pos, colorCount), std::move(colorSpace));
        case SkTileMode::kClamp:
            // Depending on how the gradient shape degenerates, there may be a more specialized
            // fallback representation for the factories to use, but this is a reasonable default.
            return SkShaders::Color(colors[colorCount - 1], std::move(colorSpace));
    }
    SkDEBUGFAIL("Should not be reached");
    return nullptr;
}

SkGradientShaderBase::ColorStopOptimizer::ColorStopOptimizer(const SkColor4f* colors,
                                                             const SkScalar* pos,
                                                             int count,
                                                             SkTileMode mode)
        : fColors(colors)
        , fPos(pos)
        , fCount(count) {

    if (!pos || count != 3) {
        return;
    }

    if (SkScalarNearlyEqual(pos[0], 0.0f) &&
        SkScalarNearlyEqual(pos[1], 0.0f) &&
        SkScalarNearlyEqual(pos[2], 1.0f)) {

        if (SkTileMode::kRepeat == mode || SkTileMode::kMirror == mode ||
            colors[0] == colors[1]) {

            // Ignore the leftmost color/pos.
            fColors += 1;
            fPos    += 1;
            fCount   = 2;
        }
    } else if (SkScalarNearlyEqual(pos[0], 0.0f) &&
               SkScalarNearlyEqual(pos[1], 1.0f) &&
               SkScalarNearlyEqual(pos[2], 1.0f)) {

        if (SkTileMode::kRepeat == mode || SkTileMode::kMirror == mode ||
            colors[1] == colors[2]) {

            // Ignore the rightmost color/pos.
            fCount  = 2;
        }
    }
}

#if defined(SK_GRAPHITE)
// Please see GrGradientShader.cpp::make_interpolated_to_dst for substantial comments
// as to why this code is structured this way.
void SkGradientShaderBase::MakeInterpolatedToDst(
        const skgpu::graphite::KeyContext& keyContext,
        skgpu::graphite::PaintParamsKeyBuilder* builder,
        skgpu::graphite::PipelineDataGatherer* gatherer,
        const skgpu::graphite::GradientShaderBlocks::GradientData& gradData,
        const SkGradientShaderBase::Interpolation& interp,
        SkColorSpace* intermediateCS) {
    using ColorSpace = SkGradientShader::Interpolation::ColorSpace;
    using namespace skgpu::graphite;

    bool inputPremul = static_cast<bool>(interp.fInPremul);

    switch (interp.fColorSpace) {
        case ColorSpace::kLab:
        case ColorSpace::kOKLab:
        case ColorSpace::kLCH:
        case ColorSpace::kOKLCH:
        case ColorSpace::kHSL:
        case ColorSpace::kHWB:
            inputPremul = false;
            break;
        default:
            break;
    }

    const SkColorInfo& dstColorInfo = keyContext.dstColorInfo();

    SkColorSpace* dstColorSpace = dstColorInfo.colorSpace() ? dstColorInfo.colorSpace()
                                                            : sk_srgb_singleton();

    SkAlphaType intermediateAlphaType = inputPremul ? kPremul_SkAlphaType
                                                    : kUnpremul_SkAlphaType;

    ColorSpaceTransformBlock::ColorSpaceTransformData data(intermediateCS, intermediateAlphaType,
                                                           dstColorSpace, dstColorInfo.alphaType());

    // The gradient block and colorSpace conversion block need to be combined together
    // (via the colorFilterShader block) so that the localMatrix block can treat them as
    // one child.
    ColorFilterShaderBlock::BeginBlock(keyContext, builder, gatherer);

        GradientShaderBlocks::BeginBlock(keyContext, builder, gatherer, gradData);
        builder->endBlock();

        ColorSpaceTransformBlock::BeginBlock(keyContext, builder, gatherer, &data);
        builder->endBlock();

    builder->endBlock();
}
#endif
