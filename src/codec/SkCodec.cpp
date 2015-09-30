/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkCodec.h"
#include "SkData.h"
#include "SkCodec_libgif.h"
#include "SkCodec_libico.h"
#include "SkCodec_libpng.h"
#include "SkCodec_wbmp.h"
#include "SkCodecPriv.h"
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
#include "SkJpegCodec.h"
#endif
#include "SkStream.h"
#include "SkWebpCodec.h"

struct DecoderProc {
    bool (*IsFormat)(SkStream*);
    SkCodec* (*NewFromStream)(SkStream*);
};

static const DecoderProc gDecoderProcs[] = {
    { SkPngCodec::IsPng, SkPngCodec::NewFromStream },
#ifndef SK_BUILD_FOR_ANDROID_FRAMEWORK
    { SkJpegCodec::IsJpeg, SkJpegCodec::NewFromStream },
#endif
    { SkWebpCodec::IsWebp, SkWebpCodec::NewFromStream },
    { SkGifCodec::IsGif, SkGifCodec::NewFromStream },
    { SkIcoCodec::IsIco, SkIcoCodec::NewFromStream },
    { SkBmpCodec::IsBmp, SkBmpCodec::NewFromStream },
    { SkWbmpCodec::IsWbmp, SkWbmpCodec::NewFromStream }
};

SkCodec* SkCodec::NewFromStream(SkStream* stream) {
    if (!stream) {
        return nullptr;
    }

    SkAutoTDelete<SkStream> streamDeleter(stream);
    
    SkAutoTDelete<SkCodec> codec(nullptr);
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

SkCodec* SkCodec::NewFromData(SkData* data) {
    if (!data) {
        return nullptr;
    }
    return NewFromStream(new SkMemoryStream(data));
}

SkCodec::SkCodec(const SkImageInfo& info, SkStream* stream)
    : fSrcInfo(info)
    , fStream(stream)
    , fNeedsRewind(false)
    , fDstInfo()
    , fOptions()
    , fCurrScanline(-1)
{}

SkCodec::~SkCodec() {}

bool SkCodec::rewindIfNeeded() {
    if (!fStream) {
        // Some codecs do not have a stream, but they hold others that do. They
        // must handle rewinding themselves.
        return true;
    }

    // Store the value of fNeedsRewind so we can update it. Next read will
    // require a rewind.
    const bool needsRewind = fNeedsRewind;
    fNeedsRewind = true;
    if (!needsRewind) {
        return true;
    }

    // startScanlineDecode will need to be called before decoding scanlines.
    fCurrScanline = -1;

    if (!fStream->rewind()) {
        return false;
    }

    return this->onRewind();
}

SkCodec::Result SkCodec::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes,
                                   const Options* options, SkPMColor ctable[], int* ctableCount) {
    if (kUnknown_SkColorType == info.colorType()) {
        return kInvalidConversion;
    }
    if (nullptr == pixels) {
        return kInvalidParameters;
    }
    if (rowBytes < info.minRowBytes()) {
        return kInvalidParameters;
    }

    if (kIndex_8_SkColorType == info.colorType()) {
        if (nullptr == ctable || nullptr == ctableCount) {
            return kInvalidParameters;
        }
    } else {
        if (ctableCount) {
            *ctableCount = 0;
        }
        ctableCount = nullptr;
        ctable = nullptr;
    }

    {
        SkAlphaType canonical;
        if (!SkColorTypeValidateAlphaType(info.colorType(), info.alphaType(), &canonical)
            || canonical != info.alphaType())
        {
            return kInvalidConversion;
        }
    }

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

    // Default options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    }
    const Result result = this->onGetPixels(info, pixels, rowBytes, *options, ctable, ctableCount);

    if ((kIncompleteInput == result || kSuccess == result) && ctableCount) {
        SkASSERT(*ctableCount >= 0 && *ctableCount <= 256);
    }
    return result;
}

SkCodec::Result SkCodec::getPixels(const SkImageInfo& info, void* pixels, size_t rowBytes) {
    return this->getPixels(info, pixels, rowBytes, nullptr, nullptr, nullptr);
}

SkCodec::Result SkCodec::startScanlineDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options* options, SkPMColor ctable[], int* ctableCount) {
    // Reset fCurrScanline in case of failure.
    fCurrScanline = -1;
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

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

    // Set options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    }

    const Result result = this->onStartScanlineDecode(dstInfo, *options, ctable, ctableCount);
    if (result != SkCodec::kSuccess) {
        return result;
    }

    fCurrScanline = 0;
    fDstInfo = dstInfo;
    fOptions = *options;
    return kSuccess;
}

SkCodec::Result SkCodec::startScanlineDecode(const SkImageInfo& dstInfo) {
    return this->startScanlineDecode(dstInfo, nullptr, nullptr, nullptr);
}

SkCodec::Result SkCodec::getScanlines(void* dst, int countLines, size_t rowBytes) {
    if (fCurrScanline < 0) {
        return kScanlineDecodingNotStarted;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if ((rowBytes < fDstInfo.minRowBytes() && countLines > 1 ) || countLines <= 0
         || fCurrScanline + countLines > fDstInfo.height()) {
        return kInvalidParameters;
    }

    const Result result = this->onGetScanlines(dst, countLines, rowBytes);
    fCurrScanline += countLines;
    return result;
}

SkCodec::Result SkCodec::skipScanlines(int countLines) {
    if (fCurrScanline < 0) {
        return kScanlineDecodingNotStarted;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if (fCurrScanline + countLines > fDstInfo.height()) {
        // Arguably, we could just skip the scanlines which are remaining,
        // and return kSuccess. We choose to return invalid so the client
        // can catch their bug.
        return SkCodec::kInvalidParameters;
    }

    const Result result = this->onSkipScanlines(countLines);
    fCurrScanline += countLines;
    return result;
}
