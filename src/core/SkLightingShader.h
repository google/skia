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

class SK_API SkLightingShader {
public:
    /** Returns a shader that lights the diffuse and normal maps with a set of lights.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse     the diffuse bitmap
        @param  normal      the normal map
        @param  lights       the lights applied to the normal map
        @param  invNormRotation rotation applied to the normal map's normals
        @param  diffLocalMatrix the local matrix for the diffuse map (transform from
                                texture coordinates to shape source coordinates). nullptr is
                                interpreted as an identity matrix.
        @param  normLocalMatrix the local matrix for the normal map (transform from
                                texture coordinates to shape source coordinates). nullptr is
                                interpreted as an identity matrix.

        nullptr will be returned if:
            either 'diffuse' or 'normal' are empty
            either 'diffuse' or 'normal' are too big (> 65535 on a side)
            'diffuse' and 'normal' aren't the same size

        The lighting equation is currently:
            result = LightColor * DiffuseColor * (Normal * LightDir) + AmbientColor

        The normal map is currently assumed to be an 8888 image where the normal at a texel
        is retrieved by:
            N.x = R-127;
            N.y = G-127;
            N.z = B-127;
            N.normalize();
        The +Z axis is thus encoded in RGB as (127, 127, 255) while the -Z axis is
        (127, 127, 0).
    */
    static sk_sp<SkShader> Make(const SkBitmap& diffuse, const SkBitmap& normal,
                                sk_sp<SkLights> lights, const SkVector& invNormRotation,
                                const SkMatrix* diffLocalMatrix, const SkMatrix* normLocalMatrix);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
