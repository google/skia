/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkSweepGradientShader_DEFINED
#define SkSweepGradientShader_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkArenaAlloc;
class SkMatrix;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkSweepGradient final : public SkGradientBaseShader {
public:
    SkSweepGradient(const SkPoint& center, SkScalar t0, SkScalar t1, const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

    const SkPoint& center() const { return fCenter; }
    SkScalar tBias() const { return fTBias; }
    SkScalar tScale() const { return fTScale; }

protected:
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc,
                              SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;

private:
    friend void ::SkRegisterSweepGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkSweepGradient)

    const SkPoint fCenter;
    const SkScalar fTBias;
    const SkScalar fTScale;
};

#endif
