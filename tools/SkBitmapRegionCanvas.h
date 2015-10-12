/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkCodec.h"

/*
 * This class implements SkBitmapRegionDecoder using an SkCodec and
 * an SkCanvas.  It uses the scanline decoder to subset the height.  It then
 * will subset the width and scale by drawing to an SkCanvas.
 */
// FIXME (msarett): This implementation does not support WEBP, because WEBP
// does not have a scanline decoder.
class SkBitmapRegionCanvas : public SkBitmapRegionDecoderInterface {
public:

    /*
     * Takes ownership of pointer to decoder
     */
    SkBitmapRegionCanvas(SkCodec* decoder);

    /*
     * Three differences from the Android version:
     *     Returns a Skia bitmap instead of an Android bitmap.
     *     Android version attempts to reuse a recycled bitmap.
     *     Removed the options object and used parameters for color type and
     *     sample size.
     */
    SkBitmap* decodeRegion(int start_x, int start_y, int width, int height,
                           int sampleSize, SkColorType prefColorType) override;

    bool conversionSupported(SkColorType colorType) override;

private:

    SkAutoTDelete<SkCodec> fDecoder;

    typedef SkBitmapRegionDecoderInterface INHERITED;

};
