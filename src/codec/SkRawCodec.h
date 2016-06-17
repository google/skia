/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRawCodec_DEFINED
#define SkRawCodec_DEFINED

#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkImageInfo.h"
#include "SkTypes.h"

class SkDngImage;
class SkStream;

/*
 *
 * This class implements the decoding for RAW images
 *
 */
class SkRawCodec : public SkCodec {
public:

    /*
     * Creates a RAW decoder
     * Takes ownership of the stream
     */
    static SkCodec* NewFromStream(SkStream*);

    ~SkRawCodec() override;

protected:

    Result onGetPixels(const SkImageInfo& dstInfo, void* dst, size_t dstRowBytes, const Options&,
            SkPMColor*, int*, int*) override;

    SkEncodedFormat onGetEncodedFormat() const override {
        return kDNG_SkEncodedFormat;
    }

    SkISize onGetScaledDimensions(float desiredScale) const override;

    bool onDimensionsSupported(const SkISize&) override;

private:

    /*
     * Creates an instance of the decoder
     * Called only by NewFromStream, takes ownership of dngImage.
     */
    SkRawCodec(SkDngImage* dngImage);

    SkAutoTDelete<SkDngImage> fDngImage;

    typedef SkCodec INHERITED;
};

#endif
