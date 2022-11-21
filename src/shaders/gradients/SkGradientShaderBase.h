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
#include "include/private/SkTArray.h"
#include "include/private/SkTemplates.h"
#include "src/core/SkVM.h"
#include "src/shaders/SkShaderBase.h"

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
                   const SkScalar pos[],
                   int colorCount,
                   SkTileMode mode,
                   const Interpolation& interpolation);

        const SkColor4f*    fColors;
        sk_sp<SkColorSpace> fColorSpace;
        const SkScalar*     fPos;
        int                 fCount;
        SkTileMode          fTileMode;
        Interpolation       fInterpolation;

        void flatten(SkWriteBuffer&) const;
    };

    class DescriptorScope : public Descriptor {
    public:
        DescriptorScope() {}

        bool unflatten(SkReadBuffer&, SkMatrix* legacyLocalMatrix);

        // fColors and fPos always point into local memory, so they can be safely mutated
        //
        SkColor4f* mutableColors() { return const_cast<SkColor4f*>(fColors); }
        SkScalar* mutablePos() { return const_cast<SkScalar*>(fPos); }

    private:
        SkSTArray<16, SkColor4f, true> fColorStorage;
        SkSTArray<16, SkScalar , true> fPosStorage;
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
    class GradientShaderBase4fContext;

    SkGradientShaderBase(SkReadBuffer& );
    void flatten(SkWriteBuffer&) const override;

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor*) const override;

    bool onAppendStages(const SkStageRec&) const override;

    skvm::Color onProgram(skvm::Builder*, skvm::Coord device, skvm::Coord local, skvm::Color paint,
                          const SkMatrixProvider&, const SkMatrix* localM, const SkColorInfo& dstCS,
                          skvm::Uniforms* uniforms, SkArenaAlloc* alloc) const override;

    virtual void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                                      SkRasterPipeline* postPipeline) const = 0;

    // Produce t from (x,y), modifying mask if it should be anything other than ~0.
    virtual skvm::F32 transformT(skvm::Builder*, skvm::Uniforms*,
                                 skvm::Coord coord, skvm::I32* mask) const = 0;

    const SkMatrix fPtsToUnit;
    SkTileMode     fTileMode;

public:
    SkScalar getPos(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigPos ? fOrigPos[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor getLegacyColor(int i) const {
        SkASSERT(i < fColorCount);
        return fOrigColors4f[i].toSkColor();
    }

    SkColor4f*          fOrigColors4f; // original colors, as floats
    SkScalar*           fOrigPos;      // original positions
    int                 fColorCount;
    sk_sp<SkColorSpace> fColorSpace;   // color space of gradient stops
    Interpolation       fInterpolation;

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    SkTileMode getTileMode() const { return fTileMode; }

private:
    // Reserve inline space for up to 4 stops.
    inline static constexpr size_t kInlineStopCount   = 4;
    inline static constexpr size_t kInlineStorageSize = (sizeof(SkColor4f) + sizeof(SkScalar))
                                               * kInlineStopCount;
    SkAutoSTMalloc<kInlineStorageSize, uint8_t> fStorage;

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
