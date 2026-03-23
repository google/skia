/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED
#define skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED

#include "include/private/base/SkTDArray.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <type_traits>

namespace skgpu::graphite {

/*
 * Naively, calculating MSAA coverage in software is O(n) per each sample location. Instead we use a
 * variation of a lookup technique introduced by Laine and Karras, adapted to 2D by Li et al., and
 * refined by the Vello project:
 * https://dl.acm.org/doi/abs/10.1111/j.1467-8659.2010.01728.x,
 * https://dl.acm.org/doi/10.1145/2980179.2982434)
 * https://github.com/linebender/vello/blob/main/vello_shaders/shader/fine.wgsl
 * https://github.com/linebender/vello/pull/64
 *
 * * To precompute the MSAA coverage of a line segment slicing through a 1x1 pixel, in Li et al. the
 *   line is parameterized by its normal vector (angle) and perpendicular distance from the center.
 *   While effective, this approach has drawbacks: angle and distance do not map linearly to the
 *   Cartesian pixel grid, leading to higher quantization errors (especially near corners), and
 *   computing the normal requires expensive square root operations (e.g., std::hypot) during
 *   rendering.
 * * Instead, we use a continuous parameterization introduced by the Vello project. We
 *   map the infinite range of standard slopes (m) and y-intercepts (b) into two new variables, s
 *   (normalized slope) and t (normalized translation), strictly bounded between 0.0 and 1.0.
 *
 * ----------------------------------
 * Squashing the Slope (m -> s)
 * ----------------------------------
 * * We squash the infinite positive slope m in [0, infinity) using the curve:
 *   s = 1 / (m + 1)
 * * This maps the extremes while avoiding division-by-zero risks:
 *   A perfectly vertical line (m = infinity) maps to s = 0.0
 *   A 45-degree line (m = 1) maps to s = 0.5
 *   A perfectly horizontal line (m = 0) maps to s = 1.0
 * * To use this in the half-plane equation (m*x - y + b >= 0), we solve for m:
 *   m = (1 - s) / s
 *   Substituting and multiplying by s gives:
 *   x(1 - s) - y*s + b*s >= 0
 *
 * ----------------------------------
 * Squashing the Translation (b -> t)
 * ----------------------------------
 * * To ensure our spatial offset stays between 0.0 and 1.0, we scale the y-intercept against our
 *   new slope parameter:
 *   t = (b + m) / (m + 1)
 * * We can rewrite t entirely in terms of s:
 *   t = (b + m) * s  =>  t = b*s + m*s
 * * Since m*s = 1 - s, we get:
 *   t = b*s + 1 - s
 * * Solving for b*s gives:
 *   b*s = s + t - 1
 *
 * ----------------------------------
 * Final Equation
 * ----------------------------------
 * * Substitute our new definition of b*s back into the half-plane equation:
 *   x(1 - s) - y*s + (s + t - 1) >= 0
 * * To group this cleanly, we add zero (+ t*s - t*s):
 *   x(1 - s) - y*s + s + t - 1 + t*s - t*s >= 0
 * * Rearranging to group by (1 - s) and s:
 *   x(1 - s) - (1 - s - t + t*s) - y*s + t*s >= 0
 * * Factoring the middle group into (1 - t)(1 - s), and factoring -s from the end:
 *   x(1 - s) - (1 - t)(1 - s) - (y - t)s >= 0
 * * Factoring out (1 - s) yields the final parameterization:
 *   (x - (1 - t))(1 - s) - (y - t)s >= 0
 *
 * ----------------------------------
 * LUT Generation
 * ----------------------------------
 * * Because s and t are strictly bounded [0.0, 1.0], we can quantize them into a discrete 2D grid.
 *   The grid has `kWidth` columns representing the translation t, and `kHeight / 2` rows
 *   representing the slope s.
 * * Since our mathematical derivation assumes a positive slope (m >= 0), we partition the LUT into
 *   two halves. The bottom half (v >= kHeight / 2) stores masks for positive slopes. The top half
 *   stores masks for negative slopes.
 * * For negative slopes, we reuse the exact same mathematical equation but geometrically flip the
 *   Y-axis of our sub-pixel sample points (y = 1.0 - y).
 * * To minimize maximum quantization error, we extract the continuous s and t values from the exact
 *   center of each grid cell (e.g., `(u + 0.5) / kWidth`).
 * * For each cell, we evaluate the half-plane equation against all N sub-pixel sample coordinates.
 *   If the result is >= 0.0, the sample is covered, and we set the corresponding bit in our integer
 *   mask using a bitwise OR (`mask |= (1 << k)`).
 *
 * * The actual positions of the subsample points use the D3D11 standard multisample pattern:
 *   https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_standard_multisample_quality_levels
 */

template <typename T> class MSAA_LUT {
public:
    static constexpr int32_t kWidth       = 64;
    static constexpr int32_t kHeight      = 64;
    static constexpr int32_t kHalfHeight  = kHeight / 2;
    static constexpr size_t  kSampleCount = sizeof(T) * 8;

    static constexpr std::array<uint8_t, kSampleCount> kPattern = []() {
        if constexpr (std::is_same_v<T, uint8_t>) {
            return std::array<uint8_t, 8>{0, 5, 3, 7, 1, 4, 6, 2};
        } else if constexpr (std::is_same_v<T, uint16_t>) {
            return std::array<uint8_t, 16>{1, 8, 4, 11, 15, 7, 3, 12, 0, 9, 5, 13, 2, 10, 6, 14};
        } else {
            SkUNREACHABLE;
        }
    }();

    static SkTDArray<T> Make() {
        constexpr float scale = 1.0f / static_cast<float>(kSampleCount);
        std::array<float, kSampleCount> subX;
        std::array<float, kSampleCount> subY;

        for (size_t k = 0; k < kSampleCount; ++k) {
            subX[k] = (static_cast<float>(kPattern[k]) + 0.5f) * scale;
            subY[k] = (static_cast<float>(k) + 0.5f) * scale;
        }

        SkTDArray<T> lut;
        lut.reserve(kWidth * kHeight);

        for (int32_t i = 0; i < kWidth * kHeight; ++i) {
            int32_t u = i % kWidth;
            int32_t v = i / kWidth;

            bool isPos = v >= kHalfHeight;

            // Extract continuous parameters from the center of the grid cells
            float t = (static_cast<float>(u) + 0.5f) / static_cast<float>(kWidth);
            float s = (static_cast<float>(v % kHalfHeight) + 0.5f) /
                       static_cast<float>(kHalfHeight);

            T mask = 0;
            for (size_t k = 0; k < kSampleCount; ++k) {
                float x = subX[k];
                float y = subY[k];

                if (!isPos) {
                    y = 1.0f - y;
                }

                float val = (x - (1.0f - t)) * (1.0f - s) - (y - t) * s;

                if (val >= 0.0f) {
                    mask |= (static_cast<T>(1) << k);
                }
            }
            lut.push_back(mask);
        }
        return lut;
    }
};

template <typename T>
SkTDArray<T> GenerateMSAALUT() {
    if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t>) {
        return MSAA_LUT<T>::Make();
    } else {
        SkUNREACHABLE;
    }
}

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_sparse_strips_MSAA_LUT_DEFINED
