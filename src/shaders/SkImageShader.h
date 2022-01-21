/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkImageShader_DEFINED
#define SkImageShader_DEFINED

#include "include/core/SkImage.h"
#include "include/core/SkM44.h"
#include "src/shaders/SkBitmapProcShader.h"
#include "src/shaders/SkShaderBase.h"

class SkImageShader : public SkShaderBase {
public:
    static sk_sp<SkShader> Make(sk_sp<SkImage>,
                                SkTileMode tmx,
                                SkTileMode tmy,
                                const SkSamplingOptions&,
                                const SkMatrix* localMatrix,
                                bool clampAsIfUnpremul = false);

    static sk_sp<SkShader> MakeRaw(sk_sp<SkImage>,
                                   SkTileMode tmx,
                                   SkTileMode tmy,
                                   const SkSamplingOptions&,
                                   const SkMatrix* localMatrix);

    // TODO(skbug.com/12784): Requires SkImage to be texture backed, and created SkShader can only
    // be used on GPU-backed surfaces.
    static sk_sp<SkShader> MakeSubset(sk_sp<SkImage>,
                                      const SkRect& subset,
                                      SkTileMode tmx,
                                      SkTileMode tmy,
                                      const SkSamplingOptions&,
                                      const SkMatrix* localMatrix,
                                      bool clampAsIfUnpremul = false);

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&) const override;
#endif

    static SkM44 CubicResamplerMatrix(float B, float C);

private:
    SK_FLATTENABLE_HOOKS(SkImageShader)

    SkImageShader(sk_sp<SkImage>,
                  const SkRect& subset,
                  SkTileMode tmx,
                  SkTileMode tmy,
                  const SkSamplingOptions&,
                  const SkMatrix* localMatrix,
                  bool raw,
                  bool clampAsIfUnpremul);

    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#endif
    SkImage* onIsAImage(SkMatrix*, SkTileMode*) const override;

    bool onAppendStages(const SkStageRec&) const override;
    SkStageUpdater* onAppendUpdatableStages(const SkStageRec&) const override;

    SkUpdatableShader* onUpdatableShader(SkArenaAlloc* alloc) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider&, const SkMatrix* localM, const SkColorInfo& dst,
                          skvm::Uniforms* uniforms, SkArenaAlloc*) const override;

    class TransformShader;
    skvm::Color makeProgram(
            skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
            const SkMatrixProvider&, const SkMatrix* localM, const SkColorInfo& dst,
            skvm::Uniforms* uniforms, const TransformShader* coordShader, SkArenaAlloc*) const;

    bool doStages(const SkStageRec&, TransformShader* = nullptr) const;

    sk_sp<SkImage>          fImage;
    const SkSamplingOptions fSampling;
    const SkTileMode        fTileModeX;
    const SkTileMode        fTileModeY;

    // TODO(skbug.com/12784): This is only supported for GPU images currently.
    // If subset == (0,0,w,h) of the image, then no subset is applied. Subset will not be empty.
    const SkRect            fSubset;

    const bool              fRaw;
    const bool              fClampAsIfUnpremul;

    friend class SkShaderBase;
    using INHERITED = SkShaderBase;
};

#endif
