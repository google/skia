/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormatXferFn.h"

#include "include/core/SkColorType.h"
#include "include/private/SkLog.h"
#include "src/core/SkAutoMalloc.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkFloatBits.h"
#include "src/core/SkHalf.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkVx.h"

#include <functional>

namespace skgpu::graphite {

namespace {

using TF = TextureFormat;

// This is intentionally not a class enum and is distinct from FormatXferOps for two reasons:
// 1. We can't use SkEnumBitmask inside template parameters for the row bit manipulation functions.
// 2. It's convenient to split the FormatXferOps into specific ops based on their conversion
//    direction that doesn't need to be exposed in the public API.
enum ExtendedFormatXferOp : uint8_t {
    kDropAlpha   = 0x1,  // FormatXferOp::kDropAlpha for CPU->GPU conversion
    kPadAlpha    = 0x2,  // FormatXferOp::kDropAlpha for GPU->CPU conversion
    kSwapRB      = 0x4,  // FormatXferOp::kSwapRB behaves the same for either conversion direction
    kForceOpaque = 0x8,  // RGBx handling to avoid raster pipeline is the same in both directions
    kIgnoreSrc   = 0x10, // The output value does not depend on the src, will be either 0 or 1.
};

using XferRowFn = std::function<void(const char* src, char* dst, int width)>;

template <typename Px, int N, typename PixelFn /* [](Vec<N,Px>, Px opaqueAlpha) -> Vec<N,Px> */>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp,
                             Px opaqueAlpha, Px zeroValue, PixelFn applyPixel) {
    using PxVec = skvx::Vec<N, Px>;

    if (srcBpp != 0) {
        // PxVec should be sufficient to hold N src and dst pixels, and should match at least one
        SkASSERT(sizeof(PxVec) >= (size_t)srcBpp*n && sizeof(PxVec) >= (size_t)dstBpp*n);
        SkASSERT(sizeof(PxVec) == (size_t)srcBpp*n || sizeof(PxVec) == (size_t)dstBpp*n);

        // NOTE: These must all be named explicitly, since using the & to auto-capture does not copy
        // them, so then their references become corrupted when executing the lambda later.
        return [n, srcBpp, dstBpp, opaqueAlpha, applyPixel](const char* src, char* dst, int width) {
            const int srcBppN = n * srcBpp;
            const int dstBppN = n * dstBpp;

            PxVec pixel{};
            while (width >= n) {
                memcpy(&pixel, src, srcBppN);
                pixel = applyPixel(pixel, opaqueAlpha);
                memcpy(dst, &pixel, dstBppN);

                width -= n;
                src += srcBppN;
                dst += dstBppN;
            }

            if (width > 0) {
                // Process tail that is less than a full vector
                SkASSERT(width < n);
                memcpy(&pixel, src, width * srcBpp);
                pixel = applyPixel(pixel, opaqueAlpha);
                memcpy(dst, &pixel, width * dstBpp);
            }
        };
    } else {
        // Ignore the source in favor of loading zeroValue into pixel each time
        SkASSERT(sizeof(PxVec) >= (size_t)dstBpp*n);

        return [n, opaqueAlpha, zeroValue, dstBpp, applyPixel](const char*, char* dst, int width) {
            const int dstBppN = n * dstBpp;

            PxVec pixel{};
            while (width >= n) {
                pixel = zeroValue;
                pixel = applyPixel(pixel, opaqueAlpha);
                memcpy(dst, &pixel, dstBppN);

                width -= n;
                dst += dstBppN;
            }

            if (width > 0) {
                // Process tail that is less than a full vector
                SkASSERT(width < n);
                pixel = zeroValue;
                pixel = applyPixel(pixel, opaqueAlpha);
                memcpy(dst, &pixel, width * dstBpp);
            }
        };
    }
}

// N represents the number of pixels being processed; each pixel is packed into a single element of
// the Vec. To implement kSwapRB, this requires that the R and B channels have the same number of
// bits. It arbitrarily requires that R is originally in the lower-significance bits relative to B.
template <typename Px, int N, uint8_t Ops, Px RShift, Px BShift, Px RBBits>
skvx::Vec<N, Px> apply_ops_packed(skvx::Vec<N, Px> pixel, Px opaqueAlpha) {
    static_assert(!(Ops & (kPadAlpha | kDropAlpha)), "Packed formats do not drop/pad alpha");
    static_assert(RShift < BShift, "Shifts are not in the correct significance order");
    static_assert(RShift + RBBits <= BShift, "Low swap channel overflows");
    static_assert(BShift + RBBits <= sizeof(Px)*8, "Hi swap channel overflows");

    if constexpr (Ops & kSwapRB) {
        static constexpr Px kChannelMask = (1 << RBBits) - 1;
        static constexpr Px kRMask = kChannelMask << RShift;
        static constexpr Px kBMask = kChannelMask << BShift;

        pixel = (pixel & ~(kRMask | kBMask)) |            // Preserve non-RB bits
                ((pixel & kRMask) << (BShift - RShift)) | // Move red (lo) to blue (hi) position
                ((pixel & kBMask) >> (BShift - RShift));  // Move blue (hi) to red (lo) position
    }

    if constexpr (Ops & kForceOpaque) {
        // This assumes the opaqueAlpha value will have 0 bits set in the R,G,B channels.
        pixel = pixel | opaqueAlpha;
    }

    return pixel;
}

// NOTE: This takes no parameters for alpha because none of the formats that use this support
// kDrop/PadAlpha. Since the channels are packed into Px, both source and dst must have the same bpp
// `opaqueAlpha` specifies the bit pattern for the entire pixel representing {0, 0, 0, 1.0}.
// `zeroValue` specifies the bit pattern for the entire pixel representing {0, 0, 0, 0}.
template <typename Px, Px RShift, Px BShift, Px RBBits>
XferRowFn xfer_rows_packed(uint8_t ops, Px opaqueAlpha, Px zeroValue = 0) {
    static constexpr int kBpp = sizeof(Px);
    static constexpr int N = 16 / sizeof(Px); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4 || N == 8);

#define RETURN_XFER_ROW_FN(ops) \
        case ops: \
            return create_xfer_row_fn<Px, N>( \
                    N, (ops) & kIgnoreSrc ? 0 : kBpp, kBpp, opaqueAlpha, zeroValue, \
                    apply_ops_packed<Px, N, (ops) & ~kIgnoreSrc, RShift, BShift, RBBits>); \

