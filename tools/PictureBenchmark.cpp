/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "picture_utils.h"

namespace sk_tools {

BenchTimer* PictureBenchmark::setupTimer() {
#if SK_SUPPORT_GPU
    PictureRenderer* renderer = getRenderer();

    if (renderer != NULL && renderer->isUsingGpuDevice()) {
        return SkNEW_ARGS(BenchTimer, (renderer->getGLContext()));
    } else {
        return SkNEW_ARGS(BenchTimer, (NULL));
    }
#else
    return SkNEW_ARGS(BenchTimer, (NULL));
#endif
}

void PipePictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    fRenderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    fRenderer.render();
    fRenderer.resetState();

    BenchTimer* timer = this->setupTimer();
    double wall_time = 0;
#if SK_SUPPORT_GPU
    double gpu_time = 0;
#endif

    for (int i = 0; i < fRepeats; ++i) {
        timer->start();
        fRenderer.render();
        timer->end();
        fRenderer.resetState();

        wall_time += timer->fWall;
#if SK_SUPPORT_GPU
        if (fRenderer.isUsingGpuDevice()) {
            gpu_time += timer->fGpu;
        }
#endif
    }

    SkString result;
    result.printf("pipe: msecs = %6.2f", wall_time / fRepeats);
#if SK_SUPPORT_GPU
    if (fRenderer.isUsingGpuDevice()) {
        result.appendf(" gmsecs = %6.2f", gpu_time / fRepeats);
    }
#endif
    result.appendf("\n");
    sk_tools::print_msg(result.c_str());

    fRenderer.end();
    SkDELETE(timer);
}

void RecordPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    BenchTimer* timer = setupTimer();
    double wall_time = 0;

    for (int i = 0; i < fRepeats + 1; ++i) {
        SkPicture replayer;
        SkCanvas* recorder = replayer.beginRecording(pict->width(), pict->height());

        timer->start();
        recorder->drawPicture(*pict);
        timer->end();

        // We want to ignore first time effects
        if (i > 0) {
            wall_time += timer->fWall;
        }
    }

    SkString result;
    result.printf("record: msecs = %6.5f\n", wall_time / fRepeats);
    sk_tools::print_msg(result.c_str());

    SkDELETE(timer);
}

void SimplePictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    fRenderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    fRenderer.render();
    fRenderer.resetState();


    BenchTimer* timer = this->setupTimer();
    double wall_time = 0;
#if SK_SUPPORT_GPU
    double gpu_time = 0;
#endif

    for (int i = 0; i < fRepeats; ++i) {
        timer->start();
        fRenderer.render();
        timer->end();
        fRenderer.resetState();

        wall_time += timer->fWall;
#if SK_SUPPORT_GPU
        if (fRenderer.isUsingGpuDevice()) {
            gpu_time += timer->fGpu;
        }
#endif
    }


    SkString result;
    result.printf("simple: msecs = %6.2f", wall_time / fRepeats);
#if SK_SUPPORT_GPU
    if (fRenderer.isUsingGpuDevice()) {
        result.appendf(" gmsecs = %6.2f", gpu_time / fRepeats);
    }
#endif
    result.appendf("\n");
    sk_tools::print_msg(result.c_str());

    fRenderer.end();
    SkDELETE(timer);
}

void TiledPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    fRenderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    fRenderer.drawTiles();
    fRenderer.resetState();

    BenchTimer* timer = setupTimer();
    double wall_time = 0;
#if SK_SUPPORT_GPU
    double gpu_time = 0;
#endif

    for (int i = 0; i < fRepeats; ++i) {
        timer->start();
        fRenderer.drawTiles();
        timer->end();
        fRenderer.resetState();

        wall_time += timer->fWall;
#if SK_SUPPORT_GPU
        if (fRenderer.isUsingGpuDevice()) {
            gpu_time += timer->fGpu;
        }
#endif
    }

    SkString result;
    if (fRenderer.getTileMinPowerOf2Width() > 0) {
        result.printf("%i_pow2tiles_%iminx%i: msecs = %6.2f", fRenderer.numTiles(),
                      fRenderer.getTileMinPowerOf2Width(), fRenderer.getTileHeight(),
                      wall_time / fRepeats);
    } else {
        result.printf("%i_tiles_%ix%i: msecs = %6.2f", fRenderer.numTiles(),
                      fRenderer.getTileWidth(), fRenderer.getTileHeight(), wall_time / fRepeats);
    }
#if SK_SUPPORT_GPU
    if (fRenderer.isUsingGpuDevice()) {
        result.appendf(" gmsecs = %6.2f", gpu_time / fRepeats);
    }
#endif
    result.appendf("\n");
    sk_tools::print_msg(result.c_str());

    fRenderer.end();
    SkDELETE(timer);
}

void UnflattenPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    BenchTimer* timer = setupTimer();
    double wall_time = 0;

    for (int i = 0; i < fRepeats + 1; ++i) {
        SkPicture replayer;
        SkCanvas* recorder = replayer.beginRecording(pict->width(), pict->height());

        recorder->drawPicture(*pict);

        timer->start();
        replayer.endRecording();
        timer->end();

        // We want to ignore first time effects
        if (i > 0) {
            wall_time += timer->fWall;
        }
    }

    SkString result;
    result.printf("unflatten: msecs = %6.4f\n", wall_time / fRepeats);
    sk_tools::print_msg(result.c_str());

    SkDELETE(timer);
}

}
