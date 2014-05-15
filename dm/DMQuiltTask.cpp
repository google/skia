#include "DMQuiltTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"

DEFINE_bool(quilt, true, "If true, draw into a quilt of small tiles and compare.");
DEFINE_int32(quiltTile, 16, "Dimension of (square) quilt tile.");

namespace DM {

QuiltTask::QuiltTask(const Task& parent, skiagm::GM* gm, SkBitmap reference)
    : CpuTask(parent)
    , fName(UnderJoin(parent.name().c_str(), "quilt"))
    , fGM(gm)
    , fReference(reference)
    {}

static int tiles_needed(int fullDimension, int tileDimension) {
    return (fullDimension + tileDimension - 1) / tileDimension;
}

void QuiltTask::draw() {
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get()));

    SkBitmap full;
    AllocatePixels(fReference, &full);
    SkCanvas fullCanvas(full);

    SkBitmap tile;
    tile.allocPixels(SkImageInfo::Make(FLAGS_quiltTile, FLAGS_quiltTile,
                                       fReference.colorType(), kPremul_SkAlphaType));
    SkCanvas tileCanvas(tile);

    for (int y = 0; y < tiles_needed(full.height(), tile.height()); y++) {
        for (int x = 0; x < tiles_needed(full.width(), tile.width()); x++) {
            SkAutoCanvasRestore ar(&tileCanvas, true/*also save now*/);

            const SkScalar xOffset = SkIntToScalar(x * tile.width()),
                           yOffset = SkIntToScalar(y * tile.height());
            SkMatrix matrix = tileCanvas.getTotalMatrix();
            matrix.postTranslate(-xOffset, -yOffset);
            tileCanvas.setMatrix(matrix);

            recorded->draw(&tileCanvas);
            tileCanvas.flush();
            fullCanvas.drawBitmap(tile, xOffset, yOffset, NULL);
        }
    }

    if (!BitmapsEqual(full, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, full)));
    }
}

bool QuiltTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }
    if (fGM->getFlags() & skiagm::GM::kSkipTiled_Flag) {
        return true;
    }
    return !FLAGS_quilt;
}

}  // namespace DM
