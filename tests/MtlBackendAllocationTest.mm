/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "tests/Test.h"

#import <metal/metal.h>
#include "src/gpu/mtl/GrMtlCaps.h"

// In BackendAllocationTest.cpp
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*,
                                                   GrMipMapped,
                                                   GrRenderable)> create,
                   SkColorType colorType, GrMipMapped mipMapped, GrRenderable renderable);

void test_color_init(GrContext* context, skiatest::Reporter* reporter,
                     std::function<GrBackendTexture (GrContext*,
                                                     const SkColor4f&,
                                                     GrMipMapped,
                                                     GrRenderable)> create,
                     SkColorType colorType, const SkColor4f& color,
                     GrMipMapped mipMapped, GrRenderable renderable);

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrMtlCaps* mtlCaps = static_cast<const GrMtlCaps*>(context->priv().caps());

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };

    struct {
        SkColorType      fColorType;
        GrMTLPixelFormat fFormat;
        // TODO: remove 'fConfig' and directly use 'fFormat' in GrMtlCaps::isFormatTexturable
        GrPixelConfig    fConfig;
        SkColor4f        fColor;
    } combinations[] = {
        { kRGBA_8888_SkColorType,          MTLPixelFormatRGBA8Unorm,
          kRGBA_8888_GrPixelConfig,        SkColors::kRed       },
        { kRGBA_8888_SkColorType,          MTLPixelFormatRGBA8Unorm_sRGB,
          kSRGBA_8888_GrPixelConfig,       SkColors::kRed       },

        { kRGB_888x_SkColorType,           MTLPixelFormatRGBA8Unorm,
          kRGBA_8888_GrPixelConfig,        { 1, 1, 0, 0.5f }    },

        { kBGRA_8888_SkColorType,          MTLPixelFormatBGRA8Unorm,
          kBGRA_8888_GrPixelConfig,        SkColors::kBlue      },
        { kBGRA_8888_SkColorType,          MTLPixelFormatBGRA8Unorm_sRGB,
          kSBGRA_8888_GrPixelConfig,       SkColors::kCyan      },

        { kRGBA_1010102_SkColorType,       MTLPixelFormatRGB10A2Unorm,
          kRGBA_1010102_GrPixelConfig,     { 0.5f, 0, 0, 1.0f } },
#ifdef SK_BUILD_FOR_IOS
        { kRGB_565_SkColorType,             MTLPixelFormatB5G6R5Unorm,
          kRGB_565_GrPixelConfig,           SkColors::kRed      },
        { kARGB_4444_SkColorType,           MTLPixelFormatABGR4Unorm,
          kRGBA_4444_GrPixelConfig,         SkColors::kGreen    },
#endif

        { kAlpha_8_SkColorType,             MTLPixelFormatA8Unorm,
          kAlpha_8_as_Alpha_GrPixelConfig,  kTransCol           },
        { kAlpha_8_SkColorType,             MTLPixelFormatR8Unorm,
          kAlpha_8_as_Red_GrPixelConfig,    kTransCol           },

        { kGray_8_SkColorType,              MTLPixelFormatR8Unorm,
          kGray_8_as_Red_GrPixelConfig,     SkColors::kDkGray   },

        { kRGBA_F32_SkColorType,            MTLPixelFormatRGBA32Float,
          kRGBA_float_GrPixelConfig,        SkColors::kRed      },

        { kRGBA_F16Norm_SkColorType,        MTLPixelFormatRGBA16Float,
          kRGBA_half_Clamped_GrPixelConfig, SkColors::kLtGray   },
        { kRGBA_F16_SkColorType,            MTLPixelFormatRGBA16Float,
          kRGBA_half_GrPixelConfig,         SkColors::kYellow   },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,             MTLPixelFormatRG32Float,
          kRG_float_GrPixelConfig,          { 0.7f, 0.7f, 0, 0 }},
        { kUnknown_SkColorType,             MTLPixelFormatRG8Unorm,
          kRG_88_GrPixelConfig,             { 0.5f, 0.5f, 0, 0 }},
        { kUnknown_SkColorType,             MTLPixelFormatR16Float,
          kAlpha_half_as_Red_GrPixelConfig, { 1.0f, 0, 0, 0.5f }},
#ifdef SK_BUILD_FOR_IOS
        { kUnknown_SkColorType,              MTLPixelFormatETC2_RGB8,
          kRGB_ETC1_GrPixelConfig,           SkColors::kRed     }
#endif
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeMtl(combo.fFormat);

        if (!mtlCaps->isConfigTexturable(combo.fConfig)) {
            continue;
        }

        // skbug.com/9086 (Metal caps may not be handling RGBA32 correctly)
        if (kRGBA_F32_SkColorType == combo.fColorType) {
            continue;
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !mtlCaps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

                if (GrRenderable::kYes == renderable) {
                    if (kRGB_888x_SkColorType == combo.fColorType) {
                        // Ganesh can't perform the blends correctly when rendering this format
                        continue;
                    }
                    if (!mtlCaps->isConfigRenderable(combo.fConfig)) {
                        continue;
                    }
                }

                {
                    auto uninitCreateMtd = [format](GrContext* context,
                                                    GrMipMapped mipMapped,
                                                    GrRenderable renderable) {
                        return context->createBackendTexture(32, 32, format,
                                                             mipMapped, renderable,
                                                             GrProtected::kNo);
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  combo.fColorType, mipMapped, renderable);
                }

                // Not implemented for Metal yet
#if 0
                {
                    auto createWithColorMtd = [format](GrContext* context,
                                                       const SkColor4f& color,
                                                       GrMipMapped mipMapped,
                                                       GrRenderable renderable) {
                        return context->priv().createBackendTexture(32, 32, format, color,
                                                                    mipMapped, renderable);
                    };

                    test_color_init(context, reporter, createWithColorMtd,
                                    combo.fColorType, combo.fColor, mipMapped, renderable);
                }
#endif
            }
        }
    }
}
