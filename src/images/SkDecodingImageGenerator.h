/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDecodingImageGenerator_DEFINED
#define SkDecodingImageGenerator_DEFINED

#include "SkImageGenerator.h"

class SkBitmap;

/**
 * Calls into SkImageDecoder::DecodeMemoryToTarget to implement a
 * SkImageGenerator
 */
class SkDecodingImageGenerator : public SkImageGenerator {
public:
    /*
     *  The constructor will take a reference to the SkData.  The
     *  destructor will unref() it.
     */
    SkDecodingImageGenerator(SkData* data);
    virtual ~SkDecodingImageGenerator();

    virtual SkData* refEncodedData() SK_OVERRIDE;

    virtual bool getInfo(SkImageInfo* info) SK_OVERRIDE;

    virtual bool getPixels(const SkImageInfo& info,
                           void* pixels,
                           size_t rowBytes) SK_OVERRIDE;

    /**
     *  Install the SkData into the destination bitmap, using a new
     *  SkDiscardablePixelRef and a new SkDecodingImageGenerator.
     */
    static bool Install(SkData* data, SkBitmap* destination);

private:
    SkData* fData;
};
#endif  // SkDecodingImageGenerator_DEFINED
