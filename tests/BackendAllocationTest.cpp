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

    if (GrRenderable::kYes == renderable) {
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

static bool colors_eq(SkColor colA, SkColor colB, int tol) {
    int maxDiff = 0;
    for (int i = 0; i < 4; ++i) {
        int diff = SkTAbs<int>((0xFF & (colA >> i*8)) - (0xFF & (colB >> i*8)));
        if (maxDiff < diff) {
            maxDiff = diff;
        }
    }

    return maxDiff <= tol;
}

static void check_solid_pixmap(SkColor expectedColor, const SkPixmap& actual,
                               SkColorType colorType, skiatest::Reporter* reporter) {
    for (int y = 0; y < actual.height(); ++y) {
        for (int x = 0; x < actual.width(); ++x) {

            SkColor actualCol = actual.getColor(x, y);

            if (colors_eq(actualCol, expectedColor, 1)) {
                continue;
            }

            ERRORF(reporter,
                   "Mismatched pixels at %d %d ct: %s expected: 0x%x actual: 0x%x\n",
                   x, y, SkColorTypeToStr(colorType), expectedColor, actualCol);
            return;
        }
    }
}

static bool isBGRA(const GrBackendFormat& format) {
    switch (format.backend()) {
#ifdef SK_METAL
    case GrBackendApi::kMetal:  return format.asMtlFormat() == MTLPixelFormatBGRA8Unorm;
#endif
#ifdef SK_DAWN
    case GrBackendApi::kDawn:   return false;
#endif
#ifdef SK_GL
    case GrBackendApi::kOpenGL: return format.asGLFormat() == GrGLFormat::kBGRA8;
#endif
#ifdef SK_VULKAN
    case GrBackendApi::kVulkan: {
        VkFormat vkFormat;
        format.asVkFormat(&vkFormat);
        return vkFormat == VK_FORMAT_B8G8R8A8_UNORM;
    }
#endif
    case GrBackendApi::kMock:   return format.asMockColorType() == GrColorType::kBGRA_8888;
    }

    SkUNREACHABLE;
}

static bool isRGB(const GrBackendFormat& format) {
    switch (format.backend()) {
        case GrBackendApi::kMetal:  return false;  // Metal doesn't even pretent to support this
#ifdef SK_DAWN
        case GrBackendApi::kDawn:   return false;
#endif
#ifdef SK_GL
        case GrBackendApi::kOpenGL: return format.asGLFormat() == GrGLFormat::kRGB8;
#endif
#ifdef SK_VULKAN
        case GrBackendApi::kVulkan: {
            VkFormat vkFormat;
            format.asVkFormat(&vkFormat);
            return vkFormat == VK_FORMAT_R8G8B8_UNORM;
        }
#endif
        case GrBackendApi::kMock:   return false;  // No GrColorType::kRGB_888
    }

    SkUNREACHABLE;
}

static SkColor get_expected_color(SkColorType skColorType,
                                  const GrBackendFormat& format,
                                  const SkColor4f& colorIn) {
#if 0
    SkColor4f colorOut;

    if (kGray_8_SkColorType == colorType) {
        // For the GPU backends, gray implies a single channel which is opaque.
        // This conversion uses the BT.709 coefficients
        float gray = 0.2126f * colorIn.fR + 0.7152f * colorIn.fG + 0.0722f * colorIn.fB;
        gray *= colorIn.fA;
        colorOut = { gray, gray, gray, 1.0f };
    } else if (kAlpha_8_SkColorType == colorType) {
        // For the GPU backends, alpha implies a single alpha channel.
        colorOut = { 0.0f, 0.0f, 0.0f, colorIn.fA };
    } else if (kARGB_4444_SkColorType == colorType) {
        // Compensate for the precision loss
        uint8_t r4 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fR * 15) * (255.0f / 15.0f));
        uint8_t g4 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fG * 15) * (255.0f / 15.0f));
        uint8_t b4 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fB * 15) * (255.0f / 15.0f));
        uint8_t a4 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fA * 15) * (255.0f / 15.0f));
        return SkColorSetARGB(a4, r4, g4, b4);
    } else if (kRGB_565_SkColorType == colorType) {
        // Fold in alpha and then make opaque - also compensate for the precision loss
        uint8_t r5 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fA * colorIn.fR * 31) * (255.0f / 31.0f));
        uint8_t g6 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fA * colorIn.fG * 63) * (255.0f / 63.0f));
        uint8_t b5 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fA * colorIn.fB * 31) * (255.0f / 31.0f));
        return SkColorSetARGB(255, r5, g6, b5);
    } else if (kRGB_888x_SkColorType == colorType) {
        // Fold in alpha and then make opaque
        uint8_t r = SkScalarRoundToInt(colorIn.fA * colorIn.fR * 255.0f);
        uint8_t g = SkScalarRoundToInt(colorIn.fA * colorIn.fG * 255.0f);
        uint8_t b = SkScalarRoundToInt(colorIn.fA * colorIn.fB * 255.0f);
        return SkColorSetARGB(255, r, g, b);
    } else if (kRGBA_1010102_SkColorType == colorType) {
        // Enh
        float newA = SkScalarRoundToInt(colorIn.fA * 3.0f) / 3.0f;
        SkASSERT(newA >= 0.0f && newA <= 1.0f);
        SkDebugf("newA %.2f\n", newA);
        uint8_t r10 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fR * 1023) * newA * (255.0f / 1023.0f));
        uint8_t g10 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fG * 1023) * newA * (255.0f / 1023.0f));
        uint8_t b10 = SkScalarRoundToInt(SkScalarRoundToInt(colorIn.fB * 1023) * newA * (255.0f / 1023.0f));
        uint8_t a2 = SkScalarRoundToInt(newA * 255.0f);
        SkDebugf("color %d %x %d %x %d %x %d %x\n", r10, r10, g10, g10, b10, b10, a2, a2);
        return SkColorSetARGB(a2, r10, g10, b10);
    } else if (kBGRA_8888_SkColorType == colorType) {
        // Manually swizzle
        if (isBGRA(format)) {
            colorOut = colorIn;
        } else {
            colorOut = { colorIn.fB, colorIn.fG, colorIn.fR, colorIn.fA };
        }
    } else {
        colorOut = colorIn;
    }

    return colorOut.toSkColor();
