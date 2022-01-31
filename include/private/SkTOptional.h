/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkTOptional_DEFINED
#define SkTOptional_DEFINED

#include <optional>

namespace skstd {

template <typename T> using optional = std::optional<T>;
using nullopt_t = std::nullopt_t;
inline constexpr nullopt_t nullopt = std::nullopt;

} // namespace skstd

#endif
