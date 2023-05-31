/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkMatrixColorFilter_DEFINED
#define SkMatrixColorFilter_DEFINED

#include "include/core/SkFlattenable.h"
#include "src/effects/colorfilters/SkColorFilterBase.h"

#include <cstdint>

class SkReadBuffer;
class SkWriteBuffer;
struct SkStageRec;

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/KeyHelpers.h"
#include "src/gpu/graphite/PaintParamsKey.h"
#endif  // SK_GRAPHITE

class SkMatrixColorFilter final : public SkColorFilterBase {
public:
    enum class Domain : uint8_t { kRGBA, kHSLA };

    explicit SkMatrixColorFilter(const float array[20], Domain);

    bool appendStages(const SkStageRec& rec, bool shaderIsOpaque) const override;

    bool onIsAlphaUnchanged() const override { return fAlphaIsUnchanged; }

    SkColorFilterBase::Type type() const override { return SkColorFilterBase::Type::kMatrix; }

#if defined(SK_GRAPHITE)
    void addToKey(const skgpu::graphite::KeyContext&,
                  skgpu::graphite::PaintParamsKeyBuilder*,
                  skgpu::graphite::PipelineDataGatherer*) const override;
#endif

    Domain domain() const { return fDomain; }
    const float* matrix() const { return fMatrix; }

private:
    friend void ::SkRegisterMatrixColorFilterFlattenable();
    SK_FLATTENABLE_HOOKS(SkMatrixColorFilter)

    void flatten(SkWriteBuffer&) const override;
    bool onAsAColorMatrix(float matrix[20]) const override;

#if defined(SK_ENABLE_SKVM)
    skvm::Color onProgram(skvm::Builder*,
                          skvm::Color,
                          const SkColorInfo& dst,
                          skvm::Uniforms* uniforms,
                          SkArenaAlloc*) const override;
#endif

    float fMatrix[20];
    bool fAlphaIsUnchanged;
    Domain fDomain;
};

#endif
