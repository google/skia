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
#include "src/image/SkImage_Picture.h"

#include <functional>

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

int draw_tiled_image(SkCanvas* canvas,
                     std::function<sk_sp<SkImage>(SkIRect)> imageProc,
                     SkISize originalSize,
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

    int nx = originalSize.width() / tileSize;
    int ny = originalSize.height() / tileSize;

    int numTilesDrawn = 0;

    skia_private::TArray<SkCanvas::ImageSetEntry> imgSet(nx * ny);

    for (int x = 0; x <= nx; x++) {
        for (int y = 0; y <= ny; y++) {
            SkRect tileR;
            // TODO: this will prevent int overflow, however at sizes > 2^24 the float can't
            // represent all the bits in the int
            int tileRight = (x == nx) ? originalSize.width() : (x + 1) * tileSize;
            int tileBottom = (y == ny) ? originalSize.height() : (y + 1) * tileSize;
            tileR.setLTRB(SkIntToScalar(x * tileSize), SkIntToScalar(y * tileSize),
                          SkIntToScalar(tileRight),    SkIntToScalar(tileBottom));

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
                    iClampRect = SkIRect::MakeWH(originalSize.width(), originalSize.height());
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

            sk_sp<SkImage> image = imageProc(iTileR);
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

            numTilesDrawn += 1;
        }
    }

    canvas->experimental_DrawEdgeAAImageSet(imgSet.data(),
                                            imgSet.size(),
                                            /* dstClips= */ nullptr,
                                            /* preViewMatrices= */ nullptr,
                                            sampling,
                                            paint,
                                            constraint);
    return numTilesDrawn;
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

bool TiledTextureUtils::CanDisableMipmap(const SkMatrix& viewM,
                                         const SkMatrix& localM,
                                         bool sharpenMipmappedTextures) {
    SkMatrix matrix;
    matrix.setConcat(viewM, localM);
    // With sharp mips, we bias mipmap lookups by -0.5. That means our final LOD is >= 0 until
    // the computed LOD is >= 0.5. At what scale factor does a texture get an LOD of
    // 0.5?
    //
    // Want:  0       = log2(1/s) - 0.5
    //        0.5     = log2(1/s)
    //        2^0.5   = 1/s
    //        1/2^0.5 = s
    //        2^0.5/2 = s
    SkScalar mipScale = sharpenMipmappedTextures ? SK_ScalarRoot2Over2 : SK_Scalar1;
    return matrix.getMinScale() >= mipScale;
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

std::tuple<bool, size_t> TiledTextureUtils::DrawAsTiledImageRect(
        SkCanvas* canvas,
        const SkImage* image,
        const SkRect& srcRect,
        const SkRect& dstRect,
        SkCanvas::QuadAAFlags aaFlags,
        const SkSamplingOptions& origSampling,
        const SkPaint* paint,
        SkCanvas::SrcRectConstraint constraint,
        bool sharpenMM,
        size_t cacheSize,
        size_t maxTextureSize) {
    if (canvas->isClipEmpty()) {
        return {true, 0};
    }

    if (!image->isTextureBacked()) {
        SkRect src;
        SkRect dst;
        SkMatrix srcToDst;
        ImageDrawMode mode = OptimizeSampleArea(SkISize::Make(image->width(), image->height()),
                                                srcRect, dstRect, /* dstClip= */ nullptr,
                                                &src, &dst, &srcToDst);
        if (mode == ImageDrawMode::kSkip) {
            return {true, 0};
        }

        SkASSERT(mode != ImageDrawMode::kDecal); // only happens if there is a 'dstClip'

        if (src.contains(image->bounds())) {
            constraint = SkCanvas::kFast_SrcRectConstraint;
        }

        SkDevice* device = SkCanvasPriv::TopDevice(canvas);
        const SkMatrix& localToDevice = device->localToDevice();

        SkSamplingOptions sampling = origSampling;
        if (sampling.mipmap != SkMipmapMode::kNone &&
            CanDisableMipmap(localToDevice, srcToDst, sharpenMM)) {
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

        int maxTileSize = maxTextureSize - 2 * tileFilterPad;
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
            // If it's a Picture-backed image we should subset the SkPicture directly rather than
            // converting to a Bitmap and then subsetting. Rendering to a bitmap will use a Raster
            // surface, and the SkPicture could have GPU data.
            if (as_IB(image)->type() == SkImage_Base::Type::kLazyPicture) {
                auto imageProc = [&](SkIRect iTileR) {
                    return image->makeSubset(nullptr, iTileR);
                };

                size_t tiles = draw_tiled_image(canvas,
                                                imageProc,
                                                image->dimensions(),
                                                tileSize,
                                                srcToDst,
                                                src,
                                                clippedSubset,
                                                paint,
                                                aaFlags,
                                                constraint,
                                                sampling);
                return {true, tiles};
            }

            // Extract pixels on the CPU, since we have to split into separate textures before
            // sending to the GPU if tiling.
            if (SkBitmap bm; as_IB(image)->getROPixels(nullptr, &bm)) {
                auto imageProc = [&](SkIRect iTileR) {
                    // We must subset as a bitmap and then turn it into an SkImage if we want
                    // caching to work. Image subsets always make a copy of the pixels and lose
                    // the association with the original's SkPixelRef.
                    if (SkBitmap subsetBmp; bm.extractSubset(&subsetBmp, iTileR)) {
                        return SkMakeImageFromRasterBitmap(subsetBmp, kNever_SkCopyPixelsMode);
                    }
                    return sk_sp<SkImage>(nullptr);
                };

                size_t tiles = draw_tiled_image(canvas,
                                                imageProc,
                                                bm.dimensions(),
                                                tileSize,
                                                srcToDst,
                                                src,
                                                clippedSubset,
                                                paint,
                                                aaFlags,
                                                constraint,
                                                sampling);
                return {true, tiles};
            }
        }
    }

    return {false, 0};
}

} // namespace skgpu
