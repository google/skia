/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkNormalSource_DEFINED
#define SkNormalSource_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/shaders/SkShaderBase.h"

class SkMatrix;
struct SkPoint3;

#if SK_SUPPORT_GPU
class GrFragmentProcessor;
#endif

/** Abstract class that generates or reads in normals for use by SkLightingShader.
*/
class SkNormalSource : public SkFlattenable {
public:
    virtual ~SkNormalSource() override;

#if SK_SUPPORT_GPU
    /** Returns a fragment processor that takes no input and outputs a normal (already rotated)
        as its output color. To be used as a child fragment processor.
    */
    virtual std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const = 0;
#endif

    class Provider {
    public:
        virtual ~Provider() {}

        /** Called for each span of the object being drawn on the CPU. Your subclass should set
            the appropriate normals that correspond to the specified device coordinates.
        */
        virtual void fillScanLine(int x, int y, SkPoint3 output[], int count) const = 0;
    };

    /** Returns an instance of 'Provider' that provides normals for the CPU pipeline. The
        necessary data will be initialized in place at 'storage'.
    */
    virtual Provider* asProvider(const SkShaderBase::ContextRec&, SkArenaAlloc*) const = 0;

    /** Returns a normal source that provides normals sourced from the the normal map argument.

        @param  map  a shader that outputs the normal map
        @param  ctm  the current canvas' total matrix, used to rotate normals when necessary.

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
    static sk_sp<SkNormalSource> MakeFromNormalMap(sk_sp<SkShader> map, const SkMatrix& ctm);

    /** Returns a normal source that provides straight-up normals only <0, 0, 1>.
    */
    static sk_sp<SkNormalSource> MakeFlat();

    static Type GetFlattenableType() { return kSkNormalSource_Type; }
    Type getFlattenableType() const override { return GetFlattenableType(); }

    static sk_sp<SkNormalSource> Deserialize(const void* data, size_t size,
                                             const SkDeserialProcs* procs = nullptr) {
        return sk_sp<SkNormalSource>(static_cast<SkNormalSource*>(
                SkFlattenable::Deserialize(GetFlattenableType(), data, size, procs).release()));
    }
    static void RegisterFlattenables();
};

#endif
