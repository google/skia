/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/skrive/src/reader/StreamReader.h"

#include "include/core/SkStream.h"

#include <cstring>

namespace skrive::internal {

static constexpr char   kBinaryPrefix[]   = "FLARE";
static constexpr size_t kBinaryPrefixSize = sizeof(kBinaryPrefix) - 1;

extern std::unique_ptr<StreamReader> MakeJsonStreamReader(const char[], size_t);
extern std::unique_ptr<StreamReader> MakeBinaryStreamReader(std::unique_ptr<SkStreamAsset>);

std::unique_ptr<StreamReader> StreamReader::Make(const sk_sp<SkData>& data) {
    if (data->size() >= kBinaryPrefixSize &&
        !memcmp(data->data(), kBinaryPrefix, kBinaryPrefixSize)) {
        auto reader = SkMemoryStream::Make(data);
        reader->skip(kBinaryPrefixSize);

        return MakeBinaryStreamReader(std::move(reader));
    }

    return MakeJsonStreamReader(static_cast<const char*>(data->data()), data->size());
}

std::unique_ptr<StreamReader> StreamReader::Make(std::unique_ptr<SkStreamAsset> stream) {
    char buf[kBinaryPrefixSize];

    if (stream->read(buf, kBinaryPrefixSize) == kBinaryPrefixSize) {
        if (!strncmp(buf, kBinaryPrefix, kBinaryPrefixSize)) {
            // binary stream - we can stay in streaming mode
            return MakeBinaryStreamReader(std::move(stream));
        }
    } else {
        // stream too short to hold anything useful
        return nullptr;
    }

    if (!stream->rewind()) {
        SkDebugf("!! failed to rewind stream.\n");
        return nullptr;
    }

    // read to memory to figure what we're dealing with
    return StreamReader::Make(SkData::MakeFromStream(stream.get(), stream->getLength()));
}

SkV2 StreamReader::readV2(const char label[]) {
    SkV2 v2{0,0};

    this->readFloatArray(label, reinterpret_cast<float*>(&v2), 2);

    return v2;
}

SkColor4f StreamReader::readColor(const char label[]) {
    SkColor4f color{0,0,0,1};

    this->readFloatArray(label, reinterpret_cast<float*>(&color), 4);

    return color;
}

} // namespace skrive::internal
