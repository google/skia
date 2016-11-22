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
    kBMP_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kBMP,
    kGIF_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kGIF,
    kICO_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kICO,
    kJPEG_SkEncodedFormat = (uint8_t)SkEncodedImageFormat::kJPEG,
    kPNG_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kPNG,
    kWBMP_SkEncodedFormat = (uint8_t)SkEncodedImageFormat::kWBMP,
    kWEBP_SkEncodedFormat = (uint8_t)SkEncodedImageFormat::kWEBP,
    kPKM_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kPKM,
    kKTX_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kKTX,
    kASTC_SkEncodedFormat = (uint8_t)SkEncodedImageFormat::kASTC,
    kDNG_SkEncodedFormat  = (uint8_t)SkEncodedImageFormat::kDNG,
};

#else

typedef SkEncodedImageFormat SkEncodedFormat;

#endif  // SK_SUPPORT_LEGACY_IMAGE_ENCODER_CLASS

#endif  // SkEncodedFormat_DEFINED
