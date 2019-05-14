/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrContext.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*, GrRenderable)> createMtd,
                   SkColorType colorType, GrRenderable renderable) {
    GrResourceCache* cache = context->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    GrBackendTexture t = createMtd(context, renderable);
    if (!t.isValid()) {
        ERRORF(reporter, "Couldn't create backendTexture for colorType %d renderable %s\n",
               colorType,
               GrRenderable::kYes == renderable ? "yes" : "no");
        return;
    }
    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    if (kUnknown_SkColorType != colorType) {
        if (GrRenderable::kYes == renderable) {
            sk_sp<SkSurface> s = SkSurface::MakeFromBackendTexture(context,
                                                                   t,
                                                                   kTopLeft_GrSurfaceOrigin,
                                                                   0,
                                                                   colorType,
                                                                   nullptr, nullptr);
            if (!s) {
                ERRORF(reporter, "Couldn't make surface from backendTexture for colorType %d\n",
                       colorType);
            } else {
                REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
            }
        }

        {
            sk_sp<SkImage> i = SkImage::MakeFromTexture(context,
                                                        t,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        colorType,
                                                        kPremul_SkAlphaType,
                                                        nullptr);
            REPORTER_ASSERT(reporter, i);
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    context->priv().deleteBackendTexture(t);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    SkColorType colorTypes[] = {
        kAlpha_8_SkColorType,
        kRGB_565_SkColorType,
        kARGB_4444_SkColorType,
        kRGBA_8888_SkColorType,
        kRGB_888x_SkColorType,
        kBGRA_8888_SkColorType,
        kRGBA_1010102_SkColorType,
        kRGB_101010x_SkColorType,
        kGray_8_SkColorType,
        kRGBA_F16Norm_SkColorType,
        kRGBA_F16_SkColorType,
        kRGBA_F32_SkColorType
    };

    SkASSERT(kLastEnum_SkColorType == SK_ARRAY_COUNT(colorTypes));

    for (auto colorType : colorTypes) {
        if (kRGB_101010x_SkColorType == colorType) {
            continue; // Ganesh has no equivalent to this
        }
        if (kRGB_888x_SkColorType == colorType &&
            !caps->isConfigTexturable(kRGB_888_GrPixelConfig)) {
            continue;
        }

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {
            if (GrRenderable::kYes == renderable) {
                if (kRGB_888x_SkColorType == colorType) {
                    // Ganesh can't perform the blends correctly when rendering this format
                    continue;
                }
                if (kGray_8_SkColorType == colorType &&
                    !caps->isConfigRenderable(kGray_8_as_Lum_GrPixelConfig) &&
                    !caps->isConfigRenderable(kGray_8_as_Red_GrPixelConfig)) {
                    continue;
                }
                if (kARGB_4444_SkColorType == colorType &&
                    !caps->isConfigRenderable(kRGBA_4444_GrPixelConfig)) {
                    continue;
                }
            }

            auto createMtd = [colorType](GrContext* context, GrRenderable renderable) {
                return context->priv().createBackendTexture(32, 32, colorType,
                                                            GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, colorType, renderable);
        }
    }

}

#include "src/gpu/gl/GrGLDefines.h"

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    struct {
        SkColorType fColorType;
        GrGLenum    fFormat;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    GR_GL_RGBA8 },
        { kRGBA_8888_SkColorType,    GR_GL_SRGB8_ALPHA8 },

        { kRGB_888x_SkColorType,     GR_GL_RGBA8 },
        { kRGB_888x_SkColorType,     GR_GL_RGB8 },

        { kBGRA_8888_SkColorType,    GR_GL_RGBA8 },
        { kBGRA_8888_SkColorType,    GR_GL_BGRA8 },
        { kBGRA_8888_SkColorType,    GR_GL_SRGB8_ALPHA8 },

        { kRGBA_1010102_SkColorType, GR_GL_RGB10_A2 },
        { kRGB_565_SkColorType,      GR_GL_RGB565 },
        { kARGB_4444_SkColorType,    GR_GL_RGBA4 },

        { kAlpha_8_SkColorType,      GR_GL_ALPHA8 },
        { kAlpha_8_SkColorType,      GR_GL_R8 },

        { kGray_8_SkColorType,       GR_GL_LUMINANCE8 },
        { kGray_8_SkColorType,       GR_GL_R8 },

        { kRGBA_F32_SkColorType,     GR_GL_RGBA32F },

        { kRGBA_F16Norm_SkColorType, GR_GL_RGBA16F },
        { kRGBA_F16_SkColorType,     GR_GL_RGBA16F },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,      GR_GL_RG32F },
        { kUnknown_SkColorType,      GR_GL_RG8 },
        { kUnknown_SkColorType,      GR_GL_R16F },
        { kUnknown_SkColorType,      GR_GL_COMPRESSED_RGB8_ETC2 },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeGL(combo.fFormat, GR_GL_TEXTURE_2D);

        if (GR_GL_COMPRESSED_RGB8_ETC2 == combo.fFormat) {
            // We current disallow uninitialized ETC1 textures
            continue;
        }

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            if (GrRenderable::kYes == renderable) {
                if (kRGB_888x_SkColorType == combo.fColorType) {
                    // Ganesh can't perform the blends correctly when rendering this format
                    continue;
                }
                if (GR_GL_LUMINANCE8 == combo.fFormat &&
                    !caps->isConfigRenderable(kGray_8_as_Lum_GrPixelConfig)) {
                    continue;
                }
                if (GR_GL_R8 == combo.fFormat && kGray_8_SkColorType == combo.fColorType &&
                    !caps->isConfigRenderable(kGray_8_as_Red_GrPixelConfig)) {
                    continue;
                }
            }

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return context->priv().createBackendTexture(32, 32, format,
                                                            GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}


#ifdef SK_VULKAN

#include "src/gpu/vk/GrVkCaps.h"

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(context->priv().caps());

    struct {
        SkColorType fColorType;
        VkFormat    fFormat;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_UNORM },
        { kRGBA_8888_SkColorType,    VK_FORMAT_R8G8B8A8_SRGB },

        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8_UNORM },
        { kRGB_888x_SkColorType,     VK_FORMAT_R8G8B8A8_UNORM },

        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_UNORM },
        { kBGRA_8888_SkColorType,    VK_FORMAT_B8G8R8A8_SRGB },

        { kRGBA_1010102_SkColorType, VK_FORMAT_A2B10G10R10_UNORM_PACK32 },
        { kRGB_565_SkColorType,      VK_FORMAT_R5G6B5_UNORM_PACK16 },
        { kARGB_4444_SkColorType,    VK_FORMAT_B4G4R4A4_UNORM_PACK16 },
        { kAlpha_8_SkColorType,      VK_FORMAT_R8_UNORM },
        { kGray_8_SkColorType,       VK_FORMAT_R8_UNORM },
        { kRGBA_F32_SkColorType,     VK_FORMAT_R32G32B32A32_SFLOAT },

        { kRGBA_F16Norm_SkColorType, VK_FORMAT_R16G16B16A16_SFLOAT },
        { kRGBA_F16_SkColorType,     VK_FORMAT_R16G16B16A16_SFLOAT },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,      VK_FORMAT_R32G32_SFLOAT },
        { kUnknown_SkColorType,      VK_FORMAT_R8G8_UNORM },
        { kUnknown_SkColorType,      VK_FORMAT_R16_SFLOAT },
        { kUnknown_SkColorType,      VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK },
    };

    for (auto combo : combinations) {
        if (!vkCaps->isFormatTexturable(combo.fFormat)) {
            continue;
        }

        GrBackendFormat format = GrBackendFormat::MakeVk(combo.fFormat);

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

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return context->priv().createBackendTexture(32, 32, format,
                                                            GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}

#endif

#ifdef SK_METAL

DEF_GPUTEST_FOR_METAL_CONTEXT(MtlBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    struct {
        SkColorType    fColorType;
        MTLPixelFormat fFormat;
    } combinations[] = {
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm },
        { kRGBA_8888_SkColorType,    MTLPixelFormatRGBA8Unorm_sRGB },

        { kRGB_888x_SkColorType,     MTLPixelFormatRGBA8Unorm },

        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm },
        { kBGRA_8888_SkColorType,    MTLPixelFormatBGRA8Unorm_sRGB },

        { kRGBA_1010102_SkColorType, MTLPixelFormatRGB10A2Unorm },
#ifdef SK_BUILD_FOR_IOS
        { kRGB_565_SkColorType,      MTLPixelFormatB5G6R5Unorm },
        { kARGB_4444_SkColorType,    MTLPixelFormatABGR4Unorm },
#endif

        { kAlpha_8_SkColorType,      MTLPixelFormatA8Unorm },
        { kAlpha_8_SkColorType,      MTLPixelFormatR8Unorm },

        { kGray_8_SkColorType,       MTLPixelFormatR8Unorm },

        { kRGBA_F32_SkColorType,     MTLPixelFormatRGBA32Float },

        { kRGBA_F16Norm_SkColorType, MTLPixelFormatRGBA16Float },
        { kRGBA_F16_SkColorType,     MTLPixelFormatRGBA16Float },

        // These backend formats don't have SkColorType equivalents
        { kUnknown_SkColorType,     MTLPixelFormatRG32Float },
        { kUnknown_SkColorType,     MTLPixelFormatRG8Unorm },
        { kUnknown_SkColorType,     MTLPixelFormatR16Float },
#ifdef SK_BUILD_FOR_IOS
        { kUnknown_SkColorType,     MTLPixelFormatETC2_RGB8 }
#endif
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeMtl(combo.fFormat);

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            if (GrRenderable::kYes == renderable) {
                if (kRGB_888x_SkColorType == combo.fColorType) {
                    // Ganesh can't perform the blends correctly when rendering this format
                    continue;
                }
            }

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return context->createBackendTexture(32, 32, format,
                                                     GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}

#endif
