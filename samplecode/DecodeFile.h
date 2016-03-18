/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"

inline bool decode_file(const char* filename, SkBitmap* bitmap,
        SkColorType colorType = kN32_SkColorType, bool requireUnpremul = false) {
    SkASSERT(kIndex_8_SkColorType != colorType);
    SkAutoTUnref<SkData> data(SkData::NewFromFileName(filename));
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data));
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
