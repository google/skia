/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "include/core/SkRefCnt.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "src/gpu/GrFragmentProcessor.h"

class SkColorSpace;

 /**
  * Represents a color space transformation
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkColorSpaceXformSteps& steps) : fSteps(steps) {}

    static sk_sp<GrColorSpaceXform> Make(SkColorSpace* src, SkAlphaType srcAT,
                                         SkColorSpace* dst, SkAlphaType dstAT);

    const SkColorSpaceXformSteps& steps() const { return fSteps; }

    /**
     * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in its
     * computed key.
     */
    static uint32_t XformKey(const GrColorSpaceXform* xform) {
        // Code generation depends on which steps we apply
        return xform ? xform->fSteps.flags.mask() : 0;
    }

    static bool Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b);

    SkColor4f apply(const SkColor4f& srcColor);

private:
    friend class GrGLSLColorSpaceXformHelper;

    SkColorSpaceXformSteps fSteps;
};

class GrColorSpaceXformEffect : public GrFragmentProcessor {
public:
    /**
     *  Returns a fragment processor that converts the input's color space from src to dst.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(SkColorSpace* src, SkAlphaType srcAT,
                                                     SkColorSpace* dst, SkAlphaType dstAT);

    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then converts
     *  the color space of the output from src to dst.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child,
                                                     SkColorSpace* src, SkAlphaType srcAT,
                                                     SkColorSpace* dst);

    /**
     * Returns a fragment processor that calls the passed in FP and then converts it with the given
     * color xform. Returns null if child is null, returns child if the xform is null (e.g. noop).
     */
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child,
                                                     sk_sp<GrColorSpaceXform> colorXform);

    const char* name() const override { return "ColorSpaceXform"; }
    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const GrColorSpaceXform* colorXform() const { return fColorXform.get(); }

private:
    GrColorSpaceXformEffect(std::unique_ptr<GrFragmentProcessor> child,
                            sk_sp<GrColorSpaceXform> colorXform);

    static OptimizationFlags OptFlags(const GrFragmentProcessor* child);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;

    sk_sp<GrColorSpaceXform> fColorXform;

    typedef GrFragmentProcessor INHERITED;
};

#endif
