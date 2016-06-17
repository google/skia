/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCodec.h"
#include "SkBitmapRegionDecoder.h"
#include "SkAndroidCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkData* data, Strategy strategy) {
    return SkBitmapRegionDecoder::Create(new SkMemoryStream(data),
            strategy);
}

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkStreamRewindable* stream, Strategy strategy) {
    SkAutoTDelete<SkStreamRewindable> streamDeleter(stream);
    switch (strategy) {
        case kAndroidCodec_Strategy: {
            SkAutoTDelete<SkAndroidCodec> codec =
                    SkAndroidCodec::NewFromStream(streamDeleter.release());
            if (nullptr == codec) {
                SkCodecPrintf("Error: Failed to create codec.\n");
                return NULL;
            }

            SkEncodedFormat format = codec->getEncodedFormat();
            switch (format) {
                case SkEncodedFormat::kJPEG_SkEncodedFormat:
                case SkEncodedFormat::kPNG_SkEncodedFormat:
                case SkEncodedFormat::kWEBP_SkEncodedFormat:
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
