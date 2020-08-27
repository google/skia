/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/core/SkSurfaceCharacterization.h"
#include "include/gpu/GrDirectContext.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/gpu/GrContextPriv.h"
#include "src/image/SkImage_Base.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"

#ifdef SK_GL
#include "src/gpu/gl/GrGLCaps.h"
#include "src/gpu/gl/GrGLDefines.h"
#include "src/gpu/gl/GrGLGpu.h"
#include "src/gpu/gl/GrGLUtil.h"
#endif

#ifdef SK_METAL
#include "include/gpu/mtl/GrMtlTypes.h"
#include "src/gpu/mtl/GrMtlCppUtil.h"
#endif

static void wait_on_backend_work_to_finish(GrDirectContext* dContext, bool* finishedCreate) {
    dContext->submit();
    while (finishedCreate && !(*finishedCreate)) {
        dContext->checkAsyncWorkCompletion();
    }
    if (finishedCreate) {
        // The same boolean (pointed to by finishedCreate) is often used multiply and sequentially
        // throughout our tests to create different backend textures.
        // Reset it here so that it can be use to signal a future backend texture's creation
        *finishedCreate = false;
    }
}

static void delete_backend_texture(GrDirectContext* dContext,
                                   const GrBackendTexture& backendTexture,
                                   bool* finishedCreate) {
    wait_on_backend_work_to_finish(dContext, finishedCreate);
    dContext->deleteBackendTexture(backendTexture);
}

static void mark_signaled(void* context) {
    *(bool*)context = true;
}

// Test wrapping of GrBackendObjects in SkSurfaces and SkImages (non-static since used in Mtl test)
void test_wrapping(GrDirectContext* dContext,
                   skiatest::Reporter* reporter,
                   std::function<GrBackendTexture (GrDirectContext*,
                                                   GrMipmapped,
                                                   GrRenderable)> create,
                   GrColorType grColorType,
                   GrMipmapped mipMapped,
                   GrRenderable renderable,
                   bool* finishedBECreate) {
    GrResourceCache* cache = dContext->priv().getResourceCache();

    const int initialCount = cache->getResourceCount();

    GrBackendTexture backendTex = create(dContext, mipMapped, renderable);
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
        delete_backend_texture(dContext, backendTex, finishedBECreate);
        return;
    }

    if (GrRenderable::kYes == renderable && dContext->colorTypeSupportedAsSurface(skColorType)) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(dContext,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  skColorType,
                                                                  nullptr, nullptr);
        if (!surf) {
            ERRORF(reporter, "Couldn't make surface from backendTexture for %s\n",
                   ToolUtils::colortype_name(skColorType));
        } else {
            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(dContext,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      skColorType,
                                                      kPremul_SkAlphaType,
                                                      nullptr);
        if (!img) {
            ERRORF(reporter, "Couldn't make image from backendTexture for %s\n",
                   ToolUtils::colortype_name(skColorType));
        } else {
            SkImage_Base* ib = as_IB(img);

            GrTextureProxy* proxy = ib->peekProxy();
            REPORTER_ASSERT(reporter, proxy);

            REPORTER_ASSERT(reporter, mipMapped == proxy->proxyMipmapped());
            REPORTER_ASSERT(reporter, proxy->isInstantiated());
            REPORTER_ASSERT(reporter, mipMapped == proxy->mipmapped());

            REPORTER_ASSERT(reporter, initialCount+1 == cache->getResourceCount());
        }
    }

    REPORTER_ASSERT(reporter, initialCount == cache->getResourceCount());

    delete_backend_texture(dContext, backendTex, finishedBECreate);
}

static bool isBGRA8(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
#ifdef SK_GL
            return format.asGLFormat() == GrGLFormat::kBGRA8;
#else
            return false;
#endif
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            format.asVkFormat(&vkFormat);
            return vkFormat == VK_FORMAT_B8G8R8A8_UNORM;
#else
            return false;
#endif
        }
        case GrBackendApi::kMetal:
#ifdef SK_METAL
            return GrMtlFormatIsBGRA8(format.asMtlFormat());
#else
            return false;
#endif
        case GrBackendApi::kDirect3D:
