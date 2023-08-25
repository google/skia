/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/TiledTextureUtils.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkRect.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSize.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkCanvasPriv.h"
#include "src/core/SkDevice.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkSamplingPriv.h"
#include "src/image/SkImage_Base.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
#endif

#if defined(SK_GRAPHITE)
#include "src/gpu/graphite/Caps.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

#if defined(GR_TEST_UTILS)
// GrContextOptions::fMaxTextureSizeOverride exists but doesn't allow for changing the
// maxTextureSize on the fly.
int gOverrideMaxTextureSize = 0;
std::atomic<int> gNumTilesDrawn{0};
#endif

//////////////////////////////////////////////////////////////////////////////
//  Helper functions for tiling a large SkBitmap

namespace {

static const int kBmpSmallTileSize = 1 << 10;

size_t get_tile_count(const SkIRect& srcRect, int tileSize)  {
    int tilesX = (srcRect.fRight / tileSize) - (srcRect.fLeft / tileSize) + 1;
    int tilesY = (srcRect.fBottom / tileSize) - (srcRect.fTop / tileSize) + 1;
    // We calculate expected tile count before we read the bitmap's pixels, so hypothetically we can
    // have lazy images with excessive dimensions that would cause (tilesX*tilesY) to overflow int.
    // In these situations we also later fail to allocate a bitmap to store the lazy image, so there
    // isn't really a performance concern around one image turning into millions of tiles.
    return SkSafeMath::Mul(tilesX, tilesY);
}

int determine_tile_size(const SkIRect& src, int maxTileSize) {
    if (maxTileSize <= kBmpSmallTileSize) {
        return maxTileSize;
    }

    size_t maxTileTotalTileSize = get_tile_count(src, maxTileSize);
    size_t smallTotalTileSize = get_tile_count(src, kBmpSmallTileSize);

    maxTileTotalTileSize *= maxTileSize * maxTileSize;
    smallTotalTileSize *= kBmpSmallTileSize * kBmpSmallTileSize;

    if (maxTileTotalTileSize > 2 * smallTotalTileSize) {
        return kBmpSmallTileSize;
    } else {
        return maxTileSize;
    }
}

// Given a bitmap, an optional src rect, and a context with a clip and matrix determine what
// pixels from the bitmap are necessary.
SkIRect determine_clipped_src_rect(SkIRect clippedSrcIRect,
                                   const SkMatrix& viewMatrix,
                                   const SkMatrix& srcToDstRect,
                                   const SkISize& imageDimensions,
                                   const SkRect* srcRectPtr) {
    SkMatrix inv = SkMatrix::Concat(viewMatrix, srcToDstRect);
    if (!inv.invert(&inv)) {
        return SkIRect::MakeEmpty();
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);
    inv.mapRect(&clippedSrcRect);
    if (srcRectPtr) {
        if (!clippedSrcRect.intersect(*srcRectPtr)) {
            return SkIRect::MakeEmpty();
        }
    }
    clippedSrcRect.roundOut(&clippedSrcIRect);
    SkIRect bmpBounds = SkIRect::MakeSize(imageDimensions);
    if (!clippedSrcIRect.intersect(bmpBounds)) {
        return SkIRect::MakeEmpty();
    }

    return clippedSrcIRect;
}

void draw_tiled_bitmap(SkCanvas* canvas,
                       const SkBitmap& bitmap,
                       int tileSize,
                       const SkMatrix& srcToDst,
                       const SkRect& srcRect,
                       const SkIRect& clippedSrcIRect,
                       const SkPaint* paint,
                       SkCanvas::QuadAAFlags origAAFlags,
                       SkCanvas::SrcRectConstraint constraint,
                       SkSamplingOptions sampling) {
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(/* imageIsMipped= */ false);
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;

#if defined(GR_TEST_UTILS)
    gNumTilesDrawn.store(0, std::memory_order_relaxed);
#endif

    skia_private::TArray<SkCanvas::ImageSetEntry> imgSet(nx * ny);

    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            tileR.setLTRB(SkIntToScalar(x * tileSize),       SkIntToScalar(y * tileSize),
                          SkIntToScalar((x + 1) * tileSize), SkIntToScalar((y + 1) * tileSize));

            if (!SkRect::Intersects(tileR, clippedSrcRect)) {
                continue;
            }

            if (!tileR.intersect(srcRect)) {
                continue;
            }

            SkIRect iTileR;
            tileR.roundOut(&iTileR);
            SkVector offset = SkPoint::Make(SkIntToScalar(iTileR.fLeft),
                                            SkIntToScalar(iTileR.fTop));
            SkRect rectToDraw = tileR;
            if (!srcToDst.mapRect(&rectToDraw)) {
                continue;
            }

            if (sampling.filter != SkFilterMode::kNearest || sampling.useCubic) {
                SkIRect iClampRect;

                if (SkCanvas::kFast_SrcRectConstraint == constraint) {
                    // In bleed mode we want to always expand the tile on all edges
                    // but stay within the bitmap bounds
                    iClampRect = SkIRect::MakeWH(bitmap.width(), bitmap.height());
                } else {
                    // In texture-domain/clamp mode we only want to expand the
                    // tile on edges interior to "srcRect" (i.e., we want to
                    // not bleed across the original clamped edges)
                    srcRect.roundOut(&iClampRect);
                }
                int outset = sampling.useCubic ? kBicubicFilterTexelPad : 1;
                skgpu::TiledTextureUtils::ClampedOutsetWithOffset(&iTileR, outset, &offset,
                                                                  iClampRect);
            }

            // We must subset as a bitmap and then turn it into an SkImage if we want caching to
            // work. Image subsets always make a copy of the pixels and lose the association with
            // the original's SkPixelRef.
            if (SkBitmap subsetBmp; bitmap.extractSubset(&subsetBmp, iTileR)) {
                sk_sp<SkImage> image = SkMakeImageFromRasterBitmap(subsetBmp,
                                                                   kNever_SkCopyPixelsMode);
                if (!image) {
                    continue;
                }

                unsigned aaFlags = SkCanvas::kNone_QuadAAFlags;
                // Preserve the original edge AA flags for the exterior tile edges.
                if (tileR.fLeft <= srcRect.fLeft && (origAAFlags & SkCanvas::kLeft_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kLeft_QuadAAFlag;
                }
                if (tileR.fRight >= srcRect.fRight && (origAAFlags & SkCanvas::kRight_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kRight_QuadAAFlag;
                }
                if (tileR.fTop <= srcRect.fTop && (origAAFlags & SkCanvas::kTop_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kTop_QuadAAFlag;
                }
                if (tileR.fBottom >= srcRect.fBottom &&
                    (origAAFlags & SkCanvas::kBottom_QuadAAFlag)) {
                    aaFlags |= SkCanvas::kBottom_QuadAAFlag;
                }

                // Offset the source rect to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);

                imgSet.push_back(SkCanvas::ImageSetEntry(std::move(image),
                                                         tileR,
                                                         rectToDraw,
                                                         /* matrixIndex= */ -1,
                                                         /* alpha= */ 1.0f,
                                                         aaFlags,
                                                         /* hasClip= */ false));

#if defined(GR_TEST_UTILS)
                (void)gNumTilesDrawn.fetch_add(+1, std::memory_order_relaxed);
#endif
            }
        }
    }

    canvas->experimental_DrawEdgeAAImageSet(imgSet.data(),
                                            imgSet.size(),
                                            /* dstClips= */ nullptr,
                                            /* preViewMatrices= */ nullptr,
                                            sampling,
                                            paint,
                                            constraint);
}

size_t get_cache_size(SkBaseDevice* device) {
#if defined(SK_GANESH)
    if (auto dContext = GrAsDirectContext(device->recordingContext())) {
        // NOTE: if the context is not a direct context, it doesn't have access to the resource
        // cache, and theoretically, the resource cache's limits could be being changed on
        // another thread, so even having access to just the limit wouldn't be a reliable
        // test during recording here.
        return dContext->getResourceCacheLimit();
    }
#endif

#if defined(SK_GRAPHITE)
    if (auto recorder = device->recorder()) {
        // For Graphite this is a pretty loose heuristic. The Recorder-local cache size (relative
        // to the large image's size) is used as a proxy for how conservative we should be when
        // allocating tiles. Since the tiles will actually be owned by the client (via an
        // ImageProvider) they won't actually add any memory pressure directly to Graphite.
        return recorder->priv().getResourceCacheLimit();
    }
#endif

    return 0;
}

int get_max_texture_size(SkCanvas* canvas) {
#if defined(SK_GANESH)
    if (GrRecordingContext* rContext = canvas->recordingContext()) {
        return rContext->maxTextureSize();
    }
#endif

#if defined(SK_GRAPHITE)
    if (auto recorder = canvas->recorder()) {
        return recorder->priv().caps()->maxTextureSize();
    }
#endif

    static const int kFallbackMaxTextureSize = 1 << 22;
    return kFallbackMaxTextureSize;                       // we should never get here
}

} // anonymous namespace

namespace skgpu {

// tileSize and clippedSubset are valid if true is returned
bool TiledTextureUtils::ShouldTileImage(SkIRect conservativeClipBounds,
                                        const SkISize& imageSize,
                                        const SkMatrix& ctm,
                                        const SkMatrix& srcToDst,
                                        const SkRect* src,
                                        int maxTileSize,
                                        size_t cacheSize,
                                        int* tileSize,
                                        SkIRect* clippedSubset) {
    // if it's larger than the max tile size, then we have no choice but tiling.
    if (imageSize.width() > maxTileSize || imageSize.height() > maxTileSize) {
        *clippedSubset = determine_clipped_src_rect(conservativeClipBounds, ctm,
                                                    srcToDst, imageSize, src);
        *tileSize = determine_tile_size(*clippedSubset, maxTileSize);
        return true;
    }

    // If the image would only produce 4 tiles of the smaller size, don't bother tiling it.
    const size_t area = imageSize.width() * imageSize.height();
    if (area < 4 * kBmpSmallTileSize * kBmpSmallTileSize) {
        return false;
    }

    // At this point we know we could do the draw by uploading the entire bitmap as a texture.
    // However, if the texture would be large compared to the cache size and we don't require most
    // of it for this draw then tile to reduce the amount of upload and cache spill.
    if (!cacheSize) {
        // We don't have access to the cacheSize so we will just upload the entire image
        // to be on the safe side and not tile.
        return false;
    }

    // An assumption here is that sw bitmap size is a good proxy for its size as a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    if (bmpSize < cacheSize / 2) {
        return false;
    }

