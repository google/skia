/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkBmpDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkGifDecoder.h"
#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/codec/SkJpegxlDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/codec/SkWbmpDecoder.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkStream.h"

#include <cstdio>
#include <memory>

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

    sk_sp<SkData> data = SkData::MakeFromStream(input.get(), input->getLength());

    std::unique_ptr<SkCodec> codec = nullptr;
    if (SkBmpDecoder::IsBmp(data->bytes(), data->size())) {
      codec = SkBmpDecoder::Decode(data, nullptr);
    } else if (SkGifDecoder::IsGif(data->bytes(), data->size())) {
      codec = SkGifDecoder::Decode(data, nullptr);
    } else if (SkIcoDecoder::IsIco(data->bytes(), data->size())) {
      codec = SkIcoDecoder::Decode(data, nullptr);
    } else if (SkJpegDecoder::IsJpeg(data->bytes(), data->size())) {
      codec = SkJpegDecoder::Decode(data, nullptr);
    } else if (SkJpegxlDecoder::IsJpegxl(data->bytes(), data->size())) {
      codec = SkJpegxlDecoder::Decode(data, nullptr);
    } else if (SkPngDecoder::IsPng(data->bytes(), data->size())) {
      codec = SkPngDecoder::Decode(data, nullptr);
    } else if (SkWbmpDecoder::IsWbmp(data->bytes(), data->size())) {
      codec = SkWbmpDecoder::Decode(data, nullptr);
    } else if (SkWebpDecoder::IsWebp(data->bytes(), data->size())) {
      codec = SkWebpDecoder::Decode(data, nullptr);
    } else {
      printf("Unsupported file format\n");
      return 1;
    }

    SkImageInfo info = codec->getInfo();
    printf("Image is %d by %d pixels.\n", info.width(), info.height());

    return 0;
}
