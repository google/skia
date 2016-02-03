/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoder.h"
#include "SkCodec.h"

/*
 * This class implements SkBitmapRegionDecoder using an SkCodec and
 * an SkCanvas.  It uses the scanline decoder to subset the height.  It then
 * will subset the width and scale by drawing to an SkCanvas.
 */
// FIXME: This class works well as a performance/quality comparison for
//        SkBitmapRegionCodec, but it lacks several capabilities that are
//        required by BitmapRegionDecoder in Android.
//        (1) WEBP decodes - because SkWebpCodec does not have a scanline
//            decoder.
//        (2) Decodes to kGray8 and kIndex8.
//        (3) Decodes to kUnpremul.
//        (4) Correcting an invalid dstColorType.  For example, if the
//            client requests kRGB_565 for a non-opaque image, rather than
//            fail, we need to go ahead and decode to kN32.
class SkBitmapRegionCanvas : public SkBitmapRegionDecoder {
public:

    /*
     * Takes ownership of pointer to decoder
     */
    SkBitmapRegionCanvas(SkCodec* decoder);

    bool decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
                      const SkIRect& desiredSubset, int sampleSize,
                      SkColorType colorType, bool requireUnpremul) override;

    bool conversionSupported(SkColorType colorType) override;

    SkEncodedFormat getEncodedFormat() override { return fDecoder->getEncodedFormat(); }

private:

    SkAutoTDelete<SkCodec> fDecoder;

    typedef SkBitmapRegionDecoder INHERITED;

};
