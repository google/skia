/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DecodeFile_DEFINED
#define DecodeFile_DEFINED

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"

static inline bool decode_file(sk_sp<SkData> data, SkBitmap* bitmap,
                               SkColorType colorType = kN32_SkColorType,
                               bool requireUnpremul = false) {
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(data));
    if (!codec) {
        return false;
    }

    SkImageInfo info = codec->getInfo().makeColorType(colorType);
    if (requireUnpremul && kPremul_SkAlphaType == info.alphaType()) {
        info = info.makeAlphaType(kUnpremul_SkAlphaType);
    }

    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    return SkCodec::kSuccess == codec->getPixels(info, bitmap->getPixels(), bitmap->rowBytes());
}

static inline bool decode_file(const char* filename, SkBitmap* bitmap,
                               SkColorType colorType = kN32_SkColorType,
                               bool requireUnpremul = false) {
    return decode_file(SkData::MakeFromFileName(filename), bitmap, colorType, requireUnpremul);
}

static inline sk_sp<SkImage> decode_file(const char filename[]) {
    sk_sp<SkData> data(SkData::MakeFromFileName(filename));
    return data ? SkImage::MakeFromEncoded(data) : nullptr;
}
#endif  // DecodeFile_DEFINED
