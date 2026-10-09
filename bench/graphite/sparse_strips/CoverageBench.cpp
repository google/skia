/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/BenchmarkDataset.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/gpu/graphite/Recorder.h"
#include "include/private/SkTDArray.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight, BenchmarkDataset kDataset>
class CoverageBench : public Benchmark {
public:
    using MakeStripsFn = bool (*)(const Tiles<kTileWidth, kTileHeight>&,
                                  WideTiles*,
                                  EndCaps*,
                                  AlphaAtlasManager*,
                                  SkPathFillType,
                                  const Polyline&,
                                  const SparseStripConfig::LUTArray&,
                                  bool isCulled,
                                  const WindingHistogram&,
                                  const SkIRect& clipRect,
                                  MsaaExactMaskObserver);

    CoverageBench(const char* name, MakeStripsFn func) : fFunc(func), fLUT(GenerateMSAALUT()) {
        using DatasetInfo = BenchmarkDatasetInfo<kDataset>;
        fName.printf("SparseStrips_%s_%s_%ux%u", name, DatasetInfo::kName, kTileWidth, kTileHeight);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kGraphite;
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
        fClip = SkIRect::MakeWH(DatasetInfo::kWidth, DatasetInfo::kHeight);
        fHistogram.resize((fClip.height() + kTileHeight - 1) / kTileHeight);
        fHistogram.clear();
        fCulled = fTiles.makeTilesMSAA(fPolyline, fClip, &fHistogram);
        fTiles.sortTiles();
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        WideTiles wides;
        EndCaps ends;
        std::unique_ptr<AlphaAtlasManager> atlasMgr =
                std::make_unique<AlphaAtlasManager>(canvas->recorder());
        for (int i = 0; i < loops; ++i) {
            fFunc(fTiles, &wides, &ends, atlasMgr.get(), SkPathFillType::kDefault, fPolyline, fLUT,
                  fCulled, fHistogram, fClip, /*MsaaExactMaskObserver=*/nullptr);
            wides.clear();
            ends.clear();
            atlasMgr->freeGpuResources();
        }
    }

private:
    SkString fName;
    MakeStripsFn fFunc;
    Polyline fPolyline;
    Tiles<kTileWidth, kTileHeight> fTiles;
    WindingHistogram fHistogram;
    SkIRect fClip;
    bool fCulled = false;
    const SparseStripConfig::LUTArray fLUT;
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
