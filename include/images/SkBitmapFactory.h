/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkBitmapFactory_DEFINED
#define SkBitmapFactory_DEFINED

class SkBitmap;
class SkData;

/**
 *  General purpose factory for decoding bitmaps.
 *
 *  Currently only provides a way to decode a bitmap or its dimensions from an SkData. Future plans
 *  include options to provide a bitmap which caches the pixel data.
 */
class SkBitmapFactory {

public:
    enum Constraints {
        /**
         *  Only decode the bounds of the bitmap. No pixels will be allocated.
         */
        kDecodeBoundsOnly_Constraint,

        /**
         *  Decode the bounds and pixels of the bitmap.
         */
        kDecodePixels_Constraint,
    };

    /**
     *  Decodes an SkData into an SkBitmap.
     *  @param SkBitmap Already created bitmap to encode into.
     *  @param SkData Encoded SkBitmap data.
     *  @param constraint Specifications for how to do the decoding.
     *  @return True on success. If false, passed in SkBitmap is unmodified.
     */
    static bool DecodeBitmap(SkBitmap*, const SkData*,
                             Constraints constraint = kDecodePixels_Constraint);
};

#endif // SkBitmapFactory_DEFINED
