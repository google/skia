/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkBitmapRegionDecoder_DEFINED
#define SkBitmapRegionDecoder_DEFINED

#include "SkBitmap.h"
#include "SkImageDecoder.h"
#include "SkStream.h"

struct SkIRect;

/**
 * SkBitmapRegionDecoder can be used to decode a specified rect from an image.
 * This is particularly useful when the original image is large and you only
 * need parts of the image.
 *
 * However, not all image codecs on all platforms support this feature so be
 * prepared to fallback to standard decoding if decodeRegion(...) returns false.
 */
class SkBitmapRegionDecoder {
public:
    SkBitmapRegionDecoder(SkImageDecoder* decoder, SkStream* stream,
                          int width, int height) {
        fDecoder = decoder;
        fStream = stream;
        fWidth = width;
        fHeight = height;
    }
    ~SkBitmapRegionDecoder() {
        SkDELETE(fDecoder);
        SkSafeUnref(fStream);
    }

    bool decodeRegion(SkBitmap* bitmap, const SkIRect& rect,
                      SkBitmap::Config pref, int sampleSize);

    SkImageDecoder* getDecoder() const { return fDecoder; }
    int getWidth() const { return fWidth; }
    int getHeight() const { return fHeight; }

private:
    SkImageDecoder* fDecoder;
    SkStream* fStream;
    int fWidth;
    int fHeight;
};

#endif
