/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrColorSpaceXform_DEFINED
#define GrColorSpaceXform_DEFINED

#include "GrColor.h"
#include "GrFragmentProcessor.h"
#include "SkMatrix44.h"
#include "SkRefCnt.h"

class SkColorSpace;

 /**
  * Represents a color gamut transformation (as a 4x4 color matrix)
  */
class GrColorSpaceXform : public SkRefCnt {
public:
    GrColorSpaceXform(const SkMatrix44& srcToDst);

    static sk_sp<GrColorSpaceXform> Make(const SkColorSpace* src, const SkColorSpace* dst);

    const SkMatrix44& srcToDst() const { return fSrcToDst; }

    /**
     * GrGLSLFragmentProcessor::GenKey() must call this and include the returned value in its
     * computed key.
     */
    static uint32_t XformKey(const GrColorSpaceXform* xform) {
        // Code generation changes if there is an xform, but it otherwise constant
        return SkToBool(xform) ? 1 : 0;
    }

    static bool Equals(const GrColorSpaceXform* a, const GrColorSpaceXform* b);

    GrColor4f unclampedXform(const GrColor4f& srcColor);
    GrColor4f clampedXform(const GrColor4f& srcColor);

private:
    SkMatrix44 fSrcToDst;
};

class GrColorSpaceXformEffect : public GrFragmentProcessor {
public:
    /**
     *  Returns a fragment processor that calls the passed in fragment processor, and then converts
     *  the color space of the output from src to dst.
     */
    static std::unique_ptr<GrFragmentProcessor> Make(std::unique_ptr<GrFragmentProcessor> child,
                                                     const SkColorSpace* src,
                                                     const SkColorSpace* dst);

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
