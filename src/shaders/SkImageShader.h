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

namespace skgpu {
class Swizzle;
}

namespace skgpu::graphite {
class KeyContext;
enum class ReadSwizzle;
}

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

    SkImageShader(sk_sp<SkImage>,
                  const SkRect& subset,
                  SkTileMode tmx, SkTileMode tmy,
                  const SkSamplingOptions&,
                  bool raw,
                  bool clampAsIfUnpremul);

    bool isOpaque() const override;

#if SK_SUPPORT_GPU
    std::unique_ptr<GrFragmentProcessor> asFragmentProcessor(const GrFPArgs&,
                                                             const MatrixRec&) const override;
#endif
#ifdef SK_GRAPHITE_ENABLED
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif
    static SkM44 CubicResamplerMatrix(float B, float C);

private:
    SK_FLATTENABLE_HOOKS(SkImageShader)

    void flatten(SkWriteBuffer&) const override;
#ifdef SK_ENABLE_LEGACY_SHADERCONTEXT
    Context* onMakeContext(const ContextRec&, SkArenaAlloc* storage) const override;
#endif
    SkImage* onIsAImage(SkMatrix*, SkTileMode*) const override;

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dst,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc*) const override;

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
