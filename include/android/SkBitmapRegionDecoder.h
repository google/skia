/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapRegionDecoder_DEFINED
#define SkBitmapRegionDecoder_DEFINED

#include "include/android/SkBRDAllocator.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkStream.h"

/*
 * This class aims to provide an interface to test multiple implementations of
 * SkBitmapRegionDecoder.
 */
class SK_API SkBitmapRegionDecoder {
public:

    enum Strategy {
        kAndroidCodec_Strategy, // Uses SkAndroidCodec for scaling and subsetting
    };

    /*
     * @param data     Refs the data while this object exists, unrefs on destruction
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */
    static SkBitmapRegionDecoder* Create(sk_sp<SkData>, Strategy strategy);

    /*
     * @param stream   Takes ownership of the stream
     * @param strategy Strategy used for scaling and subsetting
     * @return         Tries to create an SkBitmapRegionDecoder, returns NULL on failure
     */
    static SkBitmapRegionDecoder* Create(
            SkStreamRewindable* stream, Strategy strategy);

    /*
     * Decode a scaled region of the encoded image stream
     *
     * @param bitmap          Container for decoded pixels.  It is assumed that the pixels
     *                        are initially unallocated and will be allocated by this function.
     * @param allocator       Allocator for the pixels.  If this is NULL, the default
     *                        allocator (HeapAllocator) will be used.
     * @param desiredSubset   Subset of the original image to decode.
     * @param sampleSize      An integer downscaling factor for the decode.
     * @param colorType       Preferred output colorType.
     *                        New implementations should return NULL if they do not support
     *                        decoding to this color type.
     *                        The old kOriginal_Strategy will decode to a default color type
     *                        if this color type is unsupported.
     * @param requireUnpremul If the image is not opaque, we will use this to determine the
     *                        alpha type to use.
     * @param prefColorSpace  If non-null and supported, this is the color space that we will
     *                        decode into.  Otherwise, we will choose a default.
     *
     */
    virtual bool decodeRegion(SkBitmap* bitmap, SkBRDAllocator* allocator,
                              const SkIRect& desiredSubset, int sampleSize,
                              SkColorType colorType, bool requireUnpremul,
                              sk_sp<SkColorSpace> prefColorSpace = nullptr) = 0;

    virtual SkEncodedImageFormat getEncodedFormat() = 0;

    virtual SkColorType computeOutputColorType(SkColorType requestedColorType) = 0;

    virtual sk_sp<SkColorSpace> computeOutputColorSpace(SkColorType outputColorType,
            sk_sp<SkColorSpace> prefColorSpace = nullptr) = 0;


    int width() const { return fWidth; }
    int height() const { return fHeight; }

    virtual ~SkBitmapRegionDecoder() {}

protected:

    SkBitmapRegionDecoder(int width, int height)
        : fWidth(width)
        , fHeight(height)
    {}

private:
    const int fWidth;
    const int fHeight;
};

#endif
