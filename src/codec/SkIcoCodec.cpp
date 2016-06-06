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

/*
 * Assumes IsIco was called and returned true
 * Creates an Ico decoder
 * Reads enough of the stream to determine the image format
 */
SkCodec* SkIcoCodec::NewFromStream(SkStream* stream) {
    // Ensure that we do not leak the input stream
    SkAutoTDelete<SkStream> inputStream(stream);

    // Header size constants
    static const uint32_t kIcoDirectoryBytes = 6;
    static const uint32_t kIcoDirEntryBytes = 16;

    // Read the directory header
    SkAutoTDeleteArray<uint8_t> dirBuffer(new uint8_t[kIcoDirectoryBytes]);
    if (inputStream.get()->read(dirBuffer.get(), kIcoDirectoryBytes) !=
            kIcoDirectoryBytes) {
        SkCodecPrintf("Error: unable to read ico directory header.\n");
        return nullptr;
    }

    // Process the directory header
    const uint16_t numImages = get_short(dirBuffer.get(), 4);
    if (0 == numImages) {
        SkCodecPrintf("Error: No images embedded in ico.\n");
        return nullptr;
    }

    // Ensure that we can read all of indicated directory entries
    SkAutoTDeleteArray<uint8_t> entryBuffer(new uint8_t[numImages * kIcoDirEntryBytes]);
    if (inputStream.get()->read(entryBuffer.get(), numImages*kIcoDirEntryBytes) !=
            numImages*kIcoDirEntryBytes) {
        SkCodecPrintf("Error: unable to read ico directory entries.\n");
        return nullptr;
    }

    // This structure is used to represent the vital information about entries
    // in the directory header.  We will obtain this information for each
    // directory entry.
    struct Entry {
        uint32_t offset;
        uint32_t size;
    };
    SkAutoTDeleteArray<Entry> directoryEntries(new Entry[numImages]);

    // Iterate over directory entries
    for (uint32_t i = 0; i < numImages; i++) {
        // The directory entry contains information such as width, height,
        // bits per pixel, and number of colors in the color palette.  We will
        // ignore these fields since they are repeated in the header of the
        // embedded image.  In the event of an inconsistency, we would always
        // defer to the value in the embedded header anyway.

        // Specifies the size of the embedded image, including the header
        uint32_t size = get_int(entryBuffer.get(), 8 + i*kIcoDirEntryBytes);

        // Specifies the offset of the embedded image from the start of file.
        // It does not indicate the start of the pixel data, but rather the
        // start of the embedded image header.
        uint32_t offset = get_int(entryBuffer.get(), 12 + i*kIcoDirEntryBytes);

        // Save the vital fields
        directoryEntries.get()[i].offset = offset;
        directoryEntries.get()[i].size = size;
    }

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
    SkTQSort(directoryEntries.get(), directoryEntries.get() + numImages - 1,
            lessThan);

    // Now will construct a candidate codec for each of the embedded images
    uint32_t bytesRead = kIcoDirectoryBytes + numImages * kIcoDirEntryBytes;
    SkAutoTDelete<SkTArray<SkAutoTDelete<SkCodec>, true>> codecs(
            new (SkTArray<SkAutoTDelete<SkCodec>, true>)(numImages));
    for (uint32_t i = 0; i < numImages; i++) {
        uint32_t offset = directoryEntries.get()[i].offset;
        uint32_t size = directoryEntries.get()[i].size;

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
        SkAutoTUnref<SkData> data(
                SkData::NewFromStream(inputStream.get(), size));
        if (nullptr == data.get()) {
            SkCodecPrintf("Warning: could not create embedded stream.\n");
            break;
        }
        SkAutoTDelete<SkMemoryStream> embeddedStream(new SkMemoryStream(data.get()));
        bytesRead += size;

        // Check if the embedded codec is bmp or png and create the codec
        SkCodec* codec = nullptr;
        if (SkPngCodec::IsPng((const char*) data->bytes(), data->size())) {
            codec = SkPngCodec::NewFromStream(embeddedStream.release());
        } else {
            codec = SkBmpCodec::NewFromIco(embeddedStream.release());
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
    uint32_t maxSize = 0;
    uint32_t maxIndex = 0;
    for (int32_t i = 0; i < codecs->count(); i++) {
        SkImageInfo info = codecs->operator[](i)->getInfo();
        uint32_t size = info.width() * info.height();
        if (size > maxSize) {
            maxSize = size;
            maxIndex = i;
        }
    }
    int width = codecs->operator[](maxIndex)->getInfo().width();
    int height = codecs->operator[](maxIndex)->getInfo().height();
    SkEncodedInfo info = codecs->operator[](maxIndex)->getEncodedInfo();

    // Note that stream is owned by the embedded codec, the ico does not need
    // direct access to the stream.
    return new SkIcoCodec(width, height, info, codecs.release());
}

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkIcoCodec::SkIcoCodec(int width, int height, const SkEncodedInfo& info,
                       SkTArray<SkAutoTDelete<SkCodec>, true>* codecs)
    : INHERITED(width, height, info, nullptr)
    , fEmbeddedCodecs(codecs)
    , fCurrScanlineCodec(nullptr)
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
                                        const Options& opts, SkPMColor* colorTable,
                                        int* colorCount, int* rowsDecoded) {
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

        SkCodec* embeddedCodec = fEmbeddedCodecs->operator[](index);
        result = embeddedCodec->getPixels(dstInfo, dst, dstRowBytes, &opts, colorTable,
                colorCount);

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
        const SkCodec::Options& options, SkPMColor colorTable[], int* colorCount) {
    int index = 0;
    SkCodec::Result result = kInvalidScale;
    while (true) {
        index = this->chooseCodec(dstInfo.dimensions(), index);
        if (index < 0) {
            break;
        }

        SkCodec* embeddedCodec = fEmbeddedCodecs->operator[](index);
        result = embeddedCodec->startScanlineDecode(dstInfo, &options, colorTable, colorCount);
        if (kSuccess == result) {
            fCurrScanlineCodec = embeddedCodec;
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

SkCodec::SkScanlineOrder SkIcoCodec::onGetScanlineOrder() const {
    // FIXME: This function will possibly return the wrong value if it is called
    //        before startScanlineDecode().
    return fCurrScanlineCodec ? fCurrScanlineCodec->getScanlineOrder() :
            INHERITED::onGetScanlineOrder();
}

SkSampler* SkIcoCodec::getSampler(bool createIfNecessary) {
    return fCurrScanlineCodec ? fCurrScanlineCodec->getSampler(createIfNecessary) : nullptr;
}
