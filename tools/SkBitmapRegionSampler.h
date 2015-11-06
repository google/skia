/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoder.h"
#include "SkImageDecoder.h"
#include "SkTemplates.h"

/*
 * This class aims to duplicate the current implementation of
 * SkBitmapRegionDecoder in Android.
 */
class SkBitmapRegionSampler : public SkBitmapRegionDecoder {
public:

    /*
     * Takes ownership of pointer to decoder
     */
    SkBitmapRegionSampler(SkImageDecoder* decoder, int width, int height);

    bool decodeRegion(SkBitmap* bitmap, SkBitmap::Allocator* allocator,
                      const SkIRect& desiredSubset, int sampleSize,
                      SkColorType colorType, bool requireUnpremul) override;

    bool conversionSupported(SkColorType colorType) override {
        // SkBitmapRegionSampler does not allow the client to check if the conversion
        // is supported.  We will return true as a default.  If the conversion is in
        // fact not supported, decodeRegion() will ignore the prefColorType and choose
        // its own color type.  We catch this and fail non-fatally in our test code.
        return true;
    }

    SkEncodedFormat getEncodedFormat() override { return (SkEncodedFormat) fDecoder->getFormat(); }

private:

    SkAutoTDelete<SkImageDecoder> fDecoder;

    typedef SkBitmapRegionDecoder INHERITED;

};
