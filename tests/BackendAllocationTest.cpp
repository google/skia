/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*,
                                                   GrMipMapped,
                                                   GrRenderable)> create,
                   SkColorType colorType, GrMipMapped mipMapped, GrRenderable renderable) {
    GrResourceCache* cache = context->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    GrBackendTexture backendTex = create(context, mipMapped, renderable);
    if (!backendTex.isValid()) {
        ERRORF(reporter, "Couldn't create backendTexture for colorType %d renderable %s\n",
               colorType,
               GrRenderable::kYes == renderable ? "yes" : "no");
        return;
    }

    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    if (kUnknown_SkColorType == colorType) {
        context->priv().deleteBackendTexture(backendTex);
        return;
    }

    if (GrRenderable::kYes == renderable) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(context,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  colorType,
                                                                  nullptr, nullptr);
        if (!surf) {
            ERRORF(reporter, "Couldn't make surface from backendTexture for colorType %d\n",
                    colorType);
        } else {
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      colorType,
                                                      kPremul_SkAlphaType,
                                                      nullptr);
        if (!img) {
            ERRORF(reporter, "Couldn't make image from backendTexture for colorType %d\n",
                    colorType);
        } else {
            SkImage_Base* ib = as_IB(img);

            GrTextureProxy* proxy = ib->peekProxy();
            REPORTER_ASSERT(reporter, proxy);

            REPORTER_ASSERT(reporter, mipMapped == proxy->proxyMipMapped());
            REPORTER_ASSERT(reporter, proxy->isInstantiated());
            REPORTER_ASSERT(reporter, mipMapped == proxy->mipMapped());

            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    context->priv().deleteBackendTexture(backendTex);
}

static void compare_pixmaps(const SkPixmap& p1, const SkPixmap& p2, skiatest::Reporter* reporter) {
    SkASSERT(p1.info() == p2.info());
    for (int y = 0; y < p1.height(); ++y) {
        for (int x = 0; x < p1.width(); ++x) {
            REPORTER_ASSERT(reporter, p1.getColor(x, y) == p2.getColor(x, y));
            if (p1.getColor(x, y) != p2.getColor(x, y)) {
                return;
            }
        }
    }
}

// Test initialization of GrBackendObjects to a specific color
void test_color_init(GrContext* context, skiatest::Reporter* reporter,
                     std::function<GrBackendTexture (GrContext*,
                                                     const SkColor4f&,
                                                     GrMipMapped,
                                                     GrRenderable)> create,
                     SkColorType colorType, const SkColor4f& color,
                     GrMipMapped mipMapped, GrRenderable renderable) {

    GrBackendTexture backendTex = create(context, color, mipMapped, renderable);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    if (kUnknown_SkColorType == colorType) {
        // TODO: burrow in and do this manually!
        context->priv().deleteBackendTexture(backendTex);
        return;
    }

    SkImageInfo ii = SkImageInfo::Make(32, 32, colorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage expected;
    SkAssertResult(expected.tryAlloc(ii));
    expected.erase(color);

    SkAutoPixmapStorage actual;
    SkAssertResult(actual.tryAlloc(ii));
    actual.erase(SkColors::kTransparent);

    if (GrRenderable::kYes == renderable) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(context,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  colorType,
                                                                  nullptr, nullptr);
        if (surf) {
            bool result = surf->readPixels(actual, 0, 0);
            REPORTER_ASSERT(reporter, result);

            compare_pixmaps(expected, actual, reporter);

            actual.erase(SkColors::kTransparent);
        }
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      colorType,
                                                      kPremul_SkAlphaType,
                                                      nullptr);
        if (img) {
            bool result = img->readPixels(actual, 0, 0);
            REPORTER_ASSERT(reporter, result);

            compare_pixmaps(expected, actual, reporter);
        }
    }

    context->priv().deleteBackendTexture(backendTex);
}

