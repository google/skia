/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoder.h"
#include "SkAndroidCodec.h"

/*
 * This class implements SkBitmapRegionDecoder using an SkAndroidCodec.
 */
class SkBitmapRegionCodec : public SkBitmapRegionDecoder {
public:

    /*
     * Takes ownership of pointer to codec
     */
    SkBitmapRegionCodec(SkAndroidCodec* codec);

    bool decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
                      const SkIRect& desiredSubset, int sampleSize,
                      SkColorType colorType, bool requireUnpremul) override;

    bool conversionSupported(SkColorType colorType) override;

    SkEncodedFormat getEncodedFormat() override { return fCodec->getEncodedFormat(); }

private:

    SkAutoTDelete<SkAndroidCodec> fCodec;

    typedef SkBitmapRegionDecoder INHERITED;

};
