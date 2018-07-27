/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "../third_party/skcms/skcms.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"

int main(int argc, char** argv) {
    const char* image_path = argc > 1 ? argv[1] : nullptr;
    if (!image_path) {
        SkDebugf("Please pass an image to convert as the first argument to this program.\n");
        return 1;
    }

    sk_sp<SkImage> image = SkImage::MakeFromEncoded(SkData::MakeFromFileName(image_path));
    if (!image) {
        SkDebugf("Couldn't decode %s as an SkImage.\n", image_path);
        return 1;
    }

    image = image->makeRasterImage();
    if (!image) {
        SkDebugf("Converting to raster image failed.\n");
        return 1;
    }

    SkPixmap pixmap;
    if (!image->peekPixels(&pixmap)) {
        SkDebugf("We really should be able to peek raster pixels.\n");
        return 1;
    }

    SkColorSpace* src_cs = image->colorSpace() ? image->colorSpace()
                                               : sk_srgb_singleton();
    skcms_ICCProfile src_profile;
    src_cs->toProfile(&src_profile);

    const char* dst_profile_path = argc > 2 ? argv[2] : nullptr;
    skcms_ICCProfile dst_profile = *skcms_sRGB_profile();
    if (dst_profile_path) {
        sk_sp<SkData> blob = SkData::MakeFromFileName(dst_profile_path);
        if (!skcms_Parse(blob->data(), blob->size(), &dst_profile)) {
            SkDebugf("Can't parse %s as an ICC profile.\n", dst_profile_path);
            return 1;
        }
    }

    skcms_PixelFormat fmt;
    switch (pixmap.colorType()) {
        case kRGBA_8888_SkColorType: fmt = skcms_PixelFormat_RGBA_8888; break;
        case kBGRA_8888_SkColorType: fmt = skcms_PixelFormat_BGRA_8888; break;
        default:
            SkDebugf("color type %d not yet supported, imgcvt.cpp needs an update.\n",
                     pixmap.colorType());
            return 1;
    }

    if (pixmap.alphaType() != kPremul_SkAlphaType) {
        SkDebugf("not premul, that's weird.\n");
        return 1;
    }
    auto alpha = skcms_AlphaFormat_PremulAsEncoded;

    if (pixmap.rowBytes() != (size_t)pixmap.width() * pixmap.info().bytesPerPixel()) {
        SkDebugf("not a tight pixmap, need a loop here\n");
        return 1;
    }

    skcms_Transform(pixmap.addr(),          fmt,alpha, &src_profile,
                    pixmap.writable_addr(), fmt,alpha, &dst_profile,
                    pixmap.width() * pixmap.height());
    pixmap.setColorSpace(SkColorSpace::Make(dst_profile));

    sk_sp<SkImage> transformed = SkImage::MakeRasterCopy(pixmap);
    sk_sp<SkData>  png = transformed->encodeToData();

    SkFILEWStream("transformed.png").write(png->data(), png->size());

    return 0;
}
