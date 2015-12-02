/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SKPBench.h"
#include "SkCommandLineFlags.h"
#include "SkMultiPictureDraw.h"
#include "SkSurface.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#endif

// These CPU tile sizes are not good per se, but they are similar to what Chrome uses.
DEFINE_int32(CPUbenchTileW, 256, "Tile width  used for CPU SKP playback.");
DEFINE_int32(CPUbenchTileH, 256, "Tile height used for CPU SKP playback.");

DEFINE_int32(GPUbenchTileW, 1600, "Tile width  used for GPU SKP playback.");
DEFINE_int32(GPUbenchTileH, 512, "Tile height used for GPU SKP playback.");

SKPBench::SKPBench(const char* name, const SkPicture* pic, const SkIRect& clip, SkScalar scale,
                   bool useMultiPictureDraw, bool doLooping)
    : fPic(SkRef(pic))
    , fClip(clip)
    , fScale(scale)
    , fName(name)
    , fUseMultiPictureDraw(useMultiPictureDraw)
    , fDoLooping(doLooping) {
    fUniqueName.printf("%s_%.2g", name, scale);  // Scale makes this unqiue for perf.skia.org traces.
    if (useMultiPictureDraw) {
        fUniqueName.append("_mpd");
    }
}

SKPBench::~SKPBench() {
    for (int i = 0; i < fSurfaces.count(); ++i) {
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
    SkIRect bounds;
    SkAssertResult(canvas->getClipDeviceBounds(&bounds));

    const bool gpu = canvas->getGrContext() != nullptr;
    int tileW = gpu ? FLAGS_GPUbenchTileW : FLAGS_CPUbenchTileW,
        tileH = gpu ? FLAGS_GPUbenchTileH : FLAGS_CPUbenchTileH;

    tileW = SkTMin(tileW, bounds.width());
    tileH = SkTMin(tileH, bounds.height());

    int xTiles = SkScalarCeilToInt(bounds.width()  / SkIntToScalar(tileW));
    int yTiles = SkScalarCeilToInt(bounds.height() / SkIntToScalar(tileH));

    fSurfaces.setReserve(xTiles * yTiles);
    fTileRects.setReserve(xTiles * yTiles);

    SkImageInfo ii = canvas->imageInfo().makeWH(tileW, tileH);

    for (int y = bounds.fTop; y < bounds.fBottom; y += tileH) {
        for (int x = bounds.fLeft; x < bounds.fRight; x += tileW) {
            const SkIRect tileRect = SkIRect::MakeXYWH(x, y, tileW, tileH);
            *fTileRects.append() = tileRect;
            *fSurfaces.push() = canvas->newSurface(ii);

            // Never want the contents of a tile to include stuff the parent
            // canvas clips out
            SkRect clip = SkRect::Make(bounds);
            clip.offset(-SkIntToScalar(tileRect.fLeft), -SkIntToScalar(tileRect.fTop));
            fSurfaces.top()->getCanvas()->clipRect(clip);

            fSurfaces.top()->getCanvas()->setMatrix(canvas->getTotalMatrix());
            fSurfaces.top()->getCanvas()->scale(fScale, fScale);
        }
    }
}

void SKPBench::onPerCanvasPostDraw(SkCanvas* canvas) {
    // Draw the last set of tiles into the master canvas in case we're
    // saving the images
    for (int i = 0; i < fTileRects.count(); ++i) {
        SkAutoTUnref<SkImage> image(fSurfaces[i]->newImageSnapshot());
        canvas->drawImage(image,
                          SkIntToScalar(fTileRects[i].fLeft), SkIntToScalar(fTileRects[i].fTop));
        SkSafeSetNull(fSurfaces[i]);
    }

    fSurfaces.rewind();
    fTileRects.rewind();
}

bool SKPBench::isSuitableFor(Backend backend) {
    return backend != kNonRendering_Backend;
}

SkIPoint SKPBench::onGetSize() {
    return SkIPoint::Make(fClip.width(), fClip.height());
}

void SKPBench::onDraw(int loops, SkCanvas* canvas) {
    SkASSERT(fDoLooping || 1 == loops);
    if (fUseMultiPictureDraw) {
        for (int i = 0; i < loops; i++) {
            this->drawMPDPicture();
        }
    } else {
        for (int i = 0; i < loops; i++) {
            this->drawPicture();
        }
    }
#if SK_SUPPORT_GPU
    // Ensure the GrContext doesn't batch across draw loops.
    if (GrContext* context = canvas->getGrContext()) {
        context->flush();
    }
#endif
}

void SKPBench::drawMPDPicture() {
    SkMultiPictureDraw mpd;

    for (int j = 0; j < fTileRects.count(); ++j) {
        SkMatrix trans;
        trans.setTranslate(-fTileRects[j].fLeft/fScale,
                           -fTileRects[j].fTop/fScale);
        mpd.add(fSurfaces[j]->getCanvas(), fPic, &trans);
    }

    mpd.draw();

    for (int j = 0; j < fTileRects.count(); ++j) {
        fSurfaces[j]->getCanvas()->flush();
    }
}

void SKPBench::drawPicture() {
    for (int j = 0; j < fTileRects.count(); ++j) {
        const SkMatrix trans = SkMatrix::MakeTrans(-fTileRects[j].fLeft / fScale,
                                                   -fTileRects[j].fTop / fScale);
        fSurfaces[j]->getCanvas()->drawPicture(fPic, &trans, nullptr);
    }

    for (int j = 0; j < fTileRects.count(); ++j) {
        fSurfaces[j]->getCanvas()->flush();
    }
}

#if SK_SUPPORT_GPU
#include "GrGpu.h"
static void draw_pic_for_stats(SkCanvas* canvas, GrContext* context, const SkPicture* picture,
                               SkTArray<SkString>* keys, SkTArray<double>* values,
                               const char* tag) {
    context->resetGpuStats();
    canvas->drawPicture(picture);
    canvas->flush();

    int offset = keys->count();
    context->dumpGpuStatsKeyValuePairs(keys, values);
    context->dumpCacheStatsKeyValuePairs(keys, values);

    // append tag, but only to new tags
    for (int i = offset; i < keys->count(); i++, offset++) {
        (*keys)[i].appendf("_%s", tag);
    }
}
#endif

void SKPBench::getGpuStats(SkCanvas* canvas, SkTArray<SkString>* keys, SkTArray<double>* values) {
#if SK_SUPPORT_GPU
    // we do a special single draw and then dump the key / value pairs
    GrContext* context = canvas->getGrContext();
    if (!context) {
        return;
    }

    // TODO refactor this out if we want to test other subclasses of skpbench
    context->flush();
    context->freeGpuResources();
    context->resetContext();
    context->getGpu()->resetShaderCacheForTesting();
    draw_pic_for_stats(canvas, context, fPic, keys, values, "first_frame");

    // draw second frame
    draw_pic_for_stats(canvas, context, fPic, keys, values, "second_frame");

#endif
}
