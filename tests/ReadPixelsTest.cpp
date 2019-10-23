/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <initializer_list>
#include "include/core/SkCanvas.h"
#include "include/core/SkSurface.h"
#include "include/effects/SkGradientShader.h"
#include "include/gpu/GrContext.h"
#include "include/private/SkColorData.h"
#include "include/private/SkHalf.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/utils/SkNWayCanvas.h"
#include "src/core/SkAutoPixmapStorage.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkMathPriv.h"
#include "src/gpu/GrContextPriv.h"
#include "src/gpu/GrImageInfo.h"
#include "tests/Test.h"
#include "tests/TestUtils.h"
#include "tools/ToolUtils.h"
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

enum class TightRowBytes : bool { kNo, kYes };

static void init_bitmap(SkBitmap* bitmap, const SkIRect& rect, TightRowBytes tightRB,
                        SkColorType ct, SkAlphaType at) {
    SkImageInfo info = SkImageInfo::Make(rect.size(), ct, at);
    size_t rowBytes = 0;
    if (tightRB == TightRowBytes::kNo) {
        rowBytes = SkAlign4((info.width() + 16) * info.bytesPerPixel());
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
                            const SkImageInfo& surfaceInfo) {
    SkCanvas* canvas = surface->getCanvas();
    fill_src_canvas(canvas);
    for (size_t rect = 0; rect < SK_ARRAY_COUNT(gReadPixelsTestRects); ++rect) {
        const SkIRect& srcRect = gReadPixelsTestRects[rect];
        for (auto tightRB : {TightRowBytes::kYes, TightRowBytes::kNo}) {
            for (size_t c = 0; c < SK_ARRAY_COUNT(gReadPixelsConfigs); ++c) {
                SkBitmap bmp;
                init_bitmap(&bmp, srcRect, tightRB, gReadPixelsConfigs[c].fColorType,
                            gReadPixelsConfigs[c].fAlphaType);

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
    test_readpixels(reporter, surface, info);
}

static void test_readpixels_texture(skiatest::Reporter* reporter,
                                    std::unique_ptr<GrSurfaceContext> sContext,
                                    const SkImageInfo& surfaceInfo) {
    for (size_t rect = 0; rect < SK_ARRAY_COUNT(gReadPixelsTestRects); ++rect) {
        const SkIRect& srcRect = gReadPixelsTestRects[rect];
        for (auto tightRB : {TightRowBytes::kYes, TightRowBytes::kNo}) {
            for (size_t c = 0; c < SK_ARRAY_COUNT(gReadPixelsConfigs); ++c) {
                SkBitmap bmp;
                init_bitmap(&bmp, srcRect, tightRB, gReadPixelsConfigs[c].fColorType,
                            gReadPixelsConfigs[c].fAlphaType);

                // if the bitmap has pixels allocated before the readPixels,
                // note that and fill them with pattern
                bool startsWithPixels = !bmp.isNull();
                // Try doing the read directly from a non-renderable texture
                if (startsWithPixels) {
                    fill_dst_bmp_with_init_data(&bmp);
                    bool success = sContext->readPixels(bmp.info(), bmp.getPixels(), bmp.rowBytes(),
                                                        {srcRect.fLeft, srcRect.fTop});
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
                    context, renderable, origin, bmp.info(), bmp.getPixels(), bmp.rowBytes());
            auto sContext = context->priv().makeWrappedSurfaceContext(
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

static constexpr int min_rgb_channel_bits(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return 0;
        case kAlpha_8_SkColorType:            return 0;
        case kA16_unorm_SkColorType:          return 0;
        case kA16_float_SkColorType:          return 0;
        case kRGB_565_SkColorType:            return 5;
        case kARGB_4444_SkColorType:          return 4;
        case kR8G8_unorm_SkColorType:         return 8;
        case kR16G16_unorm_SkColorType:       return 16;
        case kR16G16_float_SkColorType:       return 16;
        case kRGBA_8888_SkColorType:          return 8;
        case kRGB_888x_SkColorType:           return 8;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 10;
        case kRGB_101010x_SkColorType:        return 10;
        case kGray_8_SkColorType:             return 8;   // counting gray as "rgb"
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
    }
    SkUNREACHABLE;
}

static constexpr int alpha_channel_bits(SkColorType ct) {
    switch (ct) {
        case kUnknown_SkColorType:            return 0;
        case kAlpha_8_SkColorType:            return 8;
        case kA16_unorm_SkColorType:          return 16;
        case kA16_float_SkColorType:          return 16;
        case kRGB_565_SkColorType:            return 0;
        case kARGB_4444_SkColorType:          return 4;
        case kR8G8_unorm_SkColorType:         return 0;
        case kR16G16_unorm_SkColorType:       return 0;
        case kR16G16_float_SkColorType:       return 0;
        case kRGBA_8888_SkColorType:          return 8;
        case kRGB_888x_SkColorType:           return 0;
        case kBGRA_8888_SkColorType:          return 8;
        case kRGBA_1010102_SkColorType:       return 2;
        case kRGB_101010x_SkColorType:        return 0;
        case kGray_8_SkColorType:             return 0;
        case kRGBA_F16Norm_SkColorType:       return 10;  // just counting the mantissa
        case kRGBA_F16_SkColorType:           return 10;  // just counting the mantissa
        case kRGBA_F32_SkColorType:           return 23;  // just counting the mantissa
        case kR16G16B16A16_unorm_SkColorType: return 16;
    }
    SkUNREACHABLE;
}

namespace {

struct GpuReadPixelTestRules {
    // Test unpremul sources? We could omit this and detect that creating the source of the read
    // failed but having it lets us skip generating reference color data.
    bool fAllowUnpremulSrc = true;
    // Expect read function to succeed for kUnpremul?
    bool fAllowUnpremulRead = true;
    // Are reads that are overlapping but not contained by the src bounds expected to succeed?
    bool fUncontainedRectSucceeds = true;
};

// Makes a src populated with the pixmap. The src should get its image info (or equivalent) from
// the pixmap.
template <typename T> using GpuSrcFactory = T(SkPixmap&);

// Does a read from the T into the pixmap.
template <typename T> using GpuReadSrcFn = bool(const T&, const SkIVector& offset, const SkPixmap&);

}  // anonymous namespace

template <typename T>
static void gpu_read_pixels_test_driver(skiatest::Reporter* reporter,
                                        const GpuReadPixelTestRules& rules,
                                        const std::function<GpuSrcFactory<T>>& srcFactory,
                                        const std::function<GpuReadSrcFn<T>>& read) {
    // Separate this out just to give it some line width to breathe. Note 'srcPixels' should have
    // the same image info as src. We will do a converting readPixels() on it to get the data
    // to compare with the results of 'read'.
    auto runTest = [&](const T& src, const SkPixmap& srcPixels, const SkImageInfo& readInfo,
                       const SkIVector& offset) {
        const bool csConversion =
                !SkColorSpace::Equals(readInfo.colorSpace(), srcPixels.info().colorSpace());
        const auto readCT = readInfo.colorType();
        const auto readAT = readInfo.alphaType();
        const auto srcCT = srcPixels.info().colorType();
        const auto srcAT = srcPixels.info().alphaType();
        const auto rect = SkIRect::MakeWH(readInfo.width(), readInfo.height()).makeOffset(offset);
        const auto surfBounds = SkIRect::MakeWH(srcPixels.width(), srcPixels.height());
        const size_t readBpp = SkColorTypeBytesPerPixel(readCT);

        // Make the row bytes in the dst be loose for extra stress.
        const size_t dstRB = readBpp * readInfo.width() + 10 * readBpp;
        // This will make the last row tight.
        const size_t dstSize = readInfo.computeByteSize(dstRB);
        std::unique_ptr<char[]> dstData(new char[dstSize]);
        SkPixmap dstPixels(readInfo, dstData.get(), dstRB);
        // Initialize with an arbitrary value for each byte. Later we will check that only the
        // correct part of the destination gets overwritten by 'read'.
        static constexpr auto kInitialByte = static_cast<char>(0x1B);
        std::fill_n(static_cast<char*>(dstPixels.writable_addr()),
                    dstPixels.computeByteSize(),
                    kInitialByte);

        const bool success = read(src, offset, dstPixels);

        if (!SkIRect::Intersects(rect, surfBounds)) {
            REPORTER_ASSERT(reporter, !success);
        } else if (readCT == kUnknown_SkColorType) {
            REPORTER_ASSERT(reporter, !success);
        } else if (readAT == kUnknown_SkAlphaType) {
            REPORTER_ASSERT(reporter, !success);
        } else if (!rules.fUncontainedRectSucceeds && !surfBounds.contains(rect)) {
            REPORTER_ASSERT(reporter, !success);
        } else if (!rules.fAllowUnpremulRead && readAT == kUnpremul_SkAlphaType) {
            REPORTER_ASSERT(reporter, !success);
        } else if (!success) {
            // TODO: Support reading to kGray, support kRGB_101010x at all in GPU.
            if (readCT != kGray_8_SkColorType && readCT != kRGB_101010x_SkColorType) {
                ERRORF(reporter,
                       "Read failed. Src CT: %s, Src AT: %s Read CT: %s, Read AT: %s, "
                       "Rect [%d, %d, %d, %d], CS conversion: %d\n",
                       ToolUtils::colortype_name(srcCT), ToolUtils::alphatype_name(srcAT),
                       ToolUtils::colortype_name(readCT), ToolUtils::alphatype_name(readAT),
                       rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion);
            }
            return;
        }

        bool guardOk = true;
        auto guardCheck = [](char x) { return x == kInitialByte; };

        // Considering the rect we tried to read and the surface bounds figure  out which pixels in
        // both src and dst space should actually have been read and written.
        SkIRect srcReadRect;
        if (success && srcReadRect.intersect(surfBounds, rect)) {
            SkIRect dstWriteRect = srcReadRect.makeOffset(-rect.fLeft, -rect.fTop);

            // A CS conversion allows a 3 value difference and otherwise a 2 value difference. Note
            // that sometimes read back on GPU can be lossy even when there no conversion at all
            // because GPU->CPU read may go to a lower bit depth format and then be promoted back to
            // the original type. For example, GL ES cannot read to 1010102, so we go through 8888.
            float numer = (csConversion ? 3.f : 2.f);
            int rgbBits = std::min({min_rgb_channel_bits(readCT),
                                    min_rgb_channel_bits(srcCT),
                                    8});
            float tol = numer / (1 << rgbBits);
            float alphaTol = 0;
            if (readAT != kOpaque_SkAlphaType && srcAT != kOpaque_SkAlphaType) {
                const int alphaBits = std::min(alpha_channel_bits(readCT),
                                               alpha_channel_bits(srcCT));
                alphaTol = 2.f / (1 << alphaBits);
            }

            const float tols[4] = {tol, tol, tol, alphaTol};
            auto error = std::function<ComparePixmapsErrorReporter>([&](int x, int y,
                                                                        const float diffs[4]) {
                SkASSERT(x >= 0 && y >= 0);
                ERRORF(reporter,
                       "Src CT: %s, Src AT: %s, Read CT: %s, Read AT: %s, Rect [%d, %d, %d, %d]"
                       ", CS conversion: %d\n"
                       "Error at %d, %d. Diff in floats: (%f, %f, %f %f)",
                       ToolUtils::colortype_name(srcCT), ToolUtils::alphatype_name(srcAT),
                       ToolUtils::colortype_name(readCT), ToolUtils::alphatype_name(readAT),
                       rect.fLeft, rect.fTop, rect.fRight, rect.fBottom, csConversion, x, y,
                       diffs[0], diffs[1], diffs[2], diffs[3]);
            });
            SkAutoPixmapStorage ref;
            ref.alloc(readInfo.makeWH(dstWriteRect.width(), dstWriteRect.height()));
            srcPixels.readPixels(ref, srcReadRect.x(), srcReadRect.y());
            // This is the part of dstPixels that should have been updated.
            SkPixmap actual;
            SkAssertResult(dstPixels.extractSubset(&actual, dstWriteRect));
            compare_pixels(ref, actual, tols, error);

            const auto* v = dstData.get();
            const auto* end = dstData.get() + dstSize;
            guardOk = std::all_of(v, v + dstWriteRect.top() * dstPixels.rowBytes(), guardCheck);
            v += dstWriteRect.top() * dstPixels.rowBytes();
            for (int y = dstWriteRect.top(); y < dstWriteRect.bottom(); ++y) {
                guardOk |= std::all_of(v, v + dstWriteRect.left() * readBpp, guardCheck);
                auto pad = v + dstWriteRect.right() * readBpp;
                auto rowEnd = std::min(end, v + dstPixels.rowBytes());
                // min protects against reading past the end of the tight last row.
                guardOk |= std::all_of(pad, rowEnd, guardCheck);
                v = rowEnd;
            }
            guardOk |= std::all_of(v, end, guardCheck);
        } else {
            guardOk = std::all_of(dstData.get(), dstData.get() + dstSize, guardCheck);
        }
        if (!guardOk) {
            ERRORF(reporter,
                   "Result pixels modified result outside read rect [%d, %d, %d, %d]. "
                   "Src CT: %s, Read CT: %s, CS conversion: %d",
                   rect.fLeft, rect.fTop, rect.fRight, rect.fBottom,
                   ToolUtils::colortype_name(srcCT), ToolUtils::colortype_name(readCT),
                   csConversion);
        }
    };

    static constexpr int kW = 16;
    static constexpr int kH = 16;

    // Makes the reference data that is used to populate the src. Always F32 regardless of srcCT.
    auto make_ref_f32_data = [](SkAlphaType srcAT, SkColorType srcCT) {
        // Make src data in F32 with srcAT. We will convert it to each color type we test to
        // initialize the src.
        const auto refInfo =
                SkImageInfo::Make(kW, kH, kRGBA_F32_SkColorType, srcAT, SkColorSpace::MakeSRGB());
        auto refSurf = SkSurface::MakeRaster(refInfo);
        static constexpr SkPoint kPts1[] = {{0, 0}, {kW, kH}};
        static constexpr SkColor kColors1[] = {SK_ColorGREEN, SK_ColorRED};
        SkPaint paint;
        paint.setShader(
                SkGradientShader::MakeLinear(kPts1, kColors1, nullptr, 2, SkTileMode::kClamp));
        refSurf->getCanvas()->drawPaint(paint);
        static constexpr SkPoint kPts2[] = {{kW, 0}, {0, kH}};
        static constexpr SkColor kColors2[] = {SK_ColorBLUE, SK_ColorBLACK};
        paint.setShader(
                SkGradientShader::MakeLinear(kPts2, kColors2, nullptr, 2, SkTileMode::kClamp));
        paint.setBlendMode(SkBlendMode::kPlus);
        refSurf->getCanvas()->drawPaint(paint);
        // Keep everything opaque if the src alpha type is opaque. Also, there is an issue with
        // 1010102 (the only color type where the number of alpha bits is non-zero and not the
        // same as r, g, and b). Because of the different precisions the draw below can create
        // data that isn't strictly premul (e.g. alpha is 1/3 but green is .4). SW will clamp
        // r, g, b to a if the dst is premul and a different color type. GPU doesn't do this.
        // We could but 1010102 premul is kind of dubious anyway. So for now just keep the data
        // opaque.
        if (srcAT != kOpaque_SkAlphaType &&
            (srcAT == kPremul_SkAlphaType && srcCT != kRGBA_1010102_SkColorType)) {
            static constexpr SkColor kColors3[] = {SK_ColorWHITE,
                                                   SK_ColorWHITE,
                                                   0x60FFFFFF,
                                                   SK_ColorWHITE,
                                                   SK_ColorWHITE};
            static constexpr SkScalar kPos3[] = {0.f, 0.15f, 0.5f, 0.85f, 1.f};
            paint.setShader(SkGradientShader::MakeRadial({kW / 2.f, kH / 2.f}, (kW + kH) / 10.f,
                                                         kColors3, kPos3, 5, SkTileMode::kMirror));
            paint.setBlendMode(SkBlendMode::kDstIn);
            refSurf->getCanvas()->drawPaint(paint);
        }

        const auto srcInfo = SkImageInfo::Make(kW, kH, srcCT, srcAT, SkColorSpace::MakeSRGB());
        SkAutoPixmapStorage srcPixels;
        srcPixels.alloc(srcInfo);
        refSurf->readPixels(srcPixels, 0, 0);
        return std::move(srcPixels);
    };

    for (int sat = 0; sat < kLastEnum_SkAlphaType; ++sat) {
        const auto srcAT = static_cast<SkAlphaType>(sat);
        if (srcAT == kUnknown_SkAlphaType ||
            (srcAT == kUnpremul_SkAlphaType && !rules.fAllowUnpremulSrc)) {
            continue;
        }
        for (int sct = 0; sct <= kLastEnum_SkColorType; ++sct) {
            const auto srcCT = static_cast<SkColorType>(sct);
            // Note that we only currently use srcCT for a 1010102 workaround. If we remove this we
            // can also but the ref data setup above the srcCT loop.
            SkAutoPixmapStorage srcPixels = make_ref_f32_data(srcAT, srcCT);
            auto src = srcFactory(srcPixels);
            if (!src) {
                continue;
            }
            for (int rct = 0; rct <= kLastEnum_SkColorType; ++rct) {
                const auto readCT = static_cast<SkColorType>(rct);
                for (const sk_sp<SkColorSpace>& readCS :
                     {SkColorSpace::MakeSRGB(), SkColorSpace::MakeSRGBLinear()}) {
                    for (int at = 0; at <= kLastEnum_SkAlphaType; ++at) {
                        const auto readAT = static_cast<SkAlphaType>(at);
                        if (srcAT != kOpaque_SkAlphaType && readAT == kOpaque_SkAlphaType) {
                            // This doesn't make sense.
                            continue;
                        }
                        // Test full size, partial, empty, and too wide rects.
                        for (const auto& rect : {
                                     // entire thing
                                     SkIRect::MakeWH(kW, kH),
                                     // larger on all sides
                                     SkIRect::MakeLTRB(-10, -10, kW + 10, kH + 10),
                                     // fully contained
                                     SkIRect::MakeLTRB(kW / 4, kH / 4, 3 * kW / 4, 3 * kH / 4),
                                     // outside top left
                                     SkIRect::MakeLTRB(-10, -10, -1, -1),
                                     // touching top left corner
                                     SkIRect::MakeLTRB(-10, -10, 0, 0),
                                     // overlapping top left corner
                                     SkIRect::MakeLTRB(-10, -10, kW / 4, kH / 4),
                                     // overlapping top left and top right corners
                                     SkIRect::MakeLTRB(-10, -10, kW + 10, kH / 4),
                                     // touching entire top edge
                                     SkIRect::MakeLTRB(-10, -10, kW + 10, 0),
                                     // overlapping top right corner
                                     SkIRect::MakeLTRB(3 * kW / 4, -10, kW + 10, kH / 4),
                                     // contained in x, overlapping top edge
                                     SkIRect::MakeLTRB(kW / 4, -10, 3 * kW / 4, kH / 4),
                                     // outside top right corner
                                     SkIRect::MakeLTRB(kW + 1, -10, kW + 10, -1),
                                     // touching top right corner
                                     SkIRect::MakeLTRB(kW, -10, kW + 10, 0),
                                     // overlapping top left and bottom left corners
                                     SkIRect::MakeLTRB(-10, -10, kW / 4, kH + 10),
                                     // touching entire left edge
                                     SkIRect::MakeLTRB(-10, -10, 0, kH + 10),
                                     // overlapping bottom left corner
                                     SkIRect::MakeLTRB(-10, 3 * kH / 4, kW / 4, kH + 10),
                                     // contained in y, overlapping left edge
                                     SkIRect::MakeLTRB(-10, kH / 4, kW / 4, 3 * kH / 4),
                                     // outside bottom left corner
                                     SkIRect::MakeLTRB(-10, kH + 1, -1, kH + 10),
                                     // touching bottom left corner
                                     SkIRect::MakeLTRB(-10, kH, 0, kH + 10),
                                     // overlapping bottom left and bottom right corners
                                     SkIRect::MakeLTRB(-10, 3 * kH / 4, kW + 10, kH + 10),
                                     // touching entire left edge
                                     SkIRect::MakeLTRB(0, kH, kW, kH + 10),
                                     // overlapping bottom right corner
                                     SkIRect::MakeLTRB(3 * kW / 4, 3 * kH / 4, kW + 10, kH + 10),
                                     // overlapping top right and bottom right corners
                                     SkIRect::MakeLTRB(3 * kW / 4, -10, kW + 10, kH + 10),
                             }) {
                            const auto readInfo = SkImageInfo::Make(rect.width(), rect.height(),
                                                                    readCT, readAT, readCS);
                            const SkIVector offset = rect.topLeft();
                            runTest(src, srcPixels, readInfo, offset);
                        }
                    }
                }
            }
        }
    }
}

namespace {
struct AsyncContext {
    bool fCalled = false;
    std::unique_ptr<const SkSurface::AsyncReadResult> fResult;
};
}  // anonymous namespace

// Making this a lambda in the test functions caused:
//   "error: cannot compile this forwarded non-trivially copyable parameter yet"
// on x86/Win/Clang bot, referring to 'result'.
static void async_callback(void* c, std::unique_ptr<const SkSurface::AsyncReadResult> result) {
    auto context = static_cast<AsyncContext*>(c);
    context->fResult = std::move(result);
    context->fCalled = true;
};

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(AsyncReadPixels, reporter, ctxInfo) {
    using Surface = sk_sp<SkSurface>;
    auto reader = std::function<GpuReadSrcFn<Surface>>([](const Surface& surface,
                                                          const SkIVector& offset,
                                                          const SkPixmap& pixels) {
        AsyncContext context;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);

        // Rescale quality and linearity don't matter since we're doing a non-scaling readback.
        surface->asyncRescaleAndReadPixels(pixels.info(), rect, SkSurface::RescaleGamma::kSrc,
                                           kNone_SkFilterQuality, async_callback, &context);
        while (!context.fCalled) {
            surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
        }
        if (!context.fResult) {
            return false;
        }
        SkRectMemcpy(pixels.writable_addr(), pixels.rowBytes(), context.fResult->data(0),
                     context.fResult->rowBytes(0), pixels.info().minRowBytes(), pixels.height());
        return true;
    });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = false;
    rules.fAllowUnpremulRead = false;
    rules.fUncontainedRectSucceeds = false;

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto factory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.grContext(), origin](const SkPixmap& src) {
                    if (src.colorType() == kRGB_888x_SkColorType) {
                        return Surface();
                    }
                    auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, src.info(),
                                                            0, origin, nullptr);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        gpu_read_pixels_test_driver(reporter, rules, factory, reader);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(LegacyAsyncReadPixels, reporter, ctxInfo) {
    using Surface = sk_sp<SkSurface>;
    auto reader = std::function<GpuReadSrcFn<Surface>>([](const Surface& surface,
                                                          const SkIVector& offset,
                                                          const SkPixmap& pixels) {
        struct Context {
            const SkPixmap* fPixmap = nullptr;
            bool fSuceeded = false;
            bool fCalled = false;
        };
        auto callback = [](SkSurface::ReadPixelsContext c, const void* data, size_t rowBytes) {
            auto* context = static_cast<Context*>(c);
            context->fCalled = true;
            if ((context->fSuceeded = SkToBool(data))) {
                auto* pm = context->fPixmap;
                SkRectMemcpy(pm->writable_addr(), pm->rowBytes(), data, rowBytes,
                             pm->info().minRowBytes(), pm->height());
            }
        };

        Context context;
        context.fPixmap = &pixels;
        auto rect = SkIRect::MakeSize(pixels.dimensions()).makeOffset(offset);

        // Rescale quality and linearity don't matter since we're doing a non-scaling readback.
        surface->asyncRescaleAndReadPixels(pixels.info(), rect, SkSurface::RescaleGamma::kSrc,
                                           kNone_SkFilterQuality, callback, &context);
        while (!context.fCalled) {
            surface->getCanvas()->getGrContext()->checkAsyncWorkCompletion();
        }
        return context.fSuceeded;
    });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = false;
    rules.fAllowUnpremulRead = false;
    rules.fUncontainedRectSucceeds = false;

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto factory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.grContext(), origin](const SkPixmap& src) {
                    if (src.colorType() == kRGB_888x_SkColorType) {
                        return Surface();
                    }
                    auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, src.info(),
                                                            0, origin, nullptr);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        gpu_read_pixels_test_driver(reporter, rules, factory, reader);
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadPixels_Gpu, reporter, ctxInfo) {
    using Surface = sk_sp<SkSurface>;
    auto reader = std::function<GpuReadSrcFn<Surface>>(
            [](const Surface& surface, const SkIVector& offset, const SkPixmap& pixels) {
                return surface->readPixels(pixels, offset.fX, offset.fY);
            });
    GpuReadPixelTestRules rules;
    rules.fAllowUnpremulSrc = false;
    rules.fAllowUnpremulRead = true;
    rules.fUncontainedRectSucceeds = true;

    for (GrSurfaceOrigin origin : {kTopLeft_GrSurfaceOrigin, kBottomLeft_GrSurfaceOrigin}) {
        auto factory = std::function<GpuSrcFactory<Surface>>(
                [context = ctxInfo.grContext(), origin](const SkPixmap& src) {
                    if (src.colorType() == kRGB_888x_SkColorType) {
                        return Surface();
                    }
                    auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, src.info(),
                                                            0, origin, nullptr);
                    if (surf) {
                        surf->writePixels(src, 0, 0);
                    }
                    return surf;
                });
        gpu_read_pixels_test_driver(reporter, rules, factory, reader);
    }
}

DEF_GPUTEST(AsyncReadPixelsContextShutdown, reporter, options) {
    const auto ii = SkImageInfo::Make(10, 10, kRGBA_8888_SkColorType, kPremul_SkAlphaType,
                                      SkColorSpace::MakeSRGB());
    enum class ShutdownSequence {
        kFreeResult_DestroyContext,
        kDestroyContext_FreeResult,
        kFreeResult_ReleaseAndAbandon_DestroyContext,
        kFreeResult_Abandon_DestroyContext,
        kReleaseAndAbandon_FreeResult_DestroyContext,
        kAbandon_FreeResult_DestroyContext,
        kReleaseAndAbandon_DestroyContext_FreeResult,
        kAbandon_DestroyContext_FreeResult,
    };
    for (int t = 0; t < sk_gpu_test::GrContextFactory::kContextTypeCnt; ++t) {
        auto type = static_cast<sk_gpu_test::GrContextFactory::ContextType>(t);
        for (auto sequence : {ShutdownSequence::kFreeResult_DestroyContext,
                              ShutdownSequence::kDestroyContext_FreeResult,
                              ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext,
                              ShutdownSequence::kFreeResult_Abandon_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kAbandon_FreeResult_DestroyContext,
                              ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult,
                              ShutdownSequence::kAbandon_DestroyContext_FreeResult}) {
            // Vulkan context abandoning without resource release has issues outside of the scope of
            // this test.
            if (type == sk_gpu_test::GrContextFactory::kVulkan_ContextType &&
                (sequence == ShutdownSequence::kAbandon_FreeResult_DestroyContext ||
                 sequence == ShutdownSequence::kAbandon_DestroyContext_FreeResult ||
                 sequence == ShutdownSequence::kFreeResult_Abandon_DestroyContext)) {
                continue;
            }
            for (bool yuv : {false, true}) {
                sk_gpu_test::GrContextFactory factory(options);
                auto context = factory.get(type);
                if (!context) {
                    continue;
                }
                // This test is only meaningful for contexts that support transfer buffers.
                if (!context->priv().caps()->transferBufferSupport()) {
                    continue;
                }
                auto surf = SkSurface::MakeRenderTarget(context, SkBudgeted::kYes, ii, 1, nullptr);
                if (!surf) {
                    continue;
                }
                AsyncContext cbContext;
                if (yuv) {
                    surf->asyncRescaleAndReadPixelsYUV420(
                            kIdentity_SkYUVColorSpace, SkColorSpace::MakeSRGB(), ii.bounds(),
                            ii.dimensions(), SkSurface::RescaleGamma::kSrc, kNone_SkFilterQuality,
                            &async_callback, &cbContext);
                } else {
                    surf->asyncRescaleAndReadPixels(ii, ii.bounds(), SkSurface::RescaleGamma::kSrc,
                                                    kNone_SkFilterQuality, &async_callback,
                                                    &cbContext);
                }
                while (!cbContext.fCalled) {
                    context->checkAsyncWorkCompletion();
                }
                if (!cbContext.fResult) {
                    ERRORF(reporter, "Callback failed on %s. is YUV: %d",
                           sk_gpu_test::GrContextFactory::ContextTypeName(type), yuv);
                    continue;
                }
                // The real test is that we don't crash, get Vulkan validation errors, etc, during
                // this shutdown sequence.
                switch (sequence) {
                    case ShutdownSequence::kFreeResult_DestroyContext:
                    case ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext:
                    case ShutdownSequence::kFreeResult_Abandon_DestroyContext:
                        break;
                    case ShutdownSequence::kDestroyContext_FreeResult:
                        factory.destroyContexts();
                        break;
                    case ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext:
                        factory.releaseResourcesAndAbandonContexts();
                        break;
                    case ShutdownSequence::kAbandon_FreeResult_DestroyContext:
                        factory.abandonContexts();
                        break;
                    case ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult:
                        factory.releaseResourcesAndAbandonContexts();
                        factory.destroyContexts();
                        break;
                    case ShutdownSequence::kAbandon_DestroyContext_FreeResult:
                        factory.abandonContexts();
                        factory.destroyContexts();
                        break;
                }
                cbContext.fResult.reset();
                switch (sequence) {
                    case ShutdownSequence::kFreeResult_ReleaseAndAbandon_DestroyContext:
                        factory.releaseResourcesAndAbandonContexts();
                        break;
                    case ShutdownSequence::kFreeResult_Abandon_DestroyContext:
                        factory.abandonContexts();
                        break;
                    case ShutdownSequence::kFreeResult_DestroyContext:
                    case ShutdownSequence::kDestroyContext_FreeResult:
                    case ShutdownSequence::kReleaseAndAbandon_FreeResult_DestroyContext:
                    case ShutdownSequence::kAbandon_FreeResult_DestroyContext:
                    case ShutdownSequence::kReleaseAndAbandon_DestroyContext_FreeResult:
                    case ShutdownSequence::kAbandon_DestroyContext_FreeResult:
                        break;
                }
            }
        }
    }
}
