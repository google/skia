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
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/private/SkNoncopyable.h"

#include <vector>

namespace skjson { class Value; }

namespace skottie {
namespace internal {
class AnimationBuilder;
} // namespace internal

template <typename T>
struct ValueTraits {
    static bool FromJSON(const skjson::Value&, const internal::AnimationBuilder*, T*);

    template <typename U>
    static U As(const T&);

    static bool Lerp(const T&, const T&, float, T*);
};

using ScalarValue = SkScalar;
using   Vec2Value = SkV2;
using VectorValue = std::vector<float>;

struct ShapeValue {
    std::vector<float> fData;

    operator SkPath() const;
};

} // namespace skottie

#endif // SkottieValue_DEFINED
