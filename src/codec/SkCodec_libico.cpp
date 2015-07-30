/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBmpCodec.h"
#include "SkCodec_libico.h"
#include "SkCodec_libpng.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkData.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTSort.h"

/*
 * Checks the start of the stream to see if the image is an Ico or Cur
 */
bool SkIcoCodec::IsIco(SkStream* stream) {
    const char icoSig[] = { '\x00', '\x00', '\x01', '\x00' };
    const char curSig[] = { '\x00', '\x00', '\x02', '\x00' };
    char buffer[sizeof(icoSig)];
    return stream->read(buffer, sizeof(icoSig)) == sizeof(icoSig) &&
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
    SkAutoTDeleteArray<uint8_t> dirBuffer(
            SkNEW_ARRAY(uint8_t, kIcoDirectoryBytes));
    if (inputStream.get()->read(dirBuffer.get(), kIcoDirectoryBytes) !=
            kIcoDirectoryBytes) {
        SkCodecPrintf("Error: unable to read ico directory header.\n");
        return NULL;
    }

    // Process the directory header
    const uint16_t numImages = get_short(dirBuffer.get(), 4);
    if (0 == numImages) {
        SkCodecPrintf("Error: No images embedded in ico.\n");
        return NULL;
    }

    // Ensure that we can read all of indicated directory entries
    SkAutoTDeleteArray<uint8_t> entryBuffer(
            SkNEW_ARRAY(uint8_t, numImages*kIcoDirEntryBytes));
    if (inputStream.get()->read(entryBuffer.get(), numImages*kIcoDirEntryBytes) !=
            numImages*kIcoDirEntryBytes) {
        SkCodecPrintf("Error: unable to read ico directory entries.\n");
        return NULL;
    }

    // This structure is used to represent the vital information about entries
    // in the directory header.  We will obtain this information for each
    // directory entry.
    struct Entry {
        uint32_t offset;
        uint32_t size;
    };
    SkAutoTDeleteArray<Entry> directoryEntries(SkNEW_ARRAY(Entry, numImages));

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
            SkNEW_ARGS((SkTArray<SkAutoTDelete<SkCodec>, true>), (numImages)));
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
        if (NULL == data.get()) {
            SkCodecPrintf("Warning: could not create embedded stream.\n");
            break;
        }
        SkAutoTDelete<SkMemoryStream>
                embeddedStream(SkNEW_ARGS(SkMemoryStream, (data.get())));
        bytesRead += size;

        // Check if the embedded codec is bmp or png and create the codec
        const bool isPng = SkPngCodec::IsPng(embeddedStream);
        SkAssertResult(embeddedStream->rewind());
        SkCodec* codec = NULL;
        if (isPng) {
            codec = SkPngCodec::NewFromStream(embeddedStream.detach());
        } else {
            codec = SkBmpCodec::NewFromIco(embeddedStream.detach());
        }

        // Save a valid codec
        if (NULL != codec) {
            codecs->push_back().reset(codec);
        }
    }

    // Recognize if there are no valid codecs
    if (0 == codecs->count()) {
        SkCodecPrintf("Error: could not find any valid embedded ico codecs.\n");
        return NULL;
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
    SkImageInfo info = codecs->operator[](maxIndex)->getInfo();

    // Note that stream is owned by the embedded codec, the ico does not need
    // direct access to the stream.
    return SkNEW_ARGS(SkIcoCodec, (info, codecs.detach()));
}

/*
 * Creates an instance of the decoder
 * Called only by NewFromStream
 */
SkIcoCodec::SkIcoCodec(const SkImageInfo& info,
                       SkTArray<SkAutoTDelete<SkCodec>, true>* codecs)
    : INHERITED(info, NULL)
    , fEmbeddedCodecs(codecs)
{}

/*
 * Chooses the best dimensions given the desired scale
 */
SkISize SkIcoCodec::onGetScaledDimensions(float desiredScale) const { 
    // We set the dimensions to the largest candidate image by default.
    // Regardless of the scale request, this is the largest image that we
    // will decode.
    if (desiredScale >= 1.0) {
        return this->getInfo().dimensions();
    }

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

/*
 * Initiates the Ico decode
 */
SkCodec::Result SkIcoCodec::onGetPixels(const SkImageInfo& dstInfo,
                                        void* dst, size_t dstRowBytes,
                                        const Options& opts, SkPMColor* ct,
                                        int* ptr) {
    if (opts.fSubset) {
        // Subsets are not supported.
        return kUnimplemented;
    }
    // We return invalid scale if there is no candidate image with matching
    // dimensions.
    Result result = kInvalidScale;
    for (int32_t i = 0; i < fEmbeddedCodecs->count(); i++) {
        // If the dimensions match, try to decode
        if (dstInfo.dimensions() ==
                fEmbeddedCodecs->operator[](i)->getInfo().dimensions()) {

            // Perform the decode
            result = fEmbeddedCodecs->operator[](i)->getPixels(dstInfo,
                    dst, dstRowBytes, &opts, ct, ptr);

            // On a fatal error, keep trying to find an image to decode
            if (kInvalidConversion == result || kInvalidInput == result ||
                    kInvalidScale == result) {
                SkCodecPrintf("Warning: Attempt to decode candidate ico failed.\n");
                continue;
            }

            // On success or partial success, return the result
            return result;
        }
    }

    SkCodecPrintf("Error: No matching candidate image in ico.\n");
    return result;
}
