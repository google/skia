/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "tests/Test.h"

#import <Metal/Metal.h>
#include "src/gpu/mtl/GrMtlCaps.h"

// In BackendAllocationTest.cpp
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*,
                                                   GrMipMapped,
                                                   GrRenderable)> create,
                   GrColorType colorType, GrMipMapped mipMapped, GrRenderable renderable);

void test_color_init(GrContext* context, skiatest::Reporter* reporter,
                     std::function<GrBackendTexture (GrContext*,
                                                     const SkColor4f&,
                                                     GrMipMapped,
                                                     GrRenderable)> create,
                     GrColorType colorType, const SkColor4f& color,
                     GrMipMapped mipMapped, GrRenderable renderable);

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrMtlCaps* mtlCaps = static_cast<const GrMtlCaps*>(context->priv().caps());

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };

    struct {
        GrColorType      fColorType;
        MTLPixelFormat fFormat;
        SkColor4f        fColor;
    } combinations[] = {
        { GrColorType::kRGBA_8888,        MTLPixelFormatRGBA8Unorm,      SkColors::kRed       },
        { GrColorType::kRGBA_8888_SRGB,   MTLPixelFormatRGBA8Unorm_sRGB, SkColors::kRed       },

        // In this configuration (i.e., an RGB_888x colortype with an RGBA8 backing format),
        // there is nothing to tell Skia to make the provided color opaque. Clients will need
        // to provide an opaque initialization color in this case.
        { GrColorType::kRGB_888x,         MTLPixelFormatRGBA8Unorm,      SkColors::kYellow    },

        { GrColorType::kBGRA_8888,        MTLPixelFormatBGRA8Unorm,      SkColors::kBlue      },

        { GrColorType::kRGBA_1010102,     MTLPixelFormatRGB10A2Unorm,    { 0.5f, 0, 0, 1.0f } },
#ifdef SK_BUILD_FOR_IOS
        { GrColorType::kBGR_565,          MTLPixelFormatB5G6R5Unorm,     SkColors::kRed       },
        { GrColorType::kABGR_4444,        MTLPixelFormatABGR4Unorm,      SkColors::kGreen     },
#endif

        { GrColorType::kAlpha_8,          MTLPixelFormatA8Unorm,         kTransCol            },
        { GrColorType::kAlpha_8,          MTLPixelFormatR8Unorm,         kTransCol            },
        { GrColorType::kGray_8,           MTLPixelFormatR8Unorm,         SkColors::kDkGray    },

        { GrColorType::kRGBA_F32,         MTLPixelFormatRGBA32Float,     SkColors::kRed       },

        { GrColorType::kRGBA_F16_Clamped, MTLPixelFormatRGBA16Float,     SkColors::kLtGray    },
        { GrColorType::kRGBA_F16,         MTLPixelFormatRGBA16Float,     SkColors::kYellow    },

        { GrColorType::kRG_88,            MTLPixelFormatRG8Unorm,        { 0.5f, 0.5f, 0, 1 } },
        { GrColorType::kAlpha_F16,        MTLPixelFormatR16Float,        { 1.0f, 0, 0, 0.5f } },

        { GrColorType::kAlpha_16,         MTLPixelFormatR16Unorm,        kTransCol            },
        { GrColorType::kRG_1616,          MTLPixelFormatRG16Unorm,       SkColors::kYellow    },

        { GrColorType::kRGBA_16161616,    MTLPixelFormatRGBA16Unorm,     SkColors::kLtGray    },
        { GrColorType::kRG_F16,           MTLPixelFormatRG16Float,       SkColors::kYellow    },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeMtl(combo.fFormat);

        if (!mtlCaps->isFormatTexturable(combo.fFormat)) {
            continue;
        }

        // skbug.com/9086 (Metal caps may not be handling RGBA32 correctly)
        if (GrColorType::kRGBA_F32 == combo.fColorType) {
            continue;
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !mtlCaps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

                if (GrRenderable::kYes == renderable) {
                    // We must also check whether we allow rendering to the format using the
                    // color type.
                    if (!mtlCaps->isFormatAsColorTypeRenderable(
                            combo.fColorType, GrBackendFormat::MakeMtl(combo.fFormat), 1)) {
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
                {
                    auto createWithColorMtd = [format](GrContext* context,
                                                       const SkColor4f& color,
                                                       GrMipMapped mipMapped,
                                                       GrRenderable renderable) {
                        return context->createBackendTexture(32, 32, format, color,
                                                             mipMapped, renderable);
                    };

                    test_color_init(context, reporter, createWithColorMtd,
                                    combo.fColorType, combo.fColor, mipMapped, renderable);
                }
            }
        }
    }
}
