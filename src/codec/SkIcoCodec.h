/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkImageInfo.h"
#include "SkStream.h"
#include "SkTypes.h"

/*
 * This class implements the decoding for bmp images
 */
class SkIcoCodec : public SkCodec {
public:
    static bool IsIco(const void*, size_t);

    /*
     * Assumes IsIco was called and returned true
     * Creates an Ico decoder
     * Reads enough of the stream to determine the image format
     */
    static SkCodec* NewFromStream(SkStream*);

protected:

    /*
     * Chooses the best dimensions given the desired scale
     */
    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

    /*
     * Initiates the Ico decode
     */
    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            SkPMColor*, int*, int*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kICO_SkEncodedFormat;
    }

    SkScanlineOrder onGetScanlineOrder() const override;

private:

    Result onStartScanlineDecode(const SkImageInfo& dstInfo, const SkCodec::Options& options,
            SkPMColor inputColorPtr[], int* inputColorCount) override;

    int onGetScanlines(void* dst, int count, size_t rowBytes) override;

    bool onSkipScanlines(int count) override;

    SkSampler* getSampler(bool createIfNecessary) override;

    /*
     * Searches fEmbeddedCodecs for a codec that matches requestedSize.
     * The search starts at startIndex and ends when an appropriate codec
     * is found, or we have reached the end of the array.
     *
     * @return the index of the matching codec or -1 if there is no
     *         matching codec between startIndex and the end of
     *         the array.
     */
    int chooseCodec(const SkISize& requestedSize, int startIndex);

    /*
     * Constructor called by NewFromStream
     * @param embeddedCodecs codecs for the embedded images, takes ownership
     */
    SkIcoCodec(const SkImageInfo& srcInfo, SkTArray<SkAutoTDelete<SkCodec>, true>* embeddedCodecs);

    SkAutoTDelete<SkTArray<SkAutoTDelete<SkCodec>, true>> fEmbeddedCodecs; // owned

    // Only used by the scanline decoder.  onStartScanlineDecode() will set
    // fCurrScanlineCodec to one of the fEmbeddedCodecs, if it can find a
    // codec of the appropriate size.  We will use fCurrScanlineCodec for
    // subsequent calls to onGetScanlines() or onSkipScanlines().
    // fCurrScanlineCodec is owned by this class, but should not be an
    // SkAutoTDelete.  It will be deleted by the destructor of fEmbeddedCodecs.
    SkCodec* fCurrScanlineCodec;

    typedef SkCodec INHERITED;
};
