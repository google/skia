/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkPngCompositeChunkReader.h"
#include <cstring>

class SkData;

bool SkPngCompositeChunkReader::readChunk(const char tag[], const void* data, size_t length) {
    if (fChunkReader && !fChunkReader->readChunk(tag, data, length)) {
        //  Only fail if the client's chunk reader failed; If we don't have a
        //  chunk reader we would still need to grab gainmap chunks
        return false;
    }

    // If we found a chunk but there's no data, then just skip it!
    if (data == nullptr || length == 0) {
        return true;
    }

    if (strcmp("gmAP", tag) == 0) {
        SkMemoryStream stream(data, length);
        sk_sp<SkData> streamData = stream.getData();
        SkGainmapInfo info;
        if (SkGainmapInfo::Parse(streamData.get(), info)) {
            fGainmapInfo.emplace(std::move(info));
        }
    } else if (strcmp("gdAT", tag) == 0) {
        fGainmapStream = SkMemoryStream::MakeCopy(data, length);
    }

    return true;
}
