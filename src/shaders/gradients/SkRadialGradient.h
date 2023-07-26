/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkRadialGradient_DEFINED
#define SkRadialGradient_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkPoint.h"
#include "include/core/SkScalar.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkArenaAlloc;
class SkMatrix;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkRadialGradient final : public SkGradientBaseShader {
public:
    SkRadialGradient(const SkPoint& center, SkScalar radius, const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* matrix) const override;

    const SkPoint& center() const { return fCenter; }
    SkScalar radius() const { return fRadius; }
protected:
    SkRadialGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc,
                              SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const override;
private:
    friend void ::SkRegisterRadialGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkRadialGradient)

    const SkPoint fCenter;
    const SkScalar fRadius;
};

#endif
