/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkBitmapRegionCodec.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkBitmapRegionSampler.h"
#include "SkAndroidCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkImageDecoder.h"

SkBitmapRegionDecoderInterface* SkBitmapRegionDecoderInterface::CreateBitmapRegionDecoder(
        SkData* data, Strategy strategy) {
    return SkBitmapRegionDecoderInterface::CreateBitmapRegionDecoder(new SkMemoryStream(data),
            strategy);
}

SkBitmapRegionDecoderInterface* SkBitmapRegionDecoderInterface::CreateBitmapRegionDecoder(
        SkStreamRewindable* stream, Strategy strategy) {
    SkAutoTDelete<SkStreamRewindable> streamDeleter(stream);
    switch (strategy) {
        case kOriginal_Strategy: {
            SkImageDecoder* decoder = SkImageDecoder::Factory(streamDeleter);
            int width, height;
            if (nullptr == decoder) {
                SkCodecPrintf("Error: Could not create image decoder.\n");
                return nullptr;
            }
            if (!decoder->buildTileIndex(streamDeleter.detach(), &width, &height)) {
                SkCodecPrintf("Error: Could not build tile index.\n");
                delete decoder;
                return nullptr;
            }
            return new SkBitmapRegionSampler(decoder, width, height);
        }
        case kCanvas_Strategy: {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(streamDeleter.detach()));
            if (nullptr == codec) {
                SkCodecPrintf("Error: Failed to create decoder.\n");
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
            if (NULL == codec) {
                SkCodecPrintf("Error: Failed to create codec.\n");
                return NULL;
            }
            return new SkBitmapRegionCodec(codec.detach());
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}
