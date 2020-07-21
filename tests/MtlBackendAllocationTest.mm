/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrDirectContext.h"
#include "src/gpu/GrContextPriv.h"
#include "tests/Test.h"

#import <Metal/Metal.h>
#include "src/gpu/mtl/GrMtlCaps.h"

// In BackendAllocationTest.cpp
void test_wrapping(GrDirectContext*,
                   skiatest::Reporter*,
                   std::function<GrBackendTexture (GrDirectContext*,
                                                   GrMipmapped,
                                                   GrRenderable)> create,
                   GrColorType,
                   GrMipmapped,
                   GrRenderable,
                   bool* finishedBackendCreation);

void test_color_init(GrDirectContext*,
                     skiatest::Reporter*,
                     std::function<GrBackendTexture (GrDirectContext*,
                                                     const SkColor4f&,
                                                     GrMipmapped,
                                                     GrRenderable)> create,
                     GrColorType,
                     const SkColor4f&,
                     GrMipmapped,
                     GrRenderable,
                     bool* finishedBackendCreation);

static void mark_signaled(void* context) {
    *(bool*)context = true;
}

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendAllocationTest, reporter, ctxInfo) {
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

        for (auto mipMapped : { GrMipmapped::kNo, GrMipmapped::kYes }) {
            if (GrMipmapped::kYes == mipMapped && !mtlCaps->mipmapSupport()) {
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
                                                    GrMipmapped mipMapped,
                                                    GrRenderable renderable) {
                        return dContext->createBackendTexture(32, 32, format,
                                                              mipMapped, renderable,
                                                              GrProtected::kNo);
                    };

                    test_wrapping(dContext, reporter, uninitCreateMtd,
                                  combo.fColorType, mipMapped, renderable, nullptr);
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
                    GrSwizzle swizzle;
                    switch (combo.fColorType) {
                        case GrColorType::kAlpha_8:
                            swizzle = GrSwizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_16:
                            swizzle = GrSwizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_F16:
                            swizzle = GrSwizzle("aaaa");
                            break;
                        default:
                            break;
                    }

                    bool finishedBackendCreation = false;
                    bool* finishedPtr = &finishedBackendCreation;

                    auto createWithColorMtd = [format, swizzle, finishedPtr](
                            GrDirectContext* dContext,
                            const SkColor4f& color,
                            GrMipmapped mipMapped,
                            GrRenderable renderable) {
                        auto swizzledColor = swizzle.applyTo(color);
                        return dContext->createBackendTexture(32, 32, format, swizzledColor,
                                                              mipMapped, renderable,
                                                              GrProtected::kNo,
                                                              mark_signaled, finishedPtr);
                    };
                    // We make our comparison color using SkPixmap::erase(color) on a pixmap of
                    // combo.fColorType and then calling SkPixmap::readPixels(). erase() will premul
                    // the color passed to it. However, createBackendTexture() that takes a
                    // SkColor4f is color type/alpha type unaware and will simply compute luminance
                    // from the r, g, b, channels.
                    SkColor4f color = combo.fColor;
                    if (combo.fColorType == GrColorType::kGray_8) {
                        color = {color.fR * color.fA,
                            color.fG * color.fA,
                            color.fB * color.fA,
                            1.f};
                    }
                    test_color_init(dContext, reporter, createWithColorMtd, combo.fColorType, color,
                                    mipMapped, renderable, finishedPtr);
                }
            }
        }
    }
}
