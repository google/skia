#include "DMQuiltTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkThreadPool.h"

DEFINE_bool(quilt, true, "If true, draw into a quilt of small tiles and compare.");
DEFINE_int32(quiltTile, 256, "Dimension of (square) quilt tile.");
DEFINE_bool(quiltThreaded, false, "If true, draw quilt tiles with multiple threads.");

static const char* kSuffixes[] = { "quilt", "quilt_skr" };

namespace DM {

QuiltTask::QuiltTask(const Task& parent, skiagm::GM* gm, SkBitmap reference, QuiltTask::Mode mode)
    : CpuTask(parent)
    , fMode(mode)
    , fName(UnderJoin(parent.name().c_str(), kSuffixes[mode]))
    , fGM(gm)
    , fReference(reference)
    {}

static int tiles_needed(int fullDimension, int tileDimension) {
    return (fullDimension + tileDimension - 1) / tileDimension;
}

class Tile : public SkRunnable {
public:
    Tile(int x, int y, const SkPicture& picture, SkBitmap* quilt)
        : fX(x * FLAGS_quiltTile)
        , fY(y * FLAGS_quiltTile)
        , fPicture(picture)
        , fQuilt(quilt) {}

    virtual void run() SK_OVERRIDE {
        SkBitmap tile;
        fQuilt->extractSubset(&tile, SkIRect::MakeXYWH(fX, fY, FLAGS_quiltTile, FLAGS_quiltTile));
        SkCanvas tileCanvas(tile);

        tileCanvas.translate(SkIntToScalar(-fX), SkIntToScalar(-fY));
        fPicture.draw(&tileCanvas);
        tileCanvas.flush();

        delete this;
    }

private:
    const int fX, fY;
    const SkPicture& fPicture;
    SkBitmap* fQuilt;
};

void QuiltTask::draw() {
    SkAutoTUnref<SkPicture> recorded(
            RecordPicture(fGM.get(), NULL/*bbh factory*/, kSkRecord_Mode == fMode));

    SkBitmap full;
    AllocatePixels(fReference, &full);

    int threads = 0;
    if (kSkRecord_Mode == fMode || FLAGS_quiltThreaded) {
        threads = SkThreadPool::kThreadPerCore;
    }
    SkThreadPool pool(threads);

    for (int y = 0; y < tiles_needed(full.height(), FLAGS_quiltTile); y++) {
        for (int x = 0; x < tiles_needed(full.width(), FLAGS_quiltTile); x++) {
            // Deletes itself when done.
            pool.add(new Tile(x, y, *recorded, &full));
        }
    }

    pool.wait();

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
