/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkGradientShaderPriv_DEFINED
#define SkGradientShaderPriv_DEFINED

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkGradient.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTArray.h"
#include "include/private/base/SkTemplates.h"
#include "src/core/SkColorData.h"
#include "src/shaders/SkShaderBase.h"

#include <cstddef>
#include <cstdint>

class SkArenaAlloc;
class SkRasterPipeline;
class SkReadBuffer;
class SkShader;
class SkWriteBuffer;
enum class SkTileMode;
struct SkStageRec;

class SkGradientScope {
public:
    std::optional<SkGradient> unflatten(SkReadBuffer&, SkMatrix* legacyLocalMatrix);
private:
    skia_private::STArray<16, SkColor4f> fColorStorage;
    skia_private::STArray<16, SkScalar> fPositionStorage;
};

class SkGradientBaseShader : public SkShaderBase {
public:
    using Interpolation = SkGradient::Interpolation;

    SkGradientBaseShader(const SkGradient&, const SkMatrix& ptsToUnit);
    ~SkGradientBaseShader() override;

    ShaderType type() const final { return ShaderType::kGradientBase; }

    bool isOpaque() const override;

    bool interpolateInPremul() const {
        return fInterpolation.fInPremul == Interpolation::InPremul::kYes;
    }

    const SkMatrix& getGradientMatrix() const { return fPtsToUnit; }
    size_t getColorCount() const { return fColorCount; }
    const float* getPositions() const { return fPositions; }

    const Interpolation& getInterpolation() const { return fInterpolation; }

    static bool ValidGradient(SkSpan<const SkColor4f>,
                              SkTileMode tileMode,
                              const Interpolation& interpolation);

    static sk_sp<SkShader> MakeDegenerateGradient(const SkGradient::Colors&);

    // The default SkScalarNearlyZero threshold of .0024 is too big and causes regressions for svg
    // gradients defined in the wild.
    static constexpr SkScalar kDegenerateThreshold = SK_Scalar1 / (1 << 15);

protected:
    void flatten(SkWriteBuffer&) const override;

    void commonAsAGradient(GradientInfo*) const;

    bool onAsLuminanceColor(SkColor4f*) const override;

    bool appendStages(const SkStageRec&, const SkShaders::MatrixRec&) const override;

    virtual void appendGradientStages(SkArenaAlloc* alloc,
                                      SkRasterPipeline* tPipeline,
                                      SkRasterPipeline* postPipeline) const = 0;

    const SkMatrix fPtsToUnit;
    SkTileMode fTileMode;

public:
    static void AppendGradientFillStages(SkRasterPipeline* p,
                                         SkArenaAlloc* alloc,
                                         const SkPMColor4f* colors,
                                         const SkScalar* positions,
                                         int count);

    static void AppendInterpolatedToDstStages(SkRasterPipeline* p,
                                              SkArenaAlloc* alloc,
                                              bool colorsAreOpaque,
                                              const Interpolation& interpolation,
                                              const SkColorSpace* intermediateColorSpace,
                                              const SkColorSpace* dstColorSpace);

    float getPos(size_t i) const {
        SkASSERT(i < fColorCount);
        return fPositions ? fPositions[i] : SkIntToScalar(i) / (fColorCount - 1);
    }

    SkColor getLegacyColor(size_t i) const {
        SkASSERT(i < fColorCount);
        return fColors[i].toSkColor();
    }

    SkSpan<const SkColor4f> colors() const { return {fColors, fColorCount}; }
    SkSpan<const float> positions() const {
        return {fPositions, fPositions ? fColorCount : 0};
    }

    bool firstStopIsImplicit() const { return fFirstStopIsImplicit; }
    bool lastStopIsImplicit() const { return fLastStopIsImplicit; }

    const sk_sp<SkColorSpace>& colorSpace() const { return fColorSpace; }
    const Interpolation& interpolation() const { return fInterpolation; }

    bool colorsAreOpaque() const { return fColorsAreOpaque; }

    SkTileMode getTileMode() const { return fTileMode; }

    const SkBitmap& cachedBitmap() const { return fColorsAndOffsetsBitmap; }
    void setCachedBitmap(SkBitmap b) const { fColorsAndOffsetsBitmap = b; }

private:
    SkColor4f* fColors;               // points into fStorage
    SkScalar* fPositions;             // points into fStorage, or nullptr
    sk_sp<SkColorSpace> fColorSpace;  // color space of gradient stops
    Interpolation fInterpolation;
    size_t fColorCount;               // length of fColors (and fPositions, if not nullptr)
    bool fFirstStopIsImplicit;
    bool fLastStopIsImplicit;

    // When the number of stops exceeds Graphite's uniform-based limit the colors and offsets
    // are stored in this bitmap. It is stored in the shader so it can be cached with a stable
    // id and easily regenerated if purged.
    // TODO(b/293160919) remove this field when we can store bitmaps in the cache by id.
    mutable SkBitmap fColorsAndOffsetsBitmap;

    // Reserve inline space for up to 4 stops.
    inline static constexpr size_t kInlineStopCount = 4;
    inline static constexpr size_t kInlineStorageSize =
            (sizeof(SkColor4f) + sizeof(SkScalar)) * kInlineStopCount;
    skia_private::AutoSTMalloc<kInlineStorageSize, uint8_t> fStorage;

    bool fColorsAreOpaque;
};

///////////////////////////////////////////////////////////////////////////////

struct SkColor4fXformer {
    SkColor4fXformer(const SkGradientBaseShader* shader,
                     SkColorSpace* dst,
                     bool forceExplicitPositions = false);

    using ColorStorage = skia_private::STArray<4, SkPMColor4f>;
    using PositionStorage = skia_private::STArray<4, float>;

    ColorStorage fColors;
    PositionStorage fPositionStorage;
    const float* fPositions;
    sk_sp<SkColorSpace> fIntermediateColorSpace;
};

void SkRegisterConicalGradientShaderFlattenable();
void SkRegisterLinearGradientShaderFlattenable();
void SkRegisterRadialGradientShaderFlattenable();
void SkRegisterSweepGradientShaderFlattenable();

#define MAKE_COLORS_POS_SPANS(colorPtr, posPtr, count)      \
    SkSpan<const SkColor4f> colors{colorPtr, (size_t)count};\
    SkSpan<const float> pos{};                              \
    do {                                                    \
        if (posPtr) {                                       \
            pos = {posPtr, (size_t)count};                  \
        }                                                   \
    } while (0)


#define GRADIENT_FACTORY_EARLY_EXIT(grad, lm) \
    do {                                                                                        \
        const auto& colors = grad.colors();                                                     \
        const auto& interp = grad.interpolation();                                              \
        if (!SkGradientBaseShader::ValidGradient(colors.colors(), colors.tileMode(), interp)) { \
            return nullptr;                                                                     \
        }                                                                                       \
        if (colors.colors().size() == 1) {                                                      \
            return SkShaders::Color(colors.colors().front(), colors.colorSpace());              \
        }                                                                                       \
        if (lm && !lm->invert(nullptr)) {                                                       \
            return nullptr;                                                                     \
        }                                                                                       \
    } while (0)

#endif
