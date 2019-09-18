/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLUtil.h"
#endif

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages
void test_wrapping(GrContext* context, skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrContext*,
                                                   GrMipMapped,
                                                   GrRenderable)> create,
                   GrColorType grColorType, GrMipMapped mipMapped, GrRenderable renderable) {
    GrResourceCache* cache = context->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    GrBackendTexture backendTex = create(context, mipMapped, renderable);
    if (!backendTex.isValid()) {
        ERRORF(reporter, "Couldn't create backendTexture for grColorType %d renderable %s\n",
               grColorType,
               GrRenderable::kYes == renderable ? "yes" : "no");
        return;
    }

    // Skia proper should know nothing about the new backend object
    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    SkColorType skColorType = GrColorTypeToSkColorType(grColorType);

    // Wrapping a backendTexture in an image requires an SkColorType
    if (kUnknown_SkColorType == skColorType) {
        context->deleteBackendTexture(backendTex);
        return;
    }

    if (GrRenderable::kYes == renderable && context->colorTypeSupportedAsSurface(skColorType)) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(context,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  skColorType,
                                                                  nullptr, nullptr);
        if (!surf) {
            ERRORF(reporter, "Couldn't make surface from backendTexture for colorType %d\n",
                   skColorType);
        } else {
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      skColorType,
                                                      kPremul_SkAlphaType,
                                                      nullptr);
        if (!img) {
            ERRORF(reporter, "Couldn't make image from backendTexture for skColorType %d\n",
                   skColorType);
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

    context->deleteBackendTexture(backendTex);
}

static void check_solid_pixmap(skiatest::Reporter* reporter,
                               const SkColor4f& expected, const SkPixmap& actual,
                               SkColorType ct, const char* label) {
    // we need 0.001f across the board just for noise
    // we need 0.01f across the board for 1010102
    const float tols[4] = {0.01f, 0.01f, 0.01f, 0.01f};

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, ct, label](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%s %s - mismatch at %d, %d (%f, %f, %f %f)",
                   ToolUtils::colortype_name(ct), label, x, y,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    check_solid_pixels(expected, actual, tols, error);
}

static SkColor4f get_expected_color(SkColor4f orig, SkColorType ct) {

    uint32_t components = SkColorTypeComponentFlags(ct);

    if (components & kGray_SkColorTypeComponentFlag) {
        // For the GPU backends, gray implies a single channel which is opaque.
        return { orig.fA, orig.fA, orig.fA, 1 };
    }

    float r = orig.fR, g = orig.fG, b = orig.fB, a = orig.fA;

    // Missing channels are set to 0
    if (!(components & kRed_SkColorTypeComponentFlag)) {
        r = 0;
    }
    if (!(components & kGreen_SkColorTypeComponentFlag)) {
        g = 0;
    }
    if (!(components & kBlue_SkColorTypeComponentFlag)) {
        b = 0;
    }
    // except for missing alpha - which gets set to 1
    if (!(components & kAlpha_SkColorTypeComponentFlag)) {
        a = 1;
    }

    return { r, g, b, a };
}

// Test initialization of GrBackendObjects to a specific color
void test_color_init(GrContext* context, skiatest::Reporter* reporter,
                     std::function<GrBackendTexture (GrContext*,
                                                     const SkColor4f&,
                                                     GrMipMapped,
                                                     GrRenderable)> create,
                     GrColorType grColorType, const SkColor4f& color,
                     GrMipMapped mipMapped, GrRenderable renderable) {
    GrBackendTexture backendTex = create(context, color, mipMapped, renderable);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    SkColorType skColorType = GrColorTypeToSkColorType(grColorType);

    // Can't wrap backend textures in images and surfaces w/o an SkColorType
    if (kUnknown_SkColorType == skColorType) {
        // TODO: burrow in and scrappily check that data was uploaded!
        context->deleteBackendTexture(backendTex);
        return;
    }

    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::Make(32, 32, skColorType, at);

    SkColor4f expectedColor = get_expected_color(color, skColorType);

    SkAutoPixmapStorage actual;
    SkAssertResult(actual.tryAlloc(ii));
    actual.erase(SkColors::kTransparent);

    if (GrRenderable::kYes == renderable && context->colorTypeSupportedAsSurface(skColorType)) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(context,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  skColorType,
                                                                  nullptr, nullptr);
        if (surf) {
            bool result = surf->readPixels(actual, 0, 0);
            REPORTER_ASSERT(reporter, result);

            check_solid_pixmap(reporter, expectedColor, actual, skColorType,
                               "SkSurface::readPixels");

            actual.erase(SkColors::kTransparent);
        }
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      skColorType,
                                                      at,
                                                      nullptr);
        if (img) {
            // If possible, read back the pixels and check that they're correct
            {
                bool result = img->readPixels(actual, 0, 0);
                if (!result) {
                    // TODO: we need a better way to tell a priori if readPixels will work for an
                    // arbitrary colorType
#if 0
                    ERRORF(reporter, "Couldn't readback from SkImage for colorType: %d\n",
                            colorType);
#endif
                } else {
                    check_solid_pixmap(reporter, expectedColor, actual, skColorType,
                                       "SkImage::readPixels");
                }
            }

            // Draw the wrapped image into an RGBA surface attempting to access all the
            // mipMap levels.
            {
#ifdef SK_GL
                // skbug.com/9141 (RGBA_F32 mipmaps appear to be broken on some Mali devices)
                if (GrBackendApi::kOpenGL == context->backend()) {
                    GrGLGpu* glGPU = static_cast<GrGLGpu*>(context->priv().getGpu());

                    if (kRGBA_F32_SkColorType == skColorType && GrMipMapped::kYes == mipMapped &&
                        kGLES_GrGLStandard == glGPU->ctxInfo().standard()) {
                        context->deleteBackendTexture(backendTex);
                        return;
                    }
                }
#endif

                SkImageInfo newII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);

                sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context,
                                                                    SkBudgeted::kNo,
                                                                    newII, 1,
                                                                    kTopLeft_GrSurfaceOrigin,
                                                                    nullptr);
                if (!surf) {
                    context->deleteBackendTexture(backendTex);
                    return;
                }

                SkCanvas* canvas = surf->getCanvas();

                SkPaint p;
                p.setFilterQuality(kHigh_SkFilterQuality);

                int numMipLevels = (GrMipMapped::kYes == mipMapped) ? 6 : 1;

                for (int i = 0, rectSize = 32; i < numMipLevels; ++i, rectSize /= 2) {
                    SkASSERT(rectSize >= 1);

                    SkRect r = SkRect::MakeWH(rectSize, rectSize);
                    canvas->clear(SK_ColorTRANSPARENT);
                    canvas->drawImageRect(img, r, &p);

                    SkImageInfo readbackII = SkImageInfo::Make(rectSize, rectSize,
                                                               kRGBA_8888_SkColorType,
                                                               kPremul_SkAlphaType);
                    SkAutoPixmapStorage actual2;
                    SkAssertResult(actual2.tryAlloc(readbackII));
                    actual2.erase(SkColors::kTransparent);

                    bool result = surf->readPixels(actual2, 0, 0);
                    REPORTER_ASSERT(reporter, result);

                    check_solid_pixmap(reporter, expectedColor, actual2, skColorType,
                                       "mip-level failure");
                }
            }
        }
    }

    context->deleteBackendTexture(backendTex);
}

