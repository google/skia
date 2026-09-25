/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/sparse_strips/StripGenerator.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/sparse_strips/AlphaAtlasManager.h"
#include "src/gpu/graphite/sparse_strips/Flatten.h"
#include "src/gpu/graphite/sparse_strips/MakeStrips.h"
#include "src/gpu/graphite/sparse_strips/Polyline.h"
#include "src/gpu/graphite/sparse_strips/SparseStripsConfig.h"
#include "src/gpu/graphite/sparse_strips/Tiler.h"

namespace skgpu::graphite {

StripGenerator::StripGenerator(int width,
                               int height,
                               const SkTDArray<uint8_t>& maskLUT,
                               Recorder* recorder)
        : fWidth(width)
        , fHeight(height)
        , fMaskLUT(maskLUT)
        , fRecorder(recorder) {}

StripGenerator::~StripGenerator() = default;

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    fEnds.clear();
    fWides.clear();

    AlphaAtlasManager* atlasManager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
    if (!atlasManager) {
        return false;
    }

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(path,
                                               ctm,
                                               static_cast<float>(fWidth),
                                               static_cast<float>(fHeight),
                                               &polyline);

    if (polyline.count() == 0) {
        return true;
    }

    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> tiler;
    tiler.makeTilesMSAA(polyline, fWidth, fHeight);
    tiler.sortTiles();

    if (tiler.getTiles().empty()) {
        return true;
    }

    bool success = MakeStrips::MsaaSimd<SparseStripConfig::kTileWidth,
                                        SparseStripConfig::kTileHeight>(
            tiler,
            &fWides,
            &fEnds,
            atlasManager,
            path.getFillType(),
            polyline,
            fMaskLUT,
            fWidth,
            fHeight);

    if (!success) {
        return false;
    }

    atlasManager->populateProxies(&fEnds);
    return true;
}

bool StripGenerator::cleanupNullCaps() {
    AlphaAtlasManager* atlasManager = fRecorder->priv().atlasProvider()->alphaAtlasManager();
    SkASSERT(atlasManager);
    return atlasManager->resolveNullCaps(&fEnds);
}

}  // namespace skgpu::graphite
