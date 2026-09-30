/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GpuTools.h"
#include "bench/SKPBench.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSurface.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/flags/CommandLineFlags.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

using namespace skia_private;

SKPBench::SKPBench(const char* name, const SkPicture* pic, const SkIRect& clip, SkScalar scale,
                   const SkISize& tileSize, bool doLooping)
        : fPic(SkRef(pic))
        , fClip(clip)
        , fScale(scale)
        , fTileSize(tileSize)
        , fName(name)
        , fDoLooping(doLooping) {
    fUniqueName.printf("%s_%.2g", name, scale); // Scale makes this unique for perf.skia.org traces.
}

SKPBench::~SKPBench() {}

const char* SKPBench::onGetName() {
    return fName.c_str();
}

const char* SKPBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

// Subdivides the given bounds into a grid of tiles according to fTileSize, records each tile's
// SkPicture, creates the corresponding SkSurface, and stores TileInfo entries in fTiles.
void SKPBench::createTiles(SkCanvas* canvas, const SkIRect& bounds, const SkISize& tileSize) {
    int xTiles = SkScalarCeilToInt(bounds.width()  / SkIntToScalar(tileSize.width()));
    int yTiles = SkScalarCeilToInt(bounds.height() / SkIntToScalar(tileSize.height()));

    fTiles.reserve(xTiles * yTiles);

    SkImageInfo ii = canvas->imageInfo().makeWH(tileSize.width(), tileSize.height());

    for (int y = bounds.fTop; y < bounds.fBottom; y += tileSize.height()) {
        for (int x = bounds.fLeft; x < bounds.fRight; x += tileSize.width()) {
            const SkIRect tileRect = SkIRect::MakeXYWH(x, y, tileSize.width(), tileSize.height());

            // Never want the contents of a tile to include stuff the parent
            // canvas clips out
            SkRect clip = SkRect::Make(bounds);
            clip.offset(-SkIntToScalar(tileRect.fLeft), -SkIntToScalar(tileRect.fTop));

            SkM44 mat = canvas->getLocalToDevice();
            mat.preScale(fScale, fScale);

            // Record the commands to draw what's only in the tile from the original picture.
            SkPictureRecorder recorder;
            SkCanvas* tiledCanvas = recorder.beginRecording(SkRect::Make(tileRect));
            fPic->playback(tiledCanvas);
            sk_sp<SkPicture> tiledPicture = recorder.finishRecordingAsPicture();
            sk_sp<SkSurface> tiledSurface = canvas->makeSurface(ii);
            SkASSERT(tiledSurface != nullptr);

            fTiles.emplace_back(tiledPicture, tiledSurface, tileRect, clip, mat);
        }
    }
}

void SKPBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    SkIRect bounds = canvas->getDeviceClipBounds();
    if (!fClip.isEmpty()) {
        bounds.intersect(fClip);
    }
    bounds.intersect(fPic->cullRect().roundOut());
    SkAssertResult(!bounds.isEmpty());

    if (this->shouldTile()) {
        SkISize tileSize = SkISize::Make(std::min(fTileSize.width(), bounds.width()),
                                         std::min(fTileSize.height(), bounds.height()));
        this->createTiles(canvas, bounds, tileSize);
    } else {
        // The whole picture is one tile. This will playback the whole picture and reduce ops to
        // fit just the bounds, whether clipped or not.
        this->createTiles(canvas, bounds, bounds.size());
    }
}

void SKPBench::onPerCanvasPostDraw(SkCanvas* canvas) {
    // Draw the last set of tiles into the main canvas in case we're
    // saving the images
    for (const TileInfo& tile : fTiles) {
        sk_sp<SkImage> image(tile.surface()->makeImageSnapshot());
        canvas->drawImage(image,
                          SkIntToScalar(tile.tileRect().fLeft),
                          SkIntToScalar(tile.tileRect().fTop));
    }

    fTiles.clear();
}

bool SKPBench::isSuitableFor(Backend backend) {
    return backend != Backend::kNonRendering;
}

SkISize SKPBench::onGetSize() {
    if (fClip.isEmpty()) {
        return fPic->cullRect().roundOut().size();
    } else {
        return SkISize::Make(fClip.width(), fClip.height());
    }
}

void SKPBench::onDrawFrame(int loops, SkCanvas* /* canvas */, std::function<void()> submitFrame) {
    SkASSERT(fDoLooping || 1 == loops);
    for (int i = 0; i < loops; ++i) {
        this->drawPicture();
        if (submitFrame) {
            submitFrame();
        }
    }
}

void SKPBench::drawPicture() {
    for (const TileInfo& tile : fTiles) {
        const SkMatrix trans = SkMatrix::Translate(-tile.tileRect().fLeft / fScale,
                                                   -tile.tileRect().fTop / fScale);

        SkCanvas* canvas = tile.surface()->getCanvas();

        SkAutoCanvasRestore acr(canvas, /* doSave= */ false);
        canvas->clear(SK_ColorWHITE);

        canvas->save();
        canvas->clipRect(tile.clipRect());
        canvas->setMatrix(tile.mat());

        canvas->drawPicture(tile.picture(), &trans, nullptr);
    }

    for (const TileInfo& tile : fTiles) {
        skgpu::Flush(tile.surface());
    }
}

static void draw_pic_for_stats(SkCanvas* canvas,
                               GrDirectContext* dContext,
                               const SkPicture* picture,
                               TArray<SkString>* keys,
                               TArray<double>* values) {
    dContext->priv().resetGpuStats();
    dContext->priv().resetContextStats();
    canvas->drawPicture(picture);
    dContext->flush();

    dContext->priv().dumpGpuStatsKeyValuePairs(keys, values);
    dContext->priv().dumpCacheStatsKeyValuePairs(keys, values);
    dContext->priv().dumpContextStatsKeyValuePairs(keys, values);
}

void SKPBench::getGpuStats(SkCanvas* canvas, TArray<SkString>* keys, TArray<double>* values) {
    // we do a special single draw and then dump the key / value pairs
    auto direct = canvas->recordingContext() ? canvas->recordingContext()->asDirectContext()
                                             : nullptr;
    if (!direct) {
        return;
    }

    // TODO refactor this out if we want to test other subclasses of skpbench
    direct->flushAndSubmit();
    direct->freeGpuResources();
    direct->resetContext();
    direct->priv().getGpu()->resetShaderCacheForTesting();
    draw_pic_for_stats(canvas, direct, fPic.get(), keys, values);
}

bool SKPBench::getDMSAAStats(GrRecordingContext* rContext) {
    if (!rContext || !rContext->asDirectContext()) {
        return false;
    }
    // Clear the current DMSAA stats then do a single tiled draw that resets them to the specific
    // values for our SKP.
    rContext->asDirectContext()->flushAndSubmit();
    rContext->priv().dmsaaStats() = {};
    this->drawPicture();  // Draw tiled for DMSAA stats.
    rContext->asDirectContext()->flush();
    return true;
}
