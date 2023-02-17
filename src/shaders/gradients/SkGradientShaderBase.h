/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "include/effects/SkGradientShader.h"

#include "include/core/SkMatrix.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkShaderBase.h"

#ifdef SK_GRAPHITE_ENABLED
#include "src/gpu/graphite/KeyHelpers.h"
#endif

class SkArenaAlloc;
class SkColorSpace;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkGradientShaderBase : public SkShaderBase {
public:
    using Interpolation = SkGradientShader::Interpolation;

    struct Descriptor {
        Descriptor();
        ~Descriptor();

        Descriptor(const SkColor4f colors[],
                   sk_sp<SkColorSpace> colorSpace,
                   const SkScalar positions[],
                   int colorCount,
                   SkTileMode mode,
                   const Interpolation& interpolation);

        const SkColor4f*    fColors;
        sk_sp<SkColorSpace> fColorSpace;
        const SkScalar*     fPositions;
        int                 fColorCount;  // length of fColors (and fPositions, if not nullptr)
        SkTileMode          fTileMode;
        Interpolation       fInterpolation;
    };

    class DescriptorScope : public Descriptor {
    public:
        DescriptorScope() {}

        bool unflatten(SkReadBuffer&, SkMatrix* legacyLocalMatrix);

    private:
        SkSTArray<16, SkColor4f, true> fColorStorage;
        SkSTArray<16, SkScalar , true> fPositionStorage;
    };

    SkGradientShaderBase(const Descriptor& desc, const SkMatrix& ptsToUnit);
    ~SkGradientShaderBase() override;

    bool isOpaque() const override;

    bool interpolateInPremul() const {
        return fInterpolation.fInPremul == SkGradientShader::Interpolation::InPremul::kYes;
    }

    const SkMatrix& getGradientMatrix() const { return fPtsToUnit; }

    static bool ValidGradient(const SkColor4f colors[], int count, SkTileMode tileMode,
                              const Interpolation& interpolation);

    static sk_sp<SkShader> MakeDegenerateGradient(const SkColor4f colors[], const SkScalar pos[],
                                                  int colorCount, sk_sp<SkColorSpace> colorSpace,
                                                  SkTileMode mode);

    struct ColorStopOptimizer {
        ColorStopOptimizer(const SkColor4f* colors, const SkScalar* pos, int count,
                           SkTileMode mode);

        const SkColor4f* fColors;
        const SkScalar*  fPos;
        int              fCount;
    };

    // The default SkScalarNearlyZero threshold of .0024 is too big and causes regressions for svg
    // gradients defined in the wild.
    static constexpr SkScalar kDegenerateThreshold = SK_Scalar1 / (1 << 15);

protected:
    void flatten(SkWriteBuffer&) const override;

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor*) const override;

    bool appendStages(const SkStageRec&, const MatrixRec&) const override;

    skvm::Color program(skvm::Builder*,
                        skvm::Coord device,
                        skvm::Coord local,
                        skvm::Color paint,
                        const MatrixRec&,
                        const SkColorInfo& dstCS,
                        skvm::Uniforms* uniforms,
                        SkArenaAlloc* alloc) const override;

    virtual void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                                      SkRasterPipeline* postPipeline) const = 0;

    // Produce t from (x,y), modifying mask if it should be anything other than ~0.
    virtual skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                                 skvm::Coord coord, skvm::I32* mask) const = 0;

    const SkMatrix fPtsToUnit;
    SkTileMode     fTileMode;

#ifdef SK_GRAPHITE_ENABLED
    static void MakeInterpolatedToDst(const skgpu::graphite::KeyContext&,
                                      skgpu::graphite::PaintParamsKeyBuilder*,
                                      skgpu::graphite::PipelineDataGatherer*,
                                      const skgpu::graphite::GradientShaderBlocks::GradientData&,
                                      const SkGradientShaderBase::Interpolation&,
                                      SkColorSpace* intermediateCS);
#endif

public:
    static void AppendGradientFillStages(SkRasterPipeline* p,
                                         SkArenaAlloc* alloc,
                                         const SkPMColor4f* colors,
                                         const SkScalar* positions,
                                         int count);

    SkScalar getPos(int i) const {
        SkASSERT(i < fColorCount);
        return fPositions ? fPositions[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor getLegacyColor(int i) const {
        SkASSERT(i < fColorCount);
        return fColors[i].toSkColor();
    }

    SkColor4f*          fColors;       // points into fStorage
    SkScalar*           fPositions;    // points into fStorage, or nullptr
    int                 fColorCount;   // length of fColors (and fPositions, if not nullptr)
    sk_sp<SkColorSpace> fColorSpace;   // color space of gradient stops
    Interpolation       fInterpolation;
    bool                fFirstStopIsImplicit;
    bool                fLastStopIsImplicit;

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    SkTileMode getTileMode() const { return fTileMode; }

private:
    // Reserve inline space for up to 4 stops.
    inline static constexpr size_t kInlineStopCount   = 4;
    inline static constexpr size_t kInlineStorageSize = (sizeof(SkColor4f) + sizeof(SkScalar))
                                               * kInlineStopCount;
    skia_private::AutoSTMalloc<kInlineStorageSize, uint8_t> fStorage;

    bool                                        fColorsAreOpaque;

    using INHERITED = SkShaderBase;
};

///////////////////////////////////////////////////////////////////////////////

struct SkColor4fXformer {
    SkColor4fXformer(const SkGradientShaderBase* shader, SkColorSpace* dst);

    SkSTArray<4, SkPMColor4f, true> fColors;
    sk_sp<SkColorSpace>             fIntermediateColorSpace;
};

struct SkColorConverter {
    SkColorConverter(const SkColor* colors, int count);

    SkSTArray<2, SkColor4f, true> fColors4f;
};

void SkRegisterLinearGradientShaderFlattenable();
void SkRegisterRadialGradientShaderFlattenable();
void SkRegisterSweepGradientShaderFlattenable();
void SkRegisterTwoPointConicalGradientShaderFlattenable();

#endif
