/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJpegInfo_DEFINED
#define SkJpegInfo_DEFINED

#include "SkSize.h"

class SkData;

struct SkJFIFInfo {
    SkISize fSize;
    enum Type {
        kGrayscale,
        kYCbCr,
    } fType;
};

/** Returns true iff the data seems to be a valid JFIF JPEG image.  
    If so and if info is not nullptr, populate info.

    JPEG/JFIF References:
        http://www.w3.org/Graphics/JPEG/itu-t81.pdf
        http://www.w3.org/Graphics/JPEG/jfif3.pdf
*/
bool SkIsJFIF(const SkData* skdata, SkJFIFInfo* info);

#endif  // SkJpegInfo_DEFINED
