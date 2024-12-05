/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/encode/SkPngEncoderBase.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkAssert.h"

namespace SkPngEncoderBase {

namespace {

SkEncodedInfo makeInfo(const SkImageInfo& srcInfo,
                       SkEncodedInfo::Color color,
                       int bitsPerComponent) {
    SkEncodedInfo::Alpha alpha =
            color == SkEncodedInfo::kGray_Color || color == SkEncodedInfo::kRGB_Color
                    ? SkEncodedInfo::kOpaque_Alpha
                    : SkEncodedInfo::kUnpremul_Alpha;

    return SkEncodedInfo::Make(srcInfo.width(), srcInfo.height(), color, alpha, bitsPerComponent);
}

SkEncodedInfo makeGray8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGray_Color, 8);
}

SkEncodedInfo makeGrayAlpha8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kGrayAlpha_Color, 8);
}

SkEncodedInfo makeRgb8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGB_Color, 8);
}

SkEncodedInfo makeRgba8Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 8);
}

SkEncodedInfo makeRgb16Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGB_Color, 16);
}

SkEncodedInfo makeRgba16Info(const SkImageInfo& srcInfo) {
    return makeInfo(srcInfo, SkEncodedInfo::kRGBA_Color, 16);
}

}  // namespace

std::optional<std::pair<SkEncodedInfo, transform_scanline_proc>> getTargetInfo(
        const SkImageInfo& srcInfo) {
    switch (srcInfo.colorType()) {
        case kUnknown_SkColorType:
            return std::nullopt;

        // TODO: I don't think this can just use kRGBA's procs.
        // kPremul is especially tricky here, since it's presumably TF⁻¹(rgb * a),
        // so to get at unpremul rgb we'd need to undo the transfer function first.
        case kSRGBA_8888_SkColorType:
            return std::nullopt;

        case kRGBA_8888_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_RGBX);
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_memcpy);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_rgbA);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kBGRA_8888_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_BGRX);
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_BGRA);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_bgrA);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_565_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_565);
        case kRGB_888x_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_RGBX);
        case kARGB_4444_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb8Info(srcInfo), transform_scanline_444);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba8Info(srcInfo), transform_scanline_4444);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kGray_8_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeGray8Info(srcInfo), transform_scanline_memcpy);

        case kRGBA_F16Norm_SkColorType:
        case kRGBA_F16_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F16);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F16_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_F16F16F16x_SkColorType:
            SkASSERT(srcInfo.isOpaque());
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_F16F16F16x);
        case kRGBA_F32_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F32);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_F32_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGBA_1010102_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_1010102);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_1010102_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kBGRA_1010102_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo), transform_scanline_bgra_1010102);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_1010102_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kRGB_101010x_SkColorType:
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_101010x);
        case kBGR_101010x_SkColorType:
            return std::make_pair(makeRgb16Info(srcInfo), transform_scanline_bgr_101010x);
        case kBGR_101010x_XR_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                    return std::make_pair(makeRgb16Info(srcInfo),
                                          transform_scanline_bgr_101010x_xr);
                default:
                    SkDEBUGFAIL("unsupported color type");
                    return std::nullopt;
            }
        case kBGRA_10101010_XR_SkColorType:
            switch (srcInfo.alphaType()) {
                case kOpaque_SkAlphaType:
                case kUnpremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_10101010_xr);
                case kPremul_SkAlphaType:
                    return std::make_pair(makeRgba16Info(srcInfo),
                                          transform_scanline_bgra_10101010_xr_premul);
                default:
                    SkDEBUGFAIL("unknown alpha type");
                    return std::nullopt;
            }
        case kAlpha_8_SkColorType:
            return std::make_pair(makeGrayAlpha8Info(srcInfo), transform_scanline_A8_to_GrayAlpha);
        case kR8G8_unorm_SkColorType:
        case kR16G16_unorm_SkColorType:
        case kR16G16_float_SkColorType:
        case kA16_unorm_SkColorType:
        case kA16_float_SkColorType:
        case kR16G16B16A16_unorm_SkColorType:
        case kR8_unorm_SkColorType:
        case kRGBA_10x6_SkColorType:
            return std::nullopt;
    }
    SkDEBUGFAIL("unsupported color type");
    return std::nullopt;
}

}  // namespace SkPngEncoderBase
