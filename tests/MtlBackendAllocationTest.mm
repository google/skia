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
                   std::function<GrBackendTexture (GrContext*, GrMipMapped, GrRenderable)> create,
                   SkColorType colorType, GrMipMapped mipMapped, GrRenderable renderable);

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrMtlCaps* mtlCaps = static_cast<const GrMtlCaps*>(context->priv().caps());

    struct {
        SkColorType      fColorType;
        GrMTLPixelFormat fFormat;
        // TODO: remove 'fConfig' and directly use 'fFormat' in GrMtlCaps::isFormatTexturable
        GrPixelConfig    fConfig;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm,      kRGBA_8888_GrPixelConfig      },
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm_sRGB, kSRGBA_8888_GrPixelConfig     },

        { kRGB_888x_SkColorType,     MTLPixelFormatRGBA8Unorm,      kRGBA_8888_GrPixelConfig      },

        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm,      kBGRA_8888_GrPixelConfig      },
        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm_sRGB, kSBGRA_8888_GrPixelConfig     },

        { kRGBA_1010102_SkColorType, MTLPixelFormatRGB10A2Unorm,    kRGBA_1010102_GrPixelConfig    },
#ifdef SK_BUILD_FOR_IOS
        { kRGB_565_SkColorType,      MTLPixelFormatB5G6R5Unorm,     kRGB_565_GrPixelConfig        },
        { kARGB_4444_SkColorType,    MTLPixelFormatABGR4Unorm,      kRGBA_4444_GrPixelConfig      },
#endif

        { kAlpha_8_SkColorType,      MTLPixelFormatA8Unorm,         kAlpha_8_as_Alpha_GrPixelConfig },
        { kAlpha_8_SkColorType,      MTLPixelFormatR8Unorm,         kAlpha_8_as_Red_GrPixelConfig },

        { kGray_8_SkColorType,       MTLPixelFormatR8Unorm,         kGray_8_as_Red_GrPixelConfig  },

        { kRGBA_F32_SkColorType,     MTLPixelFormatRGBA32Float,     kRGBA_float_GrPixelConfig     },

        { kRGBA_F16Norm_SkColorType, MTLPixelFormatRGBA16Float,     kRGBA_half_Clamped_GrPixelConfig },
        { kRGBA_F16_SkColorType,     MTLPixelFormatRGBA16Float,     kRGBA_half_GrPixelConfig      },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,     MTLPixelFormatRG32Float,        kRG_float_GrPixelConfig       },
        { kUnknown_SkColorType,     MTLPixelFormatRG8Unorm,         kRG_88_GrPixelConfig          },
        { kUnknown_SkColorType,     MTLPixelFormatR16Float,         kAlpha_half_as_Red_GrPixelConfig },
#ifdef SK_BUILD_FOR_IOS
        { kUnknown_SkColorType,     MTLPixelFormatETC2_RGB8,        kRGB_ETC1_GrPixelConfig       }
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
                                                             mipMapped, renderable);
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  combo.fColorType, mipMapped, renderable);
                }
            }
        }
    }
}