    switch (ops) {
        // The expected combination of ExtendedFormatXferOps
        RETURN_XFER_ROW_FN(kSwapRB)
        RETURN_XFER_ROW_FN(kIgnoreSrc)
        RETURN_XFER_ROW_FN(kForceOpaque)
        RETURN_XFER_ROW_FN(kForceOpaque | kSwapRB)
        RETURN_XFER_ROW_FN(kForceOpaque | kIgnoreSrc)
        default:
            SK_ABORT("Unsupported ExtendedFormatXferOps combination: %u", ops);
    }
#undef RETURN_XFER_ROW_FN
}

// N represents the number of pixels being processed; some Ops are only supported for C == 1 or
// C == 4. For other channel counts, those Ops are ignored at compile time (to make the ops switch
// tables easier to write). Hopefully the linker will see that they are equivalent to the template
// w/o the incompatible Ops.
template <typename Cx, int N, int CPow2, uint8_t Ops>
skvx::Vec<CPow2*N, Cx> apply_ops_by_channel(skvx::Vec<CPow2*N, Cx> pixel, Cx opaqueAlpha) {
    static_assert(CPow2 == 1 || CPow2 == 2 || CPow2 == 4);

    if constexpr (Ops & kPadAlpha && CPow2 == 4) {
        // If we are padding alpha, we are moving from a 3-channel source data (loaded in the first
        // N*3 values of pixel) to a 4-channel value. 3*N+1 holds undefined data after loading the
        // pixel, which we can set to the opaque alpha value. Then we shuffle the slots to spread
        // out each pixel's R,G, and B values and insert copies of the opaque alpha value.
        static constexpr int kA = 3*N;
        pixel[kA] = opaqueAlpha;

        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA, 6,7,8,kA, 9,10,11,kA>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA>(pixel);
        } else {
            // No shuffling needed for N=1, since pixel == shuffle<0,1,2,A>(pixel)
            static_assert(N == 1);
        }
    }

    // Ops that assume 4 channels can be applied between kPadAlpha and kDropAlpha w/o worrying about
    // how to handle the 3-channel formats.
    if constexpr (Ops & kSwapRB && CPow2 == 4) {
        // For all 3 and 4 channel formats, it is assumed that R and B are in channels 0 and 2.
        if constexpr (N == 4) {
            pixel = skvx::shuffle<2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<2,1,0,3, 6,5,4,7>(pixel);
        } else {
            static_assert(N == 1);
            pixel = skvx::shuffle<2,1,0,3>(pixel);
        }
    }

    // 2-channel formats are incompatible with forcing opaque since there's never an alpha channel
    if constexpr (Ops & kForceOpaque && (CPow2 != 2)) {
        if constexpr (CPow2 == 1) {
            // For single-channel formats, the opaque alpha gets splatted over the whole vector
            pixel = opaqueAlpha;
        } else {
            // Between kPadAlpha and kDropAlpha, it is assumed that alpha is always channel 3
            if constexpr (N == 4) {
                pixel[3] = pixel[7] = pixel[11] = pixel[15] = opaqueAlpha;
            } else if constexpr (N == 2) {
                pixel[3] = pixel[7] = opaqueAlpha;
            } else {
                static_assert(N == 1);
                pixel[3] = opaqueAlpha;
            }
        }
    }

    if constexpr (Ops & kDropAlpha && CPow2 == 4) {
        // If we are dropping alpha, we need to shuffle the R,G, and B values of the 4-channel
        // source data into the first N*3 slots of the returned pixel. The remaining N slots will
        // be ignored by the final memcpy.
        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 8,9,10, 12,13,14, 14,14,14,14>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 6,6>(pixel);
        } else {
            // No shuffling needed for N=1, since pixel == shuffle<0,1,2,_>(pixel)
            static_assert(N == 1);
        }
    }

    return pixel;
}

