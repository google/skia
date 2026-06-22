/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_SparseStripsTypes_DEFINED
#define skgpu_graphite_sparse_strips_SparseStripsTypes_DEFINED

#include "include/core/SkPoint.h"

#include <cstdint>
#include <functional>
#include <string>

namespace skgpu::graphite {

enum class FlattenMode {
    kScalar,
    kSimd,
};

struct Line {
    SkPoint p0;
    SkPoint p1;
};

// Helper struct for the intersection bits. These will be moved into a centralized header soon.
struct IntersectionBits {
    // TODO (thomsmit): reorder these into more orthodox LRTB
    static constexpr uint32_t T = 0b00001;
    static constexpr uint32_t B = 0b00010;
    static constexpr uint32_t L = 0b00100;
    static constexpr uint32_t R = 0b01000;
    static constexpr uint32_t W = 0b10000;

    static constexpr uint32_t BOT_SHIFT      = 1;
    static constexpr uint32_t LEFT_SHIFT     = 2;
    static constexpr uint32_t RIGHT_SHIFT    = 3;
    static constexpr uint32_t WINDING_SHIFT  = 4;
    static constexpr uint32_t INT_MASK_SHIFT = 5;

    static constexpr uint32_t INTERSECTION_MASK = W | R | L | B | T;
    static constexpr uint32_t MAX_LINES_PER_PATH = 1 << (32 - INT_MASK_SHIFT);

    static std::string MaskToString(uint32_t mask) {
        if (mask == 0) return "empty";

        std::string s;
        if (mask & W) s += "W";
        if (mask & T) s += (s.empty() ? "T" : " | T");
        if (mask & B) s += (s.empty() ? "B" : " | B");
        if (mask & L) s += (s.empty() ? "L" : " | L");
        if (mask & R) s += (s.empty() ? "R" : " | R");
        return s;
    }
};

#if defined(GPU_TEST_UTILS)
using MsaaExactMaskObserver = std::function<void(uint8_t exactMask)>;
#endif

} // namespace skgpu::graphite

#endif // skgpu_graphite_sparse_strips_SparseStripsTypes_DEFINED
