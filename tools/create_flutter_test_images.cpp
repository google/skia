/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpacePriv.h"
#include "SkImage.h"
#include "SkPngEncoder.h"

#include "Resources.h"

/**
 *  Create a color space that swaps the red, green, and blue channels.
 */
static sk_sp<SkColorSpace> gbr_color_space() {
    float gbr[9];
    gbr[0] = gSRGB_toXYZD50[1];
    gbr[1] = gSRGB_toXYZD50[2];
    gbr[2] = gSRGB_toXYZD50[0];
    gbr[3] = gSRGB_toXYZD50[4];
    gbr[4] = gSRGB_toXYZD50[5];
    gbr[5] = gSRGB_toXYZD50[3];
    gbr[6] = gSRGB_toXYZD50[7];
    gbr[7] = gSRGB_toXYZD50[8];
    gbr[8] = gSRGB_toXYZD50[6];
    SkMatrix44 toXYZD50(SkMatrix44::kUninitialized_Constructor);
    toXYZD50.set3x3RowMajorf(gbr);
    return SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, toXYZD50);
}

/**
 *  Create a color space with a steep transfer function.
 */
static sk_sp<SkColorSpace> tf_color_space() {
    SkColorSpaceTransferFn fn;
    fn.fA = 1.f; fn.fB = 0.f; fn.fC = 0.f; fn.fD = 0.f; fn.fE = 0.f; fn.fF = 0.f; fn.fG = 50.f;
    return SkColorSpace::MakeRGB(fn, SkColorSpace::kSRGB_Gamut);
}

/**
 *  Create a wide gamut color space.
 */
static sk_sp<SkColorSpace> wide_gamut_color_space() {
    return SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                 SkColorSpace::kRec2020_Gamut);
}

int main(int argc, char** argv) {
    sk_sp<SkImage> image = GetResourceAsImage("flutter_logo.jpg");
    if (!image) {
        SkDebugf("Cannot find flutter_logo.jpg in resources.\n");
        return 1;
    }

    // Encode an image with a gbr color space.
    SkImageInfo info = SkImageInfo::MakeN32(image->width(), image->height(), kOpaque_SkAlphaType,
                                            gbr_color_space());
    size_t rowBytes = info.minRowBytes();
    SkAutoTMalloc<uint8_t> storage(rowBytes * image->height());
    SkPixmap src(info, storage.get(), rowBytes);
    image->readPixels(src, 0, 0, SkImage::kDisallow_CachingHint);
    SkFILEWStream dst0("gbr.png");
    SkPngEncoder::Options opts;
    opts.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore; // Does not matter for opaque src
    SkAssertResult(SkPngEncoder::Encode(&dst0, src, opts));

    // Encode an image with steep transfer function.
    src.setColorSpace(tf_color_space());
    image->readPixels(src, 0, 0, SkImage::kDisallow_CachingHint);
    SkFILEWStream dst1("tf.png");
    SkAssertResult(SkPngEncoder::Encode(&dst1, src, opts));

    // Encode a wide gamut image.
    src.setColorSpace(wide_gamut_color_space());
    image->readPixels(src, 0, 0, SkImage::kDisallow_CachingHint);
    SkFILEWStream dst2("wide-gamut.png");
    SkAssertResult(SkPngEncoder::Encode(&dst2, src, opts));

    return 0;
}
