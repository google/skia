/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef CodecPriv_DEFINED
#define CodecPriv_DEFINED

#include "CommandLineFlags.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkEncodedImageFormat.h"
#include "SkImageEncoder.h"
#include "SkOSPath.h"
#include "SkStream.h"

static DEFINE_string(codecWritePath, "",
                     "Dump image decodes from codec unit tests here.");

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
    if (FLAGS_codecWritePath.isEmpty()) {
        return;
    }

    SkString filename = SkOSPath::Join(FLAGS_codecWritePath[0], name);
    filename.appendf(".png");
    SkFILEWStream file(filename.c_str());
    if (!SkEncodeImage(&file, bm, SkEncodedImageFormat::kPNG, 100)) {
        SkDebugf("failed to write '%s'\n", filename.c_str());
    }
}

#endif  // CodecPriv_DEFINED
