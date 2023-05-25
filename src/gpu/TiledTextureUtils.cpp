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
#include "src/core/SkImagePriv.h"
#include "src/core/SkSamplingPriv.h"

#if defined(SK_GANESH)
#include "include/gpu/GrDirectContext.h"
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

// This method outsets 'iRect' by 'outset' all around and then clamps its extents to
// 'clamp'. 'offset' is adjusted to remain positioned over the top-left corner
// of 'iRect' for all possible outsets/clamps.
void clamped_outset_with_offset(SkIRect* iRect, int outset, SkPoint* offset,
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

} // anonymous namespace

namespace skgpu {

// tileSize and clippedSubset are valid if true is returned
bool ShouldTileImage(GrRecordingContext* context,
                     SkIRect conservativeClipBounds,
                     uint32_t /* imageID */,
                     const SkISize& imageSize,
                     const SkMatrix& ctm,
                     const SkMatrix& srcToDst,
                     const SkRect* src,
                     int maxTileSize,
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

#if defined(SK_GANESH)
    // At this point we know we could do the draw by uploading the entire bitmap as a texture.
    // However, if the texture would be large compared to the cache size and we don't require most
    // of it for this draw then tile to reduce the amount of upload and cache spill.
    // NOTE: if the context is not a direct context, it doesn't have access to the resource cache,
    // and theoretically, the resource cache's limits could be being changed on another thread, so
    // even having access to just the limit wouldn't be a reliable test during recording here.
    // Instead, we will just upload the entire image to be on the safe side and not tile.
    auto direct = context->asDirectContext();
    if (!direct) {
        return false;
    }

    // assumption here is that sw bitmap size is a good proxy for its size as
    // a texture
    size_t bmpSize = area * sizeof(SkPMColor);  // assume 32bit pixels
    size_t cacheSize = direct->getResourceCacheLimit();
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
#else
    return false;
#endif
}

void DrawTiledBitmap(GrRecordingContext* rContext,
                     skgpu::ganesh::SurfaceDrawContext* sdc,
                     const GrClip* clip,
                     const SkBitmap& bitmap,
                     int tileSize,
                     const SkMatrixProvider& matrixProvider,
                     const SkMatrix& srcToDst,
                     const SkRect& srcRect,
                     const SkIRect& clippedSrcIRect,
                     const SkPaint& paint,
                     SkCanvas::QuadAAFlags origAAFlags,
                     SkCanvas::SrcRectConstraint constraint,
                     SkSamplingOptions sampling,
                     SkTileMode tileMode,
                     DrawImageProc drawImage) {
    if (sampling.isAniso()) {
        sampling = SkSamplingPriv::AnisoFallback(/*imageIsMipped=*/false);
    }
    SkRect clippedSrcRect = SkRect::Make(clippedSrcIRect);

    int nx = bitmap.width() / tileSize;
    int ny = bitmap.height() / tileSize;

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
            srcToDst.mapRect(&rectToDraw);
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
                clamped_outset_with_offset(&iTileR, outset, &offset, iClampRect);
            }

            // We must subset as a bitmap and then turn into an SkImage if we want caching to work.
            // Image subsets always make a copy of the pixels and lose the association with the
            // original's SkPixelRef.
            if (SkBitmap subsetBmp; bitmap.extractSubset(&subsetBmp, iTileR)) {
                auto image = SkMakeImageFromRasterBitmap(subsetBmp, kNever_SkCopyPixelsMode);

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

                // now offset it to make it "local" to our tmp bitmap
                tileR.offset(-offset.fX, -offset.fY);
                SkMatrix offsetSrcToDst = srcToDst;
                offsetSrcToDst.preTranslate(offset.fX, offset.fY);
                drawImage(rContext,
                          sdc,
                          clip,
                          matrixProvider,
                          paint,
                          image.get(),
                          tileR,
                          rectToDraw,
                          nullptr,
                          offsetSrcToDst,
                          static_cast<SkCanvas::QuadAAFlags>(aaFlags),
                          constraint,
                          sampling,
                          tileMode);
            }
        }
    }
}

} // namespace skgpu
