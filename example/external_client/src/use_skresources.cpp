/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkImage.h"
#include "include/core/SkString.h"
#include "modules/skresources/include/SkResources.h"

#include <cstdio>
#include <filesystem>

// bazel run :use_skresources -- $SKIA_HOME/resources
int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s [folder]", argv[0]);
        return 1;
    }

    SkCodecs::Register(SkJpegDecoder::Decoder());
    SkCodecs::Register(SkPngDecoder::Decoder());
    auto frp = skresources::FileResourceProvider::Make(SkString(argv[1]));

    // Try to load two arbitrary files in //resources/images

    sk_sp<skresources::ImageAsset> asset = frp->loadImageAsset("images", "baby_tux.png", "");
    if (!asset) {
        printf("Could not load baby_tux.png in images subdirectory\n");
        return 1;
    }
    sk_sp<SkImage> tux = asset->getFrameData(0).image;
    if (!tux) {
        printf("Could not decode baby_tux.png in images subdirectory\n");
        return 1;
    }
    printf("Baby Tux is %d by %d pixels big\n", tux->width(), tux->height());

    asset = frp->loadImageAsset("images", "CMYK.jpg", "");
    if (!asset || !asset->getFrameData(0).image) {
        printf("Could not load/decode CMYK.jpg in images subdirectory\n");
        return 1;
    }

    sk_sp<SkImage> cmyk = asset->getFrameData(0).image;
    if (!cmyk) {
        printf("Could not decode CMYK.jpg in images subdirectory\n");
        return 1;
    }
    printf("CMYK is %d by %d pixels big\n", cmyk->width(), cmyk->height());

    return 0;
}
