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

    /*
     * Checks the start of the stream to see if the image is a Ico or Cur
     */
    static bool IsIco(SkStream*);

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

private:

    /*
     * Constructor called by NewFromStream
     * @param embeddedCodecs codecs for the embedded images, takes ownership
     */
    SkIcoCodec(const SkImageInfo& srcInfo,
               SkTArray<SkAutoTDelete<SkCodec>, true>* embeddedCodecs);

    SkAutoTDelete<SkTArray<SkAutoTDelete<SkCodec>, true>>
            fEmbeddedCodecs; // owned

    typedef SkCodec INHERITED;
};
