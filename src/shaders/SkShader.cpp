/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkShader.h"

#include "include/core/SkColorFilter.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRefCnt.h"
#include "src/shaders/SkColorFilterShader.h"
#include "src/shaders/SkLocalMatrixShader.h"
#include "src/shaders/SkShaderBase.h"
#include "src/shaders/SkWorkingColorSpaceShader.h"

#include <utility>

class SkImage;
enum class SkTileMode;

SkImage* SkShader::isAImage(SkMatrix* localMatrix, SkTileMode xy[2]) const {
    return as_SB(this)->onIsAImage(localMatrix, xy);
}

sk_sp<SkShader> SkShader::makeWithLocalMatrix(const SkMatrix& localMatrix) const {
    const SkMatrix* lm = &localMatrix;

    sk_sp<SkShader> baseShader;
    SkMatrix otherLocalMatrix;
    sk_sp<SkShader> proxy = as_SB(this)->makeAsALocalMatrixShader(&otherLocalMatrix);
    if (proxy) {
        otherLocalMatrix = SkShaderBase::ConcatLocalMatrices(localMatrix, otherLocalMatrix);
        lm = &otherLocalMatrix;
        baseShader = proxy;
    } else {
        baseShader = sk_ref_sp(const_cast<SkShader*>(this));
    }

    return sk_make_sp<SkLocalMatrixShader>(std::move(baseShader), *lm);
}

sk_sp<SkShader> SkShader::makeWithColorFilter(sk_sp<SkColorFilter> filter) const {
    return SkColorFilterShader::Make(sk_ref_sp(this), 1.0f, std::move(filter));
}

sk_sp<SkShader> SkShader::makeWithWorkingColorSpace(sk_sp<SkColorSpace> workingSpace) const {
    return SkWorkingColorSpaceShader::Make(sk_ref_sp(this), std::move(workingSpace));
}
