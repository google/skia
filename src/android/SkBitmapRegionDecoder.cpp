/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkBitmapRegionCodec.h"
#include "SkBitmapRegionDecoder.h"
#include "SkAndroidCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkImageDecoder.h"

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkData* data, Strategy strategy) {
    return SkBitmapRegionDecoder::Create(new SkMemoryStream(data),
            strategy);
}

SkBitmapRegionDecoder* SkBitmapRegionDecoder::Create(
        SkStreamRewindable* stream, Strategy strategy) {
    SkAutoTDelete<SkStreamRewindable> streamDeleter(stream);
    switch (strategy) {
        case kCanvas_Strategy: {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(streamDeleter.detach()));
            if (nullptr == codec) {
                SkCodecPrintf("Error: Failed to create decoder.\n");
                return nullptr;
            }

            if (SkEncodedFormat::kWEBP_SkEncodedFormat == codec->getEncodedFormat()) {
                // FIXME: Support webp using a special case.  Webp does not support
                //        scanline decoding.
                return nullptr;
            }

            switch (codec->getScanlineOrder()) {
                case SkCodec::kTopDown_SkScanlineOrder:
                case SkCodec::kNone_SkScanlineOrder:
                    break;
                default:
                    SkCodecPrintf("Error: Scanline ordering not supported.\n");
                    return nullptr;
            }
            return new SkBitmapRegionCanvas(codec.detach());
        }
        case kAndroidCodec_Strategy: {
            SkAutoTDelete<SkAndroidCodec> codec =
                    SkAndroidCodec::NewFromStream(streamDeleter.detach());
            if (!codec) {
                SkCodecPrintf("Error: Failed to create codec.\n");
                return nullptr;
            }
            return new SkBitmapRegionCodec(codec.detach());
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}