// NOTE: This takes no parameters for SwapRB or DropAlpha because for all formats that use this,
// SwapRB involves swapping channel 0 and channel 2, and dropping alpha removes channel 3. This
// can be parameterized via template parameters to be able to push into the skvx::shuffle calls
// if needed in the future.
// `opaqueAlpha` specifies the bit pattern for a channel representing 1.0.
// `zeroValue` specifies the bit pattern for a channel representing 0.0.
template <typename Cx, int C>
XferRowFn xfer_rows_by_channel(uint8_t ops, Cx opaqueAlpha, Cx zeroValue=0) {
    static_assert(C == 1 || C == 2 || C == 3 || C == 4);
    static constexpr int CPow2 = SkNextPow2(C);
    static constexpr int N = 16 / (CPow2 * sizeof(Cx)); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4 || N == 8 || N == 16);

    int srcBpp = C * sizeof(Cx);
    int dstBpp = C * sizeof(Cx);
    if (ops & kDropAlpha) {
        // Going from 4-channel src to the 3-channel format
        SkASSERT(C == 3);
        srcBpp = CPow2 * sizeof(Cx);
    } else if (ops & kPadAlpha) {
        // Going from the 3-channel format to 4-channel dst
        SkASSERT(C == 3);
        dstBpp = CPow2 * sizeof(Cx);
    }

#define RETURN_XFER_ROW_FN(ops) \
        case ops: \
            return create_xfer_row_fn<Cx, N*CPow2>( \
                    N, (ops) & kIgnoreSrc ? 0 : srcBpp, dstBpp, opaqueAlpha, zeroValue, \
                    apply_ops_by_channel<Cx, N, CPow2, (ops) & ~kIgnoreSrc>);

    switch (ops) {
        // The expected combination of ExtendedFormatXferOps
        RETURN_XFER_ROW_FN(kDropAlpha)
        RETURN_XFER_ROW_FN(kDropAlpha | kSwapRB)
        RETURN_XFER_ROW_FN(kDropAlpha | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kPadAlpha)
        RETURN_XFER_ROW_FN(kPadAlpha | kSwapRB)
        RETURN_XFER_ROW_FN(kPadAlpha | kIgnoreSrc)
        RETURN_XFER_ROW_FN(kSwapRB)
        RETURN_XFER_ROW_FN(kIgnoreSrc)
        RETURN_XFER_ROW_FN(kForceOpaque)
        RETURN_XFER_ROW_FN(kForceOpaque | kSwapRB)
        RETURN_XFER_ROW_FN(kForceOpaque | kIgnoreSrc)
        default:
            SK_ABORT("Unsupported ExtendedFormatXferOps combination: %u", ops);
    }
#undef RETURN_XFERFN_CASE
}

