/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrBackendSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/private/SkColorData.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrDirectContextPriv.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/GrProxyProvider.h"
#include "tests/Test.h"
#include "tools/gpu/BackendSurfaceFactory.h"

#include <initializer_list>

static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);
static const U8CPU DEV_PAD = 0xee;

static SkPMColor get_canvas_color(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0x0;
    switch ((x+y) % 5) {
        case 0:
            a = 0xff;
            break;
        case 1:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 3:
            a = 0x00;
            break;
        case 4:
            a = 0x01;
            break;
    }
    return SkPremultiplyARGBInline(a, r, g, b);
}

// assumes any premu/.unpremul has been applied
static uint32_t pack_color_type(SkColorType ct, U8CPU a, U8CPU r, U8CPU g, U8CPU b) {
    uint32_t r32;
    uint8_t* result = reinterpret_cast<uint8_t*>(&r32);
    switch (ct) {
        case kBGRA_8888_SkColorType:
            result[0] = b;
            result[1] = g;
            result[2] = r;
            result[3] = a;
            break;
        case kRGBA_8888_SkColorType:  // fallthrough
        case kRGB_888x_SkColorType:
            result[0] = r;
            result[1] = g;
            result[2] = b;
            result[3] = a;
            break;
        default:
            SkASSERT(0);
            return 0;
    }
    return r32;
}

static uint32_t get_bitmap_color(int x, int y, int w, SkColorType ct, SkAlphaType at) {
    int n = y * w + x;
    U8CPU b = n & 0xff;
    U8CPU g = (n >> 8) & 0xff;
    U8CPU r = (n >> 16) & 0xff;
    U8CPU a = 0;
    switch ((x+y) % 5) {
        case 4:
            a = 0xff;
            break;
        case 3:
            a = 0x80;
            break;
        case 2:
            a = 0xCC;
            break;
        case 1:
            a = 0x01;
            break;
        case 0:
            a = 0x00;
            break;
    }
    if (kPremul_SkAlphaType == at) {
        r = SkMulDiv255Ceiling(r, a);
        g = SkMulDiv255Ceiling(g, a);
        b = SkMulDiv255Ceiling(b, a);
    }
    return pack_color_type(ct, a, r, g , b);
}

static void fill_surface(SkSurface* surface) {
    SkBitmap bmp;
    bmp.allocN32Pixels(DEV_W, DEV_H);
    for (int y = 0; y < DEV_H; ++y) {
        for (int x = 0; x < DEV_W; ++x) {
            *bmp.getAddr32(x, y) = get_canvas_color(x, y);
        }
    }
    surface->writePixels(bmp, 0, 0);
}

/**
 *  Lucky for us, alpha is always in the same spot (SK_A32_SHIFT), for both RGBA and BGRA.
 *  Thus this routine doesn't need to know the exact colortype
 */
static uint32_t premul(uint32_t color) {
    unsigned a = SkGetPackedA32(color);
    // these next three are not necessarily r,g,b in that order, but they are r,g,b in some order.
    unsigned c0 = SkGetPackedR32(color);
    unsigned c1 = SkGetPackedG32(color);
    unsigned c2 = SkGetPackedB32(color);
    c0 = SkMulDiv255Ceiling(c0, a);
    c1 = SkMulDiv255Ceiling(c1, a);
    c2 = SkMulDiv255Ceiling(c2, a);
    return SkPackARGB32NoCheck(a, c0, c1, c2);
}

static SkPMColor convert_to_PMColor(SkColorType ct, SkAlphaType at, uint32_t color) {
    if (kUnpremul_SkAlphaType == at) {
        color = premul(color);
    }
    switch (ct) {
        case kRGBA_8888_SkColorType: // fallthrough
        case kRGB_888x_SkColorType:
            color = SkSwizzle_RGBA_to_PMColor(color);
            break;
        case kBGRA_8888_SkColorType:
            color = SkSwizzle_BGRA_to_PMColor(color);
            break;
        default:
            SkASSERT(0);
            break;
    }
    return color;
}

