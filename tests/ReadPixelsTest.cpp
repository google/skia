/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCanvas.h"
#include "SkColorPriv.h"
#include "SkColorSpace_Base.h"
#include "SkHalf.h"
#include "SkImageInfoPriv.h"
#include "SkMathPriv.h"
#include "SkSurface.h"
#include "Test.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrContextFactory.h"
#include "GrContextPriv.h"
#include "SkGr.h"
#endif

#include <initializer_list>

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
        case kRGBA_8888_SkColorType:
            r = static_cast<U8CPU>(c[0]);
            g = static_cast<U8CPU>(c[1]);
            b = static_cast<U8CPU>(c[2]);
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
static bool check_read(skiatest::Reporter* reporter,
                       const SkBitmap& bitmap,
                       int x, int y,
                       bool checkCanvasPixels,
                       bool checkBitmapPixels,
                       SkColorType ct,
                       SkAlphaType at) {
    SkASSERT(ct == bitmap.colorType() && at == bitmap.alphaType());
    SkASSERT(!bitmap.isNull());
    SkASSERT(checkCanvasPixels || checkBitmapPixels);

    int bw = bitmap.width();
    int bh = bitmap.height();

    SkIRect srcRect = SkIRect::MakeXYWH(x, y, bw, bh);
    SkIRect clippedSrcRect = DEV_RECT;
    if (!clippedSrcRect.intersect(srcRect)) {
        clippedSrcRect.setEmpty();
    }
    if (kAlpha_8_SkColorType == ct) {
        for (int by = 0; by < bh; ++by) {
            for (int bx = 0; bx < bw; ++bx) {
                int devx = bx + srcRect.fLeft;
                int devy = by + srcRect.fTop;
                const uint8_t* alpha = bitmap.getAddr8(bx, by);

                if (clippedSrcRect.contains(devx, devy)) {
                    if (checkCanvasPixels) {
                        uint8_t canvasAlpha = SkGetPackedA32(get_src_color(devx, devy));
                        if (canvasAlpha != *alpha) {
                            ERRORF(reporter, "Expected readback alpha (%d, %d) value 0x%02x, got 0x%02x. ",
                                   bx, by, canvasAlpha, *alpha);
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
                if (checkCanvasPixels) {
                    SkPMColor canvasPixel = get_src_color(devx, devy);
                    bool didPremul;
                    SkPMColor pmPixel = convert_to_pmcolor(ct, at, pixel, &didPremul);
                    if (!check_read_pixel(pmPixel, canvasPixel, didPremul)) {
                        ERRORF(reporter, "Expected readback pixel (%d, %d) value 0x%08x, got 0x%08x. "
                               "Readback was unpremul: %d", bx, by, canvasPixel, pmPixel, didPremul);
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
    { kRGBA_8888_SkColorType,   kPremul_SkAlphaType },
    { kRGBA_8888_SkColorType,   kUnpremul_SkAlphaType },
    { kBGRA_8888_SkColorType,   kPremul_SkAlphaType },
    { kBGRA_8888_SkColorType,   kUnpremul_SkAlphaType },
    { kAlpha_8_SkColorType,     kPremul_SkAlphaType },
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

static void test_readpixels(skiatest::Reporter* reporter, const sk_sp<SkSurface>& surface,
                            BitmapInit lastBitmapInit) {
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
                bool success = canvas->readPixels(bmp, srcRect.fLeft, srcRect.fTop);
                uint32_t idAfter = surface->generationID();

                // we expect to succeed when the read isn't fully clipped
                // out.
                bool expectSuccess = SkIRect::Intersects(srcRect, DEV_RECT);
                // determine whether we expected the read to succeed.
                REPORTER_ASSERT(reporter, success == expectSuccess);
                // read pixels should never change the gen id
                REPORTER_ASSERT(reporter, idBefore == idAfter);

                if (success || startsWithPixels) {
                    check_read(reporter, bmp, srcRect.fLeft, srcRect.fTop,
                               success, startsWithPixels,
                               gReadPixelsConfigs[c].fColorType, gReadPixelsConfigs[c].fAlphaType);
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
    test_readpixels(reporter, surface, kLastAligned_BitmapInit);
}
#if SK_SUPPORT_GPU
DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadPixels_Gpu, reporter, ctxInfo) {
    if (ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D9_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_GL_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES2_ContextType) {
        // skbug.com/6742 ReadPixels_Texture & _Gpu don't work with ANGLE ES2 configs
        return;
    }

    const SkImageInfo ii = SkImageInfo::MakeN32Premul(DEV_W, DEV_H);
    for (auto& origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
        sk_sp<SkSurface> surface(SkSurface::MakeRenderTarget(ctxInfo.grContext(), SkBudgeted::kNo,
                                                             ii, 0, origin, nullptr));
        test_readpixels(reporter, surface, kLast_BitmapInit);
    }
}
#endif

#if SK_SUPPORT_GPU
static void test_readpixels_texture(skiatest::Reporter* reporter,
                                    sk_sp<GrSurfaceContext> sContext) {
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
                    uint32_t flags = 0;
                    if (gReadPixelsConfigs[c].fAlphaType == kUnpremul_SkAlphaType) {
                        flags = GrContextPriv::kUnpremul_PixelOpsFlag;
                    }
                    bool success = sContext->readPixels(bmp.info(), bmp.getPixels(),
                                                        bmp.rowBytes(),
                                                        srcRect.fLeft, srcRect.fTop, flags);
                    check_read(reporter, bmp, srcRect.fLeft, srcRect.fTop,
                               success, true,
                               gReadPixelsConfigs[c].fColorType, gReadPixelsConfigs[c].fAlphaType);
                }
            }
        }
    }
}

DEF_GPUTEST_FOR_RENDERING_CONTEXTS(ReadPixels_Texture, reporter, ctxInfo) {
    if (ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D9_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_GL_ES2_ContextType ||
        ctxInfo.type() == sk_gpu_test::GrContextFactory::kANGLE_D3D11_ES2_ContextType) {
        // skbug.com/6742 ReadPixels_Texture & _Gpu don't work with ANGLE ES2 configs
        return;
    }

    GrContext* context = ctxInfo.grContext();

    SkBitmap bmp = make_src_bitmap();

    // On the GPU we will also try reading back from a non-renderable texture.
    for (auto origin : {kBottomLeft_GrSurfaceOrigin, kTopLeft_GrSurfaceOrigin}) {
        for (auto flags : {kNone_GrSurfaceFlags, kRenderTarget_GrSurfaceFlag}) {
            GrSurfaceDesc desc;
            desc.fFlags = flags;
            desc.fWidth = DEV_W;
            desc.fHeight = DEV_H;
            desc.fConfig = kSkia8888_GrPixelConfig;
            desc.fOrigin = origin;

            sk_sp<GrTextureProxy> proxy = GrSurfaceProxy::MakeDeferred(context->resourceProvider(),
                                                                       desc, SkBudgeted::kNo,
                                                                       bmp.getPixels(),
                                                                       bmp.rowBytes());

            sk_sp<GrSurfaceContext> sContext = context->contextPriv().makeWrappedSurfaceContext(
                                                                        std::move(proxy), nullptr);

            test_readpixels_texture(reporter, std::move(sContext));
        }
    }
}
#endif

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
            return nullptr; // remove me when kIndex_8 is removed from the enum
    }

    SkASSERT(false);
    return nullptr;
}

static void test_conversion(skiatest::Reporter* r, const SkImageInfo& dstInfo,
                            const SkImageInfo& srcInfo) {
    if (!SkImageInfoIsValidRenderingCS(srcInfo)) {
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
            kGray_8_SkColorType != dstInfo.colorType())
        {
            // This conversion is legal, but we won't get the "reference" pixels since we cannot
            // represent colors in kGray8.
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
                            if (kRGBA_F16_SkColorType == dstCT && dstCS) {
                                dstCS = as_CSB(dstCS)->makeLinearGamma();
                            }

                            if (kRGBA_F16_SkColorType == srcCT && srcCS) {
                                srcCS = as_CSB(srcCS)->makeLinearGamma();
                            }

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