XferRowFn get_xfer_row_fn(TextureFormat format, uint8_t ops) {
    static constexpr uint32_t kFloatBits1 = 0x3f800000; // SkFloat2Bits isn't constexpr
    SkASSERT(kFloatBits1 == SkFloat2Bits(1.f));

    // 10 bits for _XR formats that encodes 1.0 and 0.0 given their extended normalization range.
    static constexpr uint32_t kXROne = 0x37e;
    static constexpr uint32_t kXRZero = 0x180;

    SkASSERT(ops); // For now, assume we only call into this if we have work to do.

    switch (format) {
        // Packed formats operate on a primitive that holds the entire pixel value
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
            return xfer_rows_packed<uint16_t, /*RShift=*/0, /*BShift=*/11, /*RBBits=*/5>(
                    ops, /*opaqueAlpha=*/0);

        case TF::kABGR4:
        case TF::kARGB4:
            return xfer_rows_packed<uint16_t, /*RShift=*/4, /*BShift=*/12, /*RBBits=*/4>(
                    ops, /*opaqueAlpha=*/0xF);

        case TF::kRGB10_A2:
        case TF::kBGR10_A2:
            return xfer_rows_packed<uint32_t, /*RShift=*/0, /*BShift=*/20, /*RBBits=*/10>(
                    ops, /*opaqueAlpha=*/0b11 << 30);

        case TF::kBGR10_XR:
            // NOTE: opaqueAlpha doesn't matter here since it's just BGRx data, but to match
            // SkRP's handling of BGR10A2_XR, treat the last 2 bits as regular unorm.
            return xfer_rows_packed<uint32_t, /*RShift=*/0, /*BShift=*/20, /*RBBits=*/10>(
                    ops, /*opaqueAlpha=*/0b11 << 30,
                    /*zeroValue=*/(kXRZero << 20) | (kXRZero << 10) | kXRZero);

        // The remaining formats can be operated on with each channel as a primitive
        case TF::kR8:
        case TF::kA8:
            return xfer_rows_by_channel<uint8_t, /*C=*/1>(ops, /*opaqueAlpha=*/0xFF);

        case TF::kR16:
            return xfer_rows_by_channel<uint16_t, /*C=*/1>(ops, /*opaqueAlpha=*/0xFFFF);

        case TF::kR16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/1>(ops, /*opaqueAlpha=*/SK_Half1);

        case TF::kRG8:
            return xfer_rows_by_channel<uint8_t, /*C=*/2>(ops, /*opaqueAlpha=*/0xFF);

        case TF::kRG16:
            return xfer_rows_by_channel<uint16_t, /*C=*/2>(ops, /*opaqueAlpha=*/0xFF);

        case TF::kRG16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/2>(ops, /*opaqueAlpha=*/SK_Half1);

        case TF::kRG32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/2>(ops, /*opaqueAlpha=*/kFloatBits1);

        case TF::kRGB8_sRGB:
        case TF::kRGB8:
        case TF::kBGR8:
            return xfer_rows_by_channel<uint8_t, /*C=*/3>(ops, /*opaqueAlpha=*/0xFF);

        case TF::kRGB16:
            return xfer_rows_by_channel<uint16_t, /*C=*/3>(ops, /*opaqueAlpha=*/0xFFFF);

        case TF::kRGB16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/3>(ops, /*opaqueAlpha=*/SK_Half1);

        case TF::kRGB32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/3>(ops, /*opaqueAlpha=*/kFloatBits1);

        case TF::kRGBA8:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGRA8_sRGB:
            return xfer_rows_by_channel<uint8_t, /*C=*/4>(ops, /*opaqueAlpha=*/0xFF);

        case TF::kRGBA10x6:
            // Each channel is the 10 real bits, with the least significant 6 padding bits.
            return xfer_rows_by_channel<uint16_t, /*C=*/4>(ops, /*opaqueAlpha=*/0xFFC0);

        case TF::kBGRA10x6_XR:
            // Like RGBA10x6 except the constants have to fit the extended range.
            return xfer_rows_by_channel<uint16_t, /*C=*/4>(
                    ops, /*opaqueAlpha=*/kXROne << 6, /*zeroValue=*/kXRZero << 6);

        case TF::kRGBA16:
            return xfer_rows_by_channel<uint16_t, /*C=*/4>(ops, /*opaqueAlpha=*/0xFFFF);

        case TF::kRGBA16F:
            return xfer_rows_by_channel<uint16_t, /*C=*/4>(ops, /*opaqueAlpha=*/SK_Half1);

        case TF::kRGBA32F:
            return xfer_rows_by_channel<uint32_t, /*C=*/4>(ops, /*opaqueAlpha=*/kFloatBits1);

        default:
            // Remaining cases are compressed, multiplanar, or non-color so shouldn't be reached.
            // If the first assert trips, we missed a valid transfer format in the cases above.
            // If we hit the unreachable, we missed rejecting the transfer sooner.
            SkASSERT(TextureFormatColorTypeInfo(format).second & FormatXferOp::kDisabled);
            SkUNREACHABLE;
    }
}

// --- Functions for collapsing equivalent colortypes and formats into fewer xfer ops

// An RPModifier for RPOps::Make() that handles calculating bt709 luminance. The API is that it
// must have an explicit bool operator, and an apply(SkRasterPipeline*) function.
struct BT709Luminance {
    bool fEnabled;

    explicit operator bool() const { return fEnabled; }
    void apply(SkRasterPipeline* rp) const {
        if (fEnabled) {
            rp->append(SkRasterPipelineOp::bt709_luminance_or_luma_to_alpha);
        } // else no op needed
    }
};

bool account_for_luminance(SkColorType srcCT,
                           bool hasColorSpaceTransform,
                           SkColorType* dstCT) {
    if (*dstCT == kGray_8_SkColorType) {
        if (srcCT != kGray_8_SkColorType || hasColorSpaceTransform) {
            // Luminance must be calculated by raster pipeline, but we'll have to apply it before
            // any srcToDst swizzle, so pull it out to its own RPModifier. The op stores the value
            // in the alpha channel, so that needs to be dstCT that SkRasterPipeline sees.
            *dstCT = kAlpha_8_SkColorType;
            return true;
        } // else leave it as gray->gray
    } // else trivially no extra luminance needs to be calculated, leave dstCT as-is
    return false;
}

bool rgbx_to_rgba(SkColorType* ct) {
    switch (*ct) {
        case kRGB_888x_SkColorType: *ct = kRGBA_8888_SkColorType; return true;
        case kRGB_101010x_SkColorType: *ct = kRGBA_1010102_SkColorType; return true;
        case kRGB_F16F16F16x_SkColorType: *ct = kRGBA_F16_SkColorType; return true;
        case kBGR_101010x_SkColorType: *ct = kBGRA_1010102_SkColorType; return true;
        default:
            // No colortype consolidation possible by switching to a kForceOpaque op instead.
            return false;
    }
}