static bool check_pixel(SkPMColor a, SkPMColor b, bool didPremulConversion) {
    if (!didPremulConversion) {
        return a == b;
    }
    int32_t aA = static_cast<int32_t>(SkGetPackedA32(a));
    int32_t aR = static_cast<int32_t>(SkGetPackedR32(a));
    int32_t aG = static_cast<int32_t>(SkGetPackedG32(a));
    int32_t aB = SkGetPackedB32(a);

    int32_t bA = static_cast<int32_t>(SkGetPackedA32(b));
    int32_t bR = static_cast<int32_t>(SkGetPackedR32(b));
    int32_t bG = static_cast<int32_t>(SkGetPackedG32(b));
    int32_t bB = static_cast<int32_t>(SkGetPackedB32(b));

    return aA == bA &&
           SkAbs32(aR - bR) <= 1 &&
           SkAbs32(aG - bG) <= 1 &&
           SkAbs32(aB - bB) <= 1;
}

bool write_should_succeed(const SkImageInfo& dstInfo, const SkImageInfo& srcInfo, bool isGPU) {
    if (!SkImageInfoValidConversion(dstInfo, srcInfo)) {
        return false;
    }
    if (!isGPU) {
        return true;
    }
    // The GPU backend supports writing unpremul data to a premul dst but not vice versa.
    if (srcInfo.alphaType() == kPremul_SkAlphaType &&
        dstInfo.alphaType() == kUnpremul_SkAlphaType) {
        return false;
    }
    if (!SkColorTypeIsAlwaysOpaque(srcInfo.colorType()) &&
        SkColorTypeIsAlwaysOpaque(dstInfo.colorType())) {
        return false;
    }
    // The source has no alpha value and the dst is only alpha
    if (SkColorTypeIsAlwaysOpaque(srcInfo.colorType()) &&
        SkColorTypeIsAlphaOnly(dstInfo.colorType())) {
        return false;
    }
    return true;
}

static bool check_write(skiatest::Reporter* reporter, SkSurface* surf, SkAlphaType surfaceAlphaType,
                        const SkBitmap& bitmap, int writeX, int writeY) {
    size_t canvasRowBytes;
    const uint32_t* canvasPixels;

    // Can't use canvas->peekPixels(), as we are trying to look at GPU pixels sometimes as well.
    // At some point this will be unsupported, as we won't allow accessBitmap() to magically call
    // readPixels for the client.
    SkBitmap secretDevBitmap;
    secretDevBitmap.allocN32Pixels(surf->width(), surf->height());
    if (!surf->readPixels(secretDevBitmap, 0, 0)) {
        return false;
    }

    canvasRowBytes = secretDevBitmap.rowBytes();
    canvasPixels = static_cast<const uint32_t*>(secretDevBitmap.getPixels());

    if (nullptr == canvasPixels) {
        return false;
    }

    if (surf->width() != DEV_W || surf->height() != DEV_H) {
        return false;
    }

    const SkImageInfo& bmInfo = bitmap.info();

    SkIRect writeRect = SkIRect::MakeXYWH(writeX, writeY, bitmap.width(), bitmap.height());
    for (int cy = 0; cy < DEV_H; ++cy) {
        for (int cx = 0; cx < DEV_W; ++cx) {
            SkPMColor canvasPixel = canvasPixels[cx];
            if (writeRect.contains(cx, cy)) {
                int bx = cx - writeX;
                int by = cy - writeY;
                uint32_t bmpColor8888 = get_bitmap_color(bx, by, bitmap.width(),
                                                       bmInfo.colorType(), bmInfo.alphaType());
                bool mul = (kUnpremul_SkAlphaType == bmInfo.alphaType());
                SkPMColor bmpPMColor = convert_to_PMColor(bmInfo.colorType(), bmInfo.alphaType(),
                                                          bmpColor8888);
                if (bmInfo.alphaType() == kOpaque_SkAlphaType ||
                    surfaceAlphaType == kOpaque_SkAlphaType) {
                    bmpPMColor |= 0xFF000000;
                }
                if (!check_pixel(bmpPMColor, canvasPixel, mul)) {
                    ERRORF(reporter, "Expected canvas pixel at %d, %d to be 0x%08x, got 0x%08x. "
                           "Write performed premul: %d", cx, cy, bmpPMColor, canvasPixel, mul);
                    return false;
                }
            } else {
                SkPMColor testColor = get_canvas_color(cx, cy);
                if (canvasPixel != testColor) {
                    ERRORF(reporter, "Canvas pixel outside write rect at %d, %d changed."
                           " Should be 0x%08x, got 0x%08x. ", cx, cy, testColor, canvasPixel);
                    return false;
                }
            }
        }
        if (cy != DEV_H -1) {
            const char* pad = reinterpret_cast<const char*>(canvasPixels + DEV_W);
            for (size_t px = 0; px < canvasRowBytes - 4 * DEV_W; ++px) {
                bool check;
                REPORTER_ASSERT(reporter, check = (pad[px] == static_cast<char>(DEV_PAD)));
                if (!check) {
                    return false;
                }
            }
        }
        canvasPixels += canvasRowBytes/4;
    }

    return true;
}

