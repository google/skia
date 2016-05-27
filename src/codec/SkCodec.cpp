/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorSpace.h"
#include "SkData.h"
#include "SkGifCodec.h"
#include "SkIcoCodec.h"
#include "SkJpegCodec.h"
#ifdef SK_HAS_PNG_LIBRARY
#include "SkPngCodec.h"
#endif
#include "SkRawCodec.h"
#include "SkStream.h"
#include "SkWbmpCodec.h"
#include "SkWebpCodec.h"

struct DecoderProc {
    bool (*IsFormat)(const void*, size_t);
    SkCodec* (*NewFromStream)(SkStream*);
};

static const DecoderProc gDecoderProcs[] = {
#ifdef SK_HAS_JPEG_LIBRARY
    { SkJpegCodec::IsJpeg, SkJpegCodec::NewFromStream },
#endif
#ifdef SK_HAS_WEBP_LIBRARY
    { SkWebpCodec::IsWebp, SkWebpCodec::NewFromStream },
#endif
#ifdef SK_HAS_GIF_LIBRARY
    { SkGifCodec::IsGif, SkGifCodec::NewFromStream },
#endif
#ifdef SK_HAS_PNG_LIBRARY
    { SkIcoCodec::IsIco, SkIcoCodec::NewFromStream },
#endif
    { SkBmpCodec::IsBmp, SkBmpCodec::NewFromStream },
    { SkWbmpCodec::IsWbmp, SkWbmpCodec::NewFromStream }
};

size_t SkCodec::MinBufferedBytesNeeded() {
    return WEBP_VP8_HEADER_SIZE;
}

SkCodec* SkCodec::NewFromStream(SkStream* stream,
                                SkPngChunkReader* chunkReader) {
    if (!stream) {
        return nullptr;
    }

    SkAutoTDelete<SkStream> streamDeleter(stream);

    // 14 is enough to read all of the supported types.
    const size_t bytesToRead = 14;
    SkASSERT(bytesToRead <= MinBufferedBytesNeeded());

    char buffer[bytesToRead];
    size_t bytesRead = stream->peek(buffer, bytesToRead);

    // It is also possible to have a complete image less than bytesToRead bytes
    // (e.g. a 1 x 1 wbmp), meaning peek() would return less than bytesToRead.
    // Assume that if bytesRead < bytesToRead, but > 0, the stream is shorter
    // than bytesToRead, so pass that directly to the decoder.
    // It also is possible the stream uses too small a buffer for peeking, but
    // we trust the caller to use a large enough buffer.

    if (0 == bytesRead) {
        // TODO: After implementing peek in CreateJavaOutputStreamAdaptor.cpp, this
        // printf could be useful to notice failures.
        // SkCodecPrintf("Encoded image data failed to peek!\n");

        // It is possible the stream does not support peeking, but does support
        // rewinding.
        // Attempt to read() and pass the actual amount read to the decoder.
        bytesRead = stream->read(buffer, bytesToRead);
        if (!stream->rewind()) {
            SkCodecPrintf("Encoded image data could not peek or rewind to determine format!\n");
            return nullptr;
        }
    }

    // PNG is special, since we want to be able to supply an SkPngChunkReader.
    // But this code follows the same pattern as the loop.
#ifdef SK_HAS_PNG_LIBRARY
    if (SkPngCodec::IsPng(buffer, bytesRead)) {
        return SkPngCodec::NewFromStream(streamDeleter.release(), chunkReader);
    } else
#endif
    {
        for (DecoderProc proc : gDecoderProcs) {
            if (proc.IsFormat(buffer, bytesRead)) {
                return proc.NewFromStream(streamDeleter.release());
            }
        }

#ifdef SK_CODEC_DECODES_RAW
        // Try to treat the input as RAW if all the other checks failed.
        return SkRawCodec::NewFromStream(streamDeleter.release());
#endif
    }

    return nullptr;
}

SkCodec* SkCodec::NewFromData(SkData* data, SkPngChunkReader* reader) {
    if (!data) {
        return nullptr;
    }
    return NewFromStream(new SkMemoryStream(data), reader);
}

