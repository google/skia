/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#if defined(SK_GRAPHITE)

#include "tests/graphite/precompile/PaintOptionsBuilder.h"

#include "include/gpu/graphite/precompile/PrecompileColorFilter.h"
#include "include/gpu/graphite/precompile/PrecompileShader.h"

namespace PaintOptionsUtils {

using namespace skgpu::graphite;
using PrecompileShaders::GradientShaderFlags;
using PrecompileShaders::ImageShaderFlags;
using PrecompileShaders::YUVImageShaderFlags;

Builder& Builder::hwImg(ImgColorInfo ci, ImgTileModeOptions tmOptions) {
    static const SkColorInfo kAlphaInfo(kAlpha_8_SkColorType,
                                        kUnpremul_SkAlphaType,
                                        nullptr);
    static const SkColorInfo kAlphaSRGBInfo(kAlpha_8_SkColorType,
                                            kUnpremul_SkAlphaType,
                                            SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                                  SkNamedGamut::kAdobeRGB));
    static const SkColorInfo kPremulInfo(kRGBA_8888_SkColorType,
                                         kPremul_SkAlphaType,
                                         nullptr);
    static const SkColorInfo kSRGBInfo(kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                             SkNamedGamut::kAdobeRGB));

    SkSpan<const SkColorInfo> ciSpan;
    switch (ci) {
        case kAlpha:     ciSpan = { &kAlphaInfo, 1 };    break;
        case kAlphaSRGB: ciSpan = { &kAlphaSRGBInfo, 1}; break;
        case kPremul:    ciSpan = { &kPremulInfo, 1 };   break;
        case kSRGB:      ciSpan = { &kSRGBInfo, 1 };     break;
    }

    static const SkTileMode kClampTM  = SkTileMode::kClamp;
    static const SkTileMode kRepeatTM = SkTileMode::kRepeat;

    SkSpan<const SkTileMode> tmSpan;
    switch (tmOptions) {
        case kNone:                               break;
        case kClamp:  tmSpan = { &kClampTM,  1 }; break;
        case kRepeat: tmSpan = { &kRepeatTM, 1 }; break;
    }

    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           ciSpan,
                                                           tmSpan);
    fPaintOptions.setShaders({ std::move(img) });
    return *this;
}

Builder& Builder::yuv(YUVSamplingOptions options) {
    static const SkColorInfo kSRGBInfo(kRGBA_8888_SkColorType,
                                       kPremul_SkAlphaType,
                                       SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB,
                                                             SkNamedGamut::kAdobeRGB));

    YUVImageShaderFlags flags = YUVImageShaderFlags::kNone;
    switch (options) {
        case kNoCubic:     flags = YUVImageShaderFlags::kExcludeCubic;           break;
        case kHWAndShader: flags = YUVImageShaderFlags::kNoCubicNoNonSwizzledHW; break;
    }

    sk_sp<PrecompileShader> img = PrecompileShaders::YUVImage(flags, { &kSRGBInfo, 1 });
    fPaintOptions.setShaders({ std::move(img) });
    return *this;
}

Builder& Builder::linearGrad(LinearGradientOptions options) {
    sk_sp<PrecompileShader> gradient;

    if (options == kSmall) {
        gradient = PrecompileShaders::LinearGradient(GradientShaderFlags::kSmall);
    } else if (options == kComplex) {
        gradient = PrecompileShaders::LinearGradient(
                GradientShaderFlags::kNoLarge,
                { SkGradientShader::Interpolation::InPremul::kNo,
                  SkGradientShader::Interpolation::ColorSpace::kSRGB,
                  SkGradientShader::Interpolation::HueMethod::kShorter });
    }

    fPaintOptions.setShaders({ std::move(gradient) });
    return *this;
}

Builder& Builder::blend() {
    SkColorInfo ci { kRGBA_8888_SkColorType, kPremul_SkAlphaType, nullptr };
    sk_sp<PrecompileShader> img = PrecompileShaders::Image(ImageShaderFlags::kExcludeCubic,
                                                           { &ci, 1 },
                                                           {});
    SkBlendMode kBlendModes = SkBlendMode::kPlus;
    fPaintOptions.setShaders({ PrecompileShaders::Blend({ &kBlendModes, 1 },
                                                        { std::move(img) },
                                                        { PrecompileShaders::Color() }) });
    return *this;
}

Builder& Builder::matrixCF() {
    fPaintOptions.setColorFilters({ PrecompileColorFilters::Matrix() });
    return *this;
}

Builder& Builder::porterDuffCF() {
    fPaintOptions.setColorFilters({ PrecompileColorFilters::Blend({ SkBlendMode::kSrcOver }) });
    return *this;
}

} // namespace PaintOptionsUtils

#endif // SK_GRAPHITE