#ifdef SK_DIRECT3D
            return false; // TODO
#else
            return false;
#endif
        case GrBackendApi::kDawn:
#ifdef SK_DAWN
            wgpu::TextureFormat dawnFormat;
            format.asDawnFormat(&dawnFormat);
            return dawnFormat == wgpu::TextureFormat::BGRA8Unorm;
#else
            return false;
#endif
        case GrBackendApi::kMock: {
            SkImage::CompressionType compression = format.asMockCompressionType();
            if (compression != SkImage::CompressionType::kNone) {
                return false; // No compressed formats are BGRA
            }

            return format.asMockColorType() == GrColorType::kBGRA_8888;
        }
    }
    SkUNREACHABLE;
}

static bool isRGB(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kOpenGL:
#ifdef SK_GL
            return format.asGLFormat() == GrGLFormat::kRGB8;
#else
            return false;
#endif
        case GrBackendApi::kVulkan: {
#ifdef SK_VULKAN
            VkFormat vkFormat;
            format.asVkFormat(&vkFormat);
            return vkFormat == VK_FORMAT_R8G8B8_UNORM;
#else
            return false;
#endif
        }
        case GrBackendApi::kMetal:
            return false;  // Metal doesn't even pretend to support this
        case GrBackendApi::kDirect3D:
            return false;  // Not supported in Direct3D 12
        case GrBackendApi::kDawn:
            return false;
        case GrBackendApi::kMock:
            return false;  // No GrColorType::kRGB_888
    }
    SkUNREACHABLE;
}

static void check_solid_pixmap(skiatest::Reporter* reporter,
                               const SkColor4f& expected, const SkPixmap& actual,
                               SkColorType ct, const char* label1, const char* label2) {
    // we need 0.001f across the board just for noise
    // we need 0.01f across the board for 1010102
    const float tols[4] = { 0.01f, 0.01f, 0.01f, 0.01f };

    auto error = std::function<ComparePixmapsErrorReporter>(
        [reporter, ct, label1, label2](int x, int y, const float diffs[4]) {
            SkASSERT(x >= 0 && y >= 0);
            ERRORF(reporter, "%s %s %s - mismatch at %d, %d (%f, %f, %f %f)",
                   ToolUtils::colortype_name(ct), label1, label2, x, y,
                   diffs[0], diffs[1], diffs[2], diffs[3]);
        });

    CheckSolidPixels(expected, actual, tols, error);
}

// What would raster do?
static SkColor4f get_expected_color(SkColor4f orig, SkColorType ct) {
    SkAlphaType at = SkColorTypeIsAlwaysOpaque(ct) ? kOpaque_SkAlphaType
                                                   : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::Make(2, 2, ct, at);
    SkAutoPixmapStorage pm;
    pm.alloc(ii);
    pm.erase(orig);
    SkColor tmp = pm.getColor(0, 0);
    return SkColor4f::FromColor(tmp);
}

static void check_mipmaps(GrDirectContext*, const GrBackendTexture&,
                          SkColorType, const SkColor4f expectedColors[6],
                          skiatest::Reporter*, const char* label);

static void check_base_readbacks(GrDirectContext* dContext, const GrBackendTexture& backendTex,
                                 SkColorType skColorType, GrRenderable renderable,
                                 const SkColor4f& color, skiatest::Reporter* reporter,
                                 const char* label) {
    if (isRGB(backendTex.getBackendFormat())) {
        // readPixels is busted for the RGB backend format (skbug.com/8862)
        // TODO: add a GrColorType::kRGB_888 to fix the situation
        return;
    }

    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    SkColor4f expectedColor = get_expected_color(color, skColorType);

    SkAutoPixmapStorage actual;

    {
        SkImageInfo readBackII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                   kUnpremul_SkAlphaType);

        SkAssertResult(actual.tryAlloc(readBackII));
    }

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(dContext,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      skColorType,
                                                      at,
                                                      nullptr);
        if (img) {
            actual.erase(SkColors::kTransparent);
            bool result = img->readPixels(dContext, actual, 0, 0);
            if (!result) {
                // TODO: we need a better way to tell a priori if readPixels will work for an
                // arbitrary colorType
#if 0
                ERRORF(reporter, "Couldn't readback from SkImage for colorType: %d\n", colorType);
#endif
            } else {
                check_solid_pixmap(reporter, expectedColor, actual, skColorType,
                                   label, "SkImage::readPixels");
            }
        }
    }

    // This will mark any mipmaps as dirty (bc that is what we do when we wrap a renderable
    // backend texture) so it must be done last!
    if (GrRenderable::kYes == renderable && dContext->colorTypeSupportedAsSurface(skColorType)) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(dContext,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  skColorType,
                                                                  nullptr, nullptr);
        if (surf) {
            actual.erase(SkColors::kTransparent);
            bool result = surf->readPixels(actual, 0, 0);
            REPORTER_ASSERT(reporter, result);

            check_solid_pixmap(reporter, expectedColor, actual, skColorType,
                               label, "SkSurface::readPixels");
        }
    }
}

