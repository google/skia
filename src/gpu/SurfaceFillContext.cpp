/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/SurfaceFillContext.h"

#include "src/gpu/effects/GrMatrixEffect.h"

namespace skgpu {

void SurfaceFillContext::fillRectWithFP(const SkIRect& dstRect,
                                        const SkMatrix& localMatrix,
                                        std::unique_ptr<GrFragmentProcessor> fp) {
    fp = GrMatrixEffect::Make(localMatrix, std::move(fp));
    this->fillRectWithFP(dstRect, std::move(fp));
}

} // namespace skgpu
