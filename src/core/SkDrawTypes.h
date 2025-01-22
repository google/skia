/*
 * Copyright 2025 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawTypes_DEFINED
#define SkDrawTypes_DEFINED

#include <cstddef>

enum class SkDrawCoverage : bool {
    kNo = false,
    kYes = true,
};

// A good size for creating shader contexts on the stack.
constexpr size_t kSkBlitterContextSize = 3332;

#endif
