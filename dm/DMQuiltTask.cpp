#include "DMQuiltTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkThreadPool.h"

DEFINE_bool(quilt, true, "If true, draw into a quilt of small tiles and compare.");
DEFINE_int32(quiltTile, 16, "Dimension of (square) quilt tile.");
DEFINE_bool(quiltThreaded, false, "If true, draw quilt tiles with multiple threads.");

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

class Tile : public SkRunnable {
public:
    Tile(int x, int y, SkColorType colorType,
         const SkPicture& picture, SkCanvas* canvas, SkMutex* mutex)
        : fX(x)
        , fY(y)
        , fColorType(colorType)
        , fPicture(picture)
        , fCanvas(canvas)
        , fMutex(mutex) {}

    virtual void run() SK_OVERRIDE {
        SkBitmap tile;
        tile.allocPixels(SkImageInfo::Make(FLAGS_quiltTile, FLAGS_quiltTile,
                                           fColorType, kPremul_SkAlphaType));
        SkCanvas tileCanvas(tile);

        const SkScalar xOffset = SkIntToScalar(fX * tile.width()),
                       yOffset = SkIntToScalar(fY * tile.height());
        tileCanvas.translate(-xOffset, -yOffset);
        fPicture.draw(&tileCanvas);
        tileCanvas.flush();

        {
            SkAutoMutexAcquire lock(fMutex);
            fCanvas->drawBitmap(tile, xOffset, yOffset, NULL);
        }

        delete this;
    }

private:
    const int fX, fY;
    const SkColorType fColorType;
    const SkPicture& fPicture;
    SkCanvas* fCanvas;
    SkMutex* fMutex;  // Guards fCanvas.
};

void QuiltTask::draw() {
    SkAutoTUnref<SkPicture> recorded(RecordPicture(fGM.get()));

    SkBitmap full;
    AllocatePixels(fReference, &full);
    SkCanvas fullCanvas(full);
    SkMutex mutex;  // Guards fullCanvas.

    SkThreadPool pool(FLAGS_quiltThreaded ? SkThreadPool::kThreadPerCore : 0);

    for (int y = 0; y < tiles_needed(full.height(), FLAGS_quiltTile); y++) {
        for (int x = 0; x < tiles_needed(full.width(), FLAGS_quiltTile); x++) {
            // Deletes itself when done.
            pool.add(new Tile(x, y, fReference.colorType(), *recorded, &fullCanvas, &mutex));
        }
    }

    pool.wait();
    fullCanvas.flush();

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
