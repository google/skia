/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCodec.h"
#include "SkImageInfo.h"

extern "C" {
    // FIXME: I'd like to force all platforms to use the same decoder, but this
    // means an extra dependency on Mac/Win.
    #include "png.h"
}

class SkStream;

class SkPngCodec : public SkCodec {
public:
    // Assumes IsPng was called and returned true.
    static SkCodec* NewFromStream(SkStream*);
    static bool IsPng(SkStream*);
protected:
    Result onGetPixels(const SkImageInfo&, void*, size_t, SkPMColor*, int*) SK_OVERRIDE;
private:
    png_structp             fPng_ptr;
    png_infop               fInfo_ptr;

    SkPngCodec(const SkImageInfo&, SkStream*, png_structp, png_infop);
    ~SkPngCodec();

    typedef SkCodec INHERITED;
};
