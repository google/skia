/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"

inline bool decode_memory(const void* mem, size_t size, SkBitmap* bm) {
    SkAutoTUnref<SkData> data(SkData::NewWithoutCopy(mem, size));

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromData(data.get()));
    if (!codec) {
        return false;
    }

    // Construct a color table for the decode if necessary
    SkAutoTUnref<SkColorTable> colorTable(nullptr);
    SkPMColor* colorPtr = nullptr;
    int* colorCountPtr = nullptr;
    int maxColors = 256;
    if (kIndex_8_SkColorType == codec->getInfo().colorType()) {
        SkPMColor colors[256];
        colorTable.reset(new SkColorTable(colors, maxColors));
        colorPtr = const_cast<SkPMColor*>(colorTable->readColors());
        colorCountPtr = &maxColors;
    }

    bm->allocPixels(codec->getInfo(), nullptr, colorTable.get());
    const SkCodec::Result result = codec->getPixels(codec->getInfo(), bm->getPixels(),
            bm->rowBytes(), nullptr, colorPtr, colorCountPtr);
    return result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput;
}