#include "include/core/SkMallocPixelRef.h"

// This is a tricky pattern, because we have to setConfig+rowBytes AND specify
// a custom pixelRef (which also has to specify its rowBytes), so we have to be
// sure that the two rowBytes match (and the infos match).
//
static bool alloc_row_bytes(SkBitmap* bm, const SkImageInfo& info, size_t rowBytes) {
    if (!bm->setInfo(info, rowBytes)) {
        return false;
    }
    sk_sp<SkPixelRef> pr = SkMallocPixelRef::MakeAllocate(info, rowBytes);
    bm->setPixelRef(std::move(pr), 0, 0);
    return true;
}

static void free_pixels(void* pixels, void* ctx) {
    sk_free(pixels);
}

static bool setup_bitmap(SkBitmap* bm, SkColorType ct, SkAlphaType at, int w, int h, int tightRB) {
    size_t rowBytes = tightRB ? 0 : 4 * w + 60;
    SkImageInfo info = SkImageInfo::Make(w, h, ct, at);
    if (!alloc_row_bytes(bm, info, rowBytes)) {
        return false;
    }
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            *bm->getAddr32(x, y) = get_bitmap_color(x, y, w, ct, at);
        }
    }
    return true;
}

static void call_writepixels(SkSurface* surface) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(1, 1);
    SkPMColor pixel = 0;
    surface->writePixels({info, &pixel, sizeof(SkPMColor)}, 0, 0);
}

DEF_TEST(WritePixelsSurfaceGenID, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(100, 100);
    auto surface(SkSurface::MakeRaster(info));
    uint32_t genID1 = surface->generationID();
    call_writepixels(surface.get());
    uint32_t genID2 = surface->generationID();
    REPORTER_ASSERT(reporter, genID1 != genID2);
}

