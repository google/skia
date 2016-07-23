/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sk_tool_utils.h"
#include "SkSurface.h"
#include "Resources.h"
#include "gm.h"

#include "SkMath.h"
#include "SkColorPriv.h"

static SkBitmap copy_bitmap(const SkBitmap& src, SkColorType colorType) {
    SkBitmap copy;
    src.copyTo(&copy, colorType);
    copy.setImmutable();
    return copy;
}

#define SCALE 128

// Make either A8 or gray8 bitmap.
static SkBitmap make_bitmap(SkColorType ct) {
    SkBitmap bm;
    switch (ct) {
        case kAlpha_8_SkColorType:
            bm.allocPixels(SkImageInfo::MakeA8(SCALE, SCALE));
            break;
        case kGray_8_SkColorType:
            bm.allocPixels(
                    SkImageInfo::Make(SCALE, SCALE, ct, kOpaque_SkAlphaType));
            break;
        default:
            SkASSERT(false);
            return bm;
    }
    SkAutoLockPixels autoLockPixels(bm);
    uint8_t spectrum[256];
    for (int y = 0; y < 256; ++y) {
        spectrum[y] = y;
    }
    for (int y = 0; y < 128; ++y) {
        // Shift over one byte each scanline.
        memcpy(bm.getAddr8(0, y), &spectrum[y], 128);
    }
    bm.setImmutable();
    return bm;
}

static void draw_center_letter(char c,
                               SkPaint* p,
                               SkColor color,
                               SkScalar x,
                               SkScalar y,
                               SkCanvas* canvas) {
    SkRect bounds;
    p->setColor(color);
    p->measureText(&c, 1, &bounds);
    canvas->drawText(&c, 1, x - bounds.centerX(), y - bounds.centerY(), *p);
}

static void color_wheel_native(SkCanvas* canvas) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    canvas->translate(0.5f * SCALE, 0.5f * SCALE);
    SkPaint p;
    p.setAntiAlias(false);
    p.setColor(SK_ColorWHITE);
    canvas->drawCircle(0.0f, 0.0f, SCALE * 0.5f, p);

    const double sqrt_3_over_2 = 0.8660254037844387;
    const SkScalar Z = 0.0f;
    const SkScalar D = 0.3f * SkIntToScalar(SCALE);
    const SkScalar X = SkDoubleToScalar(D * sqrt_3_over_2);
    const SkScalar Y = D * SK_ScalarHalf;
    sk_tool_utils::set_portable_typeface(&p, nullptr, SkFontStyle::FromOldStyle(SkTypeface::kBold));
    p.setTextSize(0.28125f * SCALE);
    draw_center_letter('K', &p, SK_ColorBLACK, Z, Z, canvas);
    draw_center_letter('R', &p, SK_ColorRED, Z, D, canvas);
    draw_center_letter('G', &p, SK_ColorGREEN, -X, -Y, canvas);
    draw_center_letter('B', &p, SK_ColorBLUE, X, -Y, canvas);
    draw_center_letter('C', &p, SK_ColorCYAN, Z, -D, canvas);
    draw_center_letter('M', &p, SK_ColorMAGENTA, X, Y, canvas);
    draw_center_letter('Y', &p, SK_ColorYELLOW, -X, Y, canvas);
}

template <typename T>
int find(T* array, int N, T item) {
    for (int i = 0; i < N; ++i) {
        if (array[i] == item) {
            return i;
        }
    }
    return -1;
}

static SkPMColor premultiply_color(SkColor c) {
    return SkPremultiplyARGBInline(SkColorGetA(c), SkColorGetR(c),
                                   SkColorGetG(c), SkColorGetB(c));
}

