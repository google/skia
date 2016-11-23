/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedFormat_DEFINED
#define SkEncodedFormat_DEFINED

#include "SkEncodedImageFormat.h"
#include "SkTypes.h"

#ifdef SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

enum SkEncodedFormat {
    kBMP_SkEncodedFormat  = (int)SkEncodedImageFormat::kBMP,
    kGIF_SkEncodedFormat  = (int)SkEncodedImageFormat::kGIF,
    kICO_SkEncodedFormat  = (int)SkEncodedImageFormat::kICO,
    kJPEG_SkEncodedFormat = (int)SkEncodedImageFormat::kJPEG,
    kPNG_SkEncodedFormat  = (int)SkEncodedImageFormat::kPNG,
    kWBMP_SkEncodedFormat = (int)SkEncodedImageFormat::kWBMP,
    kWEBP_SkEncodedFormat = (int)SkEncodedImageFormat::kWEBP,
    kPKM_SkEncodedFormat  = (int)SkEncodedImageFormat::kPKM,
    kKTX_SkEncodedFormat  = (int)SkEncodedImageFormat::kKTX,
    kASTC_SkEncodedFormat = (int)SkEncodedImageFormat::kASTC,
    kDNG_SkEncodedFormat  = (int)SkEncodedImageFormat::kDNG,
};

#else

typedef SkEncodedImageFormat SkEncodedFormat;

#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

#endif  // SkEncodedFormat_DEFINED
