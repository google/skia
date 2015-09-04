/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmapRegionCanvas.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkBitmapRegionSampler.h"
#include "SkScanlineDecoder.h"
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
            SkScanlineDecoder* decoder = SkScanlineDecoder::NewFromStream(stream);
            if (nullptr == decoder) {
                SkDebugf("Error: Failed to create decoder.\n");
                return nullptr;
            }
            switch (decoder->getScanlineOrder()) {
                case SkScanlineDecoder::kTopDown_SkScanlineOrder:
                case SkScanlineDecoder::kNone_SkScanlineOrder:
                    break;
                default:
                    SkDebugf("Error: Scanline ordering not supported.\n");
                    return nullptr;
            }
            return new SkBitmapRegionCanvas(decoder);
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}