static SkBitmap indexed_bitmap() {
    SkBitmap n32bitmap;
    n32bitmap.allocN32Pixels(SCALE, SCALE);
    n32bitmap.eraseColor(SK_ColorTRANSPARENT);

    SkCanvas canvas(n32bitmap);
    color_wheel_native(&canvas);
    const SkColor colors[] = {
            SK_ColorTRANSPARENT,
            SK_ColorWHITE,
            SK_ColorBLACK,
            SK_ColorRED,
            SK_ColorGREEN,
            SK_ColorBLUE,
            SK_ColorCYAN,
            SK_ColorMAGENTA,
            SK_ColorYELLOW,
    };
    SkPMColor pmColors[SK_ARRAY_COUNT(colors)];
    for (size_t i = 0; i < SK_ARRAY_COUNT(colors); ++i) {
        pmColors[i] = premultiply_color(colors[i]);
    }
    SkBitmap bm;
    SkAutoTUnref<SkColorTable> ctable(new SkColorTable(pmColors, SK_ARRAY_COUNT(pmColors)));
    SkImageInfo info = SkImageInfo::Make(SCALE, SCALE, kIndex_8_SkColorType,
                                         kPremul_SkAlphaType);
    bm.allocPixels(info, nullptr, ctable);
    SkAutoLockPixels autoLockPixels1(n32bitmap);
    SkAutoLockPixels autoLockPixels2(bm);
    for (int y = 0; y < SCALE; ++y) {
        for (int x = 0; x < SCALE; ++x) {
            SkPMColor c = *n32bitmap.getAddr32(x, y);
            int idx = find(pmColors, SK_ARRAY_COUNT(pmColors), c);
            *bm.getAddr8(x, y) = SkClampMax(idx, SK_ARRAY_COUNT(pmColors) - 1);
        }
    }
    return bm;
}

static void draw(SkCanvas* canvas,
                 const SkPaint& p,
                 const SkBitmap& src,
                 SkColorType colorType,
                 const char text[]) {
    SkASSERT(src.colorType() == colorType);
    canvas->drawBitmap(src, 0.0f, 0.0f);
    canvas->drawText(text, strlen(text), 0.0f, 12.0f, p);
}

DEF_SIMPLE_GM(all_bitmap_configs, canvas, SCALE, 6 * SCALE) {
    SkAutoCanvasRestore autoCanvasRestore(canvas, true);
    SkPaint p;
    p.setColor(SK_ColorBLACK);
    p.setAntiAlias(true);
    sk_tool_utils::set_portable_typeface(&p, nullptr);

    sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorWHITE, 8);

    SkBitmap bitmap;
    if (GetResourceAsBitmap("color_wheel.png", &bitmap)) {
        bitmap.setImmutable();
        draw(canvas, p, bitmap, kN32_SkColorType, "Native 32");

        canvas->translate(0.0f, SkIntToScalar(SCALE));
        SkBitmap copy565 = copy_bitmap(bitmap, kRGB_565_SkColorType);
        p.setColor(SK_ColorRED);
        draw(canvas, p, copy565, kRGB_565_SkColorType, "RGB 565");
        p.setColor(SK_ColorBLACK);

        canvas->translate(0.0f, SkIntToScalar(SCALE));
        SkBitmap copy4444 = copy_bitmap(bitmap, kARGB_4444_SkColorType);
        draw(canvas, p, copy4444, kARGB_4444_SkColorType, "ARGB 4444");
    } else {
        canvas->translate(0.0f, SkIntToScalar(2 * SCALE));
    }

    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapIndexed = indexed_bitmap();
    draw(canvas, p, bitmapIndexed, kIndex_8_SkColorType, "Index 8");

    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapA8 = make_bitmap(kAlpha_8_SkColorType);
    draw(canvas, p, bitmapA8, kAlpha_8_SkColorType, "Alpha 8");

    p.setColor(SK_ColorRED);
    canvas->translate(0.0f, SkIntToScalar(SCALE));
    SkBitmap bitmapG8 = make_bitmap(kGray_8_SkColorType);
    draw(canvas, p, bitmapG8, kGray_8_SkColorType, "Gray 8");
}

