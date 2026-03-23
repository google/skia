/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkPoint.h"
#include "src/base/SkRandom.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "tests/Test.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace skgpu::graphite {

namespace {

uint8_t compute_naive_mask(SkPoint p0, SkPoint p1) {
    float dx = p1.fX - p0.fX;
    float dy = p1.fY - p0.fY;
    if (std::abs(dx) < 1e-9f && std::abs(dy) < 1e-9f) return 0;
    float nx = dy;
    float ny = -dx;
    float len = std::hypot(nx, ny);
    nx /= len;
    ny /= len;

    if (nx < 0.0f) {
        nx = -nx;
        ny = -ny;
    }
    float c = nx * p0.fX + ny * p0.fY;

    uint8_t mask = 0;
    for (int i = 0; i < 8; ++i) {
        float sx = (MSAA_LUT<uint8_t>::kPattern[i] + 0.5f) / 8.0f;
        float sy = (i + 0.5f) / 8.0f;
        if (nx * sx + ny * sy - c > 0.0f) {
            mask |= (1 << i);
        }
    }
    return mask;
}

uint8_t lookup_LUT(SkPoint p0, SkPoint p1, const SkTDArray<uint8_t>& lut) {
    float dx = p1.fX - p0.fX;
    float dy = p1.fY - p0.fY;

    if (std::abs(dx) < 1e-9f && std::abs(dy) < 1e-9f) {
        return 0;
    }

    float nx = dy;
    float ny = -dx;

    // Match the Naive compute's normal orientation
    if (nx < 0.0f) {
        nx = -nx;
        ny = -ny;
    }
    float c = nx * p0.fX + ny * p0.fY;

    bool isPos = ny <= 0.0f;
    float D = nx + std::abs(ny);

    // Squash the slope
    float s = std::abs(ny) / D;
    float t = ((isPos ? nx : D) - c) / D;

    static constexpr int kWidth = MSAA_LUT<uint8_t>::kWidth;
    static constexpr int kHeight = MSAA_LUT<uint8_t>::kHeight;
    static constexpr int halfHeight = kHeight / 2;

    int u = std::clamp(static_cast<int>(std::floor(t * kWidth)), 0, kWidth - 1);
    int v_mod = std::clamp(static_cast<int>(std::floor(s * halfHeight)), 0, halfHeight - 1);
    int v = isPos ? (v_mod + halfHeight) : v_mod;

    return lut[v * kWidth + u];
}

// Note: Although it is possible for a line segment to end within a pixel, we do not test that case
// here. The LUT's quantization relies solely on a parameterized slope and intercept, effectively
// treating the segment as an infinite line that crosses the entire pixel. Downstream subsample
// masking logic independently handles the specific case of endpoints falling inside a pixel.
static SkPoint pick_random_square_point(SkRandom* rand) {
    int edge = rand->nextU() % 4;
    float t = rand->nextF();
    switch (edge) {
        case 0:
            return {t, 0.0f};
        case 1:
            return {1.0f, t};
        case 2:
            return {t, 1.0f};
        default:
            return {0.0f, t};
    }
}

template <typename LookupFunc>
void test_LUT(skiatest::Reporter* reporter,
             const char* lutName,
             const SkTDArray<uint8_t>& lut,
             uint32_t seed,
             LookupFunc lookupFunc) {
    SkRandom rand(seed);

    constexpr int kIterations = 10000;

    // Track and report the number of 1,2, and 3 sample errors. These are not reported as errors as
    // some noise is expected due to quantization to the LUT.
    constexpr uint32_t kErrorLimit = 3;
    std::array<uint32_t, kErrorLimit> minorErrorCount = {0, 0, 0};

    for (int i = 0; i < kIterations; ++i) {
        SkPoint p0 = pick_random_square_point(&rand);
        SkPoint p1 = pick_random_square_point(&rand);

        uint8_t expected = compute_naive_mask(p0, p1);
        uint8_t actual = lookupFunc(p0, p1, lut);

        uint8_t diff = expected ^ actual;
        uint32_t bitErrors = std::popcount(diff);

        if (bitErrors > kErrorLimit) {
            uint32_t hexP0X, hexP0Y, hexP1X, hexP1Y;
            std::memcpy(&hexP0X, &p0.fX, 4);
            std::memcpy(&hexP0Y, &p0.fY, 4);
            std::memcpy(&hexP1X, &p1.fX, 4);
            std::memcpy(&hexP1Y, &p1.fY, 4);
            ERRORF(reporter,
                    "[%s] CRITICAL Fail at iter %d (Seed: %u). Expected 0x%02x, "
                    "Got 0x%02x (%u bits diff)\n"
                    "P0: %f, %f (0x%08x, 0x%08x)\n"
                    "P1: %f, %f (0x%08x, 0x%08x)",
                    lutName, i, seed, expected, actual, bitErrors,
                    p0.fX, p0.fY, hexP0X, hexP0Y, p1.fX, p1.fY, hexP1X, hexP1Y);
        } else if (bitErrors > 0) {
            minorErrorCount[bitErrors - 1]++;
        }
    }

    INFOF(reporter,
          "[%s] Minor Error Summary (Seed: %u): 1-bit: %u, 2-bit: %u, 3-bit: %u",
          lutName,
          seed,
          minorErrorCount[0],
          minorErrorCount[1],
          minorErrorCount[2]);
}

}  // namespace

DEF_TEST(SparseStrips_LUTTest, reporter) {
    auto now = std::chrono::high_resolution_clock::now();
    uint32_t seed = static_cast<uint32_t>(now.time_since_epoch().count());

    const SkTDArray<uint8_t> msaaLUT = GenerateMSAALUT<uint8_t>();
    test_LUT(reporter, "SlopeBasedLUT", msaaLUT, seed, lookup_LUT);
}

}  // namespace skgpu::graphite
