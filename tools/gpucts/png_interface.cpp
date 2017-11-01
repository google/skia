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

bool WritePngRgba8888ToFile(GMK_ImageData data, const char* path) {
    SkImageInfo info = SkImageInfo::Make(data.width, data.height,
                                         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType);
    SkPixmap pm(info, data.pix, data.width * sizeof(uint32_t));
    SkFILEWStream wStream(path);
    SkPngEncoder::Options options;
    options.fUnpremulBehavior = SkTransferFunctionBehavior::kIgnore;
    return wStream.isValid() && SkPngEncoder::Encode(&wStream, pm, options);
}

GMK_ImageData ReadPngRgba8888FromFile(const char* path, std::vector<uint32_t>* pixels) {
    assert(pixels);
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(SkData::MakeFromFileName(path));
    if (!codec) {
        return {nullptr, 0, 0};
    }
    constexpr SkColorType ct = kRGBA_8888_SkColorType;
    constexpr SkAlphaType at = kUnpremul_SkAlphaType;
    SkISize size = codec->getInfo().dimensions();
    assert(!size.isEmpty());
    SkImageInfo info = SkImageInfo::Make(size.width(), size.height(), ct, at);
    size_t rowBytes = size.width() * sizeof(uint32_t);
    pixels->resize(size.width() * size.height());
    if (SkCodec::kSuccess != codec->getPixels(info, pixels->data(), rowBytes)) {
        pixels->resize(0);
        return {nullptr, 0, 0};
    }
    return {pixels->data(), size.width(), size.height()};
}
