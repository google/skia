/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearGradient_DEFINED
#define SkLinearGradient_DEFINED

#include "include/core/SkFlattenable.h"
#include "include/core/SkPoint.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

class SkArenaAlloc;
class SkMatrix;
class SkRasterPipeline;
class SkReadBuffer;
class SkWriteBuffer;

class SkLinearGradient final : public SkGradientBaseShader {
public:
    SkLinearGradient(const SkPoint pts[2], const Descriptor&);

    GradientType asGradient(GradientInfo* info, SkMatrix* localMatrix) const override;

    const SkPoint& start() const { return fStart; }
    const SkPoint& end() const { return fEnd; }
protected:
    SkLinearGradient(SkReadBuffer& buffer);
    void flatten(SkWriteBuffer& buffer) const override;

    void appendGradientStages(SkArenaAlloc* alloc, SkRasterPipeline* tPipeline,
                              SkRasterPipeline* postPipeline) const final;

private:
    friend void ::SkRegisterLinearGradientShaderFlattenable();
    SK_FLATTENABLE_HOOKS(SkLinearGradient)

    class LinearGradient4fContext;

    friend class SkGradientShader;
    using INHERITED = SkGradientBaseShader;
    const SkPoint fStart;
    const SkPoint fEnd;
};

#endif
