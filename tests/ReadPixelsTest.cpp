/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <initializer_list>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkColorData.h"
#include "include/private/SkHalf.h"
#include "include/private/SkImageInfoPriv.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrProxyProvider.h"
#include "src/gpu/SkGr.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/gpu/GrContextFactory.h"
#include "tools/gpu/ProxyUtils.h"

static const int DEV_W = 100, DEV_H = 100;
static const SkIRect DEV_RECT = SkIRect::MakeWH(DEV_W, DEV_H);
static const SkRect DEV_RECT_S = SkRect::MakeWH(DEV_W * SK_Scalar1,
                                                DEV_H * SK_Scalar1);

static SkPMColor get_src_color(int x, int y) {
    SkASSERT(x >= 0 && x < DEV_W);
    SkASSERT(y >= 0 && y < DEV_H);

    U8CPU r = x;
    U8CPU g = y;
    U8CPU b = 0xc;

    U8CPU a = 0xff;
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
        case 4:
            a = 0x01;
            break;
        case 3:
            a = 0x00;
            break;
    }
    return SkPremultiplyARGBInline(a, r, g, b);
}

static SkPMColor get_dst_bmp_init_color(int x, int y, int w) {
    int n = y * w + x;

    U8CPU b = n & 0xff;
    U8CPU g = (n >> 8) & 0xff;
    U8CPU r = (n >> 16) & 0xff;
    return SkPackARGB32(0xff, r, g , b);
}

// TODO: Make this consider both ATs
static SkPMColor convert_to_pmcolor(SkColorType ct, SkAlphaType at, const uint32_t* addr,
                                    bool* doUnpremul) {
    *doUnpremul = (kUnpremul_SkAlphaType == at);

    const uint8_t* c = reinterpret_cast<const uint8_t*>(addr);
    U8CPU a,r,g,b;
    switch (ct) {
        case kBGRA_8888_SkColorType:
            b = static_cast<U8CPU>(c[0]);
            g = static_cast<U8CPU>(c[1]);
            r = static_cast<U8CPU>(c[2]);
            a = static_cast<U8CPU>(c[3]);
            break;
        case kRGB_888x_SkColorType:  // fallthrough
        case kRGBA_8888_SkColorType:
            r = static_cast<U8CPU>(c[0]);
            g = static_cast<U8CPU>(c[1]);
            b = static_cast<U8CPU>(c[2]);
            // We set this even when for kRGB_888x because our caller will validate that it is 0xff.
            a = static_cast<U8CPU>(c[3]);
            break;
        default:
            SkDEBUGFAIL("Unexpected colortype");
            return 0;
    }

    if (*doUnpremul) {
        r = SkMulDiv255Ceiling(r, a);
        g = SkMulDiv255Ceiling(g, a);
        b = SkMulDiv255Ceiling(b, a);
    }
    return SkPackARGB32(a, r, g, b);
}

static SkBitmap make_src_bitmap() {
    static SkBitmap bmp;
    if (bmp.isNull()) {
        bmp.allocN32Pixels(DEV_W, DEV_H);
        intptr_t pixels = reinterpret_cast<intptr_t>(bmp.getPixels());
        for (int y = 0; y < DEV_H; ++y) {
            for (int x = 0; x < DEV_W; ++x) {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(pixels + y * bmp.rowBytes() + x * bmp.bytesPerPixel());
                *pixel = get_src_color(x, y);
            }
        }
    }
    return bmp;
}

static void fill_src_canvas(SkCanvas* canvas) {
    canvas->save();
    canvas->setMatrix(SkMatrix::I());
    canvas->clipRect(DEV_RECT_S, kReplace_SkClipOp);
    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);
    canvas->drawBitmap(make_src_bitmap(), 0, 0, &paint);
    canvas->restore();
}

static void fill_dst_bmp_with_init_data(SkBitmap* bitmap) {
    int w = bitmap->width();
    int h = bitmap->height();
    intptr_t pixels = reinterpret_cast<intptr_t>(bitmap->getPixels());
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            SkPMColor initColor = get_dst_bmp_init_color(x, y, w);
            if (kAlpha_8_SkColorType == bitmap->colorType()) {
                uint8_t* alpha = reinterpret_cast<uint8_t*>(pixels + y * bitmap->rowBytes() + x);
                *alpha = SkGetPackedA32(initColor);
            } else {
                SkPMColor* pixel = reinterpret_cast<SkPMColor*>(pixels + y * bitmap->rowBytes() + x * bitmap->bytesPerPixel());
                *pixel = initColor;
            }
        }
    }
}

