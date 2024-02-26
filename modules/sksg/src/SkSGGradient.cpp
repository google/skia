/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/sksg/include/SkSGGradient.h"

#include "include/core/SkColorSpace.h"
#include "include/core/SkShader.h"
#include "include/effects/SkGradientShader.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"

namespace sksg {

sk_sp<SkShader> Gradient::onRevalidateShader() {
    if (fColorStops.empty()) {
        return nullptr;
    }

    std::vector<SkColor4f> colors;
    std::vector<SkScalar>  positions;
    colors.reserve(fColorStops.size());
    positions.reserve(fColorStops.size());

    SkScalar position = 0;
    for (const auto& stop : fColorStops) {
        colors.push_back(stop.fColor);
        position = SkTPin(stop.fPosition, position, 1.0f);
        positions.push_back(position);
    }

    // TODO: detect even stop distributions, pass null for positions.
    return this->onMakeShader(colors, positions);
}

sk_sp<SkShader> LinearGradient::onMakeShader(const std::vector<SkColor4f>& colors,
                                             const std::vector<SkScalar >& positions) const {
    SkASSERT(colors.size() == positions.size());

    const SkPoint pts[] = { fStartPoint, fEndPoint };
    return SkGradientShader::MakeLinear(pts, colors.data(), nullptr, positions.data(),
                                        SkToInt(colors.size()), this->getTileMode());
}

sk_sp<SkShader> RadialGradient::onMakeShader(const std::vector<SkColor4f>& colors,
                                             const std::vector<SkScalar >& positions) const {
    SkASSERT(colors.size() == positions.size());

    return (fStartRadius <= 0 && fStartCenter == fEndCenter)
        ? SkGradientShader::MakeRadial(fEndCenter, fEndRadius,
                                       colors.data(), nullptr, positions.data(),
                                       SkToInt(colors.size()), this->getTileMode())
        : SkGradientShader::MakeTwoPointConical(fStartCenter, fStartRadius,
                                                fEndCenter, fEndRadius,
                                                colors.data(), nullptr, positions.data(),
                                                SkToInt(colors.size()), this->getTileMode());
}

} //namespace sksg
