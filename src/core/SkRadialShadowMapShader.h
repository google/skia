/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkReadBuffer.h"

#ifndef SkRadialShadowMapShader_DEFINED
#define SkRadialShadowMapShader_DEFINED

#ifdef SK_EXPERIMENTAL_SHADOWING

class SkLights;
class SkShader;

class SK_API SkRadialShadowMapShader {
public:
    /** This shader creates a 1D strip depth map for radial lights.
     *  It can only take in 1 light to generate one shader at a time.
     */
    static sk_sp<SkShader> Make(sk_sp<SkShader> occluderShader,
                                sk_sp<SkLights> light,
                                int diffuseWidth, int diffuseHeight);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
#endif
