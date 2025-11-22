/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRustSpanUtils_DEFINED
#define SkRustSpanUtils_DEFINED

#include "include/core/SkSpan.h"

// TODO(https://crbug.com/356698922): Use a real `#include` if possible.
namespace rust {
inline namespace cxxbridge1 {
template <typename T> class Slice;
}  // namespace cxxbridge1
}  // namespace rust

template <typename T> SkSpan<T> ToSkSpan(rust::Slice<T> slice) {
    // Avoiding operating on `buffer.data()` if the slice is empty helps to avoid
    // UB risk described at https://davidben.net/2024/01/15/empty-slices.html.
    if (slice.empty()) {
        return SkSpan<T>();
    }

    return SkSpan<T>(slice.data(), slice.size());
}

#endif  // SkRustSpanUtils_DEFINED