bool bgra_to_rgba(SkColorType* ct) {
    switch (*ct) {
        case kBGRA_8888_SkColorType:    *ct = kRGBA_8888_SkColorType;    return true;
        case kBGRA_1010102_SkColorType: *ct = kRGBA_1010102_SkColorType; return true;
        case kBGR_101010x_SkColorType:  *ct = kRGB_101010x_SkColorType;  return true;
        // NOTE: For now there's no RGBA version of the _XR color types, so they can't be swapped
        case kBGRA_10101010_XR_SkColorType:
        case kBGR_101010x_XR_SkColorType: [[fallthrough]];
        default:
            // No colortype consolidation possible by switching to a kSwapRB op instead.
            return false;
    }
}

uint32_t gray_adjusted_channels(SkColorType ct) {
    return ct == kGray_8_SkColorType ? kRGB_SkColorChannelFlags : SkColorTypeChannelFlags(ct);
}

uint32_t swizzle_adjusted_channels(uint32_t channels, Swizzle swizzle) {
    auto removeChannelIfConstant = [&](SkColorChannelFlag flag, int index) {
        if (swizzle[index] == '0' || swizzle[index] == '1') {
            channels &= ~flag;
        }
    };
    removeChannelIfConstant(kRed_SkColorChannelFlag, 0);
    removeChannelIfConstant(kGreen_SkColorChannelFlag, 1);
    removeChannelIfConstant(kBlue_SkColorChannelFlag, 2);
    removeChannelIfConstant(kAlpha_SkColorChannelFlag, 3);
    return channels;
}

