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
    };

    static sk_sp<SkShader> Make(sk_sp<SkImage>,
                                SkTileMode tmx,
                                SkTileMode tmy,
                                const SkMatrix* localMatrix,
                                FilterEnum,
                                bool clampAsIfUnpremul = false);

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

private:
    SK_FLATTENABLE_HOOKS(SkImageShader)

    SkImageShader(sk_sp<SkImage>,
                  SkTileMode tmx,
                  SkTileMode tmy,
                  const SkMatrix* localMatrix,
                  FilterEnum,
                  bool clampAsIfUnpremul);

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
        return fFiltering == kInheritFromPaint ? paintQuality : (SkFilterQuality)fFiltering;
    }

    sk_sp<SkImage>   fImage;
    const SkTileMode fTileModeX;
    const SkTileMode fTileModeY;
    const FilterEnum fFiltering;
    const bool       fClampAsIfUnpremul;

    friend class SkShaderBase;
    typedef SkShaderBase INHERITED;
};

#endif