static bool check_read_pixel(SkPMColor a, SkPMColor b, bool didPremulConversion) {
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

// checks the bitmap contains correct pixels after the readPixels
// if the bitmap was prefilled with pixels it checks that these weren't
// overwritten in the area outside the readPixels.
static bool check_read(skiatest::Reporter* reporter, const SkBitmap& bitmap, int x, int y,
                       bool checkSurfacePixels, bool checkBitmapPixels,
                       SkImageInfo surfaceInfo) {
    SkAlphaType bmpAT = bitmap.alphaType();
    SkColorType bmpCT = bitmap.colorType();
    SkASSERT(!bitmap.isNull());
    SkASSERT(checkSurfacePixels || checkBitmapPixels);

    int bw = bitmap.width();
    int bh = bitmap.height();

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bw, bh);
    SkIRect clippedSrcRect = DEV_RECT;
    if (!clippedSrcRect.intersect(srcRect)) {
        clippedSrcRect.setEmpty();
    }
    if (kAlpha_8_SkColorType == bmpCT) {
        for (int by = 0; by < bh; ++by) {
            for (int bx = 0; bx < bw; ++bx) {
                int devx = bx + srcRect.fLeft;
                int devy = by + srcRect.fTop;
                const uint8_t* alpha = bitmap.getAddr8(bx, by);

                if (clippedSrcRect.contains(devx, devy)) {
                    if (checkSurfacePixels) {
                        uint8_t surfaceAlpha = (surfaceInfo.alphaType() == kOpaque_SkAlphaType)
                                                       ? 0xFF
                                                       : SkGetPackedA32(get_src_color(devx, devy));
                        if (surfaceAlpha != *alpha) {
                            ERRORF(reporter,
                                   "Expected readback alpha (%d, %d) value 0x%02x, got 0x%02x. ",
                                   bx, by, surfaceAlpha, *alpha);
                            return false;
                        }
                    }
                } else if (checkBitmapPixels) {
                    uint32_t origDstAlpha = SkGetPackedA32(get_dst_bmp_init_color(bx, by, bw));
                    if (origDstAlpha != *alpha) {
                        ERRORF(reporter, "Expected clipped out area of readback to be unchanged. "
                            "Expected 0x%02x, got 0x%02x", origDstAlpha, *alpha);
                        return false;
                    }
                }
            }
        }
        return true;
    }
    for (int by = 0; by < bh; ++by) {
        for (int bx = 0; bx < bw; ++bx) {
            int devx = bx + srcRect.fLeft;
            int devy = by + srcRect.fTop;

            const uint32_t* pixel = bitmap.getAddr32(bx, by);

            if (clippedSrcRect.contains(devx, devy)) {
                if (checkSurfacePixels) {
                    SkPMColor surfacePMColor = get_src_color(devx, devy);
                    if (SkColorTypeIsAlphaOnly(surfaceInfo.colorType())) {
                        surfacePMColor &= 0xFF000000;
                    }
                    if (kOpaque_SkAlphaType == surfaceInfo.alphaType() || kOpaque_SkAlphaType == bmpAT) {
                        surfacePMColor |= 0xFF000000;
                    }
                    bool didPremul;
                    SkPMColor pmPixel = convert_to_pmcolor(bmpCT, bmpAT, pixel, &didPremul);
                    if (!check_read_pixel(pmPixel, surfacePMColor, didPremul)) {
                        ERRORF(reporter,
                               "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x. "
                               "Readback was unpremul: %d",
                               bx, by, surfacePMColor, pmPixel, didPremul);
                        return false;
                    }
                }
            } else if (checkBitmapPixels) {
                uint32_t origDstPixel = get_dst_bmp_init_color(bx, by, bw);
                if (origDstPixel != *pixel) {
                    ERRORF(reporter, "Expected clipped out area of readback to be unchanged. "
                           "Expected 0x%08x, got 0x%08x", origDstPixel, *pixel);
                    return false;
                }
            }
        }
    }
    return true;
}

