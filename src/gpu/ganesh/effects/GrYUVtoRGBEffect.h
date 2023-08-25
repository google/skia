/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkString.h"
#include "include/core/SkYUVAInfo.h"
#include "src/core/SkYUVAInfoLocation.h" // IWYU pragma: keep
#include "src/gpu/ganesh/GrCaps.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/GrProcessorUnitTest.h"
#include "src/gpu/ganesh/GrSamplerState.h"

#include <memory>

class GrYUVATextureProxies;
enum SkYUVColorSpace : int;
namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;
struct SkRect;

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const GrYUVATextureProxies& yuvaProxies,
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
                     const SkYUVAInfo::YUVALocations&,
                     const bool snap[2],
                     SkYUVColorSpace yuvColorSpace);

    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);

#if defined(GR_TEST_UTILS)
    SkString onDumpInfo() const override;
#endif

    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    SkYUVAInfo::YUVALocations   fLocations;
    SkYUVColorSpace             fYUVColorSpace;
    bool                        fSnap[2];
};
#endif
