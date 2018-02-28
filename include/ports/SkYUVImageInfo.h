/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkImageInfo_chrome_DEFINED
#define SkImageInfo_chrome_DEFINED

#ifndef SK_SUPPORT_LEGACY_YUV_COLORSPACE
 /**
 *  Describes the color space a YUV pixel.
 */
enum SkYUVColorSpace {
    /** Standard JPEG color space. */
    kJPEG_SkYUVColorSpace,
    /** SDTV standard Rec. 601 color space. Uses "studio swing" [16, 235] color
       range. See http://en.wikipedia.org/wiki/Rec._601 for details. */
    kRec601_SkYUVColorSpace,
    /** HDTV standard Rec. 709 color space. Uses "studio swing" [16, 235] color
       range. See http://en.wikipedia.org/wiki/Rec._709 for details. */
    kRec709_SkYUVColorSpace,

    kLastEnum_SkYUVColorSpace = kRec709_SkYUVColorSpace,
};
#endif

#endif
