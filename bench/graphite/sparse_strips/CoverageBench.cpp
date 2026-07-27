/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/BenchmarkDataset.h"
#include "include/core/SkPath.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Strip.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, BenchmarkDataset kDataset>
class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = void (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  SkTDArray<Strip>*,
                                  SkTDArray<uint8_t>*,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SkTDArray<uint8_t>&,
                                  MsaaExactMaskObserver);

    CoverageBench(const char* name, MakeStripsFn func) : fFunc(func) {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        fName.printf("SparseStrips_%s_%s_%ux%u", name, DatasetInfo::kName, kTileWidth, kTileHeight);
        fLUT = GenerateMSAALUT<uint8_t>();
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    void onDelayedSetup() override {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        Flatten flattener;
        std::vector<SkPath> paths = DatasetInfo::GetPaths();
        for (auto& subPath : paths) {
            flattener.processPaths<FlattenMode::kSimd>(subPath, SkMatrix(),
                                                       DatasetInfo::kWidthF, DatasetInfo::kHeightF,
                                                       &fPolyline);
        }
        fTiles.makeTilesMSAA(fPolyline, DatasetInfo::kWidth, DatasetInfo::kHeight);
        fTiles.sortTiles();
    }

    void onDraw(int loops, SkCanvas* /*canvas*/) override {
        SkTDArray<Strip> strips;
        SkTDArray<uint8_t> alphas;
        for (int i = 0; i < loops; ++i) {
            fFunc(fTiles, &strips, &alphas, SkPathFillType::kDefault, fPolyline, fLUT,
                  /*MsaaExactMaskObserver=*/nullptr);
            strips.resize(0);
            alphas.resize(0);
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    Polyline fPolyline;
    Tiles<kTileWidth, kTileHeight> fTiles;
    SkTDArray<uint8_t> fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::BenchmarkDataset::kTiger>(
                                                     "CoverageBenchScalar",
                                                     &skgpu::graphite::MakeStrips::MsaaScalar));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::BenchmarkDataset::kTiger>(
                                                     "CoverageBenchSimd",
                                                     &skgpu::graphite::MakeStrips::MsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<8, 8,
                                                     skgpu::graphite::BenchmarkDataset::kTiger>(
                                                     "CoverageBenchSimd",
                                                     &skgpu::graphite::MakeStrips::MsaaSimd));)

DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::BenchmarkDataset::kMixed>(
                                                     "CoverageBenchScalar",
                                                     &skgpu::graphite::MakeStrips::MsaaScalar));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<4, 4,
                                                     skgpu::graphite::BenchmarkDataset::kMixed>(
                                                     "CoverageBenchSimd",
                                                     &skgpu::graphite::MakeStrips::MsaaSimd));)
DEF_BENCH(return (new skgpu::graphite::CoverageBench<8, 8,
                                                     skgpu::graphite::BenchmarkDataset::kMixed>(
                                                     "CoverageBenchSimd",
                                                     &skgpu::graphite::MakeStrips::MsaaSimd));)
