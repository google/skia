/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedFormat_DEFINED
#define SkEncodedFormat_DEFINED

#include "SkEncodedImageFormat.h"

enum SkEncodedFormat : uint8_t {
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

#endif  // SkEncodedFormat_DEFINED
