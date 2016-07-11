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

class SK_API SkLightingShader {
public:
    /** Returns a shader that lights the shape, colored by the diffuseShader, using the
        normals from normalSource, with the set of lights provided.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuseShader     the shader that provides the colors
        @param  normalSource      the source for the shape's normals
        @param  lights            the lights applied to the normals

        The lighting equation is currently:
            result = LightColor * DiffuseColor * (Normal * LightDir) + AmbientColor

    */
    static sk_sp<SkShader> Make(sk_sp<SkShader> diffuseShader, sk_sp<SkNormalSource> normalSource,
                                sk_sp<SkLights> lights);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
