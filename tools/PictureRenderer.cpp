/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "PictureRenderer.h"
#include "SamplePipeControllers.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkImageEncoder.h"
#include "SkGPipe.h"
#include "SkMatrix.h"
#include "SkPicture.h"
#include "SkScalar.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTypes.h"
#include "picture_utils.h"

#if SK_SUPPORT_GPU
#include "SkGpuDevice.h"
#endif

namespace sk_tools {

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

void PictureRenderer::init(SkPicture* pict) {
    SkASSERT(NULL == fPicture);
    SkASSERT(NULL == fCanvas.get());
    if (fPicture != NULL || NULL != fCanvas.get()) {
        return;
    }

    SkASSERT(pict != NULL);
    if (NULL == pict) {
        return;
    }

    fPicture = pict;
    fCanvas.reset(this->setupCanvas());
}

SkCanvas* PictureRenderer::setupCanvas() {
    return this->setupCanvas(fPicture->width(), fPicture->height());
}

SkCanvas* PictureRenderer::setupCanvas(int width, int height) {
    switch(fDeviceType) {
        case kBitmap_DeviceType: {
            SkBitmap bitmap;
            sk_tools::setup_bitmap(&bitmap, width, height);
            return SkNEW_ARGS(SkCanvas, (bitmap));
            break;
        }
#if SK_SUPPORT_GPU
        case kGPU_DeviceType: {
            SkAutoTUnref<SkGpuDevice> device(SkNEW_ARGS(SkGpuDevice,
                                                    (fGrContext, SkBitmap::kARGB_8888_Config,
                                                    width, height)));
            return SkNEW_ARGS(SkCanvas, (device.get()));
            break;
        }
#endif
        default:
            SkASSERT(0);
    }

    return NULL;
}

void PictureRenderer::end() {
    this->resetState();
    fPicture = NULL;
    fCanvas.reset(NULL);
}

void PictureRenderer::resetState() {
#if SK_SUPPORT_GPU
    if (this->isUsingGpuDevice()) {
        SkGLContext* glContext = fGrContextFactory.getGLContext(
            GrContextFactory::kNative_GLContextType);
        SK_GL(*glContext, Finish());
    }
#endif
}

void PictureRenderer::finishDraw() {
    SkASSERT(fCanvas.get() != NULL);
    if (NULL == fCanvas.get()) {
        return;
    }

    fCanvas->flush();

#if SK_SUPPORT_GPU
    if (this->isUsingGpuDevice()) {
        SkGLContext* glContext = fGrContextFactory.getGLContext(
            GrContextFactory::kNative_GLContextType);

        SkASSERT(glContext != NULL);
        if (NULL == glContext) {
            return;
        }

        SK_GL(*glContext, Finish());
    }
#endif
}

bool PictureRenderer::write(const SkString& path) const {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return false;
    }

    SkBitmap bitmap;
    sk_tools::setup_bitmap(&bitmap, fPicture->width(), fPicture->height());

    fCanvas->readPixels(&bitmap, 0, 0);
    sk_tools::force_all_opaque(bitmap);

    return SkImageEncoder::EncodeFile(path.c_str(), bitmap, SkImageEncoder::kPNG_Type, 100);
}

void PipePictureRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    PipeController pipeController(fCanvas.get());
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    pipeCanvas->drawPicture(*fPicture);
    writer.endRecording();
    this->finishDraw();
}

void SimplePictureRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    fCanvas->drawPicture(*fPicture);
    this->finishDraw();
}

TiledPictureRenderer::TiledPictureRenderer()
    : fTileWidth(kDefaultTileWidth)
    , fTileHeight(kDefaultTileHeight)
    , fTileMinPowerOf2Width(0)
    , fTileHeightPercentage(0.0)
    , fTileWidthPercentage(0.0) {}

void TiledPictureRenderer::init(SkPicture* pict) {
    SkASSERT(pict != NULL);
    SkASSERT(0 == fTiles.count());
    if (NULL == pict || fTiles.count() != 0) {
        return;
    }

    this->INHERITED::init(pict);

    if (fTileWidthPercentage > 0) {
        fTileWidth = sk_float_ceil2int(float(fTileWidthPercentage * fPicture->width() / 100));
    }
    if (fTileHeightPercentage > 0) {
        fTileHeight = sk_float_ceil2int(float(fTileHeightPercentage * fPicture->height() / 100));
    }

    if (fTileMinPowerOf2Width > 0) {
        this->setupPowerOf2Tiles();
    } else {
        this->setupTiles();
    }
}

void TiledPictureRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    this->drawTiles();
    this->copyTilesToCanvas();
    this->finishDraw();
}

void TiledPictureRenderer::end() {
    this->deleteTiles();
    this->INHERITED::end();
}

TiledPictureRenderer::~TiledPictureRenderer() {
    this->deleteTiles();
}

void TiledPictureRenderer::clipTile(SkCanvas* tile) {
    SkRect clip = SkRect::MakeWH(SkIntToScalar(fPicture->width()),
                                 SkIntToScalar(fPicture->height()));
    tile->clipRect(clip);
}

void TiledPictureRenderer::addTile(int tile_x_start, int tile_y_start, int width, int height) {
    SkCanvas* tile = this->setupCanvas(width, height);

    tile->translate(SkIntToScalar(-tile_x_start), SkIntToScalar(-tile_y_start));
    this->clipTile(tile);

    fTiles.push(tile);
}

void TiledPictureRenderer::setupTiles() {
    for (int tile_y_start = 0; tile_y_start < fPicture->height();
         tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < fPicture->width();
             tile_x_start += fTileWidth) {
            this->addTile(tile_x_start, tile_y_start, fTileWidth, fTileHeight);
        }
    }
}

// The goal of the powers of two tiles is to minimize the amount of wasted tile
// space in the width-wise direction and then minimize the number of tiles. The
// constraints are that every tile must have a pixel width that is a power of
// two and also be of some minimal width (that is also a power of two).
//
// This is sovled by first taking our picture size and rounding it up to the
// multiple of the minimal width. The binary representation of this rounded
// value gives us the tiles we need: a bit of value one means we need a tile of
// that size.
void TiledPictureRenderer::setupPowerOf2Tiles() {
    int rounded_value = fPicture->width();
    if (fPicture->width() % fTileMinPowerOf2Width != 0) {
        rounded_value = fPicture->width() - (fPicture->width() % fTileMinPowerOf2Width)
            + fTileMinPowerOf2Width;
    }

    int num_bits = SkScalarCeilToInt(SkScalarLog2(fPicture->width()));
    int largest_possible_tile_size = 1 << num_bits;

    // The tile height is constant for a particular picture.
    for (int tile_y_start = 0; tile_y_start < fPicture->height(); tile_y_start += fTileHeight) {
        int tile_x_start = 0;
        int current_width = largest_possible_tile_size;

        while (current_width >= fTileMinPowerOf2Width) {
            // It is very important this is a bitwise AND.
            if (current_width & rounded_value) {
                this->addTile(tile_x_start, tile_y_start, current_width, fTileHeight);
                tile_x_start += current_width;
            }

            current_width >>= 1;
        }
    }
}

void TiledPictureRenderer::deleteTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        SkDELETE(fTiles[i]);
    }

    fTiles.reset();
}

void TiledPictureRenderer::drawTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i]->drawPicture(*(fPicture));
    }
}

void TiledPictureRenderer::finishDraw() {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i]->flush();
    }

#if SK_SUPPORT_GPU
    if (this->isUsingGpuDevice()) {
        SkGLContext* glContext = fGrContextFactory.getGLContext(
            GrContextFactory::kNative_GLContextType);

        SkASSERT(glContext != NULL);
        if (NULL == glContext) {
            return;
        }

        SK_GL(*glContext, Finish());
    }
#endif
}

void TiledPictureRenderer::copyTilesToCanvas() {
    for (int i = 0; i < fTiles.count(); ++i) {
        // Since SkPicture performs a save and restore when being drawn to a
        // canvas, we can be confident that the transform matrix of the canvas
        // is what we set when creating the tiles.
        SkMatrix matrix = fTiles[i]->getTotalMatrix();
        SkScalar tile_x_start = matrix.getTranslateX();
        SkScalar tile_y_start = matrix.getTranslateY();

        SkBitmap source = fTiles[i]->getDevice()->accessBitmap(false);

        fCanvas->drawBitmap(source, -tile_x_start, -tile_y_start);
    }
}

}