#else
    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    SkImageInfo ii = SkImageInfo::Make(1, 1, skColorType, at);
    SkAutoPixmapStorage pixmap;
    pixmap.alloc(ii);
    pixmap.erase(colorIn);
    SkColor result = pixmap.getColor(0, 0);

    if (kBGRA_8888_SkColorType == skColorType) {
        if (!isBGRA(format)) {
            // Manually swizzle
            return SkColorSetARGB(SkColorGetA(result), SkColorGetB(result),
                                  SkColorGetG(result), SkColorGetR(result));
        }
    }

    return result;
#endif
}

static void check_base_readbacks(GrContext* context, const GrBackendTexture& backendTex,
                                 SkColorType skColorType, GrRenderable renderable,
                                 const SkColor4f& color, skiatest::Reporter* reporter) {
    if (isRGB(backendTex.getBackendFormat())) {
        // readPixels is busted for the RGB backend format
        // TODO: add a GrColorType::kRGB_888 to fix the situation
        return;
    }

    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;


    SkColor expectedColor = get_expected_color(skColorType, backendTex.getBackendFormat(), color);

    SkImageInfo ii = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkAutoPixmapStorage actual;
    SkAssertResult(actual.tryAlloc(ii));

    {
        sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                      backendTex,
                                                      kTopLeft_GrSurfaceOrigin,
                                                      skColorType,
                                                      at,
                                                      nullptr);
        if (img) {
            actual.erase(SkColors::kTransparent);
            bool result = img->readPixels(actual, 0, 0);
            if (!result) {
                // TODO: we need a better way to tell a priori if readPixels will work for an
                // arbitrary colorType
#if 0
                ERRORF(reporter, "Couldn't readback from SkImage for colorType: %d\n",
                       colorType);
#endif
            } else {
                check_solid_pixmap(expectedColor, actual, skColorType, reporter);
            }
        }
    }

    // This will mark any mipmaps as dirty (bc that is what we do when we wrap a renderable
    // backend texture) so it must be done last!
    if (GrRenderable::kYes == renderable) {
        sk_sp<SkSurface> surf = SkSurface::MakeFromBackendTexture(context,
                                                                  backendTex,
                                                                  kTopLeft_GrSurfaceOrigin,
                                                                  0,
                                                                  skColorType,
                                                                  nullptr, nullptr);
        if (surf) {
            actual.erase(SkColors::kTransparent);
            bool result = surf->readPixels(actual, 0, 0);
            REPORTER_ASSERT(reporter, result);

            check_solid_pixmap(expectedColor, actual, skColorType, reporter);
        }
    }
}