enum class VkLayout {
    kUndefined,
    kReadOnlyOptimal,
    kColorAttachmentOptimal
};

void check_vk_layout(const GrBackendTexture& backendTex, VkLayout layout) {
#if defined(SK_VULKAN) && defined(SK_DEBUG)
    VkImageLayout expected;

    switch (layout) {
        case VkLayout::kUndefined:
            expected = VK_IMAGE_LAYOUT_UNDEFINED;
            break;
        case VkLayout::kReadOnlyOptimal:
            expected = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
        case VkLayout::kColorAttachmentOptimal:
            expected = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
        default:
            SkUNREACHABLE;
    }

    GrVkImageInfo vkII;

    if (backendTex.getVkImageInfo(&vkII)) {
        SkASSERT(expected == vkII.fImageLayout);
        SkASSERT(VK_IMAGE_TILING_OPTIMAL == vkII.fImageTiling);
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// This test is a bit different from the others in this file. It is mainly checking that, for any
// SkSurface we can create in Ganesh, we can also create a backend texture that is compatible with
// its characterization and then create a new surface that wraps that backend texture.
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(CharacterizationBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();

    for (int ct = 0; ct <= kLastEnum_SkColorType; ++ct) {
        SkColorType colorType = static_cast<SkColorType>(ct);

        SkImageInfo ii = SkImageInfo::Make(32, 32, colorType, kPremul_SkAlphaType);

        for (auto origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin } ) {
            for (bool mipMaps : { true, false } ) {
                for (int sampleCount : {1, 2}) {
                    SkSurfaceCharacterization c;

                    // Get a characterization, if possible
                    {
                        sk_sp<SkSurface> s = SkSurface::MakeRenderTarget(context, SkBudgeted::kNo,
                                                                         ii, sampleCount,
                                                                         origin, nullptr, mipMaps);
                        if (!s) {
                            continue;
                        }

                        if (!s->characterize(&c)) {
                            continue;
                        }

                        REPORTER_ASSERT(reporter, s->isCompatible(c));
                    }

                    // Test out uninitialized path
                    {
                        GrBackendTexture backendTex = context->createBackendTexture(c);
                        check_vk_layout(backendTex, VkLayout::kUndefined);
                        REPORTER_ASSERT(reporter, backendTex.isValid());
                        REPORTER_ASSERT(reporter, c.isCompatible(backendTex));

                        {
                            GrBackendFormat format = context->defaultBackendFormat(
                                                                    c.imageInfo().colorType(),
                                                                    GrRenderable::kYes);
                            REPORTER_ASSERT(reporter, format == backendTex.getBackendFormat());
                        }

                        sk_sp<SkSurface> s2 = SkSurface::MakeFromBackendTexture(context, c,
                                                                                backendTex);
                        REPORTER_ASSERT(reporter, s2);
                        REPORTER_ASSERT(reporter, s2->isCompatible(c));

                        s2 = nullptr;
                        context->deleteBackendTexture(backendTex);
                    }

                    // Test out color-initialized path
                    {
                        GrBackendTexture backendTex = context->createBackendTexture(c,
                                                                                    SkColors::kRed);
                        check_vk_layout(backendTex, VkLayout::kColorAttachmentOptimal);
                        REPORTER_ASSERT(reporter, backendTex.isValid());
                        REPORTER_ASSERT(reporter, c.isCompatible(backendTex));

                        {
                            GrBackendFormat format = context->defaultBackendFormat(
                                                                    c.imageInfo().colorType(),
                                                                    GrRenderable::kYes);
                            REPORTER_ASSERT(reporter, format == backendTex.getBackendFormat());
                        }

                        sk_sp<SkSurface> s2 = SkSurface::MakeFromBackendTexture(context, c,
                                                                                backendTex);
                        REPORTER_ASSERT(reporter, s2);
                        REPORTER_ASSERT(reporter, s2->isCompatible(c));

                        s2 = nullptr;
                        context->deleteBackendTexture(backendTex);
                    }
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendAllocationTest, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    const GrCaps* caps = context->priv().caps();

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 0.75f };

    struct {
        SkColorType   fColorType;
        SkColor4f     fColor;
    } combinations[] = {
        { kAlpha_8_SkColorType,      kTransCol                },
        { kRGB_565_SkColorType,      SkColors::kRed           },
        { kARGB_4444_SkColorType,    SkColors::kGreen         },
        { kRGBA_8888_SkColorType,    SkColors::kBlue          },
        { kRGB_888x_SkColorType,     SkColors::kCyan          },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { kBGRA_8888_SkColorType,    { 1, 0, 0, 1.0f }        },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { kRGBA_1010102_SkColorType, { .25f, .5f, .75f, 1.0f }},
        // The kRGB_101010x_SkColorType has no Ganesh correlate
        { kRGB_101010x_SkColorType,  { 0, 0.5f, 0, 0.5f }     },
        { kGray_8_SkColorType,       kGrayCol                 },
        { kRGBA_F16Norm_SkColorType, SkColors::kLtGray        },
        { kRGBA_F16_SkColorType,     SkColors::kYellow        },
        { kRGBA_F32_SkColorType,     SkColors::kGray          },
        { kRG_88_SkColorType,        { .25f, .75f, 0, 0 }     },
        { kRG_1616_SkColorType,      SkColors::kGreen         },
        { kAlpha_16_SkColorType,     kTransCol                },
        { kAlpha_F16_SkColorType,    kTransCol                },
        { kRG_F16_SkColorType,       { .25f, .75f, 0, 0 }     },
        { kRGBA_16161616_SkColorType,{ .25f, .5f, .75f, 1 }   },
    };

    GR_STATIC_ASSERT(kLastEnum_SkColorType == SK_ARRAY_COUNT(combinations));

    for (auto combo : combinations) {
        SkColorType colorType = combo.fColorType;

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
                if (!caps->getDefaultBackendFormat(SkColorTypeToGrColorType(colorType),
                                                   renderable).isValid()) {
                    continue;
                }
                if (GrRenderable::kYes == renderable) {
                    if (kRGB_888x_SkColorType == combo.fColorType) {
                        // Ganesh can't perform the blends correctly when rendering this format
                        continue;
                    }
                }

                {
                    auto uninitCreateMtd = [colorType](GrContext* context,
                                                       GrMipMapped mipMapped,
                                                       GrRenderable renderable) {
                        auto result = context->createBackendTexture(32, 32, colorType,
                                                                    mipMapped, renderable,
                                                                    GrProtected::kNo);
                        check_vk_layout(result, VkLayout::kUndefined);

#ifdef SK_DEBUG
                        {
                            GrBackendFormat format = context->defaultBackendFormat(colorType,
                                                                                   renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif

                        return result;
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  SkColorTypeToGrColorType(colorType), mipMapped, renderable);
                }

                {
                    // GL has difficulties reading back from these combinations. In particular,
                    // reading back kGray_8 is a mess.
                    if (GrBackendApi::kOpenGL == context->backend()) {
                        if (kAlpha_8_SkColorType == combo.fColorType ||
                            kGray_8_SkColorType == combo.fColorType) {
                            continue;
                        }
                    } else if (GrBackendApi::kMetal == context->backend()) {
                        // Not yet implemented for Metal
                        continue;
                    }

                    auto createWithColorMtd = [colorType](GrContext* context,
                                                          const SkColor4f& color,
                                                          GrMipMapped mipMapped,
                                                          GrRenderable renderable) {
                        auto result = context->createBackendTexture(32, 32, colorType, color,
                                                                    mipMapped, renderable,
                                                                    GrProtected::kNo);
                        check_vk_layout(result, GrRenderable::kYes == renderable
                                                        ? VkLayout::kColorAttachmentOptimal
                                                        : VkLayout::kReadOnlyOptimal);

#ifdef SK_DEBUG
                        {
                            GrBackendFormat format = context->defaultBackendFormat(colorType,
                                                                                   renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif

                        return result;
                    };

                    test_color_init(context, reporter, createWithColorMtd,
                                    SkColorTypeToGrColorType(colorType),
                                    combo.fColor, mipMapped, renderable);
                }
            }
        }
    }

}

///////////////////////////////////////////////////////////////////////////////
#ifdef SK_GL

#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLUtil.h"

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLBackendAllocationTest, reporter, ctxInfo) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    GrGLStandard standard = glCtx->gl()->fStandard;
    GrContext* context = ctxInfo.grContext();
    const GrGLCaps* glCaps = static_cast<const GrGLCaps*>(context->priv().caps());

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 0.75f };

    struct {
        GrColorType   fColorType;
        GrGLenum      fFormat;
        SkColor4f     fColor;
    } combinations[] = {
        { GrColorType::kRGBA_8888,        GR_GL_RGBA8,                SkColors::kRed       },
        { GrColorType::kRGBA_8888_SRGB,   GR_GL_SRGB8_ALPHA8,         SkColors::kRed       },

        { GrColorType::kRGB_888x,         GR_GL_RGBA8,                SkColors::kYellow    },
        { GrColorType::kRGB_888x,         GR_GL_RGB8,                 SkColors::kCyan      },

        { GrColorType::kBGRA_8888,        GR_GL_RGBA8,                SkColors::kBlue      },
        { GrColorType::kBGRA_8888,        GR_GL_BGRA8,                SkColors::kBlue      },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { GrColorType::kRGBA_1010102,     GR_GL_RGB10_A2,             { 0.5f, 0, 0, 1.0f } },
        { GrColorType::kBGR_565,          GR_GL_RGB565,               SkColors::kRed       },
        { GrColorType::kABGR_4444,        GR_GL_RGBA4,                SkColors::kGreen     },

        { GrColorType::kAlpha_8,          GR_GL_ALPHA8,               kTransCol            },
        { GrColorType::kAlpha_8,          GR_GL_R8,                   kTransCol            },

        { GrColorType::kGray_8,           GR_GL_LUMINANCE8,           kGrayCol             },
        { GrColorType::kGray_8,           GR_GL_R8,                   kGrayCol             },

        { GrColorType::kRGBA_F32,         GR_GL_RGBA32F,              SkColors::kRed       },

        { GrColorType::kRGBA_F16_Clamped, GR_GL_RGBA16F,              SkColors::kLtGray    },
        { GrColorType::kRGBA_F16,         GR_GL_RGBA16F,              SkColors::kYellow    },

        { GrColorType::kRG_88,            GR_GL_RG8,                  { 1, 0.5f, 0, 1 }    },
        { GrColorType::kAlpha_F16,        GR_GL_R16F,                 { 1.0f, 0, 0, 0.5f } },
        { GrColorType::kAlpha_F16,        GR_GL_LUMINANCE16F,         kGrayCol             },

        { GrColorType::kAlpha_16,         GR_GL_R16,                  kTransCol            },
        { GrColorType::kRG_1616,          GR_GL_RG16,                 SkColors::kYellow    },

        // Experimental (for Y416 and mutant P016/P010)
        { GrColorType::kRGBA_16161616,    GR_GL_RGBA16,               SkColors::kLtGray    },
        { GrColorType::kRG_F16,           GR_GL_RG16F,                SkColors::kYellow    },

        { GrColorType::kUnknown,          GR_GL_COMPRESSED_RGB8_ETC2, SkColors::kRed      },
        { GrColorType::kUnknown,          GR_GL_COMPRESSED_ETC1_RGB8, SkColors::kRed      },
    };

    for (auto combo : combinations) {
        GrBackendFormat format = GrBackendFormat::MakeGL(combo.fFormat, GR_GL_TEXTURE_2D);

        if (!glCaps->isFormatTexturable(format)) {
            continue;
        }

        if (GrColorType::kBGRA_8888 == combo.fColorType) {
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
                    if (!glCaps->isFormatAsColorTypeRenderable(combo.fColorType, format)) {
                        continue;
                    }
                }


                // We current disallow uninitialized compressed textures in the GL backend
                if (GR_GL_COMPRESSED_RGB8_ETC2 != combo.fFormat &&
                    GR_GL_COMPRESSED_ETC1_RGB8 != combo.fFormat) {
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

                {
                    // GL has difficulties reading back from these combinations
                    if (GrColorType::kAlpha_8 == combo.fColorType) {
                        continue;
                    }
                    if (GrRenderable::kYes != renderable) {
                        continue;
                    }

                    auto createWithColorMtd = [format](GrContext* context,
                                                       const SkColor4f& color,
                                                       GrMipMapped mipMapped,
                                                       GrRenderable renderable) {
                        return context->createBackendTexture(32, 32, format, color,
                                                             mipMapped, renderable,
                                                             GrProtected::kNo);
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

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 0.75f };

    struct {
        GrColorType fColorType;
        VkFormat    fFormat;
        SkColor4f   fColor;
    } combinations[] = {
        { GrColorType::kRGBA_8888,        VK_FORMAT_R8G8B8A8_UNORM,           SkColors::kRed      },
        { GrColorType::kRGBA_8888_SRGB,   VK_FORMAT_R8G8B8A8_SRGB,            SkColors::kRed      },

        // In this configuration (i.e., an RGB_888x colortype with an RGBA8 backing format),
        // there is nothing to tell Skia to make the provided color opaque. Clients will need
        // to provide an opaque initialization color in this case.
        { GrColorType::kRGB_888x,         VK_FORMAT_R8G8B8A8_UNORM,           SkColors::kYellow   },
        { GrColorType::kRGB_888x,         VK_FORMAT_R8G8B8_UNORM,             SkColors::kCyan     },

        { GrColorType::kBGRA_8888,        VK_FORMAT_B8G8R8A8_UNORM,           SkColors::kBlue     },

        { GrColorType::kRGBA_1010102,     VK_FORMAT_A2B10G10R10_UNORM_PACK32, { 0.5f, 0, 0, 1.0f }},
        { GrColorType::kBGR_565,          VK_FORMAT_R5G6B5_UNORM_PACK16,      SkColors::kRed      },

        { GrColorType::kABGR_4444,        VK_FORMAT_R4G4B4A4_UNORM_PACK16,    SkColors::kCyan     },
        { GrColorType::kABGR_4444,        VK_FORMAT_B4G4R4A4_UNORM_PACK16,    SkColors::kYellow   },

        { GrColorType::kAlpha_8,          VK_FORMAT_R8_UNORM,                 kTransCol           },
        // In this config (i.e., a Gray8 color type with an R8 backing format), there is nothing
        // to tell Skia this isn't an Alpha8 color type (so it will initialize the texture with
        // the alpha channel of the color). Clients should, in general, fill all the channels
        // of the provided color with the same value in such cases.
        { GrColorType::kGray_8,           VK_FORMAT_R8_UNORM,                 kGrayCol            },

        { GrColorType::kRGBA_F32,         VK_FORMAT_R32G32B32A32_SFLOAT,      SkColors::kRed      },

        { GrColorType::kRGBA_F16_Clamped, VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kLtGray   },
        { GrColorType::kRGBA_F16,         VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kYellow   },

        // These backend formats don't have SkColorType equivalents
        { GrColorType::kRG_88,            VK_FORMAT_R8G8_UNORM,               { 1, 0.5f, 0, 1 }   },
        { GrColorType::kAlpha_F16,        VK_FORMAT_R16_SFLOAT,               { 1.0f, 0, 0, 0.5f }},

        { GrColorType::kAlpha_16,         VK_FORMAT_R16_UNORM,                kTransCol           },
        { GrColorType::kRG_1616,          VK_FORMAT_R16G16_UNORM,             SkColors::kYellow   },

        // Experimental (for Y416 and mutant P016/P010)
        { GrColorType::kRGBA_16161616,    VK_FORMAT_R16G16B16A16_UNORM,       SkColors::kLtGray   },
        { GrColorType::kRG_F16,           VK_FORMAT_R16G16_SFLOAT,            SkColors::kYellow   },

        { GrColorType::kUnknown,          VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,  SkColors::kRed      },
    };

    for (auto combo : combinations) {
        if (!vkCaps->isVkFormatTexturable(combo.fFormat)) {
            continue;
        }

        GrBackendFormat format = GrBackendFormat::MakeVk(combo.fFormat);

        for (auto mipMapped : { GrMipMapped::kNo, GrMipMapped::kYes }) {
            if (GrMipMapped::kYes == mipMapped && !vkCaps->mipMapSupport()) {
                continue;
            }

            for (auto renderable : { GrRenderable::kNo, GrRenderable::kYes }) {

                if (GrRenderable::kYes == renderable) {
                    // We must also check whether we allow rendering to the format using the
                    // color type.
                    if (!vkCaps->isFormatAsColorTypeRenderable(
                            combo.fColorType, GrBackendFormat::MakeVk(combo.fFormat), 1)) {
                        continue;
                    }
                }

                // We current disallow uninitialized compressed textures in the Vulkan backend
                if (combo.fFormat != VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK) {
                    auto uninitCreateMtd = [format](GrContext* context,
                                                    GrMipMapped mipMapped,
                                                    GrRenderable renderable) {
                        GrBackendTexture beTex = context->createBackendTexture(32, 32, format,
                                                                               mipMapped,
                                                                               renderable,
                                                                               GrProtected::kNo);
                        check_vk_layout(beTex, VkLayout::kUndefined);
                        return beTex;
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  combo.fColorType, mipMapped, renderable);
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
                            SkASSERT(combo.fFormat == VK_FORMAT_R8_UNORM);
                            swizzle = GrSwizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_16:
                            SkASSERT(combo.fFormat == VK_FORMAT_R16_UNORM);
                            swizzle = GrSwizzle("aaaa");
                            break;
                        case GrColorType::kAlpha_F16:
                            SkASSERT(combo.fFormat == VK_FORMAT_R16_SFLOAT);
                            swizzle = GrSwizzle("aaaa");
                            break;
                        case GrColorType::kABGR_4444:
                            if (combo.fFormat == VK_FORMAT_B4G4R4A4_UNORM_PACK16) {
                                swizzle = GrSwizzle("bgra");
                            }
                            break;
                        default:
                            swizzle = GrSwizzle("rgba");
                            break;
                    }
                    auto createWithColorMtd = [format, swizzle](GrContext* context,
                                                                const SkColor4f& color,
                                                                GrMipMapped mipMapped,
                                                                GrRenderable renderable) {
                        auto swizzledColor = swizzle.applyTo(color);
                        GrBackendTexture beTex = context->createBackendTexture(32, 32, format,
                                                                               swizzledColor,
                                                                               mipMapped,
                                                                               renderable,
                                                                               GrProtected::kNo);
                        check_vk_layout(beTex, GrRenderable::kYes == renderable
                                                        ? VkLayout::kColorAttachmentOptimal
                                                        : VkLayout::kReadOnlyOptimal);
                        return beTex;
                    };
                    test_color_init(context, reporter, createWithColorMtd,
                                    combo.fColorType, combo.fColor, mipMapped, renderable);
                }
            }
        }
    }
}

#endif
