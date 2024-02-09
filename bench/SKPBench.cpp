/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GpuTools.h"
#include "bench/SKPBench.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "src/gpu/ganesh/GrDirectContextPriv.h"
#include "src/gpu/ganesh/GrGpu.h"
#include "tools/flags/CommandLineFlags.h"

using namespace skia_private;

// These CPU tile sizes are not good per se, but they are similar to what Chrome uses.
static DEFINE_int(CPUbenchTileW, 256, "Tile width  used for CPU SKP playback.");
static DEFINE_int(CPUbenchTileH, 256, "Tile height used for CPU SKP playback.");

static DEFINE_int(GPUbenchTileW, 1600, "Tile width  used for GPU SKP playback.");
static DEFINE_int(GPUbenchTileH, 512, "Tile height used for GPU SKP playback.");

SKPBench::SKPBench(const char* name, const SkPicture* pic, const SkIRect& clip, SkScalar scale,
                   bool doLooping)
    : fPic(SkRef(pic))
    , fClip(clip)
    , fScale(scale)
    , fName(name)
    , fDoLooping(doLooping) {
    fUniqueName.printf("%s_%.2g", name, scale);  // Scale makes this unqiue for perf.skia.org traces.
}

SKPBench::~SKPBench() {
    for (int i = 0; i < fSurfaces.size(); ++i) {
        fSurfaces[i]->unref();
    }
}

const char* SKPBench::onGetName() {
    return fName.c_str();
}

const char* SKPBench::onGetUniqueName() {
    return fUniqueName.c_str();
}

void SKPBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    SkIRect bounds = canvas->getDeviceClipBounds();
    bounds.intersect(fClip);
    bounds.intersect(fPic->cullRect().roundOut());
    SkAssertResult(!bounds.isEmpty());

#if defined(SK_GRAPHITE)
    const bool gpu = canvas->recordingContext() != nullptr || canvas->recorder() != nullptr;
#else
    const bool gpu = canvas->recordingContext() != nullptr;
#endif
    int tileW = gpu ? FLAGS_GPUbenchTileW : FLAGS_CPUbenchTileW,
        tileH = gpu ? FLAGS_GPUbenchTileH : FLAGS_CPUbenchTileH;

    tileW = std::min(tileW, bounds.width());
    tileH = std::min(tileH, bounds.height());

    int xTiles = SkScalarCeilToInt(bounds.width()  / SkIntToScalar(tileW));
    int yTiles = SkScalarCeilToInt(bounds.height() / SkIntToScalar(tileH));

    fSurfaces.reserve_exact(fSurfaces.size() + (xTiles * yTiles));
    fTileRects.reserve(xTiles * yTiles);

    SkImageInfo ii = canvas->imageInfo().makeWH(tileW, tileH);

    for (int y = bounds.fTop; y < bounds.fBottom; y += tileH) {
        for (int x = bounds.fLeft; x < bounds.fRight; x += tileW) {
            const SkIRect tileRect = SkIRect::MakeXYWH(x, y, tileW, tileH);
            *fTileRects.append() = tileRect;
            fSurfaces.emplace_back(canvas->makeSurface(ii));

            // Never want the contents of a tile to include stuff the parent
            // canvas clips out
            SkRect clip = SkRect::Make(bounds);
            clip.offset(-SkIntToScalar(tileRect.fLeft), -SkIntToScalar(tileRect.fTop));
            fSurfaces.back()->getCanvas()->clipRect(clip);

            fSurfaces.back()->getCanvas()->setMatrix(canvas->getLocalToDevice());
            fSurfaces.back()->getCanvas()->scale(fScale, fScale);
        }
    }
}

void SKPBench::onPerCanvasPostDraw(SkCanvas* canvas) {
    // Draw the last set of tiles into the main canvas in case we're
    // saving the images
    for (int i = 0; i < fTileRects.size(); ++i) {
        sk_sp<SkImage> image(fSurfaces[i]->makeImageSnapshot());
        canvas->drawImage(image,
                          SkIntToScalar(fTileRects[i].fLeft), SkIntToScalar(fTileRects[i].fTop));
    }

    fSurfaces.clear();
    fTileRects.clear();
}

bool SKPBench::isSuitableFor(Backend backend) {
    return backend != Backend::kNonRendering;
}

SkISize SKPBench::onGetSize() {
    return SkISize::Make(fClip.width(), fClip.height());
}

void SKPBench::onDraw(int loops, SkCanvas* canvas) {
    SkASSERT(fDoLooping || 1 == loops);
    while (1) {
        this->drawPicture();
        if (0 == --loops) {
            break;
        }

        auto direct = canvas->recordingContext() ? canvas->recordingContext()->asDirectContext()
                                                 : nullptr;
        // Ensure the GrContext doesn't combine ops across draw loops.
        if (direct) {
            direct->flushAndSubmit();
        }
    }
}

void SKPBench::drawMPDPicture() {
    // TODO: remove me
}

void SKPBench::drawPicture() {
    for (int j = 0; j < fTileRects.size(); ++j) {
        const SkMatrix trans = SkMatrix::Translate(-fTileRects[j].fLeft / fScale,
                                                   -fTileRects[j].fTop / fScale);
        fSurfaces[j]->getCanvas()->drawPicture(fPic.get(), &trans, nullptr);
    }

    for (int j = 0; j < fTileRects.size(); ++j) {
        skgpu::Flush(fSurfaces[j].get());
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
