/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalSource_DEFINED
#define SkNormalSource_DEFINED

#include "SkFlattenable.h"

/** Abstract class that generates or reads in normals for use by SkLightingShader. Not to be
    used as part of the API yet. Used internally by SkLightingShader.
*/
class SK_API SkNormalSource : public SkFlattenable {
public:
    virtual ~SkNormalSource() override;

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

    class Provider {
    public:
        virtual ~Provider() {};

        /** Called for each span of the object being drawn on the CPU. Your subclass should set
            the appropriate normals that correspond to the specified device coordinates.
        */
        virtual void fillScanLine(int x, int y, SkPoint3 output[], int count) const = 0;
    };

    /** Returns an instance of 'Provider' that provides normals for the CPU pipeline. The
        necessary data will be initialized in place at 'storage'.
    */
    virtual Provider* asProvider(const SkShader::ContextRec&, void* storage) const = 0;

    /** Amount of memory needed to store a provider object and its dependencies.
     */
    virtual size_t providerSize(const SkShader::ContextRec&) const = 0;

    /** Returns a normal source that provides normals sourced from the the normal map argument.
        Not to be used as part of the API yet. Used internally by SkLightingShader.

        @param  map              a shader that outputs the normal map
        @param  normRotation     rotation applied to the normal map's normals, in the
                                 [cos a, sin a] form.

        nullptr will be returned if 'map' is null

        The normal map is currently assumed to be an 8888 image where the normal at a texel
        is retrieved by:
            N.x = R-127;
            N.y = G-127;
            N.z = B-127;
            N.normalize();
        The +Z axis is thus encoded in RGB as (127, 127, 255) while the -Z axis is
        (127, 127, 0).
    */
    static sk_sp<SkNormalSource> MakeFromNormalMap(sk_sp<SkShader> map,
                                                   const SkVector& normRotation);

    SK_DEFINE_FLATTENABLE_TYPE(SkNormalSource)
    SK_DECLARE_FLATTENABLE_REGISTRAR_GROUP()
};

#endif
