/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "include/core/SkTypes.h"

#include "include/core/SkYUVAIndex.h"
#include "src/gpu/GrFragmentProcessor.h"

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(GrSurfaceProxyView views[],
                                                     const SkYUVAIndex indices[4],
                                                     SkYUVColorSpace yuvColorSpace,
                                                     GrSamplerState samplerState,
                                                     const GrCaps&,
                                                     const SkMatrix& localMatrix = SkMatrix::I(),
                                                     const SkRect* subset = nullptr,
                                                     const SkRect* domain = nullptr);
    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "YUVtoRGBEffect"; }

private:
    GrYUVtoRGBEffect(std::unique_ptr<GrFragmentProcessor> planeFPs[4],
                     int numPlanes,
                     const SkYUVAIndex yuvaIndices[4],
                     const bool snap[2],
                     SkYUVColorSpace yuvColorSpace);

    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);

#if GR_TEST_UTILS
    SkString onDumpInfo() const override;
#endif

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkYUVAIndex      fYUVAIndices[4];
    SkYUVColorSpace  fYUVColorSpace;
    bool             fSnap[2];
};
#endif
