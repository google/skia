/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/TextureFormatXferFn.h"

#include "include/core/SkColorType.h"
#include "src/base/SkAutoMalloc.h"
#include "src/base/SkFloatBits.h"
#include "src/base/SkHalf.h"
#include "src/base/SkMathPriv.h"
#include "src/base/SkVx.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpContexts.h"
#include "src/gpu/graphite/Log.h"

#include <functional>

namespace skgpu::graphite {

namespace {

using TF = TextureFormat;

// This is intentionally not a class enum and is distinct from FormatXferOps for two reasons:
// 1. We can't use SkEnumBitmask inside template parameters for the row bit manipulation functions.
// 2. It's convenient to split the FormatXferOps into specific ops based on their conversion
//    direction that doesn't need to be exposed in the public API.
enum ExtendedFormatXferOp : uint8_t {
    kDropAlpha = 0x1,
    kPadAlpha  = 0x2,
};


using XferRowFn = std::function<void(const char* src, char* dst, int width)>;

template <typename PxVec, typename PixelFn /* [](PxVec) -> PxVec */>
XferRowFn create_xfer_row_fn(int n, int srcBpp, int dstBpp, PixelFn applyPixel) {
    // PxVec should be sufficient to hold N src and dst pixels, and should match at least one
    SkASSERT(sizeof(PxVec) >= (size_t)srcBpp*n && sizeof(PxVec) >= (size_t)dstBpp*n);
    SkASSERT(sizeof(PxVec) == (size_t)srcBpp*n || sizeof(PxVec) == (size_t)dstBpp*n);

    return [n, srcBpp, dstBpp, applyPixel](const char* src, char* dst, int width) {
        const int srcBppN = n * srcBpp;
        const int dstBppN = n * dstBpp;

        PxVec pixel{};
        while (width >= n) {
            memcpy(&pixel, src, srcBppN);
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, dstBppN);

            width -= n;
            src += srcBppN;
            dst += dstBppN;
        }

        if (width > 0) {
            // Process tail that is less than a full vector
            SkASSERT(width < n);
            memcpy(&pixel, src, width * srcBpp);
            pixel = applyPixel(pixel);
            memcpy(dst, &pixel, width * dstBpp);
        }
    };
}

// N represents the number of pixels being processed; the currently supported Ops are only valid for
// a channel count of 3 (upgraded to 4 effectively) or exactly 4.
template <typename Cx, int N, uint8_t Ops, Cx OpaqueAlpha>
skvx::Vec<4*N, Cx> apply_ops_by_channel(skvx::Vec<4*N, Cx> pixel) {
    static_assert(N == 1 || N == 2 || N == 4);
    if constexpr (Ops & kPadAlpha) {
        // If we are padding alpha, we are moving from a 3-channel source data (loaded in the first
        // N*3 values of pixel) to a 4-channel value. 3*N+1 holds undefined data after loading the
        // pixel, which we can set to the opaque alpha value. Then we shuffle the slots to spread
        // out each pixel's R,G, and B values and insert copies of the opaque alpha value.
        static constexpr int kA = 3*N;
        pixel[kA] = OpaqueAlpha;

        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA, 6,7,8,kA, 9,10,11,kA>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2,kA, 3,4,5,kA>(pixel);
        } // else no shuffling needed for N=1, since pixel == shuffle<0,1,2,A>(pixel)
    }

    // TODO(michaelludwig): Add other extend ops here

    if constexpr (Ops & kDropAlpha) {
        // If we are dropping alpha, we need to shuffle the R,G, and B values of the 4-channel
        // source data into the first N*3 slots of the returned pixel. The remaining N slots will
        // be ignored by the final memcpy.
        if constexpr (N == 4) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 8,9,10, 12,13,14, 14,14,14,14>(pixel);
        } else if constexpr (N == 2) {
            pixel = skvx::shuffle<0,1,2, 4,5,6, 6,6>(pixel);
        } // else no shuffling needed for N=1, since pixel == shuffle<0,1,2,_>(pixel)
    }

    return pixel;
}

// NOTE: This takes no parameters for SwapRB or DropAlpha because for all formats that use this,
// SwapRB involves swapping channel 0 and channel 2, and dropping alpha removes channel 3. This
// can be parameterized via template parameters to be able to push into the skvx::shuffle calls
// if needed in the future.
template <typename Cx, Cx OpaqueAlpha>
XferRowFn xfer_rows_by_channel(uint8_t ops) {
    static constexpr int C = 3;
    static constexpr int CPow2 = SkNextPow2(C);
    static constexpr int N = 16 / (CPow2 * sizeof(Cx)); // Fit to 128-bit/16-byte SIMD

    static_assert(N == 1 || N == 2 || N == 4);
    using PxVec = skvx::Vec<N*CPow2, Cx>;

    int srcBpp = C * sizeof(Cx);
    int dstBpp = C * sizeof(Cx);

    if (ops & kDropAlpha) {
        // Going from 4-channel src to the 3-channel format
        srcBpp = CPow2 * sizeof(Cx);
        return create_xfer_row_fn<PxVec>(N, srcBpp, dstBpp,
                                         apply_ops_by_channel<Cx, N, kDropAlpha, OpaqueAlpha>);
    } else if (ops & kPadAlpha) {
        // Going from the 3-channel format to 4-channel dst
        dstBpp = CPow2 * sizeof(Cx);
        return create_xfer_row_fn<PxVec>(N, srcBpp, dstBpp,
                                         apply_ops_by_channel<Cx, N, kPadAlpha, OpaqueAlpha>);
    } else {
        SKGPU_LOG_F("Identity transfer should have been caught earlier");
    }

    return nullptr;
}

