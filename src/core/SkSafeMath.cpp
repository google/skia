/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafeMath.h"

static uint32_t saturate_mul32(uint32_t x, uint32_t y) {
    uint64_t bx = x;
    uint64_t by = y;
    uint64_t result = bx * by;
    if (result <= std::numeric_limits<uint32_t>::max()) {
        return static_cast<uint32_t>(result);
    }
    return std::numeric_limits<uint32_t>::max();
}

static uint64_t saturate_mul64(uint64_t x, uint64_t y) {
    if (x <= std::numeric_limits<uint64_t>::max() >> 32
        && y <= std::numeric_limits<uint64_t>::max() >> 32) {
        return x * y;
    } else {
        auto hi = [](uint64_t v) { return v >> 32; };
        auto lo = [](uint64_t v) { return v & 0xFFFFFFFF; };

        uint64_t lx_ly = lo(x) * lo(y);
        uint64_t hx_ly = hi(x) * lo(y);
        uint64_t lx_hy = lo(x) * hi(y);
        uint64_t hx_hy = hi(x) * hi(y);

        uint64_t result = std::numeric_limits<uint64_t>::max();

        // Calculate low 64 bits.
        uint64_t sum = lx_ly;
        uint64_t nextSum = sum + (hx_ly << 32);
        if (nextSum >= sum) {
            sum = nextSum;
            nextSum += (lx_hy << 32);
            if (nextSum >= sum) {
                result = nextSum;
            }
        }

        // Calculate upper 64 bits, and check they are zero.
        if ((hx_hy + (hx_ly >> 32) + (lx_hy >> 32)) != 0) {
            result = std::numeric_limits<uint64_t>::max();
        }

        #if defined(SK_DEBUG) && defined(__clang__) && defined(__x86_64__)
        auto double_check = (unsigned __int128)x * y;
                SkASSERT((double_check >> 64 != 0)
                         || result == (double_check & 0xFFFFFFFFFFFFFFFF));
                SkASSERT((double_check >> 64 == 0)
                         || result == std::numeric_limits<uint64_t>::max());
        #endif

        return result;
    }
}

size_t saturate_mul(size_t x, size_t y) {
    switch (sizeof(size_t)) {
        case sizeof(uint64_t):
            return saturate_mul64(x, y);
        case sizeof(uint32_t):
            return saturate_mul32(x, y);
        default:
            SK_ABORT("Bizarre size_t.");
    }
}

SkSafeSize SkSafeSize::operator*(const SkSafeSize& r) const {
    return SkSafeSize{saturate_mul(size_, r.size_)};
}
