/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPixelSerializer.h"

SkData* SkPixelSerializer::reencodeData(SkData* encoded) {
    return encoded ? this->onReencodeData(encoded) : nullptr;
}

SkData* SkPixelSerializer::encodePixels(const SkImageInfo& info, const void* pixels,
                                        size_t rowBytes) {
    if (kUnknown_SkColorType == info.colorType() || !pixels) {
        return nullptr;
    }
    return this->onEncodePixels(info, pixels, rowBytes);
}

SkData* SkPixelSerializer::encodePixels(const SkPixmap& pixmap) {
    return this->encodePixels(pixmap.info(), pixmap.addr(), pixmap.rowBytes());
}
