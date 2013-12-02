#include "DMTileGridTask.h"
#include "DMWriteTask.h"
#include "DMUtil.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkTileGridPicture.h"

// TODO(mtklein): Tile grid tests are currently failing.  (Skia issue 1198).  When fixed, -> true.
DEFINE_bool(tileGrid, false, "If true, run picture replay tests with a tile grid.");

namespace DM {

TileGridTask::TileGridTask(const Task& parent, skiagm::GM* gm, SkBitmap reference, SkISize tileSize)
    : Task(parent)
    , fName(UnderJoin(parent.name().c_str(), "tilegrid"))
    , fGM(gm)
    , fReference(reference)
    , fTileSize(tileSize)
    {}

static int tiles_needed(int fullDimension, int tileDimension) {
    return (fullDimension + tileDimension - 1) / tileDimension;
}

void TileGridTask::draw() {
    const SkTileGridPicture::TileGridInfo info = {
        fTileSize,
        SkISize::Make(0,0),   // Overlap between adjacent tiles.
        SkIPoint::Make(0,0),  // Offset.
    };
    const SkISize size = fGM->getISize();
    SkTileGridPicture recorded(size.width(), size.height(), info);
    RecordPicture(fGM.get(), &recorded, SkPicture::kUsePathBoundsForClip_RecordingFlag);

    SkBitmap full;
    SetupBitmap(fReference.config(), fGM.get(), &full);
    SkCanvas fullCanvas(full);

    SkBitmap tile;
    tile.setConfig(fReference.config(), fTileSize.width(), fTileSize.height());
    tile.allocPixels();
    SkCanvas tileCanvas(tile);

    SkPaint paint;
    paint.setXfermodeMode(SkXfermode::kSrc_Mode);

    for (int y = 0; y < tiles_needed(full.height(), tile.height()); y++) {
        for (int x = 0; x < tiles_needed(full.width(), tile.width()); x++) {
            SkAutoCanvasRestore ar(&tileCanvas, true/*also save now*/);

            const SkScalar xOffset = SkIntToScalar(x * tile.width()),
                           yOffset = SkIntToScalar(y * tile.height());
            SkMatrix matrix = tileCanvas.getTotalMatrix();
            matrix.postTranslate(-xOffset, -yOffset);
            tileCanvas.setMatrix(matrix);

            recorded.draw(&tileCanvas);
            tileCanvas.flush();
            fullCanvas.drawBitmap(tile, xOffset, yOffset, &paint);
        }
    }

    if (!BitmapsEqual(full, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, full)));
    }
}

bool TileGridTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }
    if (fGM->getFlags() & skiagm::GM::kSkipTiled_Flag) {
        return true;
    }
    return !FLAGS_tileGrid;
}

}  // namespace DM
