/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBenchLogger.h"
#include "BenchTimer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "picture_utils.h"
#include "TimerData.h"

namespace sk_tools {

PictureBenchmark::PictureBenchmark()
: fRepeats(1)
, fLogger(NULL)
, fRenderer(NULL)
, fLogPerIter(false)
, fPrintMin(false)
, fShowWallTime(false)
, fShowTruncatedWallTime(false)
, fShowCpuTime(true)
, fShowTruncatedCpuTime(false)
, fShowGpuTime(false)
{}

PictureBenchmark::~PictureBenchmark() {
    SkSafeUnref(fRenderer);
}

BenchTimer* PictureBenchmark::setupTimer() {
#if SK_SUPPORT_GPU
    if (fRenderer != NULL && fRenderer->isUsingGpuDevice()) {
        return SkNEW_ARGS(BenchTimer, (fRenderer->getGLContext()));
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

PictureRenderer* PictureBenchmark::setRenderer(sk_tools::PictureRenderer* renderer) {
    SkRefCnt_SafeAssign(fRenderer, renderer);
    return renderer;
}

void PictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    SkASSERT(fRenderer != NULL);
    if (NULL == fRenderer) {
        return;
    }

    fRenderer->init(pict);

    // We throw this away to remove first time effects (such as paging in this program)
    fRenderer->setup();
    fRenderer->render(false);
    fRenderer->resetState();

    BenchTimer* timer = this->setupTimer();
    bool usingGpu = false;
#if SK_SUPPORT_GPU
    usingGpu = fRenderer->isUsingGpuDevice();
#endif

    TimerData timerData(fRenderer->getPerIterTimeFormat(), fRenderer->getNormalTimeFormat());
    for (int i = 0; i < fRepeats; ++i) {
        fRenderer->setup();

        timer->start();
        fRenderer->render(false);
        timer->truncatedEnd();

        // Finishes gl context
        fRenderer->resetState();
        timer->end();

        timerData.appendTimes(timer, fRepeats - 1 == i);
    }

    const char* configName = usingGpu ? "gpu" : "raster";
    SkString result = timerData.getResult(fLogPerIter, fPrintMin, fRepeats,
                                          configName, fShowWallTime, fShowTruncatedWallTime,
                                          fShowCpuTime, fShowTruncatedCpuTime,
                                          usingGpu && fShowGpuTime);
    result.append("\n");
    this->logProgress(result.c_str());

    fRenderer->end();
    SkDELETE(timer);
}

}
