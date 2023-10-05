/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkIPoint16.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/AtlasProvider.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/RecorderPriv.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {
namespace {

}  // namespace

RasterPathAtlas::RasterPathAtlas()
    : PathAtlas(kDefaultAtlasDim, kDefaultAtlasDim) {

    // set up LRU list
    Page* currPage = &fPageArray[0];
    for (int i = 0; i < kMaxPages; ++i) {
        fPageList.addToHead(currPage);
        ++currPage;
    }
}

void RasterPathAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {

    // TODO: cycle through all the pages and handle their uploads
    Page* mru = fPageList.head();
    SkASSERT(mru);

    // build an upload for the dirty rect and record it
    if (!mru->fDirtyRect.isEmpty()) {
        std::vector<MipLevel> levels;
        levels.push_back({mru->fPixels.addr(), mru->fPixels.rowBytes()});

        SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

         if (!dc->recordUpload(recorder, mru->fTexture, colorInfo, colorInfo, levels,
                              mru->fDirtyRect, nullptr)) {
            SKGPU_LOG_W("Coverage mask upload failed!");
            return;
        }

        // TODO: Keep using this texture until full and cache the results, then get a new one.
    }
}

bool RasterPathAtlas::Page::initializeTextureIfNeeded(Recorder* recorder) {
    if (!fTexture) {
        AtlasProvider* atlasProvider = recorder->priv().atlasProvider();
        fTexture = atlasProvider->getAtlasTexture(recorder,
                                                  fRectanizer.width(),
                                                  fRectanizer.height(),
                                                  kAlpha_8_SkColorType,
                                                  /*requiresStorageUsage=*/false);
    }
    return fTexture != nullptr;
}

const TextureProxy* RasterPathAtlas::addRect(Recorder* recorder,
                                             skvx::float2 atlasSize,
                                             SkIPoint16* outPos) {
    // TODO: look through all pages and find the first one with room, and move that to MRU
    Page* mru = fPageList.head();
    SkASSERT(mru);
    if (!mru->initializeTextureIfNeeded(recorder)) {
        SKGPU_LOG_E("Failed to instantiate an atlas texture");
        return nullptr;
    }

    // An empty mask always fits, so just return the texture.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(atlasSize)) {
        return mru->fTexture.get();
    }

    if (!mru->fRectanizer.addRect(atlasSize.x(), atlasSize.y(), outPos)) {
        return nullptr;
    }

    return mru->fTexture.get();
}

const TextureProxy* RasterPathAtlas::onAddShape(Recorder* recorder,
                                                const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::float2 atlasSize,
                                                skvx::int2 deviceOffset,
                                                skvx::half2* outPos) {
    // TODO: look up shape and use cached texture

    // Try to add to Rectanizer
    SkIPoint16 iPos;
    const TextureProxy* texProxy = this->addRect(recorder, atlasSize, &iPos);
    if (!texProxy) {
        return nullptr;
    }
    *outPos = skvx::half2(iPos.x(), iPos.y());
    // If the mask is empty, just return.
    // TODO: This may not be needed if we can handle clipped out bounds with inverse fills
    // another way. See PathAtlas::addShape().
    if (!all(atlasSize)) {
        return texProxy;
    }

    // Set up render

    // The MRU page should be already set up by addRect()
    Page* mru = fPageList.head();
    SkASSERT(mru);
    SkASSERT(mru->fTexture.get() == texProxy);

    // allocate pixmap if needed
    if (!mru->fPixels.addr()) {
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(mru->fRectanizer.width(),
                                                            mru->fRectanizer.height());
        if (!mru->fPixels.tryAlloc(bmImageInfo)) {
            return nullptr;
        }
        mru->fPixels.erase(0);
    }

    // Rasterize path to backing pixmap
    // TODO: render in a separate thread?
    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst      = mru->fPixels;
    SkRasterClip rasterClip;
    SkIRect iAtlasBounds = SkIRect::MakeXYWH(iPos.x(), iPos.y(),
                                             atlasSize.x(), atlasSize.y());
    rasterClip.setRect(iAtlasBounds);
    draw.fRC       = &rasterClip;

    SkPaint paint;
    paint.setBlendMode(SkBlendMode::kSrc);  // "Replace" mode
    paint.setAntiAlias(true);
    // SkPaint's color is unpremul so this will produce alpha in every channel.
    paint.setColor(SK_ColorWHITE);
    strokeRec.applyToPaint(&paint);

    SkMatrix translatedMatrix = SkMatrix(transform);
    // The atlas transform of the shape is the linear-components (scale, rotation, skew) of
    // `localToDevice` translated by the top-left offset of `atlasBounds`, accounting for the 1
    // pixel-wide border we added earlier, so that the shape is correctly centered.
    translatedMatrix.postTranslate(iAtlasBounds.x() + 1 - deviceOffset.x(),
                                   iAtlasBounds.y() + 1 - deviceOffset.y());
    draw.fCTM = &translatedMatrix;
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    draw.drawPathCoverage(path, paint);

    // Add atlasBounds to dirtyRect for later upload
    mru->fDirtyRect.join(iAtlasBounds);

    return texProxy;
}

void RasterPathAtlas::reset() {
    // TODO: this will go away and we'll only reset the LRU page if we're out of space

    Page* mru = fPageList.head();
    SkASSERT(mru);
    mru->fRectanizer.reset();

    // clear backing data for next pass
    mru->fDirtyRect.setEmpty();
    mru->fPixels.erase(0);
}

}  // namespace skgpu::graphite