template <bool TextureIsDst>
std::pair</*ops=*/uint8_t, /*computeLuminance=*/bool> optimize_transfer(
        SkColorType* cpuCT,
        SkColorType* texBaseCT,
        Swizzle* texReadSwizzle,
        SkColorSpaceXformSteps* csSteps,
        SkEnumBitMask<FormatXferOp> xferOps) {
    // Aliases for color types that are based on the transfer direction
    SkColorType* srcCT = TextureIsDst ? cpuCT     : texBaseCT;
    SkColorType* dstCT = TextureIsDst ? texBaseCT : cpuCT;

    uint8_t finalOps = 0;

    // Some combinations of swizzle and load/store ops in raster pipeline are redundant so try to
    // make adjustments to reduce or eliminate the use of raster pipeline entirely.

    // First, adjust texBaseCT to match colortype semantics that can be inferred from swizzle for
    // alpha-only and red-only color types (hopefully creating a no-op transfer). These also test
    // the forced-opaque swizzle equivalents.
    {
        SkColorType adjustedBase = *texBaseCT;
        if (*texReadSwizzle == Swizzle("000r") || *texReadSwizzle == Swizzle("0001")) {
            // Red -> Alpha so shift the texture's "base" colortype to be the its alpha type
            switch(adjustedBase) {
                case kR8_unorm_SkColorType:  adjustedBase = kAlpha_8_SkColorType; break;
                case kR16_unorm_SkColorType: adjustedBase = kA16_unorm_SkColorType; break;
                case kR16_float_SkColorType: adjustedBase = kA16_float_SkColorType; break;
                default: break; // Go through regular RP + swizzle flow
            }
        } else if ((*texReadSwizzle == Swizzle("rrra") || *texReadSwizzle == Swizzle("rrr1")) &&
                adjustedBase == kR8_unorm_SkColorType) {
            // Red -> Gray so shift to kGray, which either ensures RP will generate the gray values
            // from a non-gray input, or will be detected as a no-op when transferring to/from
            // existing gray.
            adjustedBase = kGray_8_SkColorType;
        }

        if (adjustedBase != *texBaseCT) {
            *texBaseCT = adjustedBase;
            // Must preserve any forced opacity in the swizzle
            if ((SkColorTypeChannelFlags(adjustedBase) & kAlpha_SkColorChannelFlag) &&
                (*texReadSwizzle)[3] == '1') {
                *texReadSwizzle = Swizzle::RGB1();
            } else {
                *texReadSwizzle = Swizzle::RGBA();
            }
        }
    }

    // Second, remove normalized floating point semantics, since historically we don't enforce them
    // during transfers (for better or worse).
    {
        if (*srcCT == kRGBA_F16Norm_SkColorType) {
            *srcCT = kRGBA_F16_SkColorType;
        }
        if (*dstCT == kRGBA_F16Norm_SkColorType) {
            *dstCT = kRGBA_F16_SkColorType;
        }
    }

    // Third, switch swizzle components back to their default if the format doesn't have them
    {
        uint32_t texChannels = SkColorTypeChannelFlags(*texBaseCT);
        const char c[4] = {(*texReadSwizzle)[0], (*texReadSwizzle)[1],
                           (*texReadSwizzle)[2], (*texReadSwizzle)[3]};
        *texReadSwizzle = Swizzle((texChannels & kRed_SkColorChannelFlag)   ? c[0] : 'r',
                                  (texChannels & kGreen_SkColorChannelFlag) ? c[1] : 'g',
                                  (texChannels & kBlue_SkColorChannelFlag)  ? c[2] : 'b',
                                  (texChannels & kAlpha_SkColorChannelFlag) ? c[3] : 'a');
    }

    // Fourth, if the channels are disjoint between the src and the dst, then the dst values can
    // ignore the source, at which point we reset everything else to the identity and flag what
    // value is required by the dst (1 if the dst is alpha, 0 if it's RGB/gray).
    {
        uint32_t cpuChannels = gray_adjusted_channels(*cpuCT);
        uint32_t gpuChannels = swizzle_adjusted_channels(gray_adjusted_channels(*texBaseCT),
                                                         *texReadSwizzle);

        if (!(gpuChannels & cpuChannels)) {
            finalOps |= kIgnoreSrc;
            if (SkToBool(xferOps & FormatXferOp::kDropAlpha)) {
                // We're skipping the final xfer op handling, but we still have to make sure to drop
                // the alpha channel for 3-channel formats
                if constexpr (TextureIsDst) {
                    finalOps |= kDropAlpha;
                } else {
                    // Normally this would be kPadAlpha, but we're ignoring the src texture data,
                    // so instead it can just use kForceOpaque
                    finalOps |= kForceOpaque;
                }
            } else if (!SkColorTypeIsAlwaysOpaque(*dstCT)) {
                finalOps |= kForceOpaque;
            }

            // Disable everything else for the transfer
            *texReadSwizzle = Swizzle::RGBA();
            *srcCT = *dstCT;
            *csSteps = SkColorSpaceXformSteps{};

            return {finalOps, /*computeLuminance=*/false};
        }
    }

    // Fifth, handle masking any unknown alpha bits if the dst doesn't already mask them. This must
    // happen *after* checking for channel overlap to optimize to the kIgnoreSrc case. If not, the
    // RGBx color types getting lifted to RGBA adds a channel overlap when there wasn't one.
    {
        bool cpuMasksAlpha = rgbx_to_rgba(cpuCT);
        bool gpuMasksAlpha = rgbx_to_rgba(texBaseCT) || (*texReadSwizzle)[3] == '1';
        bool srcHasJunkAlpha = TextureIsDst ? cpuMasksAlpha : gpuMasksAlpha;
        bool dstStoresAlpha =
                (SkColorTypeChannelFlags(*dstCT) & kAlpha_SkColorChannelFlag) &&
                !(TextureIsDst ? gpuMasksAlpha : cpuMasksAlpha);

        if (srcHasJunkAlpha) {
            csSteps->fFlags.premul = false;
            csSteps->fFlags.unpremul = false;
        }

        if (dstStoresAlpha && srcHasJunkAlpha) {
            finalOps |= kForceOpaque;
        }

        // Reset any opacity forcing in the swizzle since it's handled by kForceOpaque.
        if ((*texReadSwizzle)[3] == '1') {
            *texReadSwizzle = Swizzle((*texReadSwizzle)[0],
                                      (*texReadSwizzle)[1],
                                      (*texReadSwizzle)[2],
                                      'a');
        }
    }

    // Sixth, lift gray/luminance calculation out of colortype so that its placement in the
    // raster pipeline ops list can be controlled (vs. attached to a store). If luminance has to be
    // calculated, some additional optimizations may not be possible.
    const bool computeLuminance = account_for_luminance(*srcCT, SkToBool(*csSteps), dstCT);

    // Seventh, consolidate red/blue swaps present in colortype, swizzle, and xferOps into just ops
    {
        // Any swaps from the texture's base color type, swizzle, and xfer ops can always be
        // combined since those operations are grouped together, regardless of `TextureIsDst`.
        int numRBSwaps = 0;
        if ((*texReadSwizzle)[0] == 'b' && (*texReadSwizzle)[2] == 'r') {
            // Remove the swap in the swizzle (moving towards a no-op swizzle).
            *texReadSwizzle = Swizzle::Concat(*texReadSwizzle, Swizzle::BGRA());
            numRBSwaps++;
        }
        if (bgra_to_rgba(texBaseCT)) {
            numRBSwaps++;
        }
        if (xferOps & FormatXferOp::kSwapRB) {
            numRBSwaps++;
        }

        // If there is not any RGB-dependent calculation between CPU and GPU data, we can also
        // consolidate the swap from cpu color type.
        if (!computeLuminance && !SkToBool(*csSteps)) {
            // TODO(michaelludwig): Once we push finalOps back into raster pipeline if we need SkRP,
            // we can always apply this to cpuCT to remove an implicit swap_rb op.
            SkColorType swappedCT = *cpuCT;
            if (bgra_to_rgba(&swappedCT) && swappedCT == *texBaseCT) {
                numRBSwaps++;
                *cpuCT = swappedCT;
            }
        }

        // An even number of swaps is a no-op; an odd number of swaps is the same as one swap. The
        // swap can be skipped if the source data is known to have the same values in R and B, or
        // if R and B will be discarded.
        bool srcHasRBData = *srcCT != kGray_8_SkColorType && !SkColorTypeIsAlphaOnly(*srcCT);
        bool dstKeepsRBData = !SkColorTypeIsAlphaOnly(*dstCT) || computeLuminance;
        if ((numRBSwaps & 1) && srcHasRBData && dstKeepsRBData) {
            finalOps |= kSwapRB;
        }
    }

    if (xferOps & FormatXferOp::kDropAlpha) {
        if constexpr (TextureIsDst) {
            // On CPU->GPU conversion, FormatXferOp::kDropAlpha actually drops the alpha bits
            finalOps |= kDropAlpha;
        } else {
            finalOps |= kPadAlpha;
        }
        // Remove kForceOpaque as it's redundant with kDropAlpha/kPadAlpha
        finalOps &= ~kForceOpaque;
    }

    return {finalOps, computeLuminance};
}

} // anonymous namespace

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeCpuToGpu(
        SkColorType srcCT,
        const SkColorSpaceXformSteps& csSteps,
        TextureFormat dstFormat,
        Swizzle dstReadSwizzle) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(dstFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }

    SkColorSpaceXformSteps csStepsOptimized = csSteps;
    auto [postOps, luminance] = optimize_transfer</*TextureIsDst=*/true>(
            &srcCT, &baseCT, &dstReadSwizzle, &csStepsOptimized, xferOps);

    // The CPU -> GPU transform is:
    //  SkRP{load(srcCT) ->
    //       csSteps? ->
    //       luminance? -> NOTE: luminance must be computed *before* the texture's swizzle
    //       srcToDst(dstReadSwizzle^-1)? ->
    //       store(baseCT)}? ->
    //  postOps(baseCT->TF)?
    auto rp = RPOps::Make(srcCT, baseCT, // ==> rpModifiers
                          csStepsOptimized, BT709Luminance{luminance}, dstReadSwizzle.invert());
    return TextureFormatXferFn(dstFormat, /*preOps=*/0, std::move(rp), postOps);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeGpuToCpu(
        TextureFormat srcFormat,
        Swizzle srcReadSwizzle,
        const SkColorSpaceXformSteps& csSteps,
        SkColorType dstCT) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(srcFormat);
    if (xferOps & FormatXferOp::kDisabled) {
        return std::nullopt;
    }

    SkColorSpaceXformSteps csStepsOptimized = csSteps;
    auto [preOps, luminance] = optimize_transfer</*TextureIsDst=*/false>(
            &dstCT, &baseCT, &srcReadSwizzle, &csStepsOptimized, xferOps);
    if (preOps & kIgnoreSrc) {
        // The source is the texture format, which is what defines how ops are implemented, is
        // currently set to be ignored, so pick a new format that matches the `dstCT`. Actual GPU
        // support doesn't matter, this will just ensure the CPU data is initialized correctly.
        srcFormat = PreferredTextureFormats(dstCT)[0];
    }

    // The GPU -> CPU transform is:
    //  preOps(TF->baseCT)? ->
    //  SkRP{load(baseCT) ->
    //       srcToDst(srcReadSwizzle)? ->
    //       csSteps? ->
    //       luminance? ->
    //       store(dstCT)}?
    auto rp = RPOps::Make(baseCT, dstCT, // ==> rpModifiers
                          srcReadSwizzle, csStepsOptimized, BT709Luminance{luminance});
    return TextureFormatXferFn(srcFormat, preOps, std::move(rp), /*postOps=*/0);
}

