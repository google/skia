/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkIPoint16.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RasterPathUtils.h"
#include "src/gpu/graphite/RecorderPriv.h"

namespace skgpu::graphite {

static constexpr uint32_t kDefaultAtlasDim = 4096;

static constexpr uint32_t kSmallPathPlotWidth = 512;
static constexpr uint32_t kSmallPathPlotHeight = 256;

RasterPathAtlas::RasterPathAtlas(Recorder* recorder)
        : PathAtlas(recorder, kDefaultAtlasDim, kDefaultAtlasDim)
        , fCachedAtlasMgr(fWidth, fHeight, fWidth, fHeight, recorder->priv().caps())
        , fSmallPathAtlasMgr(std::max(fWidth/2, kSmallPathPlotWidth),
                             std::max(fHeight/2, kSmallPathPlotHeight),
                             kSmallPathPlotWidth, kSmallPathPlotHeight,
                             recorder->priv().caps())
        , fUncachedAtlasMgr(fWidth, fHeight, fWidth, fHeight, recorder->priv().caps()) {
    SkASSERT(recorder);
}

void RasterPathAtlas::recordUploads(DrawContext* dc) {
    fCachedAtlasMgr.recordUploads(dc, fRecorder);
    fSmallPathAtlasMgr.recordUploads(dc, fRecorder);
    fUncachedAtlasMgr.recordUploads(dc, fRecorder);
}

const TextureProxy* RasterPathAtlas::onAddShape(const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::half2 maskSize,
                                                skvx::half2* outPos) {
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        constexpr int kMaxSmallPathSize = 162;
        // Try to locate or add to cached DrawAtlas
        const TextureProxy* proxy = nullptr;
        if (maskSize.x() <= kMaxSmallPathSize && maskSize.y() <= kMaxSmallPathSize) {
            proxy = fSmallPathAtlasMgr.findOrCreateEntry(fRecorder,
                                                         shape,
                                                         transform,
                                                         strokeRec,
                                                         maskSize,
                                                         outPos);
        }
        if (!proxy) {
            proxy = fCachedAtlasMgr.findOrCreateEntry(fRecorder,
                                                      shape,
                                                      transform,
                                                      strokeRec,
                                                      maskSize,
                                                      outPos);
        }
        if (proxy) {
            return proxy;
        }
    }

    // try to add to uncached DrawAtlas
    AtlasLocator loc;
    return fUncachedAtlasMgr.addToAtlas(fRecorder,
                                        shape,
                                        transform,
                                        strokeRec,
                                        maskSize,
                                        outPos,
                                        &loc);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool RasterPathAtlas::RasterAtlasMgr::onAddToAtlas(const Shape& shape,
                                                   const Transform& transform,
                                                   const SkStrokeRec& strokeRec,
                                                   SkIRect shapeBounds,
                                                   const AtlasLocator& locator) {
    // Rasterize path to backing pixmap.
    // This pixmap will be the size of the Plot that contains the given rect, not the entire atlas,
    // and hence the position we render at will be relative to that Plot.
    // The value of outPos is relative to the entire texture, to be used for texture coords.
    SkAutoPixmapStorage dst;
    SkIPoint renderPos = fDrawAtlas->prepForRender(locator, &dst);

    RasterMaskHelper helper(&dst);
    if (!helper.init(fDrawAtlas->plotSize())) {
        return false;
    }
    // Offset to plot location and draw
    shapeBounds.offset(renderPos.x()+kEntryPadding, renderPos.y()+kEntryPadding);
    helper.drawShape(shape, transform, strokeRec, shapeBounds);

    return true;
}

}  // namespace skgpu::graphite