XferRowFn get_xfer_row_fn(TextureFormat format, uint8_t ops) {
    static constexpr uint32_t kFloatBits1 = 0x3f800000; // SkFloat2Bits isn't constexpr
    SkASSERT(kFloatBits1 == SkFloat2Bits(1.f));
    SkASSERT(ops); // For now, assume we only call into this if we have work to do.

    switch (format) {
        case TF::kR8:
        case TF::kA8:
        case TF::kR16:
        case TF::kR16F:
        case TF::kRG8:
        case TF::kRG16:
        case TF::kRG16F:
        case TF::kRG32F:
            // 1 and 2 channel formats cannot be combined with colortypes in such a way to create
            // format conversions, so we should never reach here
            SKGPU_LOG_F("Unexpected ops (%u) requested for format %s",
                        ops, TextureFormatName(format));
            break;

        // Packed formats operate on a primitive that holds the entire pixel value
        case TF::kB5_G6_R5:
        case TF::kR5_G6_B5:
        case TF::kABGR4:
        case TF::kARGB4:
        case TF::kRGB10_A2:
        case TF::kBGR10_A2:
        case TF::kBGR10_XR:
            // TODO(michaelludwig): These formats could do r/b swaps and forcing-opaque, but
            // that isn't implemented yet.
            SKGPU_LOG_F("Unsupported texture format %s", TextureFormatName(format));
            break;

        // The remaining formats can be operated on with each channel as a primitive
        case TF::kRGB8_sRGB:
        case TF::kRGB8:
        case TF::kBGR8:
            return xfer_rows_by_channel<uint8_t, 0xFF>(ops);

        case TF::kRGB16:
            return xfer_rows_by_channel<uint16_t, 0xFFFF>(ops);

        case TF::kRGB16F:
            return xfer_rows_by_channel<uint16_t, SK_Half1>(ops);

        case TF::kRGB32F:
            return xfer_rows_by_channel<uint32_t, kFloatBits1>(ops);

        case TF::kRGBA8:
        case TF::kRGBA8_sRGB:
        case TF::kBGRA8:
        case TF::kBGRA8_sRGB:
        case TF::kRGBA16:
        case TF::kRGBA16F:
        case TF::kRGBA10x6:
        case TF::kBGRA10x6_XR:
        case TF::kRGBA32F:
            // TODO(michaelludwig): These formats could do r/b swaps and forcing-opaque, but
            // that isn't implemented yet.
            SKGPU_LOG_F("Unsupported texture format %s", TextureFormatName(format));
            break;

        default:
            // Remaining cases are compressed, multiplanar, or non-color so shouldn't be reached.
            // If the first assert trips, we missed a valid transfer format in the cases above.
            // If we hit the unreachable, we missed rejecting the transfer sooner.
            SkASSERT(TextureFormatColorTypeInfo(format).second & FormatXferOp::kDisabled);
            SkUNREACHABLE;
    }
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

    uint8_t postOps = 0;
    Swizzle srcToDst;
    if (dstReadSwizzle == Swizzle::RRRA() &&
        baseCT == kR8_unorm_SkColorType &&
        srcCT != kGray_8_SkColorType) {
        // While we are storing an R8 value, we need to adjust the baseCT in order to induce
        // SkRasterPipeline to compute luminance from the non-gray src values.
        baseCT = kGray_8_SkColorType;
        srcToDst = Swizzle::RGBA();
    } else {
        srcToDst = dstReadSwizzle.invert();
    }

    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    if (xferOps & FormatXferOp::kDropAlpha) {
        // On CPU->GPU conversion, FormatXferOp::kDropAlpha actually drops the alpha bits
        postOps |= kDropAlpha;
    }
    auto rp = RPOps::Make(srcCT, baseCT, csSteps, srcToDst);
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

    uint8_t preOps = 0;
    Swizzle srcToDst = srcReadSwizzle;
    // NOTE: no need to adjust baseCT for red vs. gray in this direction as the ambiguity is
    // irrelevant since the read swizzle pushes the "red" value into all three channels, any
    // conversion to gray works back out to the original value.
    if (xferOps & FormatXferOp::kSwapRB) {
        srcToDst = Swizzle::Concat(srcToDst, Swizzle::BGRA());
    }
    if (xferOps & FormatXferOp::kDropAlpha) {
        // On GPU->CPU conversion, FormatXferOp::kDropAlpha must pad alpha bits back
        preOps |= kPadAlpha;
    }
    auto rp = RPOps::Make(baseCT, dstCT, srcToDst, csSteps);
    return TextureFormatXferFn(srcFormat, preOps, std::move(rp), /*postOps=*/0);
}

template <typename... RPModifiers>
std::unique_ptr<TextureFormatXferFn::RPOps> TextureFormatXferFn::RPOps::Make(
        SkColorType srcColorType,
        SkColorType dstColorType,
        RPModifiers... rpModifiers) {
    if (srcColorType == dstColorType &&
        (!SkToBool(rpModifiers) && ...)) {
        return nullptr; // Identity conversion
    }
    std::unique_ptr<RPOps> ops{new RPOps(/*srcBpp=*/SkColorTypeBytesPerPixel(srcColorType),
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
