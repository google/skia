/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "picture_utils.h"
#include "CopyTilesRenderer.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkMultiPictureDraw.h"
#include "SkPicture.h"
#include "SkPixelRef.h"
#include "SkRect.h"
#include "SkString.h"

namespace sk_tools {
#if SK_SUPPORT_GPU
    CopyTilesRenderer::CopyTilesRenderer(const GrContextOptions& opts, int x, int y)
    : INHERITED(opts)
    , fXTilesPerLargeTile(x)
    , fYTilesPerLargeTile(y) { }
#else
    CopyTilesRenderer::CopyTilesRenderer(int x, int y)
    : fXTilesPerLargeTile(x)
    , fYTilesPerLargeTile(y) { }
#endif
    void CopyTilesRenderer::init(const SkPicture* pict, const SkString* writePath,
                                 const SkString* mismatchPath, const SkString* inputFilename,
                                 bool useChecksumBasedFilenames, bool useMultiPictureDraw) {
        // Do not call INHERITED::init(), which would create a (potentially large) canvas which is
        // not used by bench_pictures.
        SkASSERT(pict != NULL);
        // Only work with absolute widths (as opposed to percentages).
        SkASSERT(this->getTileWidth() != 0 && this->getTileHeight() != 0);
        fPicture.reset(pict)->ref();
        this->CopyString(&fWritePath, writePath);
        this->CopyString(&fMismatchPath, mismatchPath);
        this->CopyString(&fInputFilename, inputFilename);
        fUseChecksumBasedFilenames = useChecksumBasedFilenames;
        fUseMultiPictureDraw = useMultiPictureDraw;
        this->buildBBoxHierarchy();
        // In order to avoid allocating a large canvas (particularly important for GPU), create one
        // canvas that is a multiple of the tile size, and draw portions of the picture.
        fLargeTileWidth = fXTilesPerLargeTile * this->getTileWidth();
        fLargeTileHeight = fYTilesPerLargeTile * this->getTileHeight();
        fCanvas.reset(this->INHERITED::setupCanvas(fLargeTileWidth, fLargeTileHeight));
    }

    bool CopyTilesRenderer::render(SkBitmap** out) {
        int i = 0;
        bool success = true;
        SkBitmap dst;
        for (int x = 0; x < this->getViewWidth(); x += fLargeTileWidth) {
            for (int y = 0; y < this->getViewHeight(); y += fLargeTileHeight) {
                SkAutoCanvasRestore autoRestore(fCanvas, true);
                // Translate so that we draw the correct portion of the picture.
                // Perform a postTranslate so that the scaleFactor does not interfere with the
                // positioning.
                SkMatrix mat(fCanvas->getTotalMatrix());
                mat.postTranslate(SkIntToScalar(-x), SkIntToScalar(-y));
                fCanvas->setMatrix(mat);
                // Draw the picture
                if (fUseMultiPictureDraw) {
                    SkMultiPictureDraw mpd;

                    mpd.add(fCanvas, fPicture);

                    mpd.draw();
                } else {
                    fCanvas->drawPicture(fPicture);
                }
                // Now extract the picture into tiles
                SkBitmap baseBitmap;
                fCanvas->readPixels(SkIRect::MakeSize(fCanvas->getBaseLayerSize()), &baseBitmap);
                SkIRect subset;
                for (int tileY = 0; tileY < fLargeTileHeight; tileY += this->getTileHeight()) {
                    for (int tileX = 0; tileX < fLargeTileWidth; tileX += this->getTileWidth()) {
                        subset.set(tileX, tileY, tileX + this->getTileWidth(),
                                   tileY + this->getTileHeight());
                        SkDEBUGCODE(bool extracted =)
                        baseBitmap.extractSubset(&dst, subset);
                        SkASSERT(extracted);
                        if (!fWritePath.isEmpty()) {
                            // Similar to write() in PictureRenderer.cpp, but just encodes
                            // a bitmap directly.
                            // TODO: Share more common code with write() to do this, to properly
                            // write out the JSON summary, etc.
                            SkString pathWithNumber = SkOSPath::Join(fWritePath.c_str(),
                                                                     fInputFilename.c_str());
                            pathWithNumber.remove(pathWithNumber.size() - 4, 4);
                            pathWithNumber.appendf("%i.png", i++);
                            SkBitmap copy;
#if SK_SUPPORT_GPU
                            if (isUsingGpuDevice()) {
                                dst.pixelRef()->readPixels(&copy, &subset);
                            } else {
#endif
                                dst.copyTo(&copy);
#if SK_SUPPORT_GPU
                            }
#endif
                            success &= SkImageEncoder::EncodeFile(pathWithNumber.c_str(), copy,
                                                                  SkImageEncoder::kPNG_Type, 100);
                        }
                    }
                }
            }
        }
        return success;
    }

    SkString CopyTilesRenderer::getConfigNameInternal() {
        return SkString("copy_tiles");
    }
}
