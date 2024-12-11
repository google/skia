/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/codec/SkIcoCodec.h"

#include "include/codec/SkIcoDecoder.h"
#include "include/codec/SkPngDecoder.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/private/SkEncodedInfo.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTSort.h"
#include "src/codec/SkBmpCodec.h"
#include "src/codec/SkCodecPriv.h"
#include "src/core/SkStreamPriv.h"

#include "modules/skcms/skcms.h"
#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>

using namespace skia_private;

class SkSampler;

/*
 * Checks the start of the stream to see if the image is an Ico or Cur
 */
bool SkIcoCodec::IsIco(const void* buffer, size_t bytesRead) {
    const char icoSig[] = { '\x00', '\x00', '\x01', '\x00' };
    const char curSig[] = { '\x00', '\x00', '\x02', '\x00' };
    return bytesRead >= sizeof(icoSig) &&
            (!memcmp(buffer, icoSig, sizeof(icoSig)) ||
            !memcmp(buffer, curSig, sizeof(curSig)));
}

std::unique_ptr<SkCodec> SkIcoCodec::MakeFromStream(std::unique_ptr<SkStream> stream,
                                                    Result* result) {
    SkASSERT(result);
    if (!stream) {
        *result = SkCodec::kInvalidInput;
        return nullptr;
    }
    // It is helpful to have the entire stream in a contiguous buffer. In some cases,
    // this is already the case anyway, so this method is faster. In others, this is
    // safer than the old method, which required allocating a block of memory whose
    // byte size is stored in the stream as a uint32_t, and may result in a large or
    // failed allocation.
    sk_sp<SkData> data = nullptr;
    if (stream->getMemoryBase()) {
        // It is safe to make without copy because we'll hold onto the stream.
        data = SkData::MakeWithoutCopy(stream->getMemoryBase(), stream->getLength());
    } else {
        data = SkCopyStreamToData(stream.get());

        // If we are forced to copy the stream to a data, we can go ahead and delete the stream.
        stream.reset(nullptr);
    }

    // Header size constants
    constexpr uint32_t kIcoDirectoryBytes = 6;
    constexpr uint32_t kIcoDirEntryBytes = 16;

    // Read the directory header
    if (data->size() < kIcoDirectoryBytes) {
        SkCodecPrintf("Error: unable to read ico directory header.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    // Process the directory header
    const uint16_t numImages = SkCodecPriv::UnsafeGetShort(data->bytes(), 4);
    if (0 == numImages) {
        SkCodecPrintf("Error: No images embedded in ico.\n");
        *result = kInvalidInput;
        return nullptr;
    }

    // This structure is used to represent the vital information about entries
    // in the directory header.  We will obtain this information for each
    // directory entry.
    struct Entry {
        uint32_t offset;
        uint32_t size;
    };
    UniqueVoidPtr dirEntryBuffer(sk_malloc_canfail(sizeof(Entry) * numImages));
    if (!dirEntryBuffer) {
        SkCodecPrintf("Error: OOM allocating ICO directory for %i images.\n",
                      numImages);
        *result = kInternalError;
        return nullptr;
    }
    auto* directoryEntries = reinterpret_cast<Entry*>(dirEntryBuffer.get());

    // Iterate over directory entries
    for (uint32_t i = 0; i < numImages; i++) {
        const uint8_t* entryBuffer = data->bytes() + kIcoDirectoryBytes + i * kIcoDirEntryBytes;
        if (data->size() < kIcoDirectoryBytes + (i+1) * kIcoDirEntryBytes) {
            SkCodecPrintf("Error: Dir entries truncated in ico.\n");
            *result = kIncompleteInput;
            return nullptr;
        }

        // The directory entry contains information such as width, height,
        // bits per pixel, and number of colors in the color palette.  We will
        // ignore these fields since they are repeated in the header of the
        // embedded image.  In the event of an inconsistency, we would always
        // defer to the value in the embedded header anyway.

        // Specifies the size of the embedded image, including the header
        uint32_t size = SkCodecPriv::UnsafeGetInt(entryBuffer, 8);

        // Specifies the offset of the embedded image from the start of file.
        // It does not indicate the start of the pixel data, but rather the
        // start of the embedded image header.
        uint32_t offset = SkCodecPriv::UnsafeGetInt(entryBuffer, 12);

        // Save the vital fields
        directoryEntries[i].offset = offset;
        directoryEntries[i].size = size;
    }

    // Default Result, if no valid embedded codecs are found.
    *result = kInvalidInput;

    // It is "customary" that the embedded images will be stored in order of
    // increasing offset.  However, the specification does not indicate that
    // they must be stored in this order, so we will not trust that this is the
    // case.  Here we sort the embedded images by increasing offset.
    struct EntryLessThan {
        bool operator() (Entry a, Entry b) const {
            return a.offset < b.offset;
        }
    };
    EntryLessThan lessThan;
    SkTQSort(directoryEntries, directoryEntries + numImages, lessThan);

    // Now will construct a candidate codec for each of the embedded images
    uint32_t bytesRead = kIcoDirectoryBytes + numImages * kIcoDirEntryBytes;
    auto codecs = std::make_unique<TArray<std::unique_ptr<SkCodec>>>(numImages);
    for (uint32_t i = 0; i < numImages; i++) {
        uint32_t offset = directoryEntries[i].offset;
        uint32_t size = directoryEntries[i].size;

        // Ensure that the offset is valid
        if (offset < bytesRead) {
            SkCodecPrintf("Warning: invalid ico offset.\n");
            continue;
        }

        // If we cannot skip, assume we have reached the end of the stream and
        // stop trying to make codecs
        if (offset >= data->size()) {
            SkCodecPrintf("Warning: could not skip to ico offset.\n");
            break;
        }
        bytesRead = offset;

        if (offset + size > data->size()) {
            SkCodecPrintf("Warning: could not create embedded stream.\n");
            *result = kIncompleteInput;
            break;
        }

        sk_sp<SkData> embeddedData(SkData::MakeSubset(data.get(), offset, size));
        auto embeddedStream = SkMemoryStream::Make(embeddedData);
        bytesRead += size;

        // Check if the embedded codec is bmp or png and create the codec
        std::unique_ptr<SkCodec> codec;
        Result ignoredResult;
        if (SkPngDecoder::IsPng(embeddedData->bytes(), embeddedData->size())) {
            codec = SkPngDecoder::Decode(std::move(embeddedStream), &ignoredResult);
        } else {
            codec = SkBmpCodec::MakeFromIco(std::move(embeddedStream), &ignoredResult);
        }

        if (nullptr != codec) {
            codecs->push_back(std::move(codec));
        }
    }

    if (codecs->empty()) {
        SkCodecPrintf("Error: could not find any valid embedded ico codecs.\n");
        return nullptr;
    }

    // Use the largest codec as a "suggestion" for image info
    size_t maxSize = 0;
    int maxIndex = 0;
    for (int i = 0; i < codecs->size(); i++) {
        SkImageInfo info = codecs->at(i)->getInfo();
        size_t size = info.computeMinByteSize();

        if (size > maxSize) {
            maxSize = size;
            maxIndex = i;
        }
    }

    auto maxInfo = codecs->at(maxIndex)->getEncodedInfo().copy();

    *result = kSuccess;
    return std::unique_ptr<SkCodec>(
            new SkIcoCodec(std::move(maxInfo), std::move(stream), std::move(codecs)));
}

SkIcoCodec::SkIcoCodec(SkEncodedInfo&& info,
                       std::unique_ptr<SkStream> stream,
                       std::unique_ptr<TArray<std::unique_ptr<SkCodec>>> codecs)
        // The source skcms_PixelFormat will not be used. The embedded
        // codec's will be used instead.
        : INHERITED(std::move(info), skcms_PixelFormat(), std::move(stream))
        , fEmbeddedCodecs(std::move(codecs))
        , fCurrCodec(nullptr) {}

/*
 * Chooses the best dimensions given the desired scale
 */
SkISize SkIcoCodec::onGetScaledDimensions(float desiredScale) const {
    // We set the dimensions to the largest candidate image by default.
    // Regardless of the scale request, this is the largest image that we
    // will decode.
    int origWidth = this->dimensions().width();
    int origHeight = this->dimensions().height();
    float desiredSize = desiredScale * origWidth * origHeight;
    // At least one image will have smaller error than this initial value
    float minError = ((float) (origWidth * origHeight)) - desiredSize + 1.0f;
    int32_t minIndex = -1;
    for (int32_t i = 0; i < fEmbeddedCodecs->size(); i++) {
        auto dimensions = fEmbeddedCodecs->at(i)->dimensions();
        int width = dimensions.width();
        int height = dimensions.height();
        float error = SkTAbs(((float) (width * height)) - desiredSize);
        if (error < minError) {
            minError = error;
            minIndex = i;
        }
    }
    SkASSERT(minIndex >= 0);

    return fEmbeddedCodecs->at(minIndex)->dimensions();
}

int SkIcoCodec::chooseCodec(const SkISize& requestedSize, int startIndex) {
    SkASSERT(startIndex >= 0);

    // FIXME: Cache the index from onGetScaledDimensions?
    for (int i = startIndex; i < fEmbeddedCodecs->size(); i++) {
        if (fEmbeddedCodecs->at(i)->dimensions() == requestedSize) {
            return i;
        }
    }

    return -1;
}

bool SkIcoCodec::onDimensionsSupported(const SkISize& dim) {
    return this->chooseCodec(dim, 0) >= 0;
}

/*
 * Initiates the Ico decode
 */
SkCodec::Result SkIcoCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts,
                                        int* rowsDecoded) {
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }

    int index = 0;
    SkCodec::Result result = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        result = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &opts);
        switch (result) {
            case kSuccess:
            case kIncompleteInput:
                // The embedded codec will handle filling incomplete images, so we will indicate
                // that all of the rows are initialized.
                *rowsDecoded = dstInfo.height();
                return result;
            default:
                // Continue trying to find a valid embedded codec on a failed decode.
                break;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}

