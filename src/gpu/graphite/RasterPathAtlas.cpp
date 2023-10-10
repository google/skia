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
        size_t rowBytes = mru->fPixels.rowBytes();
        const unsigned char* dataPtr = (const unsigned char*) mru->fPixels.addr();
        dataPtr += rowBytes * mru->fDirtyRect.fTop;
        dataPtr += mru->fPixels.info().bytesPerPixel() * mru->fDirtyRect.fLeft;

        std::vector<MipLevel> levels;
        levels.push_back({dataPtr, rowBytes});

        SkColorInfo colorInfo(kAlpha_8_SkColorType, kUnknown_SkAlphaType, nullptr);

        if (!dc->recordUpload(recorder, mru->fTexture, colorInfo, colorInfo, levels,
                              mru->fDirtyRect, nullptr)) {
            SKGPU_LOG_W("Coverage mask upload failed!");
            return;
        }

        mru->fDirtyRect.setEmpty();
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

namespace {
skgpu::UniqueKey generate_key(const Shape& shape,
                              const Transform& transform,
                              const SkStrokeRec& strokeRec,
                              skvx::float2 atlasSize) {
    skgpu::UniqueKey maskKey;
    {
        static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
        skgpu::UniqueKey::Builder builder(&maskKey, kDomain, 7 + shape.keySize(),
                                          "Raster Path Mask");
        builder[0] = atlasSize.x();
        builder[1] = atlasSize.y();

        // We require the upper left 2x2 of the matrix to match exactly for a cache hit.
        SkMatrix mat = transform.matrix().asM33();
        SkScalar sx = mat.get(SkMatrix::kMScaleX);
        SkScalar sy = mat.get(SkMatrix::kMScaleY);
        SkScalar kx = mat.get(SkMatrix::kMSkewX);
        SkScalar ky = mat.get(SkMatrix::kMSkewY);
#ifdef SK_BUILD_FOR_ANDROID_FRAMEWORK
        // Fractional translate does not affect caching on Android. This is done for better cache
        // hit ratio and speed and is matching HWUI behavior, which didn't consider the matrix
        // at all when caching paths.
        SkFixed fracX = 0;
        SkFixed fracY = 0;
#else
        SkScalar tx = mat.get(SkMatrix::kMTransX);
        SkScalar ty = mat.get(SkMatrix::kMTransY);
        // Allow 8 bits each in x and y of subpixel positioning.
        SkFixed fracX = SkScalarToFixed(SkScalarFraction(tx)) & 0x0000FF00;
        SkFixed fracY = SkScalarToFixed(SkScalarFraction(ty)) & 0x0000FF00;
#endif
        builder[2] = SkFloat2Bits(sx);
        builder[3] = SkFloat2Bits(sy);
        builder[4] = SkFloat2Bits(kx);
        builder[5] = SkFloat2Bits(ky);
        // FracX and fracY are &ed with 0x0000ff00, so need to shift one down to fill 16 bits.
        uint32_t fracBits = fracX | (fracY >> 8);
        // Distinguish between hairline and filled paths. For hairlines, we also need to include
        // the cap. (SW grows hairlines by 0.5 pixel with round and square caps). Note that
        // stroke-and-fill of hairlines is turned into pure fill by SkStrokeRec, so this covers
        // all cases we might see.
        uint32_t styleBits = strokeRec.isHairlineStyle() ? ((strokeRec.getCap() << 1) | 1) : 0;
        builder[6] = fracBits | (styleBits << 16);
        shape.writeKey(&builder[7]);
    }
    return maskKey;
}
} // namespace

const TextureProxy* RasterPathAtlas::onAddShape(Recorder* recorder,
                                                const Shape& shape,
                                                const Transform& transform,
                                                const SkStrokeRec& strokeRec,
                                                skvx::float2 atlasSize,
                                                skvx::int2 deviceOffset,
                                                skvx::half2* outPos) {
    // TODO: iterate through pagelist in MRU order
    Page* mru = fPageList.head();
    SkASSERT(mru);

    // Look up shape and use cached texture and position if found.
    skgpu::UniqueKey maskKey;
    bool hasKey = shape.hasKey();
    if (hasKey) {
        maskKey = generate_key(shape, transform, strokeRec, atlasSize);
        skvx::half2* found = mru->fCachedShapes.find(maskKey);
        if (found) {
            *outPos = *found;
            return mru->fTexture.get();
        }
    }

    // Try to add to Rectanizer
    SkIPoint16 iPos;
    const TextureProxy* texProxy = this->addRect(recorder, atlasSize, &iPos);
    if (!texProxy) {
        // Failed, request flush and reset this Page
        // TODO: only reset LRU Page
        mru->fNeedsReset = true;
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

    // Add to cache
    if (hasKey) {
        mru->fCachedShapes.set(maskKey, *outPos);
    }

    return texProxy;
}

void RasterPathAtlas::reset() {
    // TODO: only reset LRU Page if needed
    Page* mru = fPageList.head();
    SkASSERT(mru);
    if (mru->fNeedsReset) {
        mru->fRectanizer.reset();

        // clear backing data for next pass
        SkASSERT(mru->fDirtyRect.isEmpty());
        mru->fPixels.erase(0);
        mru->fCachedShapes.reset();
        mru->fNeedsReset = false;
    }
}

}  // namespace skgpu::graphite
