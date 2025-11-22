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
        sk_sp<const SkData> streamData = stream.getData();
        SkGainmapInfo info;
        if (SkGainmapInfo::Parse(streamData.get(), info)) {
            fGainmapInfo.emplace(std::move(info));
        }
    } else if (strcmp("gdAT", tag) == 0) {
        fGainmapStream = SkMemoryStream::MakeCopy(data, length);
    } else if (strcmp("cLLI", tag) == 0) {
        auto sk_data = SkData::MakeWithoutCopy(data, length);
        skhdr::ContentLightLevelInformation clli;
        if (clli.parsePngChunk(sk_data.get())) {
          fHdrMetadata.setContentLightLevelInformation(clli);
        }
    } else if (strcmp("mDCV", tag) == 0) {
        auto sk_data = SkData::MakeWithoutCopy(data, length);
        skhdr::MasteringDisplayColorVolume mdcv;
        if (mdcv.parse(sk_data.get())) {
          fHdrMetadata.setMasteringDisplayColorVolume(mdcv);
        }
    }
    return true;
}
