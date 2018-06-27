/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieValue_DEFINED
#define SkottieValue_DEFINED

#include "SkPath.h"
#include "SkScalar.h"

#include <vector>

namespace  skottie {

template <typename T>
struct ValueTraits {
    static size_t Cardinality(const T&);

    template <typename U>
    static U As(const T&);
};

using ScalarValue = SkScalar;
using VectorValue = std::vector<ScalarValue>;
using ShapeValue  = SkPath;

} // namespace skottie

#endif // SkottieValue_DEFINED
