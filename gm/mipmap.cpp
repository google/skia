/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gm/gm.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkFilterQuality.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"

static sk_sp<SkImage> make_image() {
    const SkImageInfo info = SkImageInfo::MakeN32Premul(319, 52);
    auto surface(SkSurface::MakeRaster(info));
    SkCanvas* canvas = surface->getCanvas();
    canvas->drawColor(0xFFF8F8F8);

    SkPaint paint;
    paint.setAntiAlias(true);

    paint.setStyle(SkPaint::kStroke_Style);
    for (int i = 0; i < 20; ++i) {
        canvas->drawCircle(-4, 25, 20, paint);
        canvas->translate(25, 0);
    }
    return surface->makeImageSnapshot();
}

DEF_SIMPLE_GM(mipmap, canvas, 400, 200) {
    sk_sp<SkImage> img(make_image());//SkImage::NewFromEncoded(data));

    SkPaint paint;
    const SkRect dst = SkRect::MakeWH(177, 15);

    SkString str;
    str.printf("scale %g %g", dst.width() / img->width(), dst.height() / img->height());
//    canvas->drawString(str, 300, 100, SkFont(nullptr, 30), paint);

    canvas->translate(20, 20);
    for (int i = 0; i < 4; ++i) {
        paint.setFilterQuality(SkFilterQuality(i));
        canvas->drawImageRect(img.get(), dst, &paint);
        canvas->translate(0, 20);
    }
    canvas->drawImage(img.get(), 20, 20, nullptr);
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// create a circle image computed raw, so we can wrap it as a linear or srgb image
static sk_sp<SkImage> make(sk_sp<SkColorSpace> cs) {
    const int N = 100;
    SkImageInfo info = SkImageInfo::Make(N, N, kN32_SkColorType, kPremul_SkAlphaType, cs);
    SkBitmap bm;
    bm.allocPixels(info);

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            *bm.getAddr32(x, y) = (x ^ y) & 1 ? 0xFFFFFFFF : 0xFF000000;
        }
    }
    bm.setImmutable();
    return SkImage::MakeFromBitmap(bm);
}

static void show_mips(SkCanvas* canvas, SkImage* img) {
    SkPaint paint;
    paint.setFilterQuality(kMedium_SkFilterQuality);

    // Want to ensure we never draw fractional pixels, so we use an IRect
    SkIRect dst = SkIRect::MakeWH(img->width(), img->height());
    while (dst.width() > 5) {
        canvas->drawImageRect(img, SkRect::Make(dst), &paint);
        dst.offset(dst.width() + 10, 0);
        dst.fRight = dst.fLeft + dst.width()/2;
        dst.fBottom = dst.fTop + dst.height()/2;
    }
}

/*
 *  Ensure that in L32 drawing mode, both images/mips look the same as each other, and
 *  their mips are darker than the original (since the mips should ignore the gamma in L32).
 *
 *  Ensure that in S32 drawing mode, all images/mips look the same, and look correct (i.e.
 *  the mip levels match the original in brightness).
 */
DEF_SIMPLE_GM(mipmap_srgb, canvas, 260, 230) {
    sk_sp<SkImage> limg = make(nullptr);
    sk_sp<SkImage> simg = make(SkColorSpace::MakeSRGB());

    canvas->translate(10, 10);
    show_mips(canvas, limg.get());
    canvas->translate(0, limg->height() + 10.0f);
    show_mips(canvas, simg.get());
}

///////////////////////////////////////////////////////////////////////////////////////////////////

// create a gradient image computed raw, so we can wrap it as a linear or srgb image
static sk_sp<SkImage> make_g8_gradient(sk_sp<SkColorSpace> cs) {
    const int N = 100;
    SkImageInfo info = SkImageInfo::Make(N, N, kGray_8_SkColorType, kOpaque_SkAlphaType, cs);
    SkBitmap bm;
    bm.allocPixels(info);

    for (int y = 0; y < N; ++y) {
        for (int x = 0; x < N; ++x) {
            *bm.getAddr8(x, y) = static_cast<uint8_t>(255.0f * ((x + y) / (2.0f * (N - 1))));
        }
    }
    bm.setImmutable();
    return SkImage::MakeFromBitmap(bm);
}

static void show_mips_only(SkCanvas* canvas, SkImage* img) {
    SkPaint paint;
    paint.setFilterQuality(kMedium_SkFilterQuality);

    // Want to ensure we never draw fractional pixels, so we use an IRect
    SkIRect dst = SkIRect::MakeWH(img->width() / 2, img->height() / 2);
    while (dst.width() > 5) {
        canvas->drawImageRect(img, SkRect::Make(dst), &paint);
        dst.offset(dst.width() + 10, 0);
        dst.fRight = dst.fLeft + dst.width() / 2;
        dst.fBottom = dst.fTop + dst.height() / 2;
    }
}

/*
 *  Ensure that in L32 drawing mode, both images/mips look the same as each other, and
 *  their mips are darker than the original (since the mips should ignore the gamma in L32).
 *
 *  Ensure that in S32 drawing mode, all images/mips look the same, and look correct (i.e.
 *  the mip levels match the original in brightness).
 */
DEF_SIMPLE_GM(mipmap_gray8_srgb, canvas, 260, 230) {
    sk_sp<SkImage> limg = make_g8_gradient(nullptr);
    sk_sp<SkImage> simg = make_g8_gradient(SkColorSpace::MakeSRGB());

    canvas->translate(10, 10);
    show_mips_only(canvas, limg.get());
    canvas->translate(0, limg->height() + 10.0f);
    show_mips_only(canvas, simg.get());
}
