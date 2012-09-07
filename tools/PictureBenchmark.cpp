/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkTypes.h"
#include "SkBenchLogger.h"
#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "picture_utils.h"
#include "TimerData.h"

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

void PictureBenchmark::logProgress(const char msg[]) {
    if (fLogger != NULL) {
        fLogger->logProgress(msg);
    }
}

void PictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    PictureRenderer* renderer = this->getRenderer();
    SkASSERT(renderer != NULL);
    if (NULL == renderer) {
        return;
    }
    renderer->init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    renderer->setup();
    renderer->render(false);
    renderer->resetState();

    BenchTimer* timer = this->setupTimer();
    bool usingGpu = false;
#if SK_SUPPORT_GPU
    usingGpu = renderer->isUsingGpuDevice();
#endif

    TimerData timerData(renderer->getPerIterTimeFormat(), renderer->getNormalTimeFormat());
    for (int i = 0; i < fRepeats; ++i) {
        renderer->setup();

        timer->start();
        renderer->render(false);
        timer->truncatedEnd();

        // Finishes gl context
        renderer->resetState();
        timer->end();

        timerData.appendTimes(timer, fRepeats - 1 == i);
    }

    // FIXME: Pass these options on the command line.
    bool logPerIter = false;
    bool printMin = false;
    const char* configName = usingGpu ? "gpu" : "raster";
    bool showWallTime = true;
    bool showTruncatedWallTime = false;
    bool showCpuTime = false;
    bool showTruncatedCpuTime = false;
    SkString result = timerData.getResult(logPerIter, printMin, fRepeats,
                                          configName, showWallTime, showTruncatedWallTime,
                                          showCpuTime, showTruncatedCpuTime, usingGpu);
    result.append("\n");
    this->logProgress(result.c_str());

    renderer->end();
    SkDELETE(timer);
}

}
