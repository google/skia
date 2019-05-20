/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*, GrMipMapped, GrRenderable)> create,
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

    if (kUnknown_SkColorType != colorType) {
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
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    context->deleteBackendTexture(backendTex);
}

///////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    struct {
        SkColorType   fColorType;
        GrPixelConfig fConfig;
    } combinations[] = {
        { kAlpha_8_SkColorType,      kAlpha_8_GrPixelConfig           },
        { kRGB_565_SkColorType,      kRGB_565_GrPixelConfig           },
        { kARGB_4444_SkColorType,    kRGBA_4444_GrPixelConfig         },
        { kRGBA_8888_SkColorType,    kRGBA_8888_GrPixelConfig         },
        { kRGB_888x_SkColorType,     kRGB_888_GrPixelConfig           },
        { kBGRA_8888_SkColorType,    kBGRA_8888_GrPixelConfig         },
        { kRGBA_1010102_SkColorType, kRGBA_1010102_GrPixelConfig      },
        { kRGB_101010x_SkColorType,  kUnknown_GrPixelConfig           },    // No Ganesh correlate
        { kGray_8_SkColorType,       kGray_8_GrPixelConfig            },
        { kRGBA_F16Norm_SkColorType, kRGBA_half_Clamped_GrPixelConfig },
        { kRGBA_F16_SkColorType,     kRGBA_half_GrPixelConfig         },
        { kRGBA_F32_SkColorType,     kRGBA_float_GrPixelConfig        },
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

                auto createMtd = [colorType](GrContext* context,
                                             GrMipMapped mipMapped,
                                             GrRenderable renderable) {
                    return context->createBackendTexture(32, 32, colorType, mipMapped, renderable);
                };

                test_wrapping(context, reporter, createMtd,
                              colorType, mipMapped, renderable);
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
    } combinations[] = {
        { kRGBA_8888_SkColorType,    GR_GL_RGBA8,                kRGBA_8888_GrPixelConfig         },
        { kRGBA_8888_SkColorType,    GR_GL_SRGB8_ALPHA8,         kSRGBA_8888_GrPixelConfig        },

        { kRGB_888x_SkColorType,     GR_GL_RGBA8,                kRGBA_8888_GrPixelConfig         },
        { kRGB_888x_SkColorType,     GR_GL_RGB8,                 kRGB_888_GrPixelConfig           },

        { kBGRA_8888_SkColorType,    GR_GL_RGBA8,                kRGBA_8888_GrPixelConfig         },
        { kBGRA_8888_SkColorType,    GR_GL_BGRA8,                kBGRA_8888_GrPixelConfig         },
        { kBGRA_8888_SkColorType,    GR_GL_SRGB8_ALPHA8,         kSBGRA_8888_GrPixelConfig        },

        { kRGBA_1010102_SkColorType, GR_GL_RGB10_A2,             kRGBA_1010102_GrPixelConfig      },
        { kRGB_565_SkColorType,      GR_GL_RGB565,               kRGB_565_GrPixelConfig           },
        { kARGB_4444_SkColorType,    GR_GL_RGBA4,                kRGBA_4444_GrPixelConfig         },

        { kAlpha_8_SkColorType,      GR_GL_ALPHA8,               kAlpha_8_as_Alpha_GrPixelConfig  },
        { kAlpha_8_SkColorType,      GR_GL_R8,                   kAlpha_8_as_Red_GrPixelConfig    },

        { kGray_8_SkColorType,       GR_GL_LUMINANCE8,           kGray_8_as_Lum_GrPixelConfig     },
        { kGray_8_SkColorType,       GR_GL_R8,                   kGray_8_as_Red_GrPixelConfig     },

        { kRGBA_F32_SkColorType,     GR_GL_RGBA32F,              kRGBA_float_GrPixelConfig        },

        { kRGBA_F16Norm_SkColorType, GR_GL_RGBA16F,              kRGBA_half_Clamped_GrPixelConfig },
        { kRGBA_F16_SkColorType,     GR_GL_RGBA16F,              kRGBA_half_GrPixelConfig         },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,      GR_GL_RG32F,                kRG_float_GrPixelConfig          },
        { kUnknown_SkColorType,      GR_GL_RG8,                  kRG_88_GrPixelConfig             },
        { kUnknown_SkColorType,      GR_GL_R16F,                 kAlpha_half_as_Red_GrPixelConfig },
        { kUnknown_SkColorType,      GR_GL_COMPRESSED_RGB8_ETC2, kRGB_ETC1_GrPixelConfig          },
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

                auto createMtd = [format](GrContext* context,
                                          GrMipMapped mipMapped,
                                          GrRenderable renderable) {
                    return context->createBackendTexture(32, 32, format, mipMapped, renderable);
                };

                test_wrapping(context, reporter, createMtd,
                              combo.fColorType, mipMapped, renderable);
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
    } combinations[] = {
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_UNORM           },
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_SRGB            },

        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8_UNORM             },
        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8A8_UNORM           },

        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_UNORM           },
        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_SRGB            },

        { kRGBA_1010102_SkColorType, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
        { kRGB_565_SkColorType,      VK_FORMAT_R5G6B5_UNORM_PACK16      },

        { kARGB_4444_SkColorType,    VK_FORMAT_R4G4B4A4_UNORM_PACK16    },
        { kARGB_4444_SkColorType,    VK_FORMAT_B4G4R4A4_UNORM_PACK16    },

        { kAlpha_8_SkColorType,      VK_FORMAT_R8_UNORM                 },
        { kGray_8_SkColorType,       VK_FORMAT_R8_UNORM                 },
        { kRGBA_F32_SkColorType,     VK_FORMAT_R32G32B32A32_SFLOAT      },

        { kRGBA_F16Norm_SkColorType, VK_FORMAT_R16G16B16A16_SFLOAT      },
        { kRGBA_F16_SkColorType,     VK_FORMAT_R16G16B16A16_SFLOAT      },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,      VK_FORMAT_R32G32_SFLOAT            },
        { kUnknown_SkColorType,      VK_FORMAT_R8G8_UNORM               },
        { kUnknown_SkColorType,      VK_FORMAT_R16_SFLOAT               },
        { kUnknown_SkColorType,      VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK  },
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

                auto createMtd = [format](GrContext* context,
                                          GrMipMapped mipMapped,
                                          GrRenderable renderable) {
                    return context->createBackendTexture(32, 32, format, mipMapped, renderable);
                };

                test_wrapping(context, reporter, createMtd,
                              combo.fColorType, mipMapped, renderable);
            }
        }
    }
}

#endif
