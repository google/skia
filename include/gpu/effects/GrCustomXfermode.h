/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermode_DEFINED
#define GrCustomXfermode_DEFINED

#include "SkXfermode.h"

class GrTexture;

/**
 * Custom Xfer modes are used for blending when the blend mode cannot be represented using blend
 * coefficients.
 */
namespace GrCustomXfermode {
    bool IsSupportedMode(SkXfermode::Mode mode);
    GrXPFactory* CreateXPFactory(SkXfermode::Mode mode);
};

#endif
