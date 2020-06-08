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

static constexpr char kBinaryPrefix[] = "FLARE";

extern std::unique_ptr<StreamReader> MakeJsonStreamReader(const char[], size_t);
extern std::unique_ptr<StreamReader> MakeBinaryStreamReader(std::unique_ptr<SkStreamAsset>);

std::unique_ptr<StreamReader> StreamReader::Make(const char data[], size_t len) {
    if (len >= sizeof(kBinaryPrefix) &&
        strncmp(data, kBinaryPrefix, strlen(kBinaryPrefix)) == 0) {
        return MakeBinaryStreamReader(SkMemoryStream::MakeDirect(data, len));
    }

    return MakeJsonStreamReader(data, len);
}

std::unique_ptr<StreamReader> StreamReader::Make(std::unique_ptr<SkStreamAsset> stream) {
    constexpr auto peek_size = sizeof(kBinaryPrefix) - 1;
    char buf[peek_size];

    if (stream->peek(buf, peek_size) == peek_size && strncmp(buf, kBinaryPrefix, peek_size) == 0) {
        // we can stay in streaming mode
        return MakeBinaryStreamReader(std::move(stream));
    }

    // read to memory to figure what we're dealing with
    const auto data = SkData::MakeFromStream(stream.get(), stream->getLength());

    return StreamReader::Make(static_cast<const char*>(data->data()), data->size());
}

} // namespace skrive::internal
