/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "png_interface.h"

#include <cassert>

#include "SkCodec.h"
#include "SkPngEncoder.h"
#include "SkStream.h"

namespace png_interface {

bool WritePngRgba8888ToFile(int width, int height, const uint32_t* pixels, const char* path) {
    SkImageInfo info = SkImageInfo::Make(width, height,
                                         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    SkPixmap pm(info, pixels, width * sizeof(uint32_t));
    SkFILEWStream wStream(path);
    SkPngEncoder::Options options;
    options.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return wStream.isValid() && SkPngEncoder::Encode(&wStream, pm, options);
}

Image ReadPngRgba8888FromFile(const char* path) {
    Image image{0, 0, std::vector<uint32_t>()};
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(SkData::MakeFromFileName(path));
    if (codec) {
        constexpr SkColorType ct = kRGBA_8888_SkColorType;
        constexpr SkAlphaType at = kUnpremul_SkAlphaType;
        SkISize size = codec->getInfo().dimensions();
        assert(!size.isEmpty());
        SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), ct, at);
        size_t rowBytes = size.width() * sizeof(uint32_t);
        image.pixels.resize(size.width() * size.height());
        if (SkCodec::kSuccess != codec->getPixels(info, image.pixels.data(), rowBytes)) {
            image.pixels.resize(0);
        } else {
            image.width  = size.width();
            image.height = size.height();
        }
    }
    return image;
}
}
