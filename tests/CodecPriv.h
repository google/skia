/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CodecPriv_DEFINED
#define CodecPriv_DEFINED

#include "CommonFlags.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkEncodedImageFormat.h"
#include "SkImageEncoder.h"
#include "SkOSPath.h"
#include "SkStream.h"

inline bool decode_memory(const void* mem, size_t size, SkBitmap* bm) {
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(SkData::MakeWithoutCopy(mem, size)));
    if (!codec) {
        return false;
    }

    bm->allocPixels(codec->getInfo());
    const SkCodec::Result result = codec->getPixels(codec->getInfo(), bm->getPixels(),
            bm->rowBytes());
    return result == SkCodec::kSuccess || result == SkCodec::kIncompleteInput;
}

inline void write_bm(const char* name, const SkBitmap& bm) {
    if (FLAGS_writePath.isEmpty()) {
        return;
    }

    SkString filename = SkOSPath::Join(FLAGS_writePath[0], name);
    filename.appendf(".png");
    SkFILEWStream file(filename.c_str());
    if (!SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("failed to write '%s'\n", filename.c_str());
    }
}

#endif  // CodecPriv_DEFINED
