/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef BenchmarkDataset_DEFINED
#define BenchmarkDataset_DEFINED

#include "include/core/SkPath.h"

#include <cstdint>
#include <vector>

namespace skgpu::graphite {

enum class BenchmarkDataset {
    kTiger,
    kMixed,
};

template <BenchmarkDataset kDataset>
struct BenchmarkDatasetInfo;

template <>
struct BenchmarkDatasetInfo<BenchmarkDataset::kTiger> {
    static constexpr const char* kName = "Tiger";
    static constexpr uint16_t kWidth = 500;
    static constexpr uint16_t kHeight = 600;
    static constexpr float kWidthF = 500.f;
    static constexpr float kHeightF = 600.f;

    static std::vector<SkPath> GetPaths();
};

template <>
struct BenchmarkDatasetInfo<BenchmarkDataset::kMixed> {
    static constexpr const char* kName = "Mixed";
    static constexpr uint16_t kWidth = 1000;
    static constexpr uint16_t kHeight = 1000;
    static constexpr float kWidthF = 1000.f;
    static constexpr float kHeightF = 1000.f;

    static std::vector<SkPath> GetPaths();
};

}  // namespace skgpu::graphite

#endif  // BenchmarkDataset_DEFINED
