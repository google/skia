/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGr_DEFINED
#define SkGr_DEFINED

#include "SkRefCnt.h"

class GrContext;
class GrSamplerParams;
class GrTexture;
class SkBitmap;

////////////////////////////////////////////////////////////////////////////////
/** Returns a texture representing the bitmap that is compatible with the GrSamplerParams. The
    texture is inserted into the cache (unless the bitmap is marked volatile) and can be
    retrieved again via this function. */
GrTexture* GrRefCachedBitmapTexture(GrContext*, const SkBitmap&, const GrSamplerParams&);

sk_sp<GrTexture> GrMakeCachedBitmapTexture(GrContext*, const SkBitmap&, const GrSamplerParams&);

#endif