enum BitmapInit {
    kFirstBitmapInit = 0,

    kTight_BitmapInit = kFirstBitmapInit,
    kRowBytes_BitmapInit,
    kRowBytesOdd_BitmapInit,

    kLastAligned_BitmapInit = kRowBytes_BitmapInit,

#if 0  // THIS CAUSES ERRORS ON WINDOWS AND SOME ANDROID DEVICES
    kLast_BitmapInit = kRowBytesOdd_BitmapInit
#else
    kLast_BitmapInit = kLastAligned_BitmapInit
#endif
};

static BitmapInit nextBMI(BitmapInit bmi) {
    int x = bmi;
    return static_cast<BitmapInit>(++x);
}

static void init_bitmap(SkBitmap* bitmap, const SkIRect& rect, BitmapInit init, SkColorType ct,
                        SkAlphaType at) {
    SkImageInfo info = SkImageInfo::Make(rect.width(), rect.height(), ct, at);
    size_t rowBytes = 0;
    switch (init) {
        case kTight_BitmapInit:
            break;
        case kRowBytes_BitmapInit:
            rowBytes = SkAlign4((info.width() + 16) * info.bytesPerPixel());
            break;
        case kRowBytesOdd_BitmapInit:
            rowBytes = SkAlign4(info.width() * info.bytesPerPixel()) + 3;
            break;
        default:
            SkASSERT(0);
            break;
    }
    bitmap->allocPixels(info, rowBytes);
}

static const struct {
    SkColorType fColorType;
    SkAlphaType fAlphaType;
} gReadPixelsConfigs[] = {
        {kRGBA_8888_SkColorType, kPremul_SkAlphaType},
        {kRGBA_8888_SkColorType, kUnpremul_SkAlphaType},
        {kRGB_888x_SkColorType, kOpaque_SkAlphaType},
        {kBGRA_8888_SkColorType, kPremul_SkAlphaType},
        {kBGRA_8888_SkColorType, kUnpremul_SkAlphaType},
        {kAlpha_8_SkColorType, kPremul_SkAlphaType},
};
const SkIRect gReadPixelsTestRects[] = {
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

bool read_should_succeed(const SkIRect& srcRect, const SkImageInfo& dstInfo,
                         const SkImageInfo& srcInfo) {
    return SkIRect::Intersects(srcRect, DEV_RECT) && SkImageInfoValidConversion(dstInfo, srcInfo);
}

static void test_readpixels(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                            const SkImageInfo& surfaceInfo, BitmapInit lastBitmapInit) {
    SkCanvas* canvas = surface->getCanvas();
    fill_src_canvas(canvas);
    for (size_t rect = 0; rect < SK_ARRAY_COUNT(gReadPixelsTestRects); ++rect) {
        const SkIRect& srcRect = gReadPixelsTestRects[rect];
        for (BitmapInit bmi = kFirstBitmapInit; bmi <= lastBitmapInit; bmi = nextBMI(bmi)) {
            for (size_t c = 0; c < SK_ARRAY_COUNT(gReadPixelsConfigs); ++c) {
                SkBitmap bmp;
                init_bitmap(&bmp, srcRect, bmi,
                            gReadPixelsConfigs[c].fColorType, gReadPixelsConfigs[c].fAlphaType);

                // if the bitmap has pixels allocated before the readPixels,
                // note that and fill them with pattern
                bool startsWithPixels = !bmp.isNull();
                if (startsWithPixels) {
                    fill_dst_bmp_with_init_data(&bmp);
                }
                uint32_t idBefore = surface->generationID();
                bool success = surface->readPixels(bmp, srcRect.fLeft, srcRect.fTop);
                uint32_t idAfter = surface->generationID();

                // we expect to succeed when the read isn't fully clipped out and the infos are
                // compatible.
                bool expectSuccess = read_should_succeed(srcRect, bmp.info(), surfaceInfo);
                // determine whether we expected the read to succeed.
                REPORTER_ASSERT(reporter, expectSuccess == success,
                                "Read succeed=%d unexpectedly, src ct/at: %d/%d, dst ct/at: %d/%d",
                                success, surfaceInfo.colorType(), surfaceInfo.alphaType(),
                                bmp.info().colorType(), bmp.info().alphaType());
                // read pixels should never change the gen id
                REPORTER_ASSERT(reporter, idBefore == idAfter);

                if (success || startsWithPixels) {
                    check_read(reporter, bmp, srcRect.fLeft, srcRect.fTop, success,
                               startsWithPixels, surfaceInfo);
                } else {
                    // if we had no pixels beforehand and the readPixels
                    // failed then our bitmap should still not have pixels
                    REPORTER_ASSERT(reporter, bmp.isNull());
                }
            }
        }
    }
}

DEF_TEST(ReadPixels, reporter) {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    auto surface(SkSurface::MakeRaster(info));
    // SW readback fails a premul check when reading back to an unaligned rowbytes.
    test_readpixels(reporter, surface, info, kLastAligned_BitmapInit);
}
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadPixels_Gpu, reporter, ctxInfo) {
    static const SkImageInfo kImageInfos[] = {
            SkImageInfo::Make(DEV_W, DEV_H, kRGBA_8888_SkColorType, kPremul_SkAlphaType),
            SkImageInfo::Make(DEV_W, DEV_H, kBGRA_8888_SkColorType, kPremul_SkAlphaType),
            SkImageInfo::Make(DEV_W, DEV_H, kRGB_888x_SkColorType, kOpaque_SkAlphaType),
            SkImageInfo::Make(DEV_W, DEV_H, kAlpha_8_SkColorType, kPremul_SkAlphaType),
    };
    for (const auto& ii : kImageInfos) {
        for (auto& origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
            sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(
                    ctxInfo.grContext(), SkBudgeted::kNo, ii, 0, origin, nullptr));
            if (!surface) {
                continue;
            }
            test_readpixels(reporter, surface, ii, kLast_BitmapInit);
        }
    }
}

