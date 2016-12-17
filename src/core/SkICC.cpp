/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkColorSpace_Base.h"
#include "SkICC.h"

SkICC::SkICC(sk_sp<SkColorSpace> colorSpace)
    : fColorSpace(std::move(colorSpace))
{}

sk_sp<SkICC> SkICC::Make(const void* ptr, size_t len) {
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(ptr, len);
    if (!colorSpace) {
        return nullptr;
    }

    return sk_sp<SkICC>(new SkICC(std::move(colorSpace)));
}

bool SkICC::toXYZD50(SkMatrix44* toXYZD50) const {
    const SkMatrix44* m = as_CSB(fColorSpace)->toXYZD50();
    if (!m) {
        return false;
    }

    *toXYZD50 = *m;
    return true;
}

bool SkICC::isNumericalTransferFn(SkColorSpaceTransferFn* coeffs) const {
    return as_CSB(fColorSpace)->onIsNumericalTransferFn(coeffs);
}
