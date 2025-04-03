/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGrModulateAtlasCoverageEffect_DEFINED
#define GrGrModulateAtlasCoverageEffect_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkMacros.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"

#include <memory>

class GrSurfaceProxyView;
class SkMatrix;
namespace skgpu { class KeyBuilder; }
struct GrShaderCaps;

// Multiplies 'inputFP' by the coverage value in an atlas, optionally inverting or clamping to 0.
class GrModulateAtlasCoverageEffect : public GrFragmentProcessor {
public:
    enum class Flags {
        kNone = 0,
        kInvertCoverage = 1 << 0,  // Return inputColor * (1 - atlasCoverage).
        kCheckBounds = 1 << 1  // Clamp atlasCoverage to 0 if outside the path's valid atlas bounds.
    };

    SK_DECL_BITFIELD_CLASS_OPS_FRIENDS(Flags);

    GrModulateAtlasCoverageEffect(Flags flags, std::unique_ptr<GrFragmentProcessor> inputFP,
                                  GrSurfaceProxyView atlasView, const SkMatrix& devToAtlasMatrix,
                                  const SkIRect& devIBounds);

    GrModulateAtlasCoverageEffect(const GrModulateAtlasCoverageEffect& that);

    const char* name() const override {
        return "GrModulateAtlasCoverageFP";
    }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::make_unique<GrModulateAtlasCoverageEffect>(*this);
    }

private:
    void onAddToKey(const GrShaderCaps&, skgpu::KeyBuilder* b) const override;

    bool onIsEqual(const GrFragmentProcessor& that) const override {
        auto fp = that.cast<GrModulateAtlasCoverageEffect>(); // NOLINT
        return fFlags == fp.fFlags && fBounds == fp.fBounds;
    }
    std::unique_ptr<ProgramImpl> onMakeProgramImpl() const override;

    const Flags fFlags;
    const SkIRect fBounds;
};

SK_MAKE_BITFIELD_CLASS_OPS(GrModulateAtlasCoverageEffect::Flags)

#endif
