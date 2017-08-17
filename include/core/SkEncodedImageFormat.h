/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedImageFormat_DEFINED
#define SkEncodedImageFormat_DEFINED

#include <stdint.h>

/**
 *  Enum describing format of encoded data.
 */
enum class SkEncodedImageFormat {
#ifdef GOOGLE3
    kUnknown,
#endif
    kBMP,
    kGIF,
    kICO,
    kJPEG,
    kPNG,
    kWBMP,
    kWEBP,
    kPKM,
    kKTX,
    kASTC,
    kDNG,
};

#endif  // SkEncodedImageFormat_DEFINED