sk_sp<SkImage> make_not_native32_color_wheel() {
    SkBitmap n32bitmap, notN32bitmap;
    n32bitmap.allocN32Pixels(SCALE, SCALE);
    n32bitmap.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas n32canvas(n32bitmap);
    color_wheel_native(&n32canvas);
    n32canvas.flush();
    #if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
        const SkColorType ct = kRGBA_8888_SkColorType;
    #elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
        const SkColorType ct = kBGRA_8888_SkColorType;
    #endif
    static_assert(ct != kN32_SkColorType, "BRGA!=RGBA");
    SkAssertResult(n32bitmap.copyTo(&notN32bitmap, ct));
    SkASSERT(notN32bitmap.colorType() == ct);
    return SkImage::MakeFromBitmap(notN32bitmap);
}

DEF_SIMPLE_GM(not_native32_bitmap_config, canvas, SCALE, SCALE) {
    sk_sp<SkImage> notN32image(make_not_native32_color_wheel());
    SkASSERT(notN32image);
    sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorWHITE, 8);
    canvas->drawImage(notN32image.get(), 0.0f, 0.0f);
}

static uint32_t make_pixel(int x, int y, SkAlphaType alphaType) {
    SkASSERT(x >= 0 && x < SCALE);
    SkASSERT(y >= 0 && y < SCALE);

    SkScalar R = SCALE / 2.0f;

    uint32_t alpha = 0x00;

    if ((x - R) * (x - R) + (y - R) * (y - R) < R * R) {
        alpha = 0xFF;
    }

    uint32_t component;
    switch (alphaType) {
        case kPremul_SkAlphaType:
            component = alpha;
            break;
        case kUnpremul_SkAlphaType:
            component = 0xFF;
            break;
        default:
            SkFAIL("Should not get here - invalid alpha type");
            return 0xFF000000;
    }
    return alpha << 24 | component;
}

static void make_color_test_bitmap_variant(
    SkColorType colorType,
    SkAlphaType alphaType,
    sk_sp<SkColorSpace> colorSpace,
    SkBitmap* bm)
{
    SkASSERT(colorType == kRGBA_8888_SkColorType || colorType == kBGRA_8888_SkColorType);
    SkASSERT(alphaType == kPremul_SkAlphaType || alphaType == kUnpremul_SkAlphaType);
    bm->allocPixels(
        SkImageInfo::Make(SCALE, SCALE, colorType, alphaType, colorSpace));
    SkPixmap pm;
    bm->peekPixels(&pm);
    for (int y = 0; y < bm->height(); y++) {
        for (int x = 0; x < bm->width(); x++) {
            *pm.writable_addr32(x, y) = make_pixel(x, y, alphaType);
        }
    }
}

DEF_SIMPLE_GM(all_variants_8888, canvas, 4 * SCALE + 30, 2 * SCALE + 10) {
    sk_tool_utils::draw_checkerboard(canvas, SK_ColorLTGRAY, SK_ColorWHITE, 8);

    sk_sp<SkColorSpace> colorSpaces[] {
        SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named),
        nullptr,
    };
    for (auto colorSpace : colorSpaces) {
        canvas->save();
        for (auto alphaType : {kPremul_SkAlphaType, kUnpremul_SkAlphaType}) {
            canvas->save();
            for (auto colorType : {kRGBA_8888_SkColorType, kBGRA_8888_SkColorType}) {
                SkBitmap bm;
                make_color_test_bitmap_variant(colorType, alphaType, colorSpace, &bm);
                canvas->drawBitmap(bm, 0.0f, 0.0f);
                canvas->translate(SCALE + 10, 0.0f);
            }
            canvas->restore();
            canvas->translate(0.0f, SCALE + 10);
        }
        canvas->restore();
        canvas->translate(2 * (SCALE + 10), 0.0f);
    }
}
