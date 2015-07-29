
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
        SkColor     fColor;           // linear (unpremul) color. Note: alpha assumed to be 255.
    };

    /** Returns a shader that lights the diffuse and normal maps with a single light.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse the diffuse bitmap
        @param  normal  the normal map
        @param  light   the light applied to the normal map
        @param  ambient the linear (unpremul) ambient light color. Note: alpha assumed to be 255.

        NULL will be returned if:
            either 'diffuse' or 'normal' are empty
            either 'diffuse' or 'normal' are too big (> 65535 on a side)
            'diffuse' and 'normal' aren't the same size
    */
    static SkShader* Create(const SkBitmap& diffuse, const SkBitmap& normal,
                            const SkLightingShader::Light& light, const SkColor ambient);

    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