// Test initialization of GrBackendObjects to a specific color (non-static since used in Mtl test)
void test_color_init(GrDirectContext* dContext,
                     skiatest::Reporter* reporter,
                     std::function<GrBackendTexture (GrDirectContext*,
                                                     const SkColor4f&,
                                                     GrMipmapped,
                                                     GrRenderable)> create,
                     GrColorType grColorType,
                     const SkColor4f& color,
                     GrMipmapped mipMapped,
                     GrRenderable renderable,
                     bool* finishedBECreate) {
    GrBackendTexture backendTex = create(dContext, color, mipMapped, renderable);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    SkColorType skColorType = GrColorTypeToSkColorType(grColorType);

    // Can't wrap backend textures in images and surfaces w/o an SkColorType
    if (kUnknown_SkColorType == skColorType) {
        // TODO: burrow in and scrappily check that data was uploaded!
        delete_backend_texture(dContext, backendTex, finishedBECreate);
        return;
    }

    auto checkBackendTexture = [&](const SkColor4f& testColor) {
        if (mipMapped == GrMipmapped::kYes) {
            SkColor4f expectedColor = get_expected_color(testColor, skColorType);
            SkColor4f expectedColors[6] = {expectedColor, expectedColor, expectedColor,
                                           expectedColor, expectedColor, expectedColor};
            check_mipmaps(dContext, backendTex, skColorType, expectedColors,
                          reporter, "colorinit");
        }

        // The last step in this test will dirty the mipmaps so do it last
        check_base_readbacks(dContext, backendTex, skColorType, renderable, testColor,
                             reporter, "colorinit");
    };

    checkBackendTexture(color);

    // Make sure the initial create work has finished so we can test the update independently.
    wait_on_backend_work_to_finish(dContext, finishedBECreate);

    SkColor4f newColor = {color.fB , color.fR, color.fG, color.fA };

    dContext->updateBackendTexture(backendTex, skColorType, newColor, mark_signaled,
                                   finishedBECreate);

    checkBackendTexture(newColor);

    delete_backend_texture(dContext, backendTex, finishedBECreate);
}

