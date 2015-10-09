/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkBitmapRegionSampler.h"
#include "SkCodec.h"
#include "SkImageDecoder.h"

SkBitmapRegionDecoderInterface* SkBitmapRegionDecoderInterface::CreateBitmapRegionDecoder(
        SkStreamRewindable* stream, Strategy strategy) {
    switch (strategy) {
        case kOriginal_Strategy: {
            SkImageDecoder* decoder = SkImageDecoder::Factory(stream);
            int width, height;
            if (nullptr == decoder) {
                SkDebugf("Error: Could not create image decoder.\n");
                return nullptr;
            }
            if (!decoder->buildTileIndex(stream, &width, &height)) {
                SkDebugf("Error: Could not build tile index.\n");
                delete decoder;
                return nullptr;
            }
            return new SkBitmapRegionSampler(decoder, width, height);
        }
        case kCanvas_Strategy: {
            SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream));
            if (nullptr == codec) {
                SkDebugf("Error: Failed to create decoder.\n");
                return nullptr;
            }
            switch (codec->getScanlineOrder()) {
                case SkCodec::kTopDown_SkScanlineOrder:
                case SkCodec::kNone_SkScanlineOrder:
                    break;
                default:
                    SkDebugf("Error: Scanline ordering not supported.\n");
                    return nullptr;
            }
            return new SkBitmapRegionCanvas(codec.detach());
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}
