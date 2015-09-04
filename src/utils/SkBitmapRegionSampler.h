/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkBitmapRegionDecoderInterface.h"
#include "SkImageDecoder.h"
#include "SkTemplates.h"

/*
 * This class aims to duplicate the current implementation of
 * SkBitmapRegionDecoder in Android.
 */
class SkBitmapRegionSampler : public SkBitmapRegionDecoderInterface {
public:

    /*
     * Takes ownership of pointer to decoder
     */
    SkBitmapRegionSampler(SkImageDecoder* decoder, int width, int height);

    /*
     * Three differences from the Android version:
     *     Returns a Skia bitmap instead of an Android bitmap.
     *     Android version attempts to reuse a recycled bitmap.
     *     Removed the options object and used parameters for color type and
     *     sample size.
     */
    SkBitmap* decodeRegion(int start_x, int start_y, int width, int height,
                           int sampleSize, SkColorType prefColorType) override;

private:

    SkAutoTDelete<SkImageDecoder> fDecoder;

    typedef SkBitmapRegionDecoderInterface INHERITED;

};