SkCodec::Result SkIcoCodec::onStartScanlineDecode(const SkImageInfo& dstInfo,
        const SkCodec::Options& options) {
    int index = 0;
    SkCodec::Result result = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        result = embeddedCodec->startScanlineDecode(dstInfo, &options);
        if (kSuccess == result) {
            fCurrCodec = embeddedCodec;
            return result;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}

int SkIcoCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->getScanlines(dst, count, rowBytes);
}

bool SkIcoCodec::onSkipScanlines(int count) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->skipScanlines(count);
}

SkCodec::Result SkIcoCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* pixels, size_t rowBytes, const SkCodec::Options& options) {
    int index = 0;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->at(index).get();
        switch (embeddedCodec->startIncrementalDecode(dstInfo,
                pixels, rowBytes, &options)) {
            case kSuccess:
                fCurrCodec = embeddedCodec;
                return kSuccess;
            case kUnimplemented:
                // FIXME: embeddedCodec is a BMP. If scanline decoding would work,
                // return kUnimplemented so that SkSampledCodec will fall through
                // to use the scanline decoder.
                // Note that calling startScanlineDecode will require an extra
                // rewind. The embedded codec has an SkMemoryStream, which is
                // cheap to rewind, though it will do extra work re-reading the
                // header.
                // Also note that we pass nullptr for Options. This is because
                // Options that are valid for incremental decoding may not be
                // valid for scanline decoding.
                // Once BMP supports incremental decoding this workaround can go
                // away.
                if (embeddedCodec->startScanlineDecode(dstInfo) == kSuccess) {
                    return kUnimplemented;
                }
                // Move on to the next embedded codec.
                break;
            default:
                break;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return kInvalidScale;
}

SkCodec::Result SkIcoCodec::onIncrementalDecode(int* rowsDecoded) {
    SkASSERT(fCurrCodec);
    return fCurrCodec->incrementalDecode(rowsDecoded);
}

SkCodec::SkScanlineOrder SkIcoCodec::onGetScanlineOrder() const {
    // FIXME: This function will possibly return the wrong value if it is called
    //        before startScanlineDecode()/startIncrementalDecode().
    if (fCurrCodec) {
        return fCurrCodec->getScanlineOrder();
    }

    return INHERITED::onGetScanlineOrder();
}

SkSampler* SkIcoCodec::getSampler(bool createIfNecessary) {
    if (fCurrCodec) {
        return fCurrCodec->getSampler(createIfNecessary);
    }

    return nullptr;
}

namespace SkIcoDecoder {
bool IsIco(const void* data, size_t len) {
    return SkIcoCodec::IsIco(data, len);
}

std::unique_ptr<SkCodec> Decode(std::unique_ptr<SkStream> stream,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    SkCodec::Result resultStorage;
    if (!outResult) {
        outResult = &resultStorage;
    }
    return SkIcoCodec::MakeFromStream(std::move(stream), outResult);
}

std::unique_ptr<SkCodec> Decode(sk_sp<SkData> data,
                                SkCodec::Result* outResult,
                                SkCodecs::DecodeContext) {
    if (!data) {
        if (outResult) {
            *outResult = SkCodec::kInvalidInput;
        }
        return nullptr;
    }
    return Decode(SkMemoryStream::Make(std::move(data)), outResult, nullptr);
}
}  // namespace SkIcoDecoder

