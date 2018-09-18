/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkEncodedOrigin_DEFINED
#define SkEncodedOrigin_DEFINED
// These values match the orientation www.exif.org/Exif2-2.PDF.
enum SkEncodedOrigin {
    kTopLeft_SkEncodedOrigin     = 1, // Default
    kTopRight_SkEncodedOrigin    = 2, // Reflected across y-axis
    kBottomRight_SkEncodedOrigin = 3, // Rotated 180
    kBottomLeft_SkEncodedOrigin  = 4, // Reflected across x-axis
    kLeftTop_SkEncodedOrigin     = 5, // Reflected across x-axis, Rotated 90 CCW
    kRightTop_SkEncodedOrigin    = 6, // Rotated 90 CW
    kRightBottom_SkEncodedOrigin = 7, // Reflected across x-axis, Rotated 90 CW
    kLeftBottom_SkEncodedOrigin  = 8, // Rotated 90 CCW
    kDefault_SkEncodedOrigin     = kTopLeft_SkEncodedOrigin,
    kLast_SkEncodedOrigin        = kLeftBottom_SkEncodedOrigin,
};
#endif // SkEncodedOrigin_DEFINED