std::optional<TextureFormatXferFn> TextureFormatXferFn::MakeIdentity(TextureFormat format) {
    auto [baseCT, xferOps] = TextureFormatColorTypeInfo(format);
    if (xferOps & FormatXferOp::kDisabled) {
        if (TextureFormatCompressionType(format) == SkTextureCompressionType::kNone) {
            return std::nullopt;
        } // else allow compressed formats through for identity conversion uploads
    }

    return TextureFormatXferFn(format, /*preOps=*/0, /*rp=*/nullptr, /*postOps=*/0);
}

template <typename... RPModifiers>
sk_sp<TextureFormatXferFn::RPOps> TextureFormatXferFn::RPOps::Make(
        SkColorType srcColorType,
        SkColorType dstColorType,
        RPModifiers... rpModifiers) {
    if (srcColorType == dstColorType &&
        (!SkToBool(rpModifiers) && ...)) {
        return nullptr; // Identity conversion
    }

    // Luminance has to be calculated before the texture's swizzle so it's pulled out manually
    // and put in the right place in `rpModifiers`, so we don't want to encounter them where
    // the appendStore() also computes luminance.
    SkASSERT(dstColorType != kGray_8_SkColorType);

    sk_sp<RPOps> ops{new RPOps(/*srcBpp=*/SkColorTypeBytesPerPixel(srcColorType),
                               /*dstBpp=*/SkColorTypeBytesPerPixel(dstColorType))};

    // NOTE: The src and dst memory contexts are not modified here, they just provide stable
    // pointers for the appended ops to reference, and will be patched during run().
    ops->fRP.appendLoad(srcColorType, &ops->fSrcCtx);

    // We must create a copy of rpModifiers[i] in the arena, because its apply() function may
    // reference parts of itself as the context's passed to the appended raster pipeline ops
    (ops->fArena.make<decltype(rpModifiers)>(rpModifiers)->apply(&ops->fRP), ...);

    ops->fRP.appendStore(dstColorType, &ops->fDstCtx);
    return ops;
}

