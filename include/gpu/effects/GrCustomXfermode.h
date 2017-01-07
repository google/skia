/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermode_DEFINED
#define GrCustomXfermode_DEFINED

#include "SkBlendMode.h"
#include "SkRefCnt.h"

class GrTexture;
class GrXPFactory;

/**
 * Custom Xfer modes are used for blending when the blend mode cannot be represented using blend
 * coefficients.
 */
namespace GrCustomXfermode {
    bool IsSupportedMode(SkBlendMode mode);
    const GrXPFactory* Get(SkBlendMode mode);
};

#endif
