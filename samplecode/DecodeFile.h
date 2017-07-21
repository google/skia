/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DecodeFile_DEFINED
#define DecodeFile_DEFINED

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkImage.h"

static inline bool decode_file(const char* filename, SkBitmap* bitmap,
                               SkColorType colorType = kN32_SkColorType,
                               bool requireUnpremul = false) {
    sk_sp<SkData> data(SkData::MakeFromFileName(filename));
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(data));
    if (!codec) {
        return false;
    }

    auto dim = codec->dimensions();
    auto at = codec->getEncodedInfo().opaque() ? kOpaque_SkAlphaType :
        requireUnpremul ? kUnpremul_SkAlphaType : kPremul_SkAlphaType;
    auto info = SkImageInfo::Make(dim.width(), dim.height(), colorType, at);

    if (!bitmap->tryAllocPixels(info)) {
        return false;
    }

    return SkCodec::kSuccess == codec->getPixels(info, bitmap->getPixels(), bitmap->rowBytes());
}

static inline sk_sp<SkImage> decode_file(const char filename[]) {
    sk_sp<SkData> data(SkData::MakeFromFileName(filename));
    return data ? SkImage::MakeFromEncoded(data) : nullptr;
}
#endif  // DecodeFile_DEFINED
