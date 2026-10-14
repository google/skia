/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/BenchmarkDataset.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/geom/EndCaps.h"
#include "src/gpu/graphite/geom/WideTiles.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MSAA_LUT.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"
#include "src/gpu/graphite/sparse_strips/WindingHistogram.h"

#include <vector>

namespace skgpu::graphite {

template <uint16_t kTileWidth, uint16_t kTileHeight>
class CullBench : public Benchmark {
public:
    using DatasetInfo = BenchmarkDatasetInfo<BenchmarkDataset::kTiger>;

    CullBench(int percentOnscreen)
            : fPercentOnscreen(percentOnscreen)
            , fLUT(GenerateMSAALUT()) {
        fName.printf("SparseStrips_CullBench_%ux%u_%d_percent",
                     kTileWidth, kTileHeight, percentOnscreen);
    }

protected:
    const char* onGetName() override { return fName.c_str(); }

    bool isSuitableFor(Backend backend) override {
        return backend == Backend::kGraphite;
    }

    void onDelayedSetup() override {
        float tx = DatasetInfo::kWidthF * (100.0f - fPercentOnscreen) / 100.0f;
        SkMatrix matrix = SkMatrix::Translate(-tx, 0.0f);

        std::vector<SkPath> paths = DatasetInfo::GetPaths();
        for (const SkPath& path : paths) {
            fPaths.push_back(path.makeTransform(matrix));
        }
    }

    void onDraw(int loops, SkCanvas* canvas) override {
        Flatten flattener;
        WideTiles wides;
        EndCaps ends;
        std::unique_ptr<AlphaAtlasManager> atlasMgr =
                std::make_unique<AlphaAtlasManager>(canvas->recorder());

        int32_t rows = (DatasetInfo::kHeight + kTileHeight - 1) / kTileHeight;
        WindingHistogram windingHistogram(rows);
        windingHistogram.resize(rows);

        const SkIRect dim = SkIRect::MakeWH(DatasetInfo::kWidth, DatasetInfo::kHeight);

        for (int i = 0; i < loops; ++i) {
            for (const SkPath& path : fPaths) {
                Polyline polyline;
                Tiles<kTileWidth, kTileHeight> tiler;

                flattener.processPaths<FlattenMode::kSimd>(path, SkMatrix::I(), dim, &polyline);

                windingHistogram.clear();
                bool culled = tiler.makeTilesMSAA(polyline, dim, &windingHistogram);

                tiler.sortTiles();

                MakeStrips::MsaaSimd<kTileWidth, kTileHeight>(
                        tiler, &wides, &ends, atlasMgr.get(), path.getFillType(), polyline,
                        fLUT, culled, windingHistogram, dim, nullptr);

                wides.clear();
                ends.clear();
                atlasMgr->freeGpuResources();
            }
        }
    }

private:
    SkString fName;
    int fPercentOnscreen;
    std::vector<SkPath> fPaths;
    SparseStripConfig::LUTArray fLUT;
};

}  // namespace skgpu::graphite

DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(10));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(20));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(30));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(40));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(50));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(60));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(70));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(80));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(90));)
DEF_BENCH(return (new skgpu::graphite::CullBench<8, 8>(100));)
