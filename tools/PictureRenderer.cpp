#include "PictureRenderer.h"
#include "SamplePipeControllers.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#include "SkPicture.h"
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
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    fCanvas->flush();

    if (this->isUsingGpuDevice()) {
        SkGLContext* glContext = fGrContextFactory.getGLContext(
            GrContextFactory::kNative_GLContextType);
        SK_GL(*glContext, Finish());
        fGrContext->freeGpuResources();
    }
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
}

void SimplePictureRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    fCanvas->drawPicture(*fPicture);
}

TiledPictureRenderer::TiledPictureRenderer()
    : fTileWidth(kDefaultTileWidth)
    , fTileHeight(kDefaultTileHeight) {}

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

    this->setupTiles();
}

void TiledPictureRenderer::render() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    this->drawTiles();
    this->copyTilesToCanvas();
}

void TiledPictureRenderer::end() {
    this->deleteTiles();
    this->INHERITED::end();
}

TiledPictureRenderer::~TiledPictureRenderer() {
    this->deleteTiles();
}

void TiledPictureRenderer::clipTile(const TileInfo& tile) {
    SkRect clip = SkRect::MakeWH(SkIntToScalar(fPicture->width()),
                                 SkIntToScalar(fPicture->height()));
    tile.fCanvas->clipRect(clip);
}

void TiledPictureRenderer::addTile(int tile_x_start, int tile_y_start) {
    TileInfo* tile = fTiles.push();

    tile->fCanvas = this->setupCanvas(fTileWidth, fTileHeight);
    tile->fCanvas->translate(SkIntToScalar(-tile_x_start), SkIntToScalar(-tile_y_start));
    this->clipTile(*tile);
}

void TiledPictureRenderer::setupTiles() {
    for (int tile_y_start = 0; tile_y_start < fPicture->height();
         tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < fPicture->width();
             tile_x_start += fTileWidth) {
            this->addTile(tile_x_start, tile_y_start);
        }
    }
}

void TiledPictureRenderer::deleteTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        SkDELETE(fTiles[i].fCanvas);
    }

    fTiles.reset();
}

void TiledPictureRenderer::drawTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].fCanvas->drawPicture(*(fPicture));
    }
}

void TiledPictureRenderer::resetState() {
    SkASSERT(fCanvas.get() != NULL);
    SkASSERT(fPicture != NULL);
    if (NULL == fCanvas.get() || NULL == fPicture) {
        return;
    }

    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].fCanvas->flush();
    }

    this->INHERITED::resetState();
}

void TiledPictureRenderer::copyTilesToCanvas() {
    int tile_index = 0;
    for (int tile_y_start = 0; tile_y_start < fPicture->height();
         tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < fPicture->width();
             tile_x_start += fTileWidth) {
            SkASSERT(tile_index < fTiles.count());
            SkBitmap source = fTiles[tile_index].fCanvas->getDevice()->accessBitmap(false);
            fCanvas->drawBitmap(source,
                                SkIntToScalar(tile_x_start),
                                SkIntToScalar(tile_y_start));
            ++tile_index;
        }
    }
}

}
