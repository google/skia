/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"
#include "bench/Tiger.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

#include <vector>

namespace skgpu::graphite {

template<uint16_t kTileWidth, uint16_t kTileHeight, bool kShouldSort>
class TilerBench : public Benchmark {
public:
    TilerBench() {
        fName.printf("SparseStrips_Tiler%sBench_%ux%u",
                     kShouldSort ? "Sort" : "" , kTileWidth, kTileHeight);
    }

protected:
    const char* onGetName() override {
        return fName.c_str();
    }

    void onDelayedSetup() override {
        Flatten flattener;
        std::vector<SkPath> tigerPaths = Tiger::GetTigerPaths();
        for (auto& subPath : tigerPaths) {
            flattener.processPaths<FlattenMode::kSimd>(subPath, SkMatrix(),
                                                       Tiger::kTigerWidthF, Tiger::kTigerHeightF,
                                                       &fPolyline);
        }
    }

    void onDraw(int loops, SkCanvas* /*canvas*/) override {
        Tiles<kTileWidth, kTileHeight> tiler;
        for (int i = 0; i < loops; ++i) {
            tiler.reset();
            tiler.makeTilesMSAA(fPolyline, Tiger::kTigerHeight, Tiger::kTigerWidth);
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

DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4, /*kShouldSort=*/false>()); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8, /*kShouldSort=*/false>()); )
DEF_BENCH(return (new skgpu::graphite::TilerBench<4, 4, /*kShouldSort=*/true>());  )
DEF_BENCH(return (new skgpu::graphite::TilerBench<8, 8, /*kShouldSort=*/true>());  )
