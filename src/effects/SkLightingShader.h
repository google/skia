
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkLightingShader_DEFINED
#define SkLightingShader_DEFINED

#include "SkPoint3.h"
#include "SkShader.h"

class SK_API SkLightingShader {
public:
    struct Light {
        SkVector3   fDirection;       // direction towards the light (+Z is out of the screen).
                                      // If degenerate, it will be replaced with (0, 0, 1).
        SkColor3f   fColor;           // linear (unpremul) color. Range is 0..1 in each channel.
    };

    /** Returns a shader that lights the diffuse and normal maps with a single light.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse     the diffuse bitmap
        @param  normal      the normal map
        @param  light       the light applied to the normal map
        @param  ambient     the linear (unpremul) ambient light color. Range is 0..1/channel.
        @param  localMatrix the matrix mapping the textures to the dest rect 

        NULL will be returned if:
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
    static SkShader* Create(const SkBitmap& diffuse, const SkBitmap& normal,
                            const SkLightingShader::Light& light, const SkColor3f& ambient,
                            const SkMatrix* localMatrix);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
