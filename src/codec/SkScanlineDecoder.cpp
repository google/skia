/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkScanlineDecoder.h"
#include "SkBmpCodec.h"
#include "SkCodec_libpng.h"
#include "SkCodec_wbmp.h"
#include "SkCodecPriv.h"
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
#include "SkJpegCodec.h"
#endif

struct DecoderProc {
    bool (*IsFormat)(SkStream*);
    SkScanlineDecoder* (*NewFromStream)(SkStream*);
};

static const DecoderProc gDecoderProcs[] = {
    { SkPngCodec::IsPng, SkPngCodec::NewSDFromStream },
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    { SkJpegCodec::IsJpeg, SkJpegCodec::NewSDFromStream },
#endif
    { SkBmpCodec::IsBmp, SkBmpCodec::NewSDFromStream },
    { SkWbmpCodec::IsWbmp, SkWbmpCodec::NewSDFromStream },
};

SkScanlineDecoder* SkScanlineDecoder::NewFromStream(SkStream* stream) {
    if (!stream) {
        return nullptr;
    }

    SkAutoTDelete<SkStream> streamDeleter(stream);

    SkAutoTDelete<SkScanlineDecoder> codec(nullptr);
    for (uint32_t i = 0; i < SK_ARRAY_COUNT(gDecoderProcs); i++) {
        DecoderProc proc = gDecoderProcs[i];
        const bool correctFormat = proc.IsFormat(stream);
        if (!stream->rewind()) {
            return nullptr;
        }
        if (correctFormat) {
            codec.reset(proc.NewFromStream(streamDeleter.detach()));
            break;
        }
    }

    // Set the max size at 128 megapixels (512 MB for kN32).
    // This is about 4x smaller than a test image that takes a few minutes for
    // dm to decode and draw.
    const int32_t maxSize = 1 << 27;
    if (codec && codec->getInfo().width() * codec->getInfo().height() > maxSize) {
        SkCodecPrintf("Error: Image size too large, cannot decode.\n");
        return nullptr;
    } else {
        return codec.detach();
    }
}

SkScanlineDecoder* SkScanlineDecoder::NewFromData(SkData* data) {
    if (!data) {
        return nullptr;
    }
    return NewFromStream(new SkMemoryStream(data));
}


SkCodec::Result SkScanlineDecoder::start(const SkImageInfo& dstInfo,
        const SkCodec::Options* options, SkPMColor ctable[], int* ctableCount) {
    // Ensure that valid color ptrs are passed in for kIndex8 color type
    if (kIndex_8_SkColorType == dstInfo.colorType()) {
        if (nullptr == ctable || nullptr == ctableCount) {
            return SkCodec::kInvalidParameters;
        }
    } else {
        if (ctableCount) {
            *ctableCount = 0;
        }
        ctableCount = nullptr;
        ctable = nullptr;
    }

    // Set options.
    SkCodec::Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    }

    const SkCodec::Result result = this->onStart(dstInfo, *options, ctable, ctableCount);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    fCurrScanline = 0;
    fDstInfo = dstInfo;
    fOptions = *options;
    return SkCodec::kSuccess;
}

SkCodec::Result SkScanlineDecoder::start(const SkImageInfo& dstInfo) {
    return this->start(dstInfo, nullptr, nullptr, nullptr);
}

