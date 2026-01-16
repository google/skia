/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "src/shaders/gradients/SkRadialGradient.h"

#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradient.h"
#include "include/private/base/SkTArray.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkWriteBuffer.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/gradients/SkGradientBaseShader.h"

#include <cstdint>
#include <utility>

class SkArenaAlloc;
enum class SkTileMode;

static SkMatrix rad_to_unit_matrix(const SkPoint& center, SkScalar radius) {
    SkScalar inv = SkScalarInvert(radius);

    SkMatrix matrix;
    matrix.setTranslate(-center.fX, -center.fY);
    matrix.postScale(inv, inv);
    return matrix;
}

SkRadialGradient::SkRadialGradient(const SkPoint& center, SkScalar radius, const SkGradient& desc)
        : SkGradientBaseShader(desc, rad_to_unit_matrix(center, radius))
        , fCenter(center)
        , fRadius(radius) {}

SkShaderBase::GradientType SkRadialGradient::asGradient(GradientInfo* info,
                                                        SkMatrix* localMatrix) const {
    if (info) {
        commonAsAGradient(info);
        info->fPoint[0] = fCenter;
        info->fRadius[0] = fRadius;
    }
    if (localMatrix) {
        *localMatrix = SkMatrix::I();
    }
    return GradientType::kRadial;
}

sk_sp<SkFlattenable> SkRadialGradient::CreateProc(SkReadBuffer& buffer) {
    SkGradientScope scope;
    SkMatrix legacyLocalMatrix, *lmPtr = nullptr;
    auto grad = scope.unflatten(buffer, &legacyLocalMatrix);
    if (!grad) {
        return nullptr;
    }
    if (!legacyLocalMatrix.isIdentity()) {
        lmPtr = &legacyLocalMatrix;
    }
    const SkPoint center = buffer.readPoint();
    const SkScalar radius = buffer.readScalar();
    return SkShaders::RadialGradient(center, radius, *grad, lmPtr);
}

void SkRadialGradient::flatten(SkWriteBuffer& buffer) const {
    this->SkGradientBaseShader::flatten(buffer);
    buffer.writePoint(fCenter);
    buffer.writeScalar(fRadius);
}

void SkRadialGradient::appendGradientStages(SkArenaAlloc*, SkRasterPipeline* p,
                                            SkRasterPipeline*) const {
    p->append(SkRasterPipelineOp::xy_to_radius);
}

sk_sp<SkShader> SkShaders::RadialGradient(SkPoint center, float radius,
                                          const SkGradient& grad, const SkMatrix* lm) {
    if (radius < 0) {
        return nullptr;
    }

    GRADIENT_FACTORY_EARLY_EXIT(grad, lm);

    if (SkScalarNearlyZero(radius, SkGradientBaseShader::kDegenerateThreshold)) {
        // Degenerate gradient optimization, and no special logic needed for clamped radial gradient
        return SkGradientBaseShader::MakeDegenerateGradient(grad.colors());
    }

    sk_sp<SkShader> s = sk_make_sp<SkRadialGradient>(center, radius, grad);
    return s->makeWithLocalMatrix(lm ? *lm : SkMatrix::I());
}

void SkRegisterRadialGradientShaderFlattenable() {
    SK_REGISTER_FLATTENABLE(SkRadialGradient);
}