static void test_write_pixels(skiatest::Reporter* reporter, SkSurface* surface,
                              const SkImageInfo& surfaceInfo) {
    const SkIRect testRects[] = {
        // entire thing
        DEV_RECT,
        // larger on all sides
        SkIRect::MakeLTRB(-10, -10, DEV_W + 10, DEV_H + 10),
        // fully contained
        SkIRect::MakeLTRB(DEV_W / 4, DEV_H / 4, 3 * DEV_W / 4, 3 * DEV_H / 4),
        // outside top left
        SkIRect::MakeLTRB(-10, -10, -1, -1),
        // touching top left corner
        SkIRect::MakeLTRB(-10, -10, 0, 0),
        // overlapping top left corner
        SkIRect::MakeLTRB(-10, -10, DEV_W / 4, DEV_H / 4),
        // overlapping top left and top right corners
        SkIRect::MakeLTRB(-10, -10, DEV_W  + 10, DEV_H / 4),
        // touching entire top edge
        SkIRect::MakeLTRB(-10, -10, DEV_W  + 10, 0),
        // overlapping top right corner
        SkIRect::MakeLTRB(3 * DEV_W / 4, -10, DEV_W  + 10, DEV_H / 4),
        // contained in x, overlapping top edge
        SkIRect::MakeLTRB(DEV_W / 4, -10, 3 * DEV_W  / 4, DEV_H / 4),
        // outside top right corner
        SkIRect::MakeLTRB(DEV_W + 1, -10, DEV_W + 10, -1),
        // touching top right corner
        SkIRect::MakeLTRB(DEV_W, -10, DEV_W + 10, 0),
        // overlapping top left and bottom left corners
        SkIRect::MakeLTRB(-10, -10, DEV_W / 4, DEV_H + 10),
        // touching entire left edge
        SkIRect::MakeLTRB(-10, -10, 0, DEV_H + 10),
        // overlapping bottom left corner
        SkIRect::MakeLTRB(-10, 3 * DEV_H / 4, DEV_W / 4, DEV_H + 10),
        // contained in y, overlapping left edge
        SkIRect::MakeLTRB(-10, DEV_H / 4, DEV_W / 4, 3 * DEV_H / 4),
        // outside bottom left corner
        SkIRect::MakeLTRB(-10, DEV_H + 1, -1, DEV_H + 10),
        // touching bottom left corner
        SkIRect::MakeLTRB(-10, DEV_H, 0, DEV_H + 10),
        // overlapping bottom left and bottom right corners
        SkIRect::MakeLTRB(-10, 3 * DEV_H / 4, DEV_W + 10, DEV_H + 10),
        // touching entire left edge
        SkIRect::MakeLTRB(0, DEV_H, DEV_W, DEV_H + 10),
        // overlapping bottom right corner
        SkIRect::MakeLTRB(3 * DEV_W / 4, 3 * DEV_H / 4, DEV_W + 10, DEV_H + 10),
        // overlapping top right and bottom right corners
        SkIRect::MakeLTRB(3 * DEV_W / 4, -10, DEV_W + 10, DEV_H + 10),
    };

    SkCanvas* canvas = surface->getCanvas();

    static const struct {
        SkColorType fColorType;
        SkAlphaType fAlphaType;
    } gSrcConfigs[] = {
            {kRGBA_8888_SkColorType, kPremul_SkAlphaType},
            {kRGBA_8888_SkColorType, kUnpremul_SkAlphaType},
            {kRGB_888x_SkColorType, kOpaque_SkAlphaType},
            {kBGRA_8888_SkColorType, kPremul_SkAlphaType},
            {kBGRA_8888_SkColorType, kUnpremul_SkAlphaType},
    };
    for (size_t r = 0; r < SK_ARRAY_COUNT(testRects); ++r) {
        const SkIRect& rect = testRects[r];
        for (int tightBmp = 0; tightBmp < 2; ++tightBmp) {
            for (size_t c = 0; c < SK_ARRAY_COUNT(gSrcConfigs); ++c) {
                const SkColorType ct = gSrcConfigs[c].fColorType;
                const SkAlphaType at = gSrcConfigs[c].fAlphaType;

                bool isGPU = SkToBool(surface->getCanvas()->recordingContext());
                fill_surface(surface);
                SkBitmap bmp;
                REPORTER_ASSERT(reporter, setup_bitmap(&bmp, ct, at, rect.width(),
                                                       rect.height(), SkToBool(tightBmp)));
                uint32_t idBefore = surface->generationID();

                surface->writePixels(bmp, rect.fLeft, rect.fTop);

                uint32_t idAfter = surface->generationID();
                REPORTER_ASSERT(reporter, check_write(reporter, surface, surfaceInfo.alphaType(),
                                                      bmp, rect.fLeft, rect.fTop));

                // we should change the genID iff pixels were actually written.
                SkIRect canvasRect = SkIRect::MakeSize(canvas->getBaseLayerSize());
                SkIRect writeRect = SkIRect::MakeXYWH(rect.fLeft, rect.fTop,
                                                      bmp.width(), bmp.height());
                bool expectSuccess = SkIRect::Intersects(canvasRect, writeRect) &&
                                     write_should_succeed(surfaceInfo, bmp.info(), isGPU);
                REPORTER_ASSERT(reporter, expectSuccess == (idBefore != idAfter));
            }
        }
    }
}

