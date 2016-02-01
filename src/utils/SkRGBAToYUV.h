/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRGBAToYUV_DEFINED
#define SkRGBAToYUV_DEFINED

#include "SkPixmap.h"
#include "SkSize.h"

class SkImage;
// Works with any image type at the moment, but in the future it may only work with raster-backed
// images. This really should take a SkPixmap for the input, however the implementation for the
// time being requires an image.
bool SkRGBAToYUV(const SkImage*, const SkISize [3], void* const planes[3],
                 const size_t rowBytes[3], SkYUVColorSpace);

#endif
