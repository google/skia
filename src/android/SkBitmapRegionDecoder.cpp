/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/android/SkBitmapRegionDecoder.h"
#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "src/android/SkBitmapRegionCodec.h"
#include "src/codec/SkCodecPriv.h"

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        sk_sp<SkData> data, Strategy strategy) {
    return SkBitmapRegionDecoder::Create(new SkMemoryStream(data),
            strategy);
}

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkStreamRewindable* stream, Strategy strategy) {
    std::unique_ptr<SkStreamRewindable> streamDeleter(stream);
    switch (strategy) {
        case kAndroidCodec_Strategy: {
            auto codec = SkAndroidCodec::MakeFromStream(std::move(streamDeleter));
            if (nullptr == codec) {
                SkCodecPrintf("Error: Failed to create codec.\n");
                return nullptr;
            }

            switch ((SkEncodedImageFormat)codec->getEncodedFormat()) {
                case SkEncodedImageFormat::kJPEG:
                case SkEncodedImageFormat::kPNG:
                case SkEncodedImageFormat::kWEBP:
                case SkEncodedImageFormat::kHEIF:
                    break;
                default:
                    return nullptr;
            }

            return new SkBitmapRegionCodec(codec.release());
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}