// Draw the backend texture (wrapped in an SkImage) into an RGBA surface attempting to access
// all the mipMap levels.
static void check_mipmaps(GrContext* context, const GrBackendTexture& backendTex,
                          SkColorType skColorType, const SkColor expectedColors[6],
                          skiatest::Reporter* reporter) {

    SkAlphaType at = SkColorTypeIsAlwaysOpaque(skColorType) ? kOpaque_SkAlphaType
                                                            : kPremul_SkAlphaType;

    sk_sp<SkImage> img = SkImage::MakeFromTexture(context,
                                                  backendTex,
                                                  kTopLeft_GrSurfaceOrigin,
                                                  skColorType,
                                                  at,
                                                  nullptr);
    if (!img) {
        return; // What!?!
    }

#ifdef SK_GL
    // skbug.com/9141 (RGBA_F32 mipmaps appear to be broken on some Mali devices)
    if (GrBackendApi::kOpenGL == context->backend()) {
        GrGLGpu* glGPU = static_cast<GrGLGpu*>(context->priv().getGpu());

        if (kRGBA_F32_SkColorType == skColorType &&
            kGLES_GrGLStandard == glGPU->ctxInfo().standard()) {
            return;
        }
    }
#endif



    SkImageInfo newII = SkImageInfo::Make(32, 32, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    SkAutoPixmapStorage actual;
    SkAssertResult(actual.tryAlloc(newII));
    actual.erase(SkColors::kTransparent);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(context,
                                                        SkBudgeted::kNo,
                                                        newII, 1,
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

        bool result = surf->readPixels(actual, 0, 0);
        REPORTER_ASSERT(reporter, result);

        SkColor actualColor = actual.getColor(0, 0);

        if (!colors_eq(actualColor, expectedColors[i], 2)) {
            ERRORF(reporter, "Pixel mismatch colorType %s: level: %d e: 0x%x a: 0x%x\n",
                   SkColorTypeToStr(skColorType), i, expectedColors[i], actualColor);
        }
    }
}

// Test initialization of GrBackendObjects to a specific color
static void test_color_init(GrContext* context, skiatest::Reporter* reporter,
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

    if (mipMapped == GrMipMapped::kYes) {
        SkColor expectedColor = get_expected_color(skColorType, backendTex.getBackendFormat(), color);
        SkColor expectedColors[6] = { expectedColor, expectedColor, expectedColor,
                                      expectedColor, expectedColor, expectedColor };

        check_mipmaps(context, backendTex, skColorType, expectedColors, reporter);
    }

    // The last step in this test will dirty the mipmaps so do it last
    check_base_readbacks(context, backendTex, skColorType, renderable, color, reporter);

    context->deleteBackendTexture(backendTex);
}

static int make_pixmaps(SkColorType skColorType, GrMipMapped mipMapped,
                        const SkColor4f colors[6], SkAutoPixmapStorage pixmaps[6]) {
    int levelSize = 32;

    int numMipLevels = mipMapped == GrMipMapped::kYes ? 6 : 1;

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

// Test initialization of GrBackendObjects to src pixels
static void test_pixmap_init(GrContext* context, skiatest::Reporter* reporter,
                             std::function<GrBackendTexture (GrContext*,
                                                             const SkPixmap srcData[],
                                                             int numLevels,
                                                             GrRenderable)> create,
                             SkColorType skColorType, GrMipMapped mipMapped,
                             GrRenderable renderable) {

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

    GrBackendTexture backendTex = create(context, pixmaps, numMipLevels, renderable);
    if (!backendTex.isValid()) {
        // errors here should be reported by the test_wrapping test
        return;
    }

    if (mipMapped == GrMipMapped::kYes) {
        SkColor expectedColors[6] = {
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[0]),
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[1]),
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[2]),
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[3]),
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[4]),
            get_expected_color(skColorType, backendTex.getBackendFormat(), colors[5]),
        };

        check_mipmaps(context, backendTex, skColorType, expectedColors, reporter);
    }

    // The last step in this test will dirty the mipmaps so do it last
    check_base_readbacks(context, backendTex, skColorType, renderable, colors[0], reporter);

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
        GrPixelConfig fConfig1;
        SkColor4f     fColor;
    } combinations[] = {
        { kAlpha_8_SkColorType,      kAlpha_8_GrPixelConfig,           kTransCol           },
        { kRGB_565_SkColorType,      kRGB_565_GrPixelConfig,           SkColors::kRed      },
        { kARGB_4444_SkColorType,    kRGBA_4444_GrPixelConfig,         SkColors::kGreen    },
        { kRGBA_8888_SkColorType,    kRGBA_8888_GrPixelConfig,         SkColors::kBlue     },
        { kRGB_888x_SkColorType,     kRGB_888_GrPixelConfig,           SkColors::kCyan     },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { kBGRA_8888_SkColorType,    kBGRA_8888_GrPixelConfig,         { 1, 0, 0, 1.0f }   },
        // TODO: readback is busted when alpha = 0.5f (perhaps premul vs. unpremul)
        { kRGBA_1010102_SkColorType, kRGBA_1010102_GrPixelConfig,      { 0.5f, 0, 0, 1.0f }},
        // The kRGB_101010x_SkColorType has no Ganesh correlate
        { kRGB_101010x_SkColorType,  kUnknown_GrPixelConfig,           { 0, 0.5f, 0, 0.5f }},
        { kGray_8_SkColorType,       kGray_8_GrPixelConfig,            kGrayCol            },
        { kRGBA_F16Norm_SkColorType, kRGBA_half_Clamped_GrPixelConfig, SkColors::kLtGray   },
        { kRGBA_F16_SkColorType,     kRGBA_half_GrPixelConfig,         SkColors::kYellow   },
        { kRGBA_F32_SkColorType,     kRGBA_float_GrPixelConfig,        SkColors::kGray     },
    };

