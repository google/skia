/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGaussianEdgeShader_DEFINED
#define SkGaussianEdgeShader_DEFINED

#include "SkShader.h"

class SK_API SkGaussianEdgeShader {
public:
    /** Returns a shader that applies a Gaussian blur depending on distance to the edge
    * Currently this is only useable with Circle and RRect shapes on the GPU backend.
    * Raster will draw nothing.
    */
    static sk_sp<SkShader> Make();

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()

private:
    SkGaussianEdgeShader(); // can't be instantiated
};

#endif