static void test_readpixels_texture(skiatest::Reporter* reporter,
                                    sk_sp<GrSurfaceContext> sContext,
                                    const SkImageInfo& surfaceInfo) {
    for (size_t rect = 0; rect < SK_ARRAY_COUNT(gReadPixelsTestRects); ++rect) {
        const SkIRect& srcRect = gReadPixelsTestRects[rect];
        for (BitmapInit bmi = kFirstBitmapInit; bmi <= kLast_BitmapInit; bmi = nextBMI(bmi)) {
            for (size_t c = 0; c < SK_ARRAY_COUNT(gReadPixelsConfigs); ++c) {
                SkBitmap bmp;
                init_bitmap(&bmp, srcRect, bmi,
                            gReadPixelsConfigs[c].fColorType, gReadPixelsConfigs[c].fAlphaType);

                // if the bitmap has pixels allocated before the readPixels,
                // note that and fill them with pattern
                bool startsWithPixels = !bmp.isNull();
                // Try doing the read directly from a non-renderable texture
                if (startsWithPixels) {
                    fill_dst_bmp_with_init_data(&bmp);
                    bool success = sContext->readPixels(bmp.info(), bmp.getPixels(),
                            bmp.rowBytes(), {srcRect.fLeft, srcRect.fTop});
                    auto expectSuccess = read_should_succeed(srcRect, bmp.info(), surfaceInfo);
                    REPORTER_ASSERT(
                            reporter, expectSuccess == success,
                            "Read succeed=%d unexpectedly, src ct/at: %d/%d, dst ct/at: %d/%d",
                            success, surfaceInfo.colorType(), surfaceInfo.alphaType(),
                            bmp.info().colorType(), bmp.info().alphaType());
                    if (success) {
                        check_read(reporter, bmp, srcRect.fLeft, srcRect.fTop, success, true,
                                   surfaceInfo);
                    }
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadPixels_Texture, reporter, ctxInfo) {
    GrContext* context = ctxInfo.grContext();
    SkBitmap bmp = make_src_bitmap();

    // On the GPU we will also try reading back from a non-renderable texture.
    for (auto origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
        for (auto renderable : {GrRenderable::kNo, GrRenderable::kYes}) {
            sk_sp<GrTextureProxy> proxy = sk_gpu_test::MakeTextureProxyFromData(
                    context, renderable, DEV_W, DEV_H, bmp.colorType(), bmp.alphaType(), origin,
                    bmp.getPixels(), bmp.rowBytes());
            sk_sp<GrSurfaceContext> sContext = context->priv().makeWrappedSurfaceContext(
                    std::move(proxy), SkColorTypeToGrColorType(bmp.colorType()),
                    kPremul_SkAlphaType);
            auto info = SkImageInfo::Make(DEV_W, DEV_H, kN32_SkColorType, kPremul_SkAlphaType);
            test_readpixels_texture(reporter, std::move(sContext), info);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t kNumPixels = 5;

// The five reference pixels are: red, green, blue, white, black.
// Five is an interesting number to test because we'll exercise a full 4-wide SIMD vector
// plus a tail pixel.
static const uint32_t rgba[kNumPixels] = {
        0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF, 0xFF000000
};
static const uint32_t bgra[kNumPixels] = {
        0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFFFF, 0xFF000000
};
static const uint16_t rgb565[kNumPixels] = {
        SK_R16_MASK_IN_PLACE, SK_G16_MASK_IN_PLACE, SK_B16_MASK_IN_PLACE, 0xFFFF, 0x0
};

static const uint16_t rgba4444[kNumPixels] = { 0xF00F, 0x0F0F, 0x00FF, 0xFFFF, 0x000F };

static const uint64_t kRed      = (uint64_t) SK_Half1 <<  0;
static const uint64_t kGreen    = (uint64_t) SK_Half1 << 16;
static const uint64_t kBlue     = (uint64_t) SK_Half1 << 32;
static const uint64_t kAlpha    = (uint64_t) SK_Half1 << 48;
static const uint64_t f16[kNumPixels] = {
        kAlpha | kRed, kAlpha | kGreen, kAlpha | kBlue, kAlpha | kBlue | kGreen | kRed, kAlpha
};

static const uint8_t alpha8[kNumPixels] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
static const uint8_t gray8[kNumPixels] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

static const void* five_reference_pixels(SkColorType colorType) {
    switch (colorType) {
        case kUnknown_SkColorType:
            return nullptr;
        case kAlpha_8_SkColorType:
            return alpha8;
        case kRGB_565_SkColorType:
            return rgb565;
        case kARGB_4444_SkColorType:
            return rgba4444;
        case kRGBA_8888_SkColorType:
            return rgba;
        case kBGRA_8888_SkColorType:
            return bgra;
        case kGray_8_SkColorType:
            return gray8;
        case kRGBA_F16_SkColorType:
            return f16;
        default:
            return nullptr;
    }

    SkASSERT(false);
    return nullptr;
}

static void test_conversion(skiatest::Reporter* r, const SkImageInfo& dstInfo,
                            const SkImageInfo& srcInfo) {
    if (!SkImageInfoIsValid(srcInfo)) {
        return;
    }

    const void* srcPixels = five_reference_pixels(srcInfo.colorType());
    SkPixmap srcPixmap(srcInfo, srcPixels, srcInfo.minRowBytes());
    sk_sp<SkImage> src = SkImage::MakeFromRaster(srcPixmap, nullptr, nullptr);
    REPORTER_ASSERT(r, src);

    // Enough space for 5 pixels when color type is F16, more than enough space in other cases.
    uint64_t dstPixels[kNumPixels];
    SkPixmap dstPixmap(dstInfo, dstPixels, dstInfo.minRowBytes());
    bool success = src->readPixels(dstPixmap, 0, 0);
    REPORTER_ASSERT(r, success == SkImageInfoValidConversion(dstInfo, srcInfo));

    if (success) {
        if (kGray_8_SkColorType == srcInfo.colorType() &&
            kGray_8_SkColorType != dstInfo.colorType()) {
            // TODO: test (r,g,b) == (gray,gray,gray)?
            return;
        }

        if (kGray_8_SkColorType == dstInfo.colorType() &&
            kGray_8_SkColorType != srcInfo.colorType()) {
            // TODO: test gray = luminance?
            return;
        }

        if (kAlpha_8_SkColorType == srcInfo.colorType() &&
            kAlpha_8_SkColorType != dstInfo.colorType()) {
            // TODO: test output = black with this alpha?
            return;
        }

        REPORTER_ASSERT(r, 0 == memcmp(dstPixels, five_reference_pixels(dstInfo.colorType()),
                                       kNumPixels * SkColorTypeBytesPerPixel(dstInfo.colorType())));
    }
}

DEF_TEST(ReadPixels_ValidConversion, reporter) {
    const SkColorType kColorTypes[] = {
            kUnknown_SkColorType,
            kAlpha_8_SkColorType,
            kRGB_565_SkColorType,
            kARGB_4444_SkColorType,
            kRGBA_8888_SkColorType,
            kBGRA_8888_SkColorType,
            kGray_8_SkColorType,
            kRGBA_F16_SkColorType,
    };

    const SkAlphaType kAlphaTypes[] = {
            kUnknown_SkAlphaType,
            kOpaque_SkAlphaType,
            kPremul_SkAlphaType,
            kUnpremul_SkAlphaType,
    };

    const sk_sp<SkColorSpace> kColorSpaces[] = {
            nullptr,
            SkColorSpace::MakeSRGB(),
    };

    for (SkColorType dstCT : kColorTypes) {
        for (SkAlphaType dstAT: kAlphaTypes) {
            for (sk_sp<SkColorSpace> dstCS : kColorSpaces) {
                for (SkColorType srcCT : kColorTypes) {
                    for (SkAlphaType srcAT: kAlphaTypes) {
                        for (sk_sp<SkColorSpace> srcCS : kColorSpaces) {
                            test_conversion(reporter,
                                            SkImageInfo::Make(kNumPixels, 1, dstCT, dstAT, dstCS),
                                            SkImageInfo::Make(kNumPixels, 1, srcCT, srcAT, srcCS));
                        }
                    }
                }
            }
        }
    }
}

static int min_rgb_channel_bits(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:      return 0;
        case kAlpha_8_SkColorType:      return 8;
        case kRGB_565_SkColorType:      return 5;
        case kARGB_4444_SkColorType:    return 4;
        case kRGBA_8888_SkColorType:    return 8;
        case kRGB_888x_SkColorType:     return 8;
        case kBGRA_8888_SkColorType:    return 8;
        case kRGBA_1010102_SkColorType: return 10;
        case kRGB_101010x_SkColorType:  return 10;
        case kGray_8_SkColorType:       return 8;   // counting gray as "rgb"
        case kRGBA_F16Norm_SkColorType: return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:     return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:     return 23;  // just counting the mantissa
    }
    SK_ABORT("Unexpected color type.");
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(AsyncReadPixels, reporter, ctxInfo) {
    struct Context {
        SkPixmap* fPixmap = nullptr;
        bool fSuceeded = false;
        bool fCalled = false;
    };
    auto callback = [](SkSurface::ReleaseContext context, const void* data, size_t rowBytes) {
        auto* pm = static_cast<Context*>(context)->fPixmap;
        static_cast<Context*>(context)->fCalled = true;
        if ((static_cast<Context*>(context)->fSuceeded = SkToBool(data))) {
            auto dst = static_cast<char*>(pm->writable_addr());
            const auto* src = static_cast<const char*>(data);
            for (int y = 0; y < pm->height(); ++y, src += rowBytes, dst += pm->rowBytes()) {
                memcpy(dst, src, pm->width() * SkColorTypeBytesPerPixel(pm->colorType()));
            }
        }
    };
    for (auto origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        static constexpr int kW = 16;
        static constexpr int kH = 16;
        for (int sct = 0; sct <= kLastEnum_SkColorType; ++sct) {
            auto surfCT = static_cast<SkColorType>(sct);
            auto info = SkImageInfo::Make(kW, kH, surfCT, kPremul_SkAlphaType, nullptr);
            auto surf = SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo, info, 1,
                                                    origin, nullptr);
            if (!surf) {
                continue;
            }
            float d = std::sqrt((float)surf->width() * surf->width() +
                                (float)surf->height() * surf->height());
            for (int j = 0; j < surf->height(); ++j) {
                for (int i = 0; i < surf->width(); ++i) {
                    float r = i / (float)surf->width();
                    float g = 1.f - i / (float)surf->height();
                    float b = std::sqrt((float)i * i + (float)j * j) / d;
                    SkPaint paint;
                    paint.setColor4f(SkColor4f{r, g, b, 1.f}, nullptr);
                    surf->getCanvas()->drawRect(SkRect::MakeXYWH(i, j, 1, 1), paint);
                }
            }
            for (const auto& rect : {SkIRect::MakeWH(kW, kH),                  // full size
                                     SkIRect::MakeLTRB(1, 2, kW - 3, kH - 4),  // partial
                                     SkIRect::MakeXYWH(1, 1, 0, 0),            // empty: fail
                                     SkIRect::MakeWH(kW + 1, kH / 2)}) {       // too wide: fail
                for (int rct = 0; rct <= kLastEnum_SkColorType; ++rct) {
                    auto readCT = static_cast<SkColorType>(rct);
                    for (const sk_sp<SkColorSpace>& readCS :
                         {sk_sp<SkColorSpace>(), SkColorSpace::MakeSRGBLinear()}) {
                        SkAutoPixmapStorage result;
                        Context context;
                        context.fPixmap = &result;
                        info = SkImageInfo::Make(rect.width(), rect.height(), readCT,
                                                 kPremul_SkAlphaType, readCS);
                        result.alloc(info);
                        memset(result.writable_addr(), 0xAB, result.computeByteSize());
                        // Rescale quality and linearity don't matter since we're doing a non-
                        // scaling readback.
                        surf->asyncRescaleAndReadPixels(info, rect, SkSurface::RescaleGamma::kSrc,
                                                        kNone_SkFilterQuality, callback, &context);
                        while (!context.fCalled) {
                            ctxInfo.grContext()->checkAsyncWorkCompletion();
                        }
                        if (rect.isEmpty() || !SkIRect::MakeWH(kW, kH).contains(rect)) {
                            REPORTER_ASSERT(reporter, !context.fSuceeded);
                        }
                        if (context.fSuceeded) {
                            REPORTER_ASSERT(reporter, readCT != kUnknown_SkColorType &&
                                                      !rect.isEmpty());
                        } else {
                            // TODO: Support reading to kGray.
                            auto surfBounds = SkIRect::MakeWH(surf->width(), surf->height());
                            if (readCT != kUnknown_SkColorType && readCT != kGray_8_SkColorType &&
                                !rect.isEmpty() && surfBounds.contains(rect)) {
                                ERRORF(reporter,
                                       "Async read failed. Surf Color Type: %d, Read CT: %d,"
                                       "Rect [%d, %d, %d, %d], origin: %d, CS conversion: %d\n",
                                       surfCT, readCT, rect.fLeft, rect.fTop, rect.fRight,
                                       rect.fBottom, origin, (bool)readCS);
                            }
                            continue;
                        }
                        // We use a synchronous read as the source of truth.
                        SkAutoPixmapStorage ref;
                        ref.alloc(info);
                        memset(ref.writable_addr(), 0xCD, ref.computeByteSize());
                        if (!surf->readPixels(ref, rect.fLeft, rect.fTop)) {
                            continue;
                        }
                        // When there is no conversion, don't allow a difference.
                        float tol = 0.f;
                        if (readCS || readCT != surfCT) {
                            // When there is a conversion allow a diff of two values when no
                            // color space conversion and three otherwise. Except for alpha where
                            // we allow no difference. Allow intermediate truncation to an 8 bit per
                            // channel format.
                            int rgbBits = std::min({min_rgb_channel_bits(readCT),
                                                    min_rgb_channel_bits(surfCT), 8});

                            tol = (readCS ? 3.f : 2.f) / (1 << rgbBits);
                        }
                        const float tols[4] = {tol, tol, tol, 0};
                        auto error = std::function<ComparePixmapsErrorReporter>(
                                [&](int x, int y, const float diffs[4]) {
                                    SkASSERT(x >= 0 && y >= 0);
                                    ERRORF(reporter,
                                           "Surf Color Type: %d, Read CT: %d, Rect [%d, %d, %d, %d]"
                                           ", origin: %d, CS conversion: %d\n"
                                           "Error at %d, %d. Diff in floats: (%f, %f, %f %f)",
                                           surfCT, readCT, rect.fLeft, rect.fTop, rect.fRight,
                                           rect.fBottom, origin, (bool)readCS, x, y, diffs[0],
                                           diffs[1], diffs[2], diffs[3]);
                                });
                        compare_pixels(ref, result, tols, error);
                    }
                }
            }
        }
    }
}
