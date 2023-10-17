/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/mtl/GrMtlCaps.h"
#include "tests/Test.h"
#include "tools/gpu/ManagedBackendTexture.h"

#import <Metal/Metal.h>

using sk_gpu_test::ManagedBackendTexture;

// In BackendAllocationTest.cpp
void test_wrapping(GrDirectContext*,
                   skiatest::Reporter*,
                   const std::function<sk_sp<ManagedBackendTexture>(
                           GrDirectContext*, skgpu::Mipmapped, GrRenderable)>& create,
                   GrColorType,
                   skgpu::Mipmapped,
                   GrRenderable);

void test_color_init(
        GrDirectContext*,
        skiatest::Reporter*,
        const std::function<sk_sp<ManagedBackendTexture>(
                GrDirectContext*, const SkColor4f&, skgpu::Mipmapped, GrRenderable)>& create,
        GrColorType,
        const SkColor4f&,
        skgpu::Mipmapped,
        GrRenderable);

void test_pixmap_init(GrDirectContext*,
                      skiatest::Reporter*,
                      const std::function<sk_sp<ManagedBackendTexture>(GrDirectContext*,
                                                                       const SkPixmap srcData[],
                                                                       int numLevels,
                                                                       GrSurfaceOrigin,
                                                                       GrRenderable)>& create,
                      SkColorType,
                      GrSurfaceOrigin,
                      skgpu::Mipmapped,
                      GrRenderable);

DEF_GANESH_TEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
    auto dContext = ctxInfo.directContext();
    const GrMtlCaps* mtlCaps = static_cast<const GrMtlCaps*>(dContext->priv().caps());

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 0.75f };

    struct {
        GrColorType      fColorType;
        MTLPixelFormat   fFormat;
        SkColor4f        fColor;
    } combinations[] = {
        { GrColorType::kRGBA_8888,        MTLPixelFormatRGBA8Unorm,      SkColors::kRed       },
        { GrColorType::kRGBA_8888_SRGB,   MTLPixelFormatRGBA8Unorm_sRGB, SkColors::kRed       },

        // In this configuration (i.e., an RGB_888x colortype with an RGBA8 backing format),
        // there is nothing to tell Skia to make the provided color opaque. Clients will need
        // to provide an opaque initialization color in this case.
        { GrColorType::kRGB_888x,         MTLPixelFormatRGBA8Unorm,      SkColors::kYellow    },

        { GrColorType::kBGRA_8888,        MTLPixelFormatBGRA8Unorm,      SkColors::kBlue      },

        { GrColorType::kRGBA_1010102,     MTLPixelFormatRGB10A2Unorm,
                                                                    { 0.25f, 0.5f, 0.75f, 1.0f } },
#ifdef SK_BUILD_FOR_MAC
        { GrColorType::kBGRA_1010102,     MTLPixelFormatBGR10A2Unorm,
                                                                    { 0.25f, 0.5f, 0.75f, 1.0f } },
#endif
#ifdef SK_BUILD_FOR_IOS
        { GrColorType::kBGR_565,          MTLPixelFormatB5G6R5Unorm,     SkColors::kRed       },
        { GrColorType::kABGR_4444,        MTLPixelFormatABGR4Unorm,      SkColors::kGreen     },
#endif

        { GrColorType::kAlpha_8,          MTLPixelFormatA8Unorm,         kTransCol            },
        { GrColorType::kAlpha_8,          MTLPixelFormatR8Unorm,         kTransCol            },
        { GrColorType::kGray_8,           MTLPixelFormatR8Unorm,         kGrayCol             },

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

        for (auto mipmapped : {skgpu::Mipmapped::kNo, skgpu::Mipmapped::kYes}) {
            if (skgpu::Mipmapped::kYes == mipmapped && !mtlCaps->mipmapSupport()) {
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
                    auto uninitCreateMtd = [format](GrDirectContext* dContext,
                                                    skgpu::Mipmapped mipmapped,
                                                    GrRenderable renderable) {
                        return ManagedBackendTexture::MakeWithoutData(dContext,
                                                                      32, 32,
                                                                      format,
                                                                      mipmapped,
                                                                      renderable,
                                                                      GrProtected::kNo);
                    };

                    test_wrapping(dContext, reporter, uninitCreateMtd, combo.fColorType, mipmapped,
                                  renderable);
                }

                {
                    // We're creating backend textures without specifying a color type "view" of
                    // them at the public API level. Therefore, Ganesh will not apply any swizzles
                    // before writing the color to the texture. However, our validation code does
                    // rely on interpreting the texture contents via a SkColorType and therefore
                    // swizzles may be applied during the read step.
                    // Ideally we'd update our validation code to use a "raw" read that doesn't
                    // impose a color type but for now we just munge the data we upload to match the
                    // expectation.
                    skgpu::Swizzle swizzle;
                    switch (combo.fColorType) {
                        case GrColorType::kAlpha_8:
                            swizzle = skgpu::Swizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_16:
                            swizzle = skgpu::Swizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_F16:
                            swizzle = skgpu::Swizzle("aaaa");
                            break;
                        default:
                            break;
                    }

                    auto createWithColorMtd = [format, swizzle](GrDirectContext* dContext,
                                                                const SkColor4f& color,
                                                                skgpu::Mipmapped mipmapped,
                                                                GrRenderable renderable) {
                        auto swizzledColor = swizzle.applyTo(color);
                        return ManagedBackendTexture::MakeWithData(dContext,
                                                                   32, 32,
                                                                   format,
                                                                   swizzledColor,
                                                                   mipmapped,
                                                                   renderable,
                                                                   GrProtected::kNo);
                    };
                    test_color_init(dContext, reporter, createWithColorMtd, combo.fColorType,
                                    combo.fColor, mipmapped, renderable);
                }

                for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
                    SkColorType skColorType = GrColorTypeToSkColorType(combo.fColorType);
                    if (skColorType == kUnknown_SkColorType) {
                        break;
                    }

                    auto createWithSrcDataMtd = [](GrDirectContext* dContext,
                                                   const SkPixmap srcData[],
                                                   int numLevels,
                                                   GrSurfaceOrigin origin,
                                                   GrRenderable renderable) {
                        SkASSERT(srcData && numLevels);
                        return ManagedBackendTexture::MakeWithData(dContext,
                                                                   srcData,
                                                                   numLevels,
                                                                   origin,
                                                                   renderable,
                                                                   GrProtected::kNo);
                    };

                    test_pixmap_init(dContext,
                                     reporter,
                                     createWithSrcDataMtd,
                                     skColorType,
                                     origin,
                                     mipmapped,
                                     renderable);

                }
            }
        }
    }
}
