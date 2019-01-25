/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLightingShader_DEFINED
#define SkLightingShader_DEFINED

#include "SkLights.h"
#include "SkShader.h"

class SkBitmap;
class SkMatrix;
class SkNormalSource;

class SkLightingShader {
public:
    /** Returns a shader that lights the shape, colored by the diffuseShader, using the
        normals from normalSource, with the set of lights provided.

        @param  diffuseShader     the shader that provides the colors. If nullptr, uses the paint's
                                  color.
        @param  normalSource      the source for the shape's normals. If nullptr, assumes straight
                                  up normals (<0,0,1>).
        @param  lights            the lights applied to the normals

        The lighting equation is currently:
            result = (LightColor * dot(Normal, LightDir) + AmbientColor) * DiffuseColor

    */
    static sk_sp<SkShader> Make(sk_sp<SkShader> diffuseShader, sk_sp<SkNormalSource> normalSource,
                                sk_sp<SkLights> lights);

    static void RegisterFlattenables();
};

#endif
