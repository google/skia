/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/BenchmarkDataset.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <vector>

namespace skgpu::graphite {

template<uint16_t kTileWidth, uint16_t kTileHeight, bool kShouldSort,
         BenchmarkDataset kDataset = BenchmarkDataset::kTiger>
class TilerBench : public Benchmark {
public:
    TilerBench() {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        fName.printf("SparseStrips_Tiler%sBench_%s_%ux%u",
                     kShouldSort ? "Sort" : "", DatasetInfo::kName, kTileWidth, kTileHeight);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        Flatten flattener;
        std::vector<SkPath> paths = DatasetInfo::GetPaths();
        for (auto& subPath : paths) {
            flattener.processPaths<FlattenMode::kSimd>(subPath, SkMatrix(),
                                                       DatasetInfo::kWidthF, DatasetInfo::kHeightF,
                                                       &fPolyline);
        }
    }

    void onDraw(int loops, SkCanvas* /*canvas*/) override {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        Tiles<kTileWidth, kTileHeight> tiler;
        for (int i = 0; i < loops; ++i) {
            tiler.reset();
            tiler.makeTilesMSAA(fPolyline, DatasetInfo::kWidth, DatasetInfo::kHeight);
            if constexpr (kShouldSort) {
                tiler.sortTiles();
            }
        }
    }

private:

    SkString fName;
    Polyline fPolyline;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4, /*kShouldSort=*/false>); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8, /*kShouldSort=*/false>); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4, /*kShouldSort=*/true>); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8, /*kShouldSort=*/true>); )
