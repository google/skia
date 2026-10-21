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
                               const SparseStripConfig::LUTArray& maskLUT,
                               Recorder* recorder)
        : fWidth(width)
        , fHeight(height)
        , fMaskLUT(maskLUT)
        , fRecorder(recorder)
        , fWindingHistogram((height + SparseStripConfig::kTileHeight - 1) /
                            SparseStripConfig::kTileHeight)
        , fCulled(true) {}

StripGenerator::~StripGenerator() = default;

#if defined(GPU_TEST_UTILS)
StripGenerator::DebugCaptureFn& StripGenerator::DebugCaptureCallbackStorage() {
    // Leaked so that there is no exit-time destructor.
    static DebugCaptureFn* gFn = new DebugCaptureFn();
    return *gFn;
}

void StripGenerator::SetDebugCaptureCallback(DebugCaptureFn fn) {
    DebugCaptureCallbackStorage() = std::move(fn);
}

const StripGenerator::DebugCaptureFn& StripGenerator::GetDebugCaptureCallback() {
    return DebugCaptureCallbackStorage();
}
#endif

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm) {
    return this->processGeometry(path, ctm, SkIRect::MakeWH(fWidth, fHeight));
}

bool StripGenerator::processGeometry(const SkPath& path, const SkMatrix& ctm, const SkIRect& clip) {
#if defined(GPU_TEST_UTILS)
    if (const DebugCaptureFn& captureFn = GetDebugCaptureCallback()) {
        captureFn(path, ctm, clip, SkISize::Make(fWidth, fHeight));
    }
#endif
    fEnds.clear();
    fWides.clear();

    SkIRect tileClip = clip;
    if (!tileClip.intersect(SkIRect::MakeWH(fWidth, fHeight))) {
        return true;
    }

    // Snap the top-left down to the tile grid so that the first tile row and column are whole.
    // Although the clip does get tile aligned when outsetBoundsToGrid() is called, it is
    // immediately intersected with the scissor afterwards, so the clip still requires aligning
    // here.
    tileClip.fLeft =
            (tileClip.fLeft / SparseStripConfig::kTileWidth) * SparseStripConfig::kTileWidth;
    tileClip.fTop =
            (tileClip.fTop / SparseStripConfig::kTileHeight) * SparseStripConfig::kTileHeight;

    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    if (!atlasManager) {
        return false;
    }

    Flatten flattener;
    Polyline polyline;
    flattener.processPaths<FlattenMode::kSimd>(path, ctm, tileClip, &polyline);

    if (polyline.count() == 0 && !path.isInverseFillType()) {
        return true;
    }

    fWindingHistogram.resize((fHeight + SparseStripConfig::kTileHeight - 1) /
                             SparseStripConfig::kTileHeight);
    if (fCulled) {
        fWindingHistogram.clear();
    }
#if defined(SK_DEBUG)
    // Tiler only writes to the histogram when it reports a culling event, so if the previous draw
    // did not cull, the histogram must still be zero from the last time it was cleared.
    for (int16_t winding : fWindingHistogram.data()) {
        SkASSERT(winding == 0);
    }
#endif

    Tiles<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight> tiler;
    fCulled = tiler.makeTilesMSAA(polyline, tileClip, &fWindingHistogram);
    tiler.sortTiles();

    bool success =
            MakeStrips::MsaaSimd<SparseStripConfig::kTileWidth, SparseStripConfig::kTileHeight>(
                    tiler,
                    &fWides,
                    &fEnds,
                    atlasManager,
                    path.getFillType(),
                    polyline,
                    fMaskLUT,
                    fCulled,
                    fWindingHistogram,
                    tileClip);

    if (!success) {
        return false;
    }

    if (!fEnds.empty()) {
        atlasManager->populateProxies(&fEnds);
    }
    return true;
}

bool StripGenerator::cleanupNullCaps() {
    AlphaAtlasManager* atlasManager =
            fRecorder->priv().getOrCreateAtlasProvider()->alphaAtlasManager();
    SkASSERT(atlasManager);
    return atlasManager->resolveNullCaps(&fEnds);
}

}  // namespace skgpu::graphite
