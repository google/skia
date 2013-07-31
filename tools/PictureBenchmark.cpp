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

namespace sk_tools {

PictureBenchmark::PictureBenchmark()
: fRepeats(1)
, fLogger(NULL)
, fRenderer(NULL)
, fTimerResult(TimerData::kAvg_Result)
, fTimerTypes(0)
, fTimeIndividualTiles(false)
{}

PictureBenchmark::~PictureBenchmark() {
    SkSafeUnref(fRenderer);
}

void PictureBenchmark::setTimersToShow(bool wall,
                                       bool truncatedWall,
                                       bool cpu,
                                       bool truncatedCpu,
                                       bool gpu) {
    fTimerTypes = 0;
    fTimerTypes |= wall ? TimerData::kWall_Flag : 0;
    fTimerTypes |= truncatedWall ? TimerData::kTruncatedWall_Flag : 0;
    fTimerTypes |= cpu ? TimerData::kCpu_Flag : 0;
    fTimerTypes |= truncatedCpu ? TimerData::kTruncatedCpu_Flag : 0;
    fTimerTypes |= gpu ? TimerData::kGpu_Flag : 0;
}

BenchTimer* PictureBenchmark::setupTimer(bool useGLTimer) {
#if SK_SUPPORT_GPU
    if (useGLTimer && fRenderer != NULL && fRenderer->isUsingGpuDevice()) {
        return SkNEW_ARGS(BenchTimer, (fRenderer->getGLContext()));
    }
#endif
    return SkNEW_ARGS(BenchTimer, (NULL));
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
    fRenderer->render(NULL);
    fRenderer->resetState(true);

    bool usingGpu = false;
#if SK_SUPPORT_GPU
    usingGpu = fRenderer->isUsingGpuDevice();
#endif

    uint32_t timerTypes = fTimerTypes;
    if (!usingGpu) {
        timerTypes &= ~TimerData::kGpu_Flag;
    }

    SkString timeFormat;
    if (TimerData::kPerIter_Result == fTimerResult) {
        timeFormat = fRenderer->getPerIterTimeFormat();
    } else {
        timeFormat = fRenderer->getNormalTimeFormat();
    }

    if (fTimeIndividualTiles) {
        TiledPictureRenderer* tiledRenderer = fRenderer->getTiledRenderer();
        SkASSERT(tiledRenderer && tiledRenderer->supportsTimingIndividualTiles());
        if (NULL == tiledRenderer || !tiledRenderer->supportsTimingIndividualTiles()) {
            return;
        }
        int xTiles, yTiles;
        if (!tiledRenderer->tileDimensions(xTiles, yTiles)) {
            return;
        }

        // Insert a newline so that each tile is reported on its own line (separate from the line
        // that describes the skp being run).
        this->logProgress("\n");

        int x, y;
        while (tiledRenderer->nextTile(x, y)) {
            // There are two timers, which will behave slightly differently:
            // 1) longRunningTimer, along with perTileTimerData, will time how long it takes to draw
            // one tile fRepeats times, and take the average. As such, it will not respect thea
            // logPerIter or printMin options, since it does not know the time per iteration. It
            // will also be unable to call flush() for each tile.
            // The goal of this timer is to make up for a system timer that is not precise enough to
            // measure the small amount of time it takes to draw one tile once.
            //
            // 2) perTileTimer, along with perTileTimerData, will record each run separately, and
            // then take the average. As such, it supports logPerIter and printMin options.
            //
            // Although "legal", having two gpu timers running at the same time
            // seems to cause problems (i.e., INVALID_OPERATIONs) on several
            // platforms. To work around this, we disable the gpu timer on the
            // long running timer.
            SkAutoTDelete<BenchTimer> longRunningTimer(this->setupTimer());
            TimerData longRunningTimerData(1);
            SkAutoTDelete<BenchTimer> perTileTimer(this->setupTimer(false));
            TimerData perTileTimerData(fRepeats);
            longRunningTimer->start();
            for (int i = 0; i < fRepeats; ++i) {
                perTileTimer->start();
                tiledRenderer->drawCurrentTile();
                perTileTimer->truncatedEnd();
                tiledRenderer->resetState(false);
                perTileTimer->end();
                SkAssertResult(perTileTimerData.appendTimes(perTileTimer.get()));
            }
            longRunningTimer->truncatedEnd();
            tiledRenderer->resetState(true);
            longRunningTimer->end();
            SkAssertResult(longRunningTimerData.appendTimes(longRunningTimer.get()));

            SkString configName = tiledRenderer->getConfigName();
            configName.appendf(": tile [%i,%i] out of [%i,%i]", x, y, xTiles, yTiles);

            SkString result = perTileTimerData.getResult(timeFormat.c_str(), fTimerResult,
                                                         configName.c_str(), timerTypes);
            result.append("\n");

// TODO(borenet): Turn off per-iteration tile time reporting for now.  Avoiding logging the time
// for every iteration for each tile cuts down on data file size by a significant amount. Re-enable
// this once we're loading the bench data directly into a data store and are no longer generating
// SVG graphs.
#if 0
            this->logProgress(result.c_str());
#endif

            configName.append(" <averaged>");
            SkString longRunningResult = longRunningTimerData.getResult(
                tiledRenderer->getNormalTimeFormat().c_str(),
                TimerData::kAvg_Result,
                configName.c_str(), timerTypes, fRepeats);
            longRunningResult.append("\n");
            this->logProgress(longRunningResult.c_str());
        }
    } else {
        SkAutoTDelete<BenchTimer> timer(this->setupTimer());
        TimerData timerData(fRepeats);
        for (int i = 0; i < fRepeats; ++i) {
            fRenderer->setup();

            timer->start();
            fRenderer->render(NULL);
            timer->truncatedEnd();

            // Finishes gl context
            fRenderer->resetState(true);
            timer->end();

            SkAssertResult(timerData.appendTimes(timer.get()));
        }

        SkString configName = fRenderer->getConfigName();

        SkString result = timerData.getResult(timeFormat.c_str(),
                                              fTimerResult,
                                              configName.c_str(),
                                              timerTypes);
        result.append("\n");
        this->logProgress(result.c_str());
    }

    fRenderer->end();
}

}
