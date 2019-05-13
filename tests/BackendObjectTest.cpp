/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/GrBackendObject.h"
#include "src/gpu/GrContextPriv.h"
#include "include/core/SkSurface.h"
#include "tests/Test.h"

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<sk_sp<GrBackendObject> (GrContext*, GrRenderable)> createMtd,
                   SkColorType colorType, GrRenderable renderable) {
    GrResourceCache* cache = context->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    sk_sp<GrBackendObject> o = createMtd(context, renderable);
    REPORTER_ASSERT(reporter, o);
    if (!o) {
        return;
    }
    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    if (kUnknown_SkColorType != colorType) {
        if (GrRenderable::kYes == renderable) {
            sk_sp<SkSurface> s = SkSurface::MakeFromBackendTexture(context,
                                                                   o->backendTexture(),
                                                                   kTopLeft_GrSurfaceOrigin,
                                                                   0,
                                                                   colorType,
                                                                   nullptr, nullptr);
            REPORTER_ASSERT(reporter, s);
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }

        {
            sk_sp<SkImage> i = SkImage::MakeFromTexture(context,
                                                        o->backendTexture(),
                                                        kTopLeft_GrSurfaceOrigin,
                                                        colorType,
                                                        kPremul_SkAlphaType,
                                                        nullptr);
            REPORTER_ASSERT(reporter, i);
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());
    o.reset();
}



DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

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

    SkASSERT(kLastEnum_SkColorType+1 == SK_ARRAY_COUNT(colorTypes));

    for (auto colorType : colorTypes) {
        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            auto createMtd = [colorType](GrContext* context, GrRenderable renderable) {
                return GrBackendObject::Make(context, 32, 32, colorType,
                                             GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, colorType, renderable);
        }
    }

}

#include "src/gpu/gl/GrGLDefines.h"

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

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

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return GrBackendObject::Make(context, 32, 32, format,
                                             GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}


#ifdef SK_VULKAN

DEF_GPUTEST_FOR_VULKAN_CONTEXT(VkBackendObjectTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

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
        GrBackendFormat format = GrBackendFormat::MakeVk(combo.fFormat);

        for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return GrBackendObject::Make(context, 32, 32, format,
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

            auto createMtd = [format](GrContext* context, GrRenderable renderable) {
                return GrBackendObject::Make(context, 32, 32, format,
                                             GrMipMapped::kNo, renderable);
            };

            test_wrapping(context, reporter, createMtd, combo.fColorType, renderable);
        }
    }
}

#endif
