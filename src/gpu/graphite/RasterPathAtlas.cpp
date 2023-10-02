/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/graphite/RasterPathAtlas.h"

#include "include/core/SkColorSpace.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkRasterClip.h"
#include "src/gpu/graphite/DrawContext.h"
#include "src/gpu/graphite/Log.h"
#include "src/gpu/graphite/TextureProxy.h"

namespace skgpu::graphite {
namespace {

// TODO: select atlas size dynamically? Take ContextOptions::fMaxTextureAtlasSize into account?

// TODO: for now
constexpr uint16_t kAtlasDim = 4096;

}  // namespace

RasterPathAtlas::RasterPathAtlas() : PathAtlas(kAtlasDim, kAtlasDim) {}

void RasterPathAtlas::recordUploads(DrawContext* dc, Recorder* recorder) {
    // build an upload for the dirty rect and record it
    if (!fDirtyRect.isEmpty()) {
        std::vector<MipLevel> levels;
        levels.push_back({fPixels.addr(), fPixels.rowBytes()});

        SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

        SkASSERT(this->texture());
        if (!dc->recordUpload(recorder, sk_ref_sp(this->texture()), colorInfo, colorInfo, levels,
                              fDirtyRect, nullptr)) {
            SKGPU_LOG_W("Coverage mask upload failed!");
            return;
        }

        // TODO: Keep using this texture until full and cache the results, then get a new one.
    }
}

void RasterPathAtlas::onAddShape(const Shape& shape,
                                 const Transform& transform,
                                 const Rect& atlasBounds,
                                 skvx::int2 deviceOffset,
                                 const SkStrokeRec& strokeRec) {
    // TODO: look up shape and use cached texture
    // Need to push this up into addShape() somehow

    // allocate pixmap if needed
    if (!fPixels.addr()) {
        const SkImageInfo bmImageInfo = SkImageInfo::MakeA8(kAtlasDim, kAtlasDim);
        if (!fPixels.tryAlloc(bmImageInfo)) {
            return;
        }
        fPixels.erase(0);
    }

    // Rasterize path to backing pixmap
    // TODO: render in a separate thread?
    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fDst      = fPixels;
    SkRasterClip rasterClip;
    SkIRect iAtlasBounds = atlasBounds.asSkIRect();
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
    translatedMatrix.postTranslate(atlasBounds.x() + 1 - deviceOffset.x(),
                                   atlasBounds.y() + 1 - deviceOffset.y());
    draw.fCTM = &translatedMatrix;
    SkPath path = shape.asPath();
    if (path.isInverseFillType()) {
        // The shader will handle the inverse fill in this case
        path.toggleInverseFillType();
    }
    draw.drawPathCoverage(path, paint);

    // Add atlasBounds to dirtyRect for later upload
    fDirtyRect.join(iAtlasBounds);

    // TODO: cache shape data and texture used
}

void RasterPathAtlas::onReset() {
    // clear backing data for next pass
    fDirtyRect.setEmpty();
    fPixels.erase(0);
}

PathAtlas::MaskFormat RasterPathAtlas::coverageMaskFormat(const Caps*) const {
    return {kAlpha_8_SkColorType, /*requiresStorageUsage=*/false};
}

}  // namespace skgpu::graphite