bool TextureFormatXferFn::RPOps::setStrides(size_t srcRowBytes,
                                            size_t dstRowBytes,
                                            uint8_t otherOps) {
    // SkRasterPipeline operates in pixel units for its strides, so we should only be relying on
    // RP's built-in row stride handling if the data is aligned to the pixel size.
    if (srcRowBytes % fSrcBpp == 0 && dstRowBytes % fDstBpp == 0 && otherOps == 0) {
        fSrcCtx.stride = SkTo<int>(srcRowBytes / fSrcBpp);
        fDstCtx.stride = SkTo<int>(dstRowBytes / fDstBpp);
        return true;
    } else {
        // Control loop must proceed row by row, so stride can be 0
        fSrcCtx.stride = 0;
        fDstCtx.stride = 0;
        return false;
    }
}

// TODO(michaelludwig): This is a WIP implementation, it is not focusing on performance yet.
void TextureFormatXferFn::run(int width, int height,
                              const void* src, size_t srcRowBytes,
                              void* dst, size_t dstRowBytes) const {
    SkASSERT(width >= 1 && height >= 1);

    int rpInvokeCount;
    SkAutoMalloc tempRowStorage; // empty if no FormatXferOps have to be applied

    if (fRP && fRP->setStrides(srcRowBytes, dstRowBytes, fPreOps | fPostOps)) {
        // Conversions occur entirely within SkRasterPipeline, so we can configure the
        // MemoryCtx's to process the whole 2D image.
        rpInvokeCount = 1;
    } else {
        // Conversions will have to occur row-by-row. The SkRP row function will patch the
        // memory contexts to each row's offset address so we can leave stride as 0.
        SkASSERT(!fRP || (fRP->fSrcCtx.stride == 0 && fRP->fDstCtx.stride == 0));
        rpInvokeCount = height;
        height = 1;
    }

    skia_private::STArray<2, XferRowFn> rowFns; // At most 2 actions per row
    if (fPreOps) {
        // `src` is definitively the texture
        if (fRP) {
            // We need a temporary buffer equal to srcBpp*width to hold the output of the preOps
            // that is used as the source of data for SkRasterPipeline (executed per row).
            tempRowStorage.reset(fRP->fSrcBpp * width);
        }
        rowFns.push_back(get_xfer_row_fn(fFormat, fPreOps));
    }

    if (fRP) {
        rowFns.push_back([&](const char* src, char* dst, int width) {
            // NOTE: When height != 1, this invocation actually processes the entire image.
            // Otherwise we assume src and dst have been offset by y so we update the MemoryCtx's
            // pixel addresses.
            fRP->fSrcCtx.pixels = const_cast<char*>(src); // This won't be written to
            fRP->fDstCtx.pixels = dst;
            fRP->fRP.run(0, 0, width, height);
        });
    }

    if (fPostOps) {
        // `dst` is definitively the texture
        if (fRP) {
            // We need a temporary buffer equal to dstBpp*width to hold the output of the
            // SkRasterPipeline conversion that is used as the input to postOps (executed per row).
            tempRowStorage.reset(fRP->fDstBpp * width);
        }
        rowFns.push_back(get_xfer_row_fn(fFormat, fPostOps));
    }

    if (rowFns.empty()) {
        // Identity conversion function still needs to move the data
        const int bpp = TextureFormatBytesPerBlock(fFormat);
        rowFns.push_back([bpp](const char* src, char* dst, int width) {
            memcpy(dst, src, bpp * width);
        });
    }

    for (int y = 0; y < rpInvokeCount; ++y) {
        // Always start by processing `src`
        const char* input = static_cast<const char*>(src) + y * srcRowBytes;
        for (int i = 0; i < rowFns.size(); ++i) {
            // And either output to the temporary row or the final `dst`
            char* target = i == rowFns.size() - 1 ? (static_cast<char*>(dst) + y * dstRowBytes)
                                                  : static_cast<char*>(tempRowStorage.get());
            rowFns[i](input, target, width);
            // If there's more than one rowFn, switch to using the temporary row as input
            input = target;
        }
    }
}

} // namespace skgpu::graphite

// TODO(b/390473370): TextureUploadWriter isn't very useful with TextureFormatXferFn's
// capabilities and the writer model doesn't work well with the more explicit placement of dst
// data that has to happen. UploadBufferManager should just provide a span and then we can call
// run() directly and delete TextureUploadWriter entirely.
#include "src/gpu/BufferWriter.h"

namespace skgpu {

void TextureUploadWriter::convert(size_t offset, int width, int height,
                                  const void* src, size_t srcRowBytes,
                                  const graphite::TextureFormatXferFn& dst, size_t dstRowBytes) {
    this->validate(offset + dstRowBytes * height);
    void* dstPtr = SkTAddOffset<void>(fPtr, offset);
    dst.run(width, height, src, srcRowBytes, dstPtr, dstRowBytes);
}

} // namespace skgpu
