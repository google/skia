/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkAndroidCodec.h"

/*
 * This class implements SkBitmapRegionDecoder using an SkAndroidCodec.
 */
class SkBitmapRegionCodec : public SkBitmapRegionDecoderInterface {
public:

    /*
     * Takes ownership of pointer to codec
     */
    SkBitmapRegionCodec(SkAndroidCodec* codec);

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

    SkAutoTDelete<SkAndroidCodec> fCodec;

    typedef SkBitmapRegionDecoderInterface INHERITED;

};
