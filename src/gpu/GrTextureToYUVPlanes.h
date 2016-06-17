/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureToYUVPlanes_DEFINED
#define GrTextureToYUVPlanes_DEFINED

#include "SkImageInfo.h"
#include "SkSize.h"

class GrTexture;

bool GrTextureToYUVPlanes(GrTexture* texture, const SkISize[3], void* const planes[3],
                          const size_t rowBytes[3], SkYUVColorSpace);

#endif
