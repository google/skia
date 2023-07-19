/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieValue_DEFINED
#define SkottieValue_DEFINED

#include "include/core/SkColor.h"
#include "include/core/SkM44.h"
#include "include/core/SkPath.h"

#include <initializer_list>
#include <vector>

namespace skjson { class Value; }

namespace skottie {

using ScalarValue = SkScalar;
using   Vec2Value = SkV2;

class VectorValue : public std::vector<float> {
public:
    VectorValue() = default;

    VectorValue(std::initializer_list<float> l) : INHERITED(l) {}

    operator SkV3()      const;
private:
    using INHERITED = std::vector<float>;
};

class ColorValue final : public VectorValue {
public:
    ColorValue() = default;

    ColorValue(std::initializer_list<float> l) : INHERITED(l) {}

    operator SkColor()   const;
    operator SkColor4f() const;

private:
    using INHERITED = VectorValue;
};

class ShapeValue final : public std::vector<float> {
public:
    operator SkPath() const;
};

} // namespace skottie

#endif // SkottieValue_DEFINED
