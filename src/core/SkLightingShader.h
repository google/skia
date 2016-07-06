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
    /** Returns a shader that lights the diffuse map using the normals and a set of lights.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse     the diffuse bitmap
        @param  lights       the lights applied to the normal map
        @param  diffLocalMatrix the local matrix for the diffuse map (transform from
                                texture coordinates to shape source coordinates). nullptr is
                                interpreted as an identity matrix.
        @param  normalSource the source for the normals

        nullptr will be returned if:
            'diffuse' is empty
            'diffuse' is too big (> 65535 on any side)

        The lighting equation is currently:
            result = LightColor * DiffuseColor * (Normal * LightDir) + AmbientColor

    */
    static sk_sp<SkShader> Make(const SkBitmap& diffuse, sk_sp<SkLights> lights,
                                const SkMatrix* diffLocalMatrix,
                                sk_sp<SkNormalSource> normalSource);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