// Draw the backend texture (wrapped in an SkImage) into an RGBA surface, attempting to access
// all the mipMap levels.
static void check_mipmaps(GrDirectContext* dContext, const GrBackendTexture& backendTex,
                          SkColorType skColorType, const SkColor4f expectedColors[6],
                          skiatest::Reporter* reporter, const char* label) {

#ifdef SK_GL
    // skbug.com/9141 (RGBA_F32 mipmaps appear to be broken on some Mali devices)
    if (GrBackendApi::kOpenGL == dContext->backend()) {
        GrGLGpu* glGPU = static_cast<GrGLGpu*>(dContext->priv().getGpu());

        if (kRGBA_F32_SkColorType == skColorType &&
            kGLES_GrGLStandard == glGPU->ctxInfo().standard()) {
            return;
        }
    }
#endif

    if (isRGB(backendTex.getBackendFormat())) {
        // readPixels is busted for the RGB backend format (skbug.com/8862)
        // TODO: add a GrColorType::kRGB_888 to fix the situation
        return;
    }

    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    sk_sp<SkImage> img = SkImage::MakeFromTexture(dContext,
                                                  backendTex,
                                                  kTopLeft_GrSurfaceOrigin,
                                                  skColorType,
                                                  at,
                                                  nullptr);
    if (!img) {
        return;
    }

    SkImageInfo readbackSurfaceII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType,
                                                      kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(dContext,
                                                        SkBudgeted::kNo,
                                                        readbackSurfaceII, 1,
                                                        kTopLeft_GrSurfaceOrigin,
                                                        nullptr);
    if (!surf) {
        return;
    }

    SkCanvas* canvas = surf->getCanvas();

    SkPaint p;
    p.setFilterQuality(kHigh_SkFilterQuality);

    int numMipLevels = 6;

    for (int i = 0, rectSize = 32; i < numMipLevels; ++i, rectSize /= 2) {
        SkASSERT(rectSize >= 1);

        SkRect r = SkRect::MakeWH(rectSize, rectSize);
        canvas->clear(SK_ColorTRANSPARENT);
        canvas->drawImageRect(img, r, &p);

        SkImageInfo readbackII = SkImageInfo::Make(rectSize, rectSize,
                                                   kRGBA_8888_SkColorType,
                                                   kUnpremul_SkAlphaType);
        SkAutoPixmapStorage actual2;
        SkAssertResult(actual2.tryAlloc(readbackII));
        actual2.erase(SkColors::kTransparent);

        bool result = surf->readPixels(actual2, 0, 0);
        REPORTER_ASSERT(reporter, result);

        SkString str;
        str.appendf("mip-level %d", i);

        check_solid_pixmap(reporter, expectedColors[i], actual2, skColorType,
                           label, str.c_str());
    }
}

static int make_pixmaps(SkColorType skColorType, GrMipmapped mipMapped,
                        const SkColor4f colors[6], SkAutoPixmapStorage pixmaps[6]) {
    int levelSize = 32;
    int numMipLevels = mipMapped == GrMipmapped::kYes ? 6 : 1;
    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;
    for (int level = 0; level < numMipLevels; ++level) {
        SkImageInfo ii = SkImageInfo::Make(levelSize, levelSize, skColorType, at);
        pixmaps[level].alloc(ii);
        pixmaps[level].erase(colors[level]);
        levelSize /= 2;
    }
    return numMipLevels;
}