    // Figure out how much of the src we will need based on the src rect and clipping. Reject if
    // tiling memory savings would be < 50%.
    *clippedSubset = determine_clipped_src_rect(conservativeClipBounds, ctm,
                                                srcToDst, imageSize, src);
    *tileSize = kBmpSmallTileSize; // already know whole bitmap fits in one max sized tile.
    size_t usedTileBytes = get_tile_count(*clippedSubset, kBmpSmallTileSize) *
                           kBmpSmallTileSize * kBmpSmallTileSize *
                           sizeof(SkPMColor);  // assume 32bit pixels;

    return usedTileBytes * 2 < bmpSize;
}

/**
 * Optimize the src rect sampling area within an image (sized 'width' x 'height') such that
 * 'outSrcRect' will be completely contained in the image's bounds. The corresponding rect
 * to draw will be output to 'outDstRect'. The mapping between src and dst will be cached in
 * 'outSrcToDst'. Outputs are not always updated when kSkip is returned.
 *
 * 'dstClip' should be null when there is no additional clipping.
 */
TiledTextureUtils::ImageDrawMode TiledTextureUtils::OptimizeSampleArea(const SkISize& imageSize,
                                                                       const SkRect& origSrcRect,
                                                                       const SkRect& origDstRect,
                                                                       const SkPoint dstClip[4],
                                                                       SkRect* outSrcRect,
                                                                       SkRect* outDstRect,
                                                                       SkMatrix* outSrcToDst) {
    if (origSrcRect.isEmpty() || origDstRect.isEmpty()) {
        return ImageDrawMode::kSkip;
    }

    *outSrcToDst = SkMatrix::RectToRect(origSrcRect, origDstRect);

    SkRect src = origSrcRect;
    SkRect dst = origDstRect;

    const SkRect srcBounds = SkRect::Make(imageSize);

    if (!srcBounds.contains(src)) {
        if (!src.intersect(srcBounds)) {
            return ImageDrawMode::kSkip;
        }
        outSrcToDst->mapRect(&dst, src);

        // Both src and dst have gotten smaller. If dstClip is provided, confirm it is still
        // contained in dst, otherwise cannot optimize the sample area and must use a decal instead
        if (dstClip) {
            for (int i = 0; i < 4; ++i) {
                if (!dst.contains(dstClip[i].fX, dstClip[i].fY)) {
                    // Must resort to using a decal mode restricted to the clipped 'src', and
                    // use the original dst rect (filling in src bounds as needed)
                    *outSrcRect = src;
                    *outDstRect = origDstRect;
                    return ImageDrawMode::kDecal;
                }
            }
        }
    }

    // The original src and dst were fully contained in the image, or there was no dst clip to
    // worry about, or the clip was still contained in the restricted dst rect.
    *outSrcRect = src;
    *outDstRect = dst;
    return ImageDrawMode::kOptimized;
}

bool TiledTextureUtils::CanDisableMipmap(const SkMatrix& viewM, const SkMatrix& localM) {
    SkMatrix matrix;
    matrix.setConcat(viewM, localM);
    // We bias mipmap lookups by -0.5. That means our final LOD is >= 0 until
    // the computed LOD is >= 0.5. At what scale factor does a texture get an LOD of
    // 0.5?
    //
    // Want:  0       = log2(1/s) - 0.5
    //        0.5     = log2(1/s)
    //        2^0.5   = 1/s
    //        1/2^0.5 = s
    //        2^0.5/2 = s
    return matrix.getMinScale() >= SK_ScalarRoot2Over2;
}


// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
void TiledTextureUtils::ClampedOutsetWithOffset(SkIRect* iRect, int outset, SkPoint* offset,
                                                const SkIRect& clamp) {
    iRect->outset(outset, outset);

    int leftClampDelta = clamp.fLeft - iRect->fLeft;
    if (leftClampDelta > 0) {
        offset->fX -= outset - leftClampDelta;
        iRect->fLeft = clamp.fLeft;
    } else {
        offset->fX -= outset;
    }

    int topClampDelta = clamp.fTop - iRect->fTop;
    if (topClampDelta > 0) {
        offset->fY -= outset - topClampDelta;
        iRect->fTop = clamp.fTop;
    } else {
        offset->fY -= outset;
    }

    if (iRect->fRight > clamp.fRight) {
        iRect->fRight = clamp.fRight;
    }
    if (iRect->fBottom > clamp.fBottom) {
        iRect->fBottom = clamp.fBottom;
    }
}

bool TiledTextureUtils::DrawAsTiledImageRect(SkCanvas* canvas,
                                             const SkImage* image,
                                             const SkRect& srcRect,
                                             const SkRect& dstRect,
                                             SkCanvas::QuadAAFlags aaFlags,
                                             const SkSamplingOptions& origSampling,
                                             const SkPaint* paint,
                                             SkCanvas::SrcRectConstraint constraint) {
    if (canvas->isClipEmpty()) {
        return true;
    }

    if (!image->isTextureBacked()) {
        SkRect src;
        SkRect dst;
        SkMatrix srcToDst;
        ImageDrawMode mode = OptimizeSampleArea(SkISize::Make(image->width(), image->height()),
                                                srcRect, dstRect, /* dstClip= */ nullptr,
                                                &src, &dst, &srcToDst);
        if (mode == ImageDrawMode::kSkip) {
            return true;
        }

        SkASSERT(mode != ImageDrawMode::kDecal); // only happens if there is a 'dstClip'

        if (src.contains(image->bounds())) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }

        SkBaseDevice* device = SkCanvasPriv::TopDevice(canvas);
        const SkMatrix& localToDevice = device->localToDevice();

        SkSamplingOptions sampling = origSampling;
        if (sampling.mipmap != SkMipmapMode::kNone && CanDisableMipmap(localToDevice, srcToDst)) {
            sampling = SkSamplingOptions(sampling.filter);
        }

        SkIRect clipRect = device->devClipBounds();

        int tileFilterPad;
        if (sampling.useCubic) {
            tileFilterPad = kBicubicFilterTexelPad;
        } else if (sampling.filter == SkFilterMode::kLinear || sampling.isAniso()) {
            // Aniso will fallback to linear filtering in the tiling case.
            tileFilterPad = 1;
        } else {
            tileFilterPad = 0;
        }

        int maxTileSize = get_max_texture_size(canvas) - 2*tileFilterPad;
#if defined(GR_TEST_UTILS)
        if (gOverrideMaxTextureSize) {
            maxTileSize = gOverrideMaxTextureSize - 2 * tileFilterPad;
        }
#endif

        size_t cacheSize = get_cache_size(device);

        int tileSize;
        SkIRect clippedSubset;
        if (ShouldTileImage(clipRect,
                            image->dimensions(),
                            localToDevice,
                            srcToDst,
                            &src,
                            maxTileSize,
                            cacheSize,
                            &tileSize,
                            &clippedSubset)) {
            // Extract pixels on the CPU, since we have to split into separate textures before
            // sending to the GPU if tiling.
            if (SkBitmap bm; as_IB(image)->getROPixels(nullptr, &bm)) {
                draw_tiled_bitmap(canvas,
                                  bm,
                                  tileSize,
                                  srcToDst,
                                  src,
                                  clippedSubset,
                                  paint,
                                  aaFlags,
                                  constraint,
                                  sampling);
                return true;
            }
        }
    }

    return false;
}

} // namespace skgpu
