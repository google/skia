/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkData.h"
#include "SkImageGenerator.h"

class SkCodecImageGenerator : public SkImageGenerator {
public:
    /*
     * If this data represents an encoded image that we know how to decode,
     * return an SkCodecImageGenerator.  Otherwise return nullptr.
     *
     * Refs the data if an image generator can be returned.  Otherwise does
     * not affect the data.
     */
    static SkImageGenerator* NewFromEncodedCodec(SkData* data);

protected:
    SkData* onRefEncodedData(SK_REFENCODEDDATA_CTXPARAM) override;

    bool onGetPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, SkPMColor ctable[],
            int* ctableCount) override;

    bool onGetYUV8Planes(SkISize sizes[3], void* planes[3], size_t rowBytes[3],
            SkYUVColorSpace* colorSpace) override;

private:
    /*
     * Takes ownership of codec
     * Refs the data
     */
    SkCodecImageGenerator(SkCodec* codec, SkData* data);

    SkAutoTDelete<SkCodec> fCodec;
    SkAutoTUnref<SkData> fData;

    typedef SkImageGenerator INHERITED;
};
