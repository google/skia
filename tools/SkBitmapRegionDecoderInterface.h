/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapRegionDecoder_DEFINED
#define SkBitmapRegionDecoder_DEFINED

#include "SkBitmap.h"
#include "SkStream.h"

/*
 * This class aims to provide an interface to test multiple implementations of
 * SkBitmapRegionDecoder.
 */
class SkBitmapRegionDecoderInterface {
public:

    enum Strategy {
        kCanvas_Strategy,       // Draw to the canvas, uses SkCodec
        kOriginal_Strategy,     // Sampling, uses SkImageDecoder
        kAndroidCodec_Strategy, // Uses SkAndroidCodec for scaling and subsetting
    };

    /*
     * @param data     Refs the data while this object exists, unrefs on destruction
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */
    static SkBitmapRegionDecoderInterface* CreateBitmapRegionDecoder(
            SkData* data, Strategy strategy);

    /*
     * Decode a scaled region of the encoded image stream
     *
     * @param start_x    X-coordinate of upper-left corner of region.
     *                   This coordinate is unscaled, relative to the original dimensions.
     * @param start_y    Y-coordinate of upper-left corner of region.
     *                   This coordinate is unscaled, relative to the original dimensions.
     * @param width      Width of the region to decode.
     *                   This distance is unscaled, relative to the original dimensions.
     * @param height     Height of the region to decode.
     *                   This distance is unscaled, relative to the original dimensions.
     * @param sampleSize An integer downscaling factor for the decode.
     * @param colorType  Preferred output colorType.
     *                   New implementations should return NULL if they do not support
     *                   decoding to this color type.
     *                   The old kOriginal_Strategy will decode to a default color type
     *                   if this color type is unsupported.
     * @return           Pointer to a bitmap of the decoded region on success, NULL on
     *                   failure.
     */
    virtual SkBitmap* decodeRegion(int start_x, int start_y, int width,
                                   int height, int sampleSize,
                                   SkColorType colorType) = 0;
    /*
     * @param  Requested destination color type
     * @return true if we support the requested color type and false otherwise
     */
    virtual bool conversionSupported(SkColorType colorType) = 0;

    int width() const { return fWidth; }
    int height() const { return fHeight; }

    virtual ~SkBitmapRegionDecoderInterface() {}

protected:

    SkBitmapRegionDecoderInterface(int width, int height)
        : fWidth(width)
        , fHeight(height)
    {}

private:
    const int fWidth;
    const int fHeight;
};

#endif