// Test initialization of GrBackendObjects using SkPixmaps
static void test_pixmap_init(GrDirectContext* dContext,
                             skiatest::Reporter* reporter,
                             std::function<GrBackendTexture (GrDirectContext*,
                                                             const SkPixmap srcData[],
                                                             int numLevels,
                                                             GrRenderable)> create,
                             SkColorType skColorType,
                             GrMipmapped mipMapped,
                             GrRenderable renderable,
                             bool* finishedBECreate) {
    SkAutoPixmapStorage pixmapMem[6];
    SkColor4f colors[6] = {
        { 1.0f, 0.0f, 0.0f, 1.0f }, // R
        { 0.0f, 1.0f, 0.0f, 0.9f }, // G
        { 0.0f, 0.0f, 1.0f, 0.7f }, // B
        { 0.0f, 1.0f, 1.0f, 0.5f }, // C
        { 1.0f, 0.0f, 1.0f, 0.3f }, // M
        { 1.0f, 1.0f, 0.0f, 0.2f }, // Y
    };

    int numMipLevels = make_pixmaps(skColorType, mipMapped, colors, pixmapMem);
    SkASSERT(numMipLevels);

    // TODO: this is tedious. Should we pass in an array of SkBitmaps instead?
    SkPixmap pixmaps[6];
    for (int i = 0; i < numMipLevels; ++i) {
        pixmaps[i].reset(pixmapMem[i].info(), pixmapMem[i].addr(), pixmapMem[i].rowBytes());
    }

    GrBackendTexture backendTex = create(dContext, pixmaps, numMipLevels, renderable);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    if (skColorType == kBGRA_8888_SkColorType && !isBGRA8(backendTex.getBackendFormat())) {
        // When kBGRA is backed by an RGBA something goes wrong in the swizzling
        delete_backend_texture(dContext, backendTex, finishedBECreate);
        return;
    }

    auto checkBackendTexture = [&](SkColor4f colors[6]) {
        if (mipMapped == GrMipmapped::kYes) {
            SkColor4f expectedColors[6] = {
                    get_expected_color(colors[0], skColorType),
                    get_expected_color(colors[1], skColorType),
                    get_expected_color(colors[2], skColorType),
                    get_expected_color(colors[3], skColorType),
                    get_expected_color(colors[4], skColorType),
                    get_expected_color(colors[5], skColorType),
            };

            check_mipmaps(dContext, backendTex, skColorType, expectedColors, reporter, "pixmap");
        }

        // The last step in this test will dirty the mipmaps so do it last
        check_base_readbacks(dContext, backendTex, skColorType, renderable, colors[0], reporter,
                             "pixmap");
    };

    checkBackendTexture(colors);

    // Make sure the initial create work has finished so we can test the update independently.
    wait_on_backend_work_to_finish(dContext, finishedBECreate);

    SkColor4f colorsNew[6] = {
        {1.0f, 1.0f, 0.0f, 0.2f},  // Y
        {1.0f, 0.0f, 0.0f, 1.0f},  // R
        {0.0f, 1.0f, 0.0f, 0.9f},  // G
        {0.0f, 0.0f, 1.0f, 0.7f},  // B
        {0.0f, 1.0f, 1.0f, 0.5f},  // C
        {1.0f, 0.0f, 1.0f, 0.3f},  // M
    };
    make_pixmaps(skColorType, mipMapped, colorsNew, pixmapMem);
    for (int i = 0; i < numMipLevels; ++i) {
        pixmaps[i].reset(pixmapMem[i].info(), pixmapMem[i].addr(), pixmapMem[i].rowBytes());
    }

    // Upload new data and make sure everything still works
    dContext->updateBackendTexture(backendTex, pixmaps, numMipLevels, mark_signaled,
                                   finishedBECreate);

    checkBackendTexture(colorsNew);

    delete_backend_texture(dContext, backendTex, finishedBECreate);
}

