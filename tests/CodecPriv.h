/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CodecPriv_DEFINED
#define CodecPriv_DEFINED

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"

inline bool decode_memory(const void* mem, size_t size, SkBitmap* bm) {
    std::unique_ptr<SkCodec> codec(SkCodec::NewFromData(SkData::MakeWithoutCopy(mem, size)));
    if (!codec) {
        return false;
    }

    bm->allocPixels(codec->getInfo());
    const SkCodec::Result result = codec->getPixels(codec->getInfo(), bm->getPixels(),
            bm->rowBytes());
    return result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput;
}
#endif  // CodecPriv_DEFINED