///////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    struct {
        SkColorType   fColorType;
        GrPixelConfig fConfig;
        SkColor4f     fColor;
    } combinations[] = {
        { kAlpha_8_SkColorType,      kAlpha_8_GrPixelConfig,           { 0, 0.25f, 0.75f, 0.5f } },
        { kRGB_565_SkColorType,      kRGB_565_GrPixelConfig,           SkColors::kRed            },
        { kARGB_4444_SkColorType,    kRGBA_4444_GrPixelConfig,         SkColors::kGreen          },
        { kRGBA_8888_SkColorType,    kRGBA_8888_GrPixelConfig,         SkColors::kBlue           },
        { kRGB_888x_SkColorType,     kRGB_888_GrPixelConfig,           SkColors::kCyan           },
        { kBGRA_8888_SkColorType,    kBGRA_8888_GrPixelConfig,         SkColors::kMagenta        },
        { kRGBA_1010102_SkColorType, kRGBA_1010102_GrPixelConfig,      { 0.5f, 0, 0, 0.5f }      },
        // The kRGB_101010x_SkColorType has no Ganesh correlate
        { kRGB_101010x_SkColorType,  kUnknown_GrPixelConfig,           { 0, 0.5f, 0, 0.5f }      },
        { kGray_8_SkColorType,       kGray_8_GrPixelConfig,            SkColors::kYellow         },
        { kRGBA_F16Norm_SkColorType, kRGBA_half_Clamped_GrPixelConfig, SkColors::kLtGray         },
        { kRGBA_F16_SkColorType,     kRGBA_half_GrPixelConfig,         SkColors::kDkGray         },
        { kRGBA_F32_SkColorType,     kRGBA_float_GrPixelConfig,        SkColors::kGray           },
    };

    SkASSERT(kLastEnum_SkColorType == SK_ARRAY_COUNT(combinations));

    for (auto combo : combinations) {
        SkColorType colorType = combo.fColorType;

        if (!caps->isConfigTexturable(combo.fConfig)) {
            continue;
        }

        if (GrBackendApi::kMetal == context->backend()) {
            // skbug.com/9086 (Metal caps may not be handling RGBA32 correctly)
            if (kRGBA_F32_SkColorType == combo.fColorType) {
                continue;
            }
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !caps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {
                if (GrRenderable::kYes == renderable) {
                    if (kRGB_888x_SkColorType == combo.fColorType) {
                        // Ganesh can't perform the blends correctly when rendering this format
                        continue;
                    }
                    if (!caps->isConfigRenderable(combo.fConfig)) {
                        continue;
                    }
                }

                {
                    auto uninitCreateMtd = [colorType](GrContext* context,
                                                       GrMipMapped mipMapped,
                                                       GrRenderable renderable) {
                        return context->priv().createBackendTexture(32, 32, colorType,
                                                                    mipMapped, renderable);
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  colorType, mipMapped, renderable);
                }

                {
                    auto createWithColorMtd = [colorType](GrContext* context,
                                                          const SkColor4f& color,
                                                          GrMipMapped mipMapped,
                                                          GrRenderable renderable) {
                        return context->priv().createBackendTexture(32, 32, colorType, color,
                                                                    mipMapped, renderable);
                    };

                    test_color_init(context, reporter, createWithColorMtd,
                                    colorType, combo.fColor, mipMapped, renderable);
                }
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
#ifdef SK_GL

#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLDefines.h"

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLBackendAllocationTest, reporter, ctxInfo) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    GrGLStandard standard = glCtx->gl()->fStandard;
    GrContext* context = ctxInfo.grContext();
    const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(context->priv().caps());

    struct {
        SkColorType   fColorType;
        GrGLenum      fFormat;
        // TODO: remove 'fConfig' and directly use 'fFormat' in GrGLCaps::isFormatTexturable
        GrPixelConfig fConfig;
        SkColor4f     fColor;
    } combinations[] = {
        { kRGBA_8888_SkColorType,           GR_GL_RGBA8,
          kRGBA_8888_GrPixelConfig,         SkColors::kRed      },
        { kRGBA_8888_SkColorType,           GR_GL_SRGB8_ALPHA8,
          kSRGBA_8888_GrPixelConfig,        SkColors::kRed      },

        { kRGB_888x_SkColorType,            GR_GL_RGBA8,
          kRGBA_8888_GrPixelConfig,         SkColors::kRed      },
        { kRGB_888x_SkColorType,            GR_GL_RGB8,
          kRGB_888_GrPixelConfig,           SkColors::kRed      },

        { kBGRA_8888_SkColorType,           GR_GL_RGBA8,
          kRGBA_8888_GrPixelConfig,         SkColors::kRed      },
        { kBGRA_8888_SkColorType,           GR_GL_BGRA8,
          kBGRA_8888_GrPixelConfig,         SkColors::kRed      },
        { kBGRA_8888_SkColorType,           GR_GL_SRGB8_ALPHA8,
          kSBGRA_8888_GrPixelConfig,        SkColors::kRed      },

        { kRGBA_1010102_SkColorType,        GR_GL_RGB10_A2,
          kRGBA_1010102_GrPixelConfig,      SkColors::kRed      },
        { kRGB_565_SkColorType,             GR_GL_RGB565,
          kRGB_565_GrPixelConfig,           SkColors::kRed      },
        { kARGB_4444_SkColorType,           GR_GL_RGBA4,
          kRGBA_4444_GrPixelConfig,         SkColors::kRed      },

        { kAlpha_8_SkColorType,             GR_GL_ALPHA8,
          kAlpha_8_as_Alpha_GrPixelConfig,  SkColors::kRed      },
        { kAlpha_8_SkColorType,             GR_GL_R8,
          kAlpha_8_as_Red_GrPixelConfig,    SkColors::kRed      },

        { kGray_8_SkColorType,              GR_GL_LUMINANCE8,
          kGray_8_as_Lum_GrPixelConfig,     SkColors::kRed      },
        { kGray_8_SkColorType,              GR_GL_R8,
          kGray_8_as_Red_GrPixelConfig,     SkColors::kRed      },

        { kRGBA_F32_SkColorType,            GR_GL_RGBA32F,
          kRGBA_float_GrPixelConfig,        SkColors::kRed      },

        { kRGBA_F16Norm_SkColorType,        GR_GL_RGBA16F,
          kRGBA_half_Clamped_GrPixelConfig, SkColors::kRed      },
        { kRGBA_F16_SkColorType,            GR_GL_RGBA16F,
          kRGBA_half_GrPixelConfig,         SkColors::kRed      },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,             GR_GL_RG32F,
          kRG_float_GrPixelConfig,          SkColors::kRed      },
        { kUnknown_SkColorType,             GR_GL_RG8,
          kRG_88_GrPixelConfig,             SkColors::kRed      },
        { kUnknown_SkColorType,             GR_GL_R16F,
          kAlpha_half_as_Red_GrPixelConfig, SkColors::kRed},
        { kUnknown_SkColorType,             GR_GL_COMPRESSED_RGB8_ETC2,
          kRGB_ETC1_GrPixelConfig,          SkColors::kRed      },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeGL(combo.fFormat, GR_GL_TEXTURE_2D);

        if (GR_GL_COMPRESSED_RGB8_ETC2 == combo.fFormat) {
            // We current disallow uninitialized ETC1 textures in the GL backend
            continue;
        }
        if (!glCaps->isConfigTexturable(combo.fConfig)) {
            continue;
        }

        if (kBGRA_8888_SkColorType == combo.fColorType) {
            if (GR_GL_RGBA8 == combo.fFormat && kGL_GrGLStandard != standard) {
                continue;
            }
            if (GR_GL_BGRA8 == combo.fFormat && kGL_GrGLStandard == standard) {
                continue;
            }
        }

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !glCaps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

                if (GrRenderable::kYes == renderable) {
                    if (kRGB_888x_SkColorType == combo.fColorType) {
                        // Ganesh can't perform the blends correctly when rendering this format
                        continue;
                    }
                    if (!glCaps->isConfigRenderable(combo.fConfig)) {
                        continue;
                    }
                }

                {
                    auto createMtd = [format](GrContext* context,
                                              GrMipMapped mipMapped,
                                              GrRenderable renderable) {
                        return context->priv().createBackendTexture(32, 32, format,
                                                                    mipMapped, renderable);
                    };

                    test_wrapping(context, reporter, createMtd,
                                  combo.fColorType, mipMapped, renderable);
                }

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
            }
        }
    }
}

