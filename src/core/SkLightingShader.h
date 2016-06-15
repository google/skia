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
    /** Abstract class that generates or reads in normals for use by SkLightingShader. Currently
        implements the GPU side only. Not to be used as part of the API yet. Used internally by
        SkLightingShader.
    */
    class NormalSource : public SkFlattenable {
    public:
        virtual ~NormalSource();

#if SK_SUPPORT_GPU

        /** Returns a fragment processor that takes no input and outputs a normal (already rotated)
            as its output color. To be used as a child fragment processor.
        */
        virtual sk_sp<GrFragmentProcessor> asFragmentProcessor(
                GrContext* context,
                const SkMatrix& viewM,
                const SkMatrix* localMatrix,
                SkFilterQuality filterQuality,
                SkSourceGammaTreatment gammaTreatment) const = 0;
#endif

        SK_DEFINE_FLATTENABLE_TYPE(NormalSource)
    };

    /** Returns a normal source that provides normals sourced from the the normal map argument.
        Not to be used as part of the API yet. Used internally by SkLightingShader.

        @param  normal                the normal map
        @param  invNormRotation       rotation applied to the normal map's normals
        @param  normLocalM            the local matrix for the normal map

        nullptr will be returned if
            'normal' is empty
            'normal' too big (> 65535 on either side)

        The normal map is currently assumed to be an 8888 image where the normal at a texel
        is retrieved by:
            N.x = R-127;
            N.y = G-127;
            N.z = B-127;
            N.normalize();
        The +Z axis is thus encoded in RGB as (127, 127, 255) while the -Z axis is
        (127, 127, 0).
    */
    class NormalMapSource {
    public:
        static sk_sp<NormalSource> Make(const SkBitmap& normal, const SkVector& invNormRotation,
                                        const SkMatrix* normLocalM);

        SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
    };

    /** Returns a shader that lights the diffuse and normal maps with a set of lights.

        It returns a shader with a reference count of 1.
        The caller should decrement the shader's reference count when done with the shader.
        It is an error for count to be < 2.
        @param  diffuse     the diffuse bitmap
        @param  normal      the normal map
        @param  lights       the lights applied to the normal map
        @param  invNormRotation rotation applied to the normal map's normals
        @param  diffLocalMatrix the local matrix for the diffuse texture
        @param  normLocalMatrix the local matrix for the normal map

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