DEF_TEST(WritePixels, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    for (auto& tightRowBytes : { true, false }) {
        const size_t rowBytes = tightRowBytes ? info.minRowBytes() : 4 * DEV_W + 100;
        const size_t size = info.computeByteSize(rowBytes);
        void* pixels = sk_malloc_throw(size);
        // if rowBytes isn't tight then set the padding to a known value
        if (!tightRowBytes) {
            memset(pixels, DEV_PAD, size);
        }
        auto surface(SkSurface::MakeRasterDirectReleaseProc(info, pixels, rowBytes,
                                                            free_pixels, nullptr));
        test_write_pixels(reporter, surface.get(), info);
    }
}

static void test_write_pixels(skiatest::Reporter* reporter,
                              GrRecordingContext* rContext,
                              int sampleCnt) {
    const SkImageInfo ii = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    for (auto& origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
        sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(rContext,
                                                             SkBudgeted::kNo, ii, sampleCnt,
                                                             origin, nullptr));
        if (surface) {
            test_write_pixels(reporter, surface.get(), ii);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WritePixels_Gpu, reporter, ctxInfo) {
    test_write_pixels(reporter, ctxInfo.directContext(), 1);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WritePixelsMSAA_Gpu, reporter, ctxInfo) {
    test_write_pixels(reporter, ctxInfo.directContext(), 1);
}

static void test_write_pixels_non_texture(skiatest::Reporter* reporter,
                                          GrDirectContext* dContext,
                                          int sampleCnt) {
    // Dawn currently doesn't support writePixels to a texture-as-render-target.
    // See http://skbug.com/10336.
    if (GrBackendApi::kDawn == dContext->backend()) {
        return;
    }
    for (auto& origin : { kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin }) {
        SkColorType colorType = kN32_SkColorType;
        auto surface = sk_gpu_test::MakeBackendRenderTargetSurface(dContext,
                                                                   {DEV_W, DEV_H},
                                                                   origin,
                                                                   sampleCnt,
                                                                   colorType);
        if (surface) {
            auto ii = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
            test_write_pixels(reporter, surface.get(), ii);
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WritePixelsNonTexture_Gpu, reporter, ctxInfo) {
    test_write_pixels_non_texture(reporter, ctxInfo.directContext(), 1);
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WritePixelsNonTextureMSAA_Gpu, reporter, ctxInfo) {
    test_write_pixels_non_texture(reporter, ctxInfo.directContext(), 4);
}

static sk_sp<SkSurface> create_surf(GrRecordingContext* rContext, int width, int height) {
    const SkImageInfo ii = SkImageInfo::Make(width, height,
                                             kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surf = SkSurface::MakeRenderTarget(rContext, SkBudgeted::kYes, ii);
    surf->flushAndSubmit();
    return surf;
}

static sk_sp<SkImage> upload(const sk_sp<SkSurface>& surf, SkColor color) {
    const SkImageInfo smII = SkImageInfo::Make(16, 16, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    SkBitmap bm;
    bm.allocPixels(smII);
    bm.eraseColor(color);

    surf->writePixels(bm, 0, 0);

    return surf->makeImageSnapshot();
}

// This is tests whether the first writePixels is completed before the
// second writePixels takes effect (i.e., that writePixels correctly flushes
// in between uses of the shared backing resource).
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(WritePixelsPendingIO, reporter, ctxInfo) {
    auto context = ctxInfo.directContext();
    GrProxyProvider* proxyProvider = context->priv().proxyProvider();
    const GrCaps* caps = context->priv().caps();

    static const int kFullSize = 62;
    static const int kHalfSize = 31;

    static const uint32_t kLeftColor = 0xFF222222;
    static const uint32_t kRightColor = 0xFFAAAAAA;

    const SkImageInfo fullII = SkImageInfo::Make(kFullSize, kFullSize,
                                                 kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    const SkImageInfo halfII = SkImageInfo::Make(kHalfSize, kFullSize,
                                                 kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> dest = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, fullII);

    {
        // Seed the resource cached with a scratch texture that will be reused by writePixels
        static constexpr SkISize kDims = {32, 64};

        const GrBackendFormat format = caps->getDefaultBackendFormat(GrColorType::kRGBA_8888,
                                                                     GrRenderable::kNo);

        sk_sp<GrTextureProxy> temp = proxyProvider->createProxy(
                format, kDims, GrRenderable::kNo, 1, GrMipmapped::kNo, SkBackingFit::kApprox,
                SkBudgeted::kYes, GrProtected::kNo);
        temp->instantiate(context->priv().resourceProvider());
    }

    // Create the surfaces and flush them to ensure there is no lingering pendingIO
    sk_sp<SkSurface> leftSurf = create_surf(context, kHalfSize, kFullSize);
    sk_sp<SkSurface> rightSurf = create_surf(context, kHalfSize, kFullSize);

    sk_sp<SkImage> leftImg = upload(std::move(leftSurf), kLeftColor);
    dest->getCanvas()->drawImage(std::move(leftImg), 0, 0);

    sk_sp<SkImage> rightImg = upload(std::move(rightSurf), kRightColor);
    dest->getCanvas()->drawImage(std::move(rightImg), kHalfSize, 0);

    SkBitmap bm;
    bm.allocPixels(fullII);
    SkAssertResult(dest->readPixels(bm, 0, 0));

    bool isCorrect = true;
    for (int y = 0; isCorrect && y < 16; ++y) {
        const uint32_t* sl = bm.getAddr32(0, y);

        for (int x = 0; x < 16; ++x) {
            if (kLeftColor != sl[x]) {
                isCorrect = false;
                break;
            }
        }
        for (int x = kHalfSize; x < kHalfSize+16; ++x) {
            if (kRightColor != sl[x]) {
                isCorrect = false;
                break;
            }
        }
    }

    REPORTER_ASSERT(reporter, isCorrect);
}

DEF_TEST(WritePixels_InvalidRowBytes, reporter) {
    auto dstII = SkImageInfo::Make({10, 10}, kRGBA_8888_SkColorType, kPremul_SkAlphaType);
    auto surf = SkSurface::MakeRaster(dstII);
    for (int ct = 0; ct < kLastEnum_SkColorType + 1; ++ct) {
        auto colorType = static_cast<SkColorType>(ct);

        size_t bpp = SkColorTypeBytesPerPixel(colorType);
        if (bpp <= 1) {
            continue;
        }
        auto srcII = dstII.makeColorType(colorType);
        size_t badRowBytes = (surf->width() + 1)*bpp - 1;
        auto storage = std::make_unique<char[]>(badRowBytes*surf->height());
        memset(storage.get(), 0, badRowBytes * surf->height());
        // SkSurface::writePixels doesn't report bool, SkCanvas's does.
        REPORTER_ASSERT(reporter,
                        !surf->getCanvas()->writePixels(srcII, storage.get(), badRowBytes, 0, 0));
    }
}