enum class VkLayout {
    kUndefined,
    kReadOnlyOptimal,
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
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ColorTypeBackendAllocationTest, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    const GrCaps* caps = context->priv().caps();

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 0.75f };

    struct {
        SkColorType   fColorType;
        SkColor4f     fColor;
    } combinations[] = {
        { kAlpha_8_SkColorType,           kTransCol                },
        { kRGB_565_SkColorType,           SkColors::kRed           },
        { kARGB_4444_SkColorType,         SkColors::kGreen         },
        { kRGBA_8888_SkColorType,         SkColors::kBlue          },
        { kRGB_888x_SkColorType,          SkColors::kCyan          },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { kBGRA_8888_SkColorType,         { 1, 0, 0, 1.0f }        },
        // TODO: readback is busted for *10A2 when alpha = 0.5f (perhaps premul vs. unpremul)
        { kRGBA_1010102_SkColorType,      { 0.25f, 0.5f, 0.75f, 1.0f }},
        { kBGRA_1010102_SkColorType,      { 0.25f, 0.5f, 0.75f, 1.0f }},
        // RGB/BGR 101010x have no Ganesh correlate
        { kRGB_101010x_SkColorType,       { 0, 0.5f, 0, 0.5f }     },
        { kBGR_101010x_SkColorType,       { 0, 0.5f, 0, 0.5f }     },
        { kGray_8_SkColorType,            kGrayCol                 },
        { kRGBA_F16Norm_SkColorType,      SkColors::kLtGray        },
        { kRGBA_F16_SkColorType,          SkColors::kYellow        },
        { kRGBA_F32_SkColorType,          SkColors::kGray          },
        { kR8G8_unorm_SkColorType,        { .25f, .75f, 0, 1 }     },
        { kR16G16_unorm_SkColorType,      SkColors::kGreen         },
        { kA16_unorm_SkColorType,         kTransCol                },
        { kA16_float_SkColorType,         kTransCol                },
        { kR16G16_float_SkColorType,      { .25f, .75f, 0, 1 }     },
        { kR16G16B16A16_unorm_SkColorType,{ .25f, .5f, .75f, 1 }   },
    };

    static_assert(kLastEnum_SkColorType == SK_ARRAY_COUNT(combinations));

    for (auto combo : combinations) {
        SkColorType colorType = combo.fColorType;

        if (GrBackendApi::kMetal == context->backend()) {
            // skbug.com/9086 (Metal caps may not be handling RGBA32 correctly)
            if (kRGBA_F32_SkColorType == combo.fColorType) {
                continue;
            }
        }

        for (auto mipMapped : { GrMipmapped::kNo, GrMipmapped::kYes }) {
            if (GrMipmapped::kYes == mipMapped && !caps->mipmapSupport()) {
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
                    auto uninitCreateMtd = [colorType](GrDirectContext* dContext,
                                                       GrMipmapped mipMapped,
                                                       GrRenderable renderable) {
                        auto result = dContext->createBackendTexture(32, 32, colorType,
                                                                     mipMapped, renderable,
                                                                     GrProtected::kNo);
                        check_vk_layout(result, VkLayout::kUndefined);

#ifdef SK_DEBUG
                        {
                            GrBackendFormat format = dContext->defaultBackendFormat(colorType,
                                                                                    renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif

                        return result;
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
                                  SkColorTypeToGrColorType(colorType), mipMapped, renderable,
                                  nullptr);
                }

                bool finishedBackendCreation = false;
                bool* finishedPtr = &finishedBackendCreation;

                {

                    auto createWithColorMtd = [colorType, finishedPtr](GrDirectContext* dContext,
                                                                       const SkColor4f& color,
                                                                       GrMipmapped mipMapped,
                                                                       GrRenderable renderable) {
                        auto result = dContext->createBackendTexture(32, 32, colorType, color,
                                                                     mipMapped, renderable,
                                                                     GrProtected::kNo,
                                                                     mark_signaled, finishedPtr);
                        check_vk_layout(result, VkLayout::kReadOnlyOptimal);

#ifdef SK_DEBUG
                        {
                            GrBackendFormat format = dContext->defaultBackendFormat(colorType,
                                                                                   renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif

                        return result;
                    };
                    // We make our comparison color using SkPixmap::erase(color) on a pixmap of
                    // combo.fColorType and then calling SkPixmap::readPixels(). erase() will premul
                    // the color passed to it. However, createBackendTexture() that takes a
                    // SkColor4f is color type / alpha type unaware and will simply compute
                    // luminance from the r, g, b, channels.
                    SkColor4f color = combo.fColor;
                    if (colorType == kGray_8_SkColorType) {
                        color = {color.fR * color.fA,
                                 color.fG * color.fA,
                                 color.fB * color.fA,
                                 1.f};
                    }
                    test_color_init(context, reporter, createWithColorMtd,
                                    SkColorTypeToGrColorType(colorType), color, mipMapped,
                                    renderable, finishedPtr);
                }

                {
                    auto createWithSrcDataMtd = [finishedPtr](GrDirectContext* dContext,
                                                              const SkPixmap srcData[],
                                                              int numLevels,
                                                              GrRenderable renderable) {
                        SkASSERT(srcData && numLevels);
                        auto result = dContext->createBackendTexture(srcData, numLevels, renderable,
                                                                     GrProtected::kNo, mark_signaled,
                                                                     finishedPtr);
                        check_vk_layout(result, VkLayout::kReadOnlyOptimal);
#ifdef SK_DEBUG
                        {
                            auto format = dContext->defaultBackendFormat(srcData[0].colorType(),
                                                                         renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif
                        return result;
                    };

                    test_pixmap_init(context, reporter, createWithSrcDataMtd, colorType, mipMapped,
                                     renderable, finishedPtr);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
#ifdef SK_GL

DEF_GPUTEST_FOR_ALL_GL_CONTEXTS(GLBackendAllocationTest, reporter, ctxInfo) {
    sk_gpu_test::GLTestContext* glCtx = ctxInfo.glContext();
    GrGLStandard standard = glCtx->gl()->fStandard;
    auto context = ctxInfo.directContext();
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
        { GrColorType::kRGBA_1010102,     GR_GL_RGB10_A2,             { 0.25f, 0.5f, 0.75f, 1.f }},
        { GrColorType::kBGRA_1010102,     GR_GL_RGB10_A2,             { 0.25f, 0.5f, 0.75f, 1.f }},
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

        { GrColorType::kRGBA_16161616,    GR_GL_RGBA16,               SkColors::kLtGray    },
        { GrColorType::kRG_F16,           GR_GL_RG16F,                SkColors::kYellow    },
    };

    for (auto combo : combinations) {
        for (GrGLenum target : {GR_GL_TEXTURE_2D, GR_GL_TEXTURE_RECTANGLE}) {
            GrBackendFormat format = GrBackendFormat::MakeGL(combo.fFormat, target);

            if (!glCaps->isFormatTexturable(format)) {
                continue;
            }

            if (GrColorType::kBGRA_8888 == combo.fColorType ||
                GrColorType::kBGRA_1010102 == combo.fColorType) {
                // We allow using a GL_RGBA8 or GR_GL_RGB10_A2 texture as BGRA on desktop GL but not
                // ES
                if (kGL_GrGLStandard != standard &&
                    (GR_GL_RGBA8 == combo.fFormat || GR_GL_RGB10_A2 == combo.fFormat)) {
                    continue;
                }
            }

            for (auto mipMapped : {GrMipmapped::kNo, GrMipmapped::kYes}) {
                if (GrMipmapped::kYes == mipMapped &&
                    (!glCaps->mipmapSupport() || target == GR_GL_TEXTURE_RECTANGLE)) {
                    continue;
                }

                for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
                    if (GrRenderable::kYes == renderable) {
                        if (!glCaps->isFormatAsColorTypeRenderable(combo.fColorType, format)) {
                            continue;
                        }
                    }

                    {
                        auto uninitCreateMtd = [format](GrDirectContext* dContext,
                                                        GrMipmapped mipMapped,
                                                        GrRenderable renderable) {
                            return dContext->createBackendTexture(32, 32, format, mipMapped,
                                                                  renderable, GrProtected::kNo);
                        };

                        test_wrapping(context, reporter, uninitCreateMtd, combo.fColorType,
                                      mipMapped, renderable, nullptr);
                    }

                    {
                        // We're creating backend textures without specifying a color type "view" of
                        // them at the public API level. Therefore, Ganesh will not apply any
                        // swizzles before writing the color to the texture. However, our validation
                        // code does rely on interpreting the texture contents via a SkColorType and
                        // therefore swizzles may be applied during the read step. Ideally we'd
                        // update our validation code to use a "raw" read that doesn't impose a
                        // color type but for now we just munge the data we upload to match the
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
                            return dContext->createBackendTexture(
                                    32, 32, format, swizzledColor, mipMapped, renderable,
                                    GrProtected::kNo, mark_signaled, finishedPtr);
                        };
                        // We make our comparison color using SkPixmap::erase(color) on a pixmap of
                        // combo.fColorType and then calling SkPixmap::readPixels(). erase() will
                        // premul the color passed to it. However, createBackendTexture() that takes
                        // a SkColor4f is color type/alpha type unaware and will simply compute
                        // luminance from the r, g, b, channels.
                        SkColor4f color = combo.fColor;
                        if (combo.fColorType == GrColorType::kGray_8) {
                            color = {color.fR * color.fA,
                                     color.fG * color.fA,
                                     color.fB * color.fA,
                                     1.f};
                        }

                        test_color_init(context, reporter, createWithColorMtd, combo.fColorType,
                                        color, mipMapped, renderable, finishedPtr);
                    }
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
    auto context = ctxInfo.directContext();
    const GrVkCaps* vkCaps = static_cast<const GrVkCaps*>(context->priv().caps());

    constexpr SkColor4f kTransCol { 0, 0.25f, 0.75f, 0.5f };
    constexpr SkColor4f kGrayCol { 0.75f, 0.75f, 0.75f, 1 };

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

        { GrColorType::kRGBA_1010102,     VK_FORMAT_A2B10G10R10_UNORM_PACK32,
                                                                      { 0.25f, 0.5f, 0.75f, 1.0f }},
        { GrColorType::kBGRA_1010102,     VK_FORMAT_A2R10G10B10_UNORM_PACK32,
                                                                      { 0.25f, 0.5f, 0.75f, 1.0f }},
        { GrColorType::kBGR_565,          VK_FORMAT_R5G6B5_UNORM_PACK16,      SkColors::kRed      },

        { GrColorType::kABGR_4444,        VK_FORMAT_R4G4B4A4_UNORM_PACK16,    SkColors::kCyan     },
        { GrColorType::kABGR_4444,        VK_FORMAT_B4G4R4A4_UNORM_PACK16,    SkColors::kYellow   },

        { GrColorType::kAlpha_8,          VK_FORMAT_R8_UNORM,                 kTransCol           },
        // In this config (i.e., a Gray8 color type with an R8 backing format), there is nothing
        // to tell Skia this isn't an Alpha8 color type (so it will initialize the texture with
        // the alpha channel of the color). Clients should, in general, fill all the channels
        // of the provided color with the same value in such cases.
        { GrColorType::kGray_8,           VK_FORMAT_R8_UNORM,                 kGrayCol            },

        { GrColorType::kRGBA_F16_Clamped, VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kLtGray   },
        { GrColorType::kRGBA_F16,         VK_FORMAT_R16G16B16A16_SFLOAT,      SkColors::kYellow   },

        { GrColorType::kRG_88,            VK_FORMAT_R8G8_UNORM,               { 1, 0.5f, 0, 1 }   },
        { GrColorType::kAlpha_F16,        VK_FORMAT_R16_SFLOAT,               { 1.0f, 0, 0, 0.5f }},

        { GrColorType::kAlpha_16,         VK_FORMAT_R16_UNORM,                kTransCol           },
        { GrColorType::kRG_1616,          VK_FORMAT_R16G16_UNORM,             SkColors::kYellow   },
        { GrColorType::kRGBA_16161616,    VK_FORMAT_R16G16B16A16_UNORM,       SkColors::kLtGray   },
        { GrColorType::kRG_F16,           VK_FORMAT_R16G16_SFLOAT,            SkColors::kYellow   },
    };

    for (auto combo : combinations) {
        if (!vkCaps->isVkFormatTexturable(combo.fFormat)) {
            continue;
        }

        GrBackendFormat format = GrBackendFormat::MakeVk(combo.fFormat);

        for (auto mipMapped : { GrMipmapped::kNo, GrMipmapped::kYes }) {
            if (GrMipmapped::kYes == mipMapped && !vkCaps->mipmapSupport()) {
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

                {
                    auto uninitCreateMtd = [format](GrDirectContext* dContext,
                                                    GrMipmapped mipMapped,
                                                    GrRenderable renderable) {
                        GrBackendTexture beTex = dContext->createBackendTexture(32, 32, format,
                                                                                mipMapped,
                                                                                renderable,
                                                                                GrProtected::kNo);
                        check_vk_layout(beTex, VkLayout::kUndefined);
                        return beTex;
                    };

                    test_wrapping(context, reporter, uninitCreateMtd,
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

                    bool finishedBackendCreation = false;
                    bool* finishedPtr = &finishedBackendCreation;

                    auto createWithColorMtd = [format, swizzle, finishedPtr](
                            GrDirectContext* dContext,
                            const SkColor4f& color,
                            GrMipmapped mipMapped,
                            GrRenderable renderable) {
                        auto swizzledColor = swizzle.applyTo(color);
                        GrBackendTexture beTex = dContext->createBackendTexture(32, 32, format,
                                                                                swizzledColor,
                                                                                mipMapped,
                                                                                renderable,
                                                                                GrProtected::kNo,
                                                                                mark_signaled,
                                                                                finishedPtr);
                        check_vk_layout(beTex, VkLayout::kReadOnlyOptimal);
                        return beTex;
                    };
                    test_color_init(context, reporter, createWithColorMtd,
                                    combo.fColorType, combo.fColor, mipMapped, renderable,
                                    finishedPtr);
                }
            }
        }
    }
}

#endif