#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef SK_VULKAN

#include "src/gpu/vk/GrVkCaps.h"

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(context->priv().caps());

    struct {
        SkColorType fColorType;
        VkFormat    fFormat;
        SkColor4f   fColor;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_UNORM,           SkColors::kRed },
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_SRGB,            SkColors::kRed },

        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8_UNORM,             SkColors::kRed },
        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8A8_UNORM,           SkColors::kRed },

        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_UNORM,           SkColors::kRed },
        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_SRGB,            SkColors::kRed },

        { kRGBA_1010102_SkColorType, VK_FORMAT_A2B10G10R10_UNORM_PACK32, SkColors::kRed },
        { kRGB_565_SkColorType,      VK_FORMAT_R5G6B5_UNORM_PACK16,      SkColors::kRed },

        { kARGB_4444_SkColorType,    VK_FORMAT_R4G4B4A4_UNORM_PACK16,    SkColors::kRed },
        { kARGB_4444_SkColorType,    VK_FORMAT_B4G4R4A4_UNORM_PACK16,    SkColors::kRed },

        { kAlpha_8_SkColorType,      VK_FORMAT_R8_UNORM,                 SkColors::kRed },
        { kGray_8_SkColorType,       VK_FORMAT_R8_UNORM,                 SkColors::kRed },
        { kRGBA_F32_SkColorType,     VK_FORMAT_R32G32B32A32_SFLOAT,      SkColors::kRed },

        { kRGBA_F16Norm_SkColorType, VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kRed },
        { kRGBA_F16_SkColorType,     VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kRed },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,      VK_FORMAT_R32G32_SFLOAT,            SkColors::kRed },
        { kUnknown_SkColorType,      VK_FORMAT_R8G8_UNORM,               SkColors::kRed },
        { kUnknown_SkColorType,      VK_FORMAT_R16_SFLOAT,               SkColors::kRed },
        { kUnknown_SkColorType,      VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,  SkColors::kRed },
    };

    for (auto combo : combinations) {
        if (!vkCaps->isFormatTexturable(combo.fFormat)) {
            continue;
        }

        GrBackendFormat format = GrBackendFormat::MakeVk(combo.fFormat);

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !vkCaps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

                if (GrRenderable::kYes == renderable) {
                    if (kRGB_888x_SkColorType == combo.fColorType) {
                        // Ganesh can't perform the blends correctly when rendering this format
                        continue;
                    }
                    if (!vkCaps->isFormatRenderable(combo.fFormat)) {
                        continue;
                    }
                }

                {
                    auto createMtd = [format](GrContext* context,
                                              GrMipMapped mipMapped,
                                              GrRenderable renderable) {
                        return context->priv().createBackendTexture(32, 32, format,
                                                                    mipMapped, renderable);
                    };

                    test_wrapping(context, reporter, createMtd,
                                  combo.fColorType, mipMapped, renderable);
                }

                {

                }
            }
        }
    }
}

#endif
