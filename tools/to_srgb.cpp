// Copyright 2019 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkData.h"
#include "SkPngEncoder.h"
#include "SkStream.h"

#include <cstdio>

static bool decode(const char* path, SkBitmap* dst) {
    auto codec = SkCodec::MakeFromData(SkData::MakeFromFileName(path));
    if (!codec) {
        return false;
    }
    SkISize size = codec->getInfo().dimensions();
    SkASSERT(!size.isEmpty());
    dst->allocPixels(SkImageInfo::Make(
                size.width(), size.height(), kRGBA_8888_SkColorType,
                kUnpremul_SkAlphaType, SkColorSpace::MakeSRGB()));
    return SkCodec::kSuccess == codec->getPixels(dst->pixmap());
}

// For each path in the command-line arguments, reencode as RGBA_8888_sRGB PNG
// mas
// with the same name.
int main(int argc, char** argv) {
    SkPngEncoder::Options encOpts;
    encOpts.fFilterFlags = SkPngEncoder::FilterFlag::kNone;
    encOpts.fZLibLevel = 1;  // fast encode;
    bool good = true;
    for (int i = 1; i < argc; ++i) {
        const char* path = argv[i];
        SkBitmap bitmap;
        if (!decode(path, &bitmap)) {
            fprintf(stderr, "Error decoding '%s'\n", path);
            good = false;
            continue;
        }
        SkFILEWStream o(path);
        if (!SkPngEncoder::Encode(&o, bitmap.pixmap(), encOpts)) {
            good = false;
            fprintf(stderr, "Error encoding '%s'\n", path);
        }
    }
    return good ? 0 : 1;
}
