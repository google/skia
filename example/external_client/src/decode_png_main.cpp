/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"

#include <cstdio>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.png>", argv[0]);
        return 1;
    }

    std::unique_ptr<SkFILEStream> input = SkFILEStream::Make(argv[1]);
    if (!input || !input->isValid()) {
        printf("Cannot open file %s\n", argv[1]);
        return 1;
    }

    SkCodec::Result result;
    auto codec = SkPngDecoder::Decode(std::move(input), &result);
    if (!codec) {
        printf("Cannot decode file %s as a PNG\n", argv[1]);
        printf("Result code: %d\n", result);
        return 1;
    }

    SkImageInfo info = codec->getInfo();
    printf("Image is %d by %d pixels.\n", info.width(), info.height());

    return 0;
}