SkCodec::SkCodec(int width, int height, const SkEncodedInfo& info, SkStream* stream,
        sk_sp<SkColorSpace> colorSpace, Origin origin)
    : fEncodedInfo(info)
    , fSrcInfo(info.makeImageInfo(width, height, colorSpace))
    , fStream(stream)
    , fNeedsRewind(false)
    , fColorSpace(std::move(colorSpace))
    , fOrigin(origin)
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

    if (!this->rewindIfNeeded()) {
        return kCouldNotRewind;
    }

    // Default options.
    Options optsStorage;
    if (nullptr == options) {
        options = &optsStorage;
    } else if (options->fSubset) {
        SkIRect subset(*options->fSubset);
        if (!this->onGetValidSubset(&subset) || subset != *options->fSubset) {
            // FIXME: How to differentiate between not supporting subset at all
            // and not supporting this particular subset?
            return kUnimplemented;
        }
    }

    // FIXME: Support subsets somehow? Note that this works for SkWebpCodec
    // because it supports arbitrary scaling/subset combinations.
    if (!this->dimensionsSupported(info.dimensions())) {
        return kInvalidScale;
    }

    // On an incomplete decode, the subclass will specify the number of scanlines that it decoded
    // successfully.
    int rowsDecoded = 0;
    const Result result = this->onGetPixels(info, pixels, rowBytes, *options, ctable, ctableCount,
            &rowsDecoded);

    if ((kIncompleteInput == result || kSuccess == result) && ctableCount) {
        SkASSERT(*ctableCount >= 0 && *ctableCount <= 256);
    }

    // A return value of kIncompleteInput indicates a truncated image stream.
    // In this case, we will fill any uninitialized memory with a default value.
    // Some subclasses will take care of filling any uninitialized memory on
    // their own.  They indicate that all of the memory has been filled by
    // setting rowsDecoded equal to the height.
    if (kIncompleteInput == result && rowsDecoded != info.height()) {
        this->fillIncompleteImage(info, pixels, rowBytes, options->fZeroInitialized, info.height(),
                rowsDecoded);
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
    } else if (options->fSubset) {
        SkIRect size = SkIRect::MakeSize(dstInfo.dimensions());
        if (!size.contains(*options->fSubset)) {
            return kInvalidInput;
        }

        // We only support subsetting in the x-dimension for scanline decoder.
        // Subsetting in the y-dimension can be accomplished using skipScanlines().
        if (options->fSubset->top() != 0 || options->fSubset->height() != dstInfo.height()) {
            return kInvalidInput;
        }
    }

    // FIXME: Support subsets somehow?
    if (!this->dimensionsSupported(dstInfo.dimensions())) {
        return kInvalidScale;
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

int SkCodec::getScanlines(void* dst, int countLines, size_t rowBytes) {
    if (fCurrScanline < 0) {
        return 0;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if (countLines <= 0 || fCurrScanline + countLines > fDstInfo.height()) {
        return 0;
    }

    const int linesDecoded = this->onGetScanlines(dst, countLines, rowBytes);
    if (linesDecoded < countLines) {
        this->fillIncompleteImage(this->dstInfo(), dst, rowBytes, this->options().fZeroInitialized,
                countLines, linesDecoded);
    }
    fCurrScanline += countLines;
    return linesDecoded;
}

bool SkCodec::skipScanlines(int countLines) {
    if (fCurrScanline < 0) {
        return false;
    }

    SkASSERT(!fDstInfo.isEmpty());
    if (countLines < 0 || fCurrScanline + countLines > fDstInfo.height()) {
        // Arguably, we could just skip the scanlines which are remaining,
        // and return true. We choose to return false so the client
        // can catch their bug.
        return false;
    }

    bool result = this->onSkipScanlines(countLines);
    fCurrScanline += countLines;
    return result;
}

int SkCodec::outputScanline(int inputScanline) const {
    SkASSERT(0 <= inputScanline && inputScanline < this->getInfo().height());
    return this->onOutputScanline(inputScanline);
}

int SkCodec::onOutputScanline(int inputScanline) const {
    switch (this->getScanlineOrder()) {
        case kTopDown_SkScanlineOrder:
        case kNone_SkScanlineOrder:
            return inputScanline;
        case kBottomUp_SkScanlineOrder:
            return this->getInfo().height() - inputScanline - 1;
        default:
            // This case indicates an interlaced gif and is implemented by SkGifCodec.
            SkASSERT(false);
            return 0;
    }
}

static void fill_proc(const SkImageInfo& info, void* dst, size_t rowBytes,
        uint32_t colorOrIndex, SkCodec::ZeroInitialized zeroInit, SkSampler* sampler) {
    if (sampler) {
        sampler->fill(info, dst, rowBytes, colorOrIndex, zeroInit);
    } else {
        SkSampler::Fill(info, dst, rowBytes, colorOrIndex, zeroInit);
    }
}

void SkCodec::fillIncompleteImage(const SkImageInfo& info, void* dst, size_t rowBytes,
        ZeroInitialized zeroInit, int linesRequested, int linesDecoded) {

    void* fillDst;
    const uint32_t fillValue = this->getFillValue(info.colorType());
    const int linesRemaining = linesRequested - linesDecoded;
    SkSampler* sampler = this->getSampler(false);

    int fillWidth = info.width();
    if (fOptions.fSubset) {
        fillWidth = fOptions.fSubset->width();
    }

    switch (this->getScanlineOrder()) {
        case kTopDown_SkScanlineOrder:
        case kNone_SkScanlineOrder: {
            const SkImageInfo fillInfo = info.makeWH(fillWidth, linesRemaining);
            fillDst = SkTAddOffset<void>(dst, linesDecoded * rowBytes);
            fill_proc(fillInfo, fillDst, rowBytes, fillValue, zeroInit, sampler);
            break;
        }
        case kBottomUp_SkScanlineOrder: {
            fillDst = dst;
            const SkImageInfo fillInfo = info.makeWH(fillWidth, linesRemaining);
            fill_proc(fillInfo, fillDst, rowBytes, fillValue, zeroInit, sampler);
            break;
        }
        case kOutOfOrder_SkScanlineOrder: {
            SkASSERT(1 == linesRequested || this->getInfo().height() == linesRequested);
            const SkImageInfo fillInfo = info.makeWH(fillWidth, 1);
            for (int srcY = linesDecoded; srcY < linesRequested; srcY++) {
                fillDst = SkTAddOffset<void>(dst, this->outputScanline(srcY) * rowBytes);
                fill_proc(fillInfo, fillDst, rowBytes, fillValue, zeroInit, sampler);
            }
            break;
        }
    }
}
