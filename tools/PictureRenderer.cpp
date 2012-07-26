#include "PictureRenderer.h"
#include "SamplePipeControllers.h"
#include "SkCanvas.h"
#include "SkDevice.h"
#include "SkGPipe.h"
#include "SkPicture.h"
#include "SkTDArray.h"
#include "SkTypes.h"
#include "picture_utils.h"

namespace sk_tools {

enum {
    kDefaultTileWidth = 256,
    kDefaultTileHeight = 256
};

void PipePictureRenderer::render(SkPicture* pict, SkCanvas* canvas) {
    PipeController pipeController(canvas);
    SkGPipeWriter writer;
    SkCanvas* pipeCanvas = writer.startRecording(&pipeController);
    pipeCanvas->drawPicture(*pict);
    writer.endRecording();
}

void SimplePictureRenderer::render(SkPicture* pict, SkCanvas* canvas) {
    canvas->drawPicture(*pict);
}

TiledPictureRenderer::TiledPictureRenderer()
    : fTileWidth(kDefaultTileWidth)
    , fTileHeight(kDefaultTileHeight) {}

void TiledPictureRenderer::init(const SkPicture& pict) {
    deleteTiles();
    setupTiles(pict);
}

void TiledPictureRenderer::render(SkPicture* pict, SkCanvas* canvas) {
    for (int i = 0; i < fTiles.count(); ++i) {
        fTiles[i].fCanvas->drawPicture(*pict);
    }

    copyTilesToCanvas(*pict, canvas);
}

TiledPictureRenderer::~TiledPictureRenderer() {
    deleteTiles();
}

void TiledPictureRenderer::clipTile(const SkPicture& picture, const TileInfo& tile) {
    SkRect clip = SkRect::MakeWH(SkIntToScalar(picture.width()),
                                 SkIntToScalar(picture.height()));
    tile.fCanvas->clipRect(clip);
}

void TiledPictureRenderer::addTile(const SkPicture& picture, int tile_x_start, int tile_y_start) {
    TileInfo* tile = fTiles.push();

    tile->fBitmap = SkNEW(SkBitmap);
    sk_tools::setup_bitmap(tile->fBitmap, fTileWidth, fTileHeight);

    tile->fCanvas = SkNEW_ARGS(SkCanvas, (*(tile->fBitmap)));
    tile->fCanvas->translate(SkIntToScalar(-tile_x_start), SkIntToScalar(-tile_y_start));
    clipTile(picture, *tile);
}

void TiledPictureRenderer::setupTiles(const SkPicture& picture) {
    for (int tile_y_start = 0; tile_y_start < picture.height();
         tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < picture.width();
             tile_x_start += fTileWidth) {
            addTile(picture, tile_x_start, tile_y_start);
        }
    }
}

void TiledPictureRenderer::deleteTiles() {
    for (int i = 0; i < fTiles.count(); ++i) {
        SkDELETE(fTiles[i].fCanvas);
        SkDELETE(fTiles[i].fBitmap);
    }

    fTiles.reset();
}

void TiledPictureRenderer::copyTilesToCanvas(const SkPicture& pict, SkCanvas* destination) {
    int tile_index = 0;
    for (int tile_y_start = 0; tile_y_start < pict.height();
         tile_y_start += fTileHeight) {
        for (int tile_x_start = 0; tile_x_start < pict.width();
             tile_x_start += fTileWidth) {
            SkASSERT(tile_index < fTiles.count());
            SkBitmap source = fTiles[tile_index].fCanvas->getDevice()->accessBitmap(false);
            destination->drawBitmap(source, 
                                    SkIntToScalar(tile_x_start), 
                                    SkIntToScalar(tile_y_start));
            ++tile_index;
        }
    }
}

}
