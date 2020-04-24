/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkJpegInfo_DEFINED
#define SkJpegInfo_DEFINED

#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkSize.h"
#include "include/private/SkEncodedInfo.h"

/** Returns true if the data seems to be a valid JPEG image with a known colorType.

    @param [out] size        Image size in pixels
    @param [out] colorType   Encoded color type (kGray_Color, kYUV_Color, several others).
    @param [out] orientation EXIF Orientation of the image.
*/
bool SkGetJpegInfo(const void* data, size_t len,
                   SkISize* size,
                   SkEncodedInfo::Color* colorType,
                   SkEncodedOrigin* orientation);

#endif  // SkJpegInfo_DEFINED
