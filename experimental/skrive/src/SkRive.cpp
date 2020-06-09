/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/include/SkRive.h"

#include "experimental/skrive/src/reader/StreamReader.h"
#include "include/core/SkStream.h"

namespace skrive {

namespace internal {

extern sk_sp<Artboard> parse_artboard(StreamReader*);

void parse_artboards(const sk_sp<SkRive>& skrive, StreamReader* sr) {
    for (;;) {
        StreamReader::AutoBlock block(sr);
        if (block.type() == StreamReader::BlockType::kEoB) {
            break;
        }
        if (block.type() != StreamReader::BlockType::kActorArtboard) {
            SkDebugf("!! Unexpected artboard block type: %d\n", block.type());
            continue;
        }

        skrive->artboards().push_back(parse_artboard(sr));
    }
}

static sk_sp<SkRive> parse_skrive(std::unique_ptr<StreamReader> sr) {
    if (!sr) {
        return nullptr;
    }

    auto skrive = sk_make_sp<SkRive>();

    for (;;) {
        StreamReader::AutoBlock block(sr);
        if (block.type() == StreamReader::BlockType::kEoB) {
            break;
        }

        switch (block.type()) {
        case StreamReader::BlockType::kArtboards:
            parse_artboards(skrive, sr.get());
            break;
        default:
            SkDebugf("!! Unsupported block type: %d\n", block.type());
            break;
        }
    }

    return skrive;
}

} // namespace internal

sk_sp<SkRive> SkRive::Builder::make(std::unique_ptr<SkStreamAsset> stream) {
    return parse_skrive(internal::StreamReader::Make(std::move(stream)));
}

} // namespace skrive
