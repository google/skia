/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrCustomXfermode_DEFINED
#define GrCustomXfermode_DEFINED

#include "SkXfermode.h"

class GrFragmentProcessor;
class GrTexture;

/**
 * Custom Xfer modes are used for blending when the blend mode cannot be represented using blend
 * coefficients. It is assumed that all blending is done within the processors' emit code. For each
 * blend mode there should be a matching fragment processor (used when blending with a background
 * texture) and xfer processor.
 */
namespace GrCustomXfermode {
    bool IsSupportedMode(SkXfermode::Mode mode); 

    GrFragmentProcessor* CreateFP(GrProcessorDataManager*, SkXfermode::Mode mode,
                                  GrTexture* background);

    GrXPFactory* CreateXPFactory(SkXfermode::Mode mode);
};

#endif
