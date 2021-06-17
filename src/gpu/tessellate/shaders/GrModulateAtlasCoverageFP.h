/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrModulateAtlasCoverageFP_DEFINED
#define GrGrModulateAtlasCoverageFP_DEFINED

#include "src/gpu/GrFragmentProcessor.h"

// Multiplies 'inputFP' by the coverage value in an atlas, optionally inverting or clamping to 0.
class GrModulateAtlasCoverageFP : public GrFragmentProcessor {
public:
    enum class Flags {
        kNone = 0,
        kInvertCoverage = 1 << 0,  // Return inputColor * (1 - atlasCoverage).
        kCheckBounds = 1 << 1  // Clamp atlasCoverage to 0 if outside the path's valid atlas bounds.
    };

    GR_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    GrModulateAtlasCoverageFP(Flags flags, std::unique_ptr<GrFragmentProcessor> inputFP,
                              GrSurfaceProxyView atlasView, const SkMatrix& devToAtlasMatrix,
                              const SkIRect& devIBounds);

    GrModulateAtlasCoverageFP(const GrModulateAtlasCoverageFP& that);

    const char* name() const override {
        return "GrModulateAtlasCoverageFP";
    }
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder* b) const override {
        b->add32(fFlags & Flags::kCheckBounds);
    }
    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::make_unique<GrModulateAtlasCoverageFP>(*this);
    }
    bool onIsEqual(const GrFragmentProcessor& that) const override {
        auto fp = that.cast<GrModulateAtlasCoverageFP>();
        return fFlags == fp.fFlags && fBounds == fp.fBounds;
    }
    std::unique_ptr<GrGLSLFragmentProcessor> onMakeProgramImpl() const override;

private:
    const Flags fFlags;
    const SkIRect fBounds;
};

GR_MAKE_BITFIELD_CLASS_OPS(GrModulateAtlasCoverageFP::Flags);

#endif
