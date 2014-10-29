#include "DMQuiltTask.h"
#include "DMUtil.h"
#include "DMWriteTask.h"

#include "SkBBHFactory.h"
#include "SkCommandLineFlags.h"
#include "SkPicture.h"
#include "SkTaskGroup.h"

DEFINE_bool(quilt, true, "If true, draw GM via a picture into a quilt of small tiles and compare.");
DEFINE_int32(quiltTile, 256, "Dimension of (square) quilt tile.");

namespace DM {

static const char* kBBHs[] = { "nobbh", "rtree", "tilegrid" };
QuiltTask::QuiltTask(const Task& parent, skiagm::GM* gm, SkBitmap reference, QuiltTask::BBH bbh)
    : CpuTask(parent)
    , fBBH(bbh)
    , fName(UnderJoin(parent.name().c_str(), kBBHs[bbh]))
    , fGM(gm)
    , fReference(reference)
    {}

static int tiles_needed(int fullDimension, int tileDimension) {
    return (fullDimension + tileDimension - 1) / tileDimension;
}

struct DrawTileArgs {
    int x, y;
    const SkPicture* picture;
    SkBitmap* quilt;
};

static void draw_tile(DrawTileArgs* arg) {
    const DrawTileArgs& a = *arg;
    SkBitmap tile;
    a.quilt->extractSubset(&tile, SkIRect::MakeXYWH(a.x, a.y, FLAGS_quiltTile, FLAGS_quiltTile));
    SkCanvas tileCanvas(tile);
    tileCanvas.translate(SkIntToScalar(-a.x), SkIntToScalar(-a.y));
    a.picture->playback(&tileCanvas);
    tileCanvas.flush();
}

void QuiltTask::draw() {
    SkAutoTDelete<SkBBHFactory> factory;
    switch (fBBH) {
        case kNone_BBH: break;
        case kRTree_BBH:
            factory.reset(SkNEW(SkRTreeFactory));
            break;
        case kTileGrid_BBH: {
            const SkTileGridFactory::TileGridInfo tiles = {
                { FLAGS_quiltTile, FLAGS_quiltTile },
                /*overlap: */{0, 0},
                /*offset:  */{0, 0},
            };
            factory.reset(SkNEW_ARGS(SkTileGridFactory, (tiles)));
            break;
        }
    }

    // A couple GMs draw wrong when using a bounding box hierarchy.
    // This almost certainly means we have a bug to fix, but for now
    // just draw without a bounding box hierarchy.
    if (fGM->getFlags() & skiagm::GM::kNoBBH_Flag) {
        factory.reset(NULL);
    }

    SkAutoTUnref<const SkPicture> recorded(RecordPicture(fGM.get(), factory.get()));

    SkBitmap full;
    AllocatePixels(fReference, &full);

    if (fGM->getFlags() & skiagm::GM::kSkipTiled_Flag) {
        // Some GMs don't draw exactly the same when tiled.  Draw them in one go.
        SkCanvas canvas(full);
        recorded->playback(&canvas);
        canvas.flush();
    } else {
        // Draw tiles in parallel into the same bitmap, simulating aggressive impl-side painting.
        int xTiles = tiles_needed(full.width(),  FLAGS_quiltTile),
            yTiles = tiles_needed(full.height(), FLAGS_quiltTile);
        SkTDArray<DrawTileArgs> args;
        args.setCount(xTiles*yTiles);
        for (int y = 0; y < yTiles; y++) {
            for (int x = 0; x < xTiles; x++) {
                DrawTileArgs arg = { x*FLAGS_quiltTile, y*FLAGS_quiltTile, recorded, &full };
                args[y*xTiles + x] = arg;
            }
        }
        SkTaskGroup().batch(draw_tile, args.begin(), args.count());
    }

    if (!BitmapsEqual(full, fReference)) {
        this->fail();
        this->spawnChild(SkNEW_ARGS(WriteTask, (*this, "GM", full)));
    }
}

bool QuiltTask::shouldSkip() const {
    if (fGM->getFlags() & skiagm::GM::kSkipPicture_Flag) {
        return true;
    }
    return !FLAGS_quilt;
}

}  // namespace DM
