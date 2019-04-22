/*
* Copyright 2018 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#include "skcms.h"
#include "SkCanvas.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkImage.h"
#include "SkStream.h"
#include "SkSurface.h"

static void write_png(const char* path, sk_sp<SkImage> img) {
    sk_sp<SkData>  png = img->encodeToData();
    SkFILEWStream(path).write(png->data(), png->size());
}

int main(int argc, char** argv) {
    const char* source_path = argc > 1 ? argv[1] : nullptr;
    if (!source_path) {
        SkDebugf("Please pass an image or profile to convert"
                 " as the first argument to this program.\n");
        return 1;
    }

    const char* dst_profile_path = argc > 2 ? argv[2] : nullptr;
    skcms_ICCProfile dst_profile = *skcms_sRGB_profile();
    sk_sp<SkData> dst_blob;
    if (dst_profile_path) {
        dst_blob = SkData::MakeFromFileName(dst_profile_path);
        if (!skcms_Parse(dst_blob->data(), dst_blob->size(), &dst_profile)) {
            SkDebugf("Can't parse %s as an ICC profile.\n", dst_profile_path);
            return 1;
        }
    }

    auto blob = SkData::MakeFromFileName(source_path);

    skcms_ICCProfile src_profile;
    if (skcms_Parse(blob->data(), blob->size(), &src_profile)) {
        // Transform white, black, primaries, and primary complements.
        float src[] = {
           0,0,0,
           1,1,1,

           1,0,0,
           0,1,0,
           0,0,1,

           0,1,1,
           1,0,1,
           1,1,0,
        };
        float dst[24] = {0};

        if (!skcms_Transform(
                    src, skcms_PixelFormat_RGB_fff, skcms_AlphaFormat_Unpremul, &src_profile,
                    dst, skcms_PixelFormat_RGB_fff, skcms_AlphaFormat_Unpremul, &dst_profile,
                    8)) {
            SkDebugf("Cannot transform.\n");
            return 1;
        }
        for (int i = 0; i < 8; i++) {
            SkDebugf("(%g, %g, %g) --> (%+.4f, %+.4f, %+.4f)\n",
                     src[3*i+0], src[3*i+1], src[3*i+2],
                     dst[3*i+0], dst[3*i+1], dst[3*i+2]);
        }
        return 0;
    }

    sk_sp<SkImage> image = SkImage::MakeFromEncoded(blob);
    if (!image) {
        SkDebugf("Couldn't decode %s as an SkImage or an ICC profile.\n", source_path);
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

    sk_sp<SkColorSpace> dst_cs = SkColorSpace::Make(dst_profile);
    if (!dst_cs) {
        SkDebugf("We can't convert to this destination profile as-is. Coercing it.\n");
        if (skcms_MakeUsableAsDestinationWithSingleCurve(&dst_profile)) {
            dst_cs = SkColorSpace::Make(dst_profile);
        }
        if (!dst_cs) {
            SkDebugf("We can't convert to this destination profile at all.\n");
            return 1;
        }
    }

    { // transform with skcms
        SkColorSpace* src_cs = image->colorSpace() ? image->colorSpace()
                                                   : sk_srgb_singleton();
        src_cs->toProfile(&src_profile);

        skcms_PixelFormat fmt;
        switch (pixmap.colorType()) {
            case kRGBA_8888_SkColorType: fmt = skcms_PixelFormat_RGBA_8888; break;
            case kBGRA_8888_SkColorType: fmt = skcms_PixelFormat_BGRA_8888; break;
            default:
                SkDebugf("color type %d not yet supported, imgcvt.cpp needs an update.\n",
                         pixmap.colorType());
                return 1;
        }

        if (pixmap.alphaType() == kUnpremul_SkAlphaType) {
            SkDebugf("not premul, that's weird.\n");
            return 1;
        }
        auto alpha = skcms_AlphaFormat_PremulAsEncoded;

        if (pixmap.rowBytes() != (size_t)pixmap.width() * pixmap.info().bytesPerPixel()) {
            SkDebugf("not a tight pixmap, need a loop here\n");
            return 1;
        }

        if (!skcms_Transform(pixmap.addr(),          fmt,alpha, &src_profile,
                             pixmap.writable_addr(), fmt,alpha, &dst_profile,
                             pixmap.width() * pixmap.height())) {
            SkDebugf("skcms_Transform() failed\n");
            return 1;
        }
        pixmap.setColorSpace(dst_cs);

        write_png("transformed-skcms.png", SkImage::MakeRasterCopy(pixmap));
    }

    { // transform with writePixels()
        sk_sp<SkSurface> surface = SkSurface::MakeRaster(pixmap.info().makeColorSpace(dst_cs));
        if (!surface) {
            SkDebugf("couldn't create a surface\n");
            return 1;
        }

        surface->writePixels(pixmap, 0,0);

        write_png("transformed-writepixels.png", surface->makeImageSnapshot());
    }

    { // transform by drawing
        sk_sp<SkSurface> surface = SkSurface::MakeRaster(pixmap.info().makeColorSpace(dst_cs));
        if (!surface) {
            SkDebugf("couldn't create a surface\n");
            return 1;
        }

        surface->getCanvas()->drawImage(image, 0,0);

        write_png("transformed-draw.png", surface->makeImageSnapshot());
    }

    return 0;
}
