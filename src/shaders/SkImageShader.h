/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShader_DEFINED
#define SkImageShader_DEFINED

#include "include/core/SkImage.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkShaderBase.h"

// private subclass of SkStageUpdater
class SkImageStageUpdater;

class SkImageShader : public SkShaderBase {
public:
    enum FilterEnum {   // first 4 entries match SkFilterQuality
        kNone,
        kLow,
        kMedium,
        kHigh,
        // this is the special value for backward compatibility
        kInheritFromPaint,
        // this signals we should use the new SkFilterOptions
        kUseFilterOptions,
        // use fCubic and ignore FilterOptions
        kUseCubicResampler,

        kLast = kUseCubicResampler,
    };

    static sk_sp<SkShader> Make(sk_sp<SkImage>,
                                SkTileMode tmx,
                                SkTileMode tmy,
                                const SkMatrix* localMatrix,
                                FilterEnum,
                                bool clampAsIfUnpremul = false);

    static sk_sp<SkShader> Make(sk_sp<SkImage>,
                                SkTileMode tmx,
                                SkTileMode tmy,
                                const SkSamplingOptions&,
                                const SkMatrix* localMatrix);

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    static SkM44 CubicResamplerMatrix(float B, float C);

private:
    SK_FLATTENABLE_HOOKS(SkImageShader)

    SkImageShader(sk_sp<SkImage>,
                  SkTileMode tmx,
                  SkTileMode tmy,
                  const SkMatrix* localMatrix,
                  FilterEnum,
                  bool clampAsIfUnpremul);
    SkImageShader(sk_sp<SkImage>,
                  SkTileMode tmx,
                  SkTileMode tmy,
                  const SkSamplingOptions&,
                  const SkMatrix* localMatrix);

    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#endif
    SkImage* onIsAImage(SkMatrix*, SkTileMode*) const override;

    bool onAppendStages(const SkStageRec&) const override;
    SkStageUpdater* onAppendUpdatableStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider&, const SkMatrix* localM,
                          SkFilterQuality quality, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    bool doStages(const SkStageRec&, SkImageStageUpdater* = nullptr) const;

    SkFilterQuality resolveFiltering(SkFilterQuality paintQuality) const {
        switch (fFilterEnum) {
            case kUseCubicResampler: return kHigh_SkFilterQuality;   // TODO: handle explicitly
            case kUseFilterOptions:  return kNone_SkFilterQuality;   // TODO: handle explicitly
            case kInheritFromPaint:  return paintQuality;
            default: break;
        }
        return (SkFilterQuality)fFilterEnum;
    }

    sk_sp<SkImage>   fImage;
    const SkTileMode fTileModeX;
    const SkTileMode fTileModeY;
    const FilterEnum fFilterEnum;
    const bool       fClampAsIfUnpremul;

    // only use this if fFilterEnum == kUseFilterOptions
    SkFilterOptions  fFilterOptions;
    // only use this if fFilterEnum == kUseCubicResampler or kHigh
    SkCubicResampler fCubic = {1/3.0f, 1/3.0f};  // Default to Mitchell-Netravali.

    friend class SkShaderBase;
    using INHERITED = SkShaderBase;
};

#endif
