/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkIcoCodec.h"
#include "SkPngCodec.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSort.h"

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

SkCodec* SkIcoCodec::NewFromStream(SkStream* stream, Result* result) {
    // Ensure that we do not leak the input stream
    std::unique_ptr<SkStream> inputStream(stream);

    // Header size constants
    static const uint32_t kIcoDirectoryBytes = 6;
    static const uint32_t kIcoDirEntryBytes = 16;

    // Read the directory header
    std::unique_ptr<uint8_t[]> dirBuffer(new uint8_t[kIcoDirectoryBytes]);
    if (inputStream.get()->read(dirBuffer.get(), kIcoDirectoryBytes) !=
            kIcoDirectoryBytes) {
        SkCodecPrintf("Error: unable to read ico directory header.\n");
        *result = kIncompleteInput;
        return nullptr;
    }

    // Process the directory header
    const uint16_t numImages = get_short(dirBuffer.get(), 4);
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
    SkAutoFree dirEntryBuffer(sk_malloc_flags(sizeof(Entry) * numImages,
                                              SK_MALLOC_TEMP));
    if (!dirEntryBuffer) {
        SkCodecPrintf("Error: OOM allocating ICO directory for %i images.\n",
                      numImages);
        *result = kInternalError;
        return nullptr;
    }
    auto* directoryEntries = reinterpret_cast<Entry*>(dirEntryBuffer.get());

    // Iterate over directory entries
    for (uint32_t i = 0; i < numImages; i++) {
        uint8_t entryBuffer[kIcoDirEntryBytes];
        if (inputStream->read(entryBuffer, kIcoDirEntryBytes) !=
                kIcoDirEntryBytes) {
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
        uint32_t size = get_int(entryBuffer, 8);

        // Specifies the offset of the embedded image from the start of file.
        // It does not indicate the start of the pixel data, but rather the
        // start of the embedded image header.
        uint32_t offset = get_int(entryBuffer, 12);

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
    SkTQSort(directoryEntries, &directoryEntries[numImages - 1], lessThan);

    // Now will construct a candidate codec for each of the embedded images
    uint32_t bytesRead = kIcoDirectoryBytes + numImages * kIcoDirEntryBytes;
    std::unique_ptr<SkTArray<std::unique_ptr<SkCodec>, true>> codecs(
            new (SkTArray<std::unique_ptr<SkCodec>, true>)(numImages));
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
        if (inputStream.get()->skip(offset - bytesRead) != offset - bytesRead) {
            SkCodecPrintf("Warning: could not skip to ico offset.\n");
            break;
        }
        bytesRead = offset;

        // Create a new stream for the embedded codec
        SkAutoFree buffer(sk_malloc_flags(size, 0));
        if (!buffer) {
            SkCodecPrintf("Warning: OOM trying to create embedded stream.\n");
            break;
        }

        if (inputStream->read(buffer.get(), size) != size) {
            SkCodecPrintf("Warning: could not create embedded stream.\n");
            *result = kIncompleteInput;
            break;
        }

        sk_sp<SkData> data(SkData::MakeFromMalloc(buffer.release(), size));
        std::unique_ptr<SkMemoryStream> embeddedStream(new SkMemoryStream(data));
        bytesRead += size;

        // Check if the embedded codec is bmp or png and create the codec
        SkCodec* codec = nullptr;
        Result dummyResult;
        if (SkPngCodec::IsPng((const char*) data->bytes(), data->size())) {
            codec = SkPngCodec::NewFromStream(embeddedStream.release(), &dummyResult);
        } else {
            codec = SkBmpCodec::NewFromIco(embeddedStream.release(), &dummyResult);
        }

        // Save a valid codec
        if (nullptr != codec) {
            codecs->push_back().reset(codec);
        }
    }

    // Recognize if there are no valid codecs
    if (0 == codecs->count()) {
        SkCodecPrintf("Error: could not find any valid embedded ico codecs.\n");
        return nullptr;
    }

    // Use the largest codec as a "suggestion" for image info
    size_t maxSize = 0;
    int maxIndex = 0;
    for (int i = 0; i < codecs->count(); i++) {
        SkImageInfo info = codecs->operator[](i)->getInfo();
        size_t size = info.getSafeSize(info.minRowBytes());

        if (size > maxSize) {
            maxSize = size;
            maxIndex = i;
        }
    }
    int width = codecs->operator[](maxIndex)->getInfo().width();
    int height = codecs->operator[](maxIndex)->getInfo().height();
    SkEncodedInfo info = codecs->operator[](maxIndex)->getEncodedInfo();
    SkColorSpace* colorSpace = codecs->operator[](maxIndex)->getInfo().colorSpace();

    *result = kSuccess;
    // The original stream is no longer needed, because the embedded codecs own their
    // own streams.
    return new SkIcoCodec(width, height, info, codecs.release(), sk_ref_sp(colorSpace));
}

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkIcoCodec::SkIcoCodec(int width, int height, const SkEncodedInfo& info,
                       SkTArray<std::unique_ptr<SkCodec>, true>* codecs,
                       sk_sp<SkColorSpace> colorSpace)
    // The source SkColorSpaceXform::ColorFormat will not be used. The embedded
    // codec's will be used instead.
    : INHERITED(width, height, info, SkColorSpaceXform::ColorFormat(), nullptr,
                std::move(colorSpace))
    , fEmbeddedCodecs(codecs)
    , fCurrScanlineCodec(nullptr)
    , fCurrIncrementalCodec(nullptr)
{}

/*
 * Chooses the best dimensions given the desired scale
 */
SkISize SkIcoCodec::onGetScaledDimensions(float desiredScale) const {
    // We set the dimensions to the largest candidate image by default.
    // Regardless of the scale request, this is the largest image that we
    // will decode.
    int origWidth = this->getInfo().width();
    int origHeight = this->getInfo().height();
    float desiredSize = desiredScale * origWidth * origHeight;
    // At least one image will have smaller error than this initial value
    float minError = ((float) (origWidth * origHeight)) - desiredSize + 1.0f;
    int32_t minIndex = -1;
    for (int32_t i = 0; i < fEmbeddedCodecs->count(); i++) {
        int width = fEmbeddedCodecs->operator[](i)->getInfo().width();
        int height = fEmbeddedCodecs->operator[](i)->getInfo().height();
        float error = SkTAbs(((float) (width * height)) - desiredSize);
        if (error < minError) {
            minError = error;
            minIndex = i;
        }
    }
    SkASSERT(minIndex >= 0);

    return fEmbeddedCodecs->operator[](minIndex)->getInfo().dimensions();
}

int SkIcoCodec::chooseCodec(const SkISize& requestedSize, int startIndex) {
    SkASSERT(startIndex >= 0);

    // FIXME: Cache the index from onGetScaledDimensions?
    for (int i = startIndex; i < fEmbeddedCodecs->count(); i++) {
        if (fEmbeddedCodecs->operator[](i)->getInfo().dimensions() == requestedSize) {
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

        SkCodec* embeddedCodec = fEmbeddedCodecs->operator[](index).get();
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

        SkCodec* embeddedCodec = fEmbeddedCodecs->operator[](index).get();
        result = embeddedCodec->startScanlineDecode(dstInfo, &options);
        if (kSuccess == result) {
            fCurrScanlineCodec = embeddedCodec;
            fCurrIncrementalCodec = nullptr;
            return result;
        }

        index++;
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}

int SkIcoCodec::onGetScanlines(void* dst, int count, size_t rowBytes) {
    SkASSERT(fCurrScanlineCodec);
    return fCurrScanlineCodec->getScanlines(dst, count, rowBytes);
}

bool SkIcoCodec::onSkipScanlines(int count) {
    SkASSERT(fCurrScanlineCodec);
    return fCurrScanlineCodec->skipScanlines(count);
}

SkCodec::Result SkIcoCodec::onStartIncrementalDecode(const SkImageInfo& dstInfo,
        void* pixels, size_t rowBytes, const SkCodec::Options& options) {
    int index = 0;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->operator[](index).get();
        switch (embeddedCodec->startIncrementalDecode(dstInfo,
                pixels, rowBytes, &options)) {
            case kSuccess:
                fCurrIncrementalCodec = embeddedCodec;
                fCurrScanlineCodec = nullptr;
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
    SkASSERT(fCurrIncrementalCodec);
    return fCurrIncrementalCodec->incrementalDecode(rowsDecoded);
}

SkCodec::SkScanlineOrder SkIcoCodec::onGetScanlineOrder() const {
    // FIXME: This function will possibly return the wrong value if it is called
    //        before startScanlineDecode()/startIncrementalDecode().
    if (fCurrScanlineCodec) {
        SkASSERT(!fCurrIncrementalCodec);
        return fCurrScanlineCodec->getScanlineOrder();
    }

    if (fCurrIncrementalCodec) {
        return fCurrIncrementalCodec->getScanlineOrder();
    }

    return INHERITED::onGetScanlineOrder();
}

SkSampler* SkIcoCodec::getSampler(bool createIfNecessary) {
    if (fCurrScanlineCodec) {
        SkASSERT(!fCurrIncrementalCodec);
        return fCurrScanlineCodec->getSampler(createIfNecessary);
    }

    if (fCurrIncrementalCodec) {
        return fCurrIncrementalCodec->getSampler(createIfNecessary);
    }

    return nullptr;
}