//    GR_STATIC_ASSERT(kLastEnum_SkColorType == SK_ARRAY_COUNT(combinations));

    for (auto combo : combinations) {
        SkColorType colorType = combo.fColorType;

        if (GrBackendApi::kMetal == context->backend()) {
            // skbug.com/9086 (Metal caps may not be handling RGBA32 correctly)
            if (kRGBA_F32_SkColorType == combo.fColorType) {
                continue;
            }
        }

        for (auto mipMapped : { /*GrMipMapped::kNo,*/ GrMipMapped::kYes }) {
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
#if 0
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
#endif

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

#if 0
                {
                    auto createWithSrcDataMtd = [](GrContext* context,
                                                   const SkPixmap srcData[],
                                                   int numLevels,
                                                   GrRenderable renderable) {
                        SkASSERT(srcData && numLevels);

                        auto result = context->priv().createBackendTexture(srcData, numLevels,
                                                                           renderable,
                                                                           GrProtected::kNo);
                        check_vk_layout(result, VkLayout::kReadOnlyOptimal);

#ifdef SK_DEBUG
                        {
                            auto format = context->defaultBackendFormat(srcData[0].colorType(),
                                                                        renderable);
                            SkASSERT(format == result.getBackendFormat());
                        }
#endif
                        return result;
                    };

                    test_pixmap_init(context, reporter, createWithSrcDataMtd, colorType,
                                     mipMapped, renderable);
                }
#endif
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

        { GrColorType::kRG_88,            GR_GL_RG8,                  { 0.5f, 0.5f, 0, 0 } },
        { GrColorType::kAlpha_F16,        GR_GL_R16F,                 { 1.0f, 0, 0, 0.5f } },
        { GrColorType::kAlpha_F16,        GR_GL_LUMINANCE16F,         kGrayCol             },

        { GrColorType::kR_16,             GR_GL_R16,                  SkColors::kRed       },
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
        { GrColorType::kRG_88,            VK_FORMAT_R8G8_UNORM,               { 0.5f, 0.5f, 0, 0 }},
        { GrColorType::kAlpha_F16,        VK_FORMAT_R16_SFLOAT,               { 1.0f, 0, 0, 0.5f }},

        { GrColorType::kR_16,             VK_FORMAT_R16_UNORM,                SkColors::kRed      },
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
