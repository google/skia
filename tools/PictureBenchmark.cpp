/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Timer.h"
#include "PictureBenchmark.h"
#include "SkCanvas.h"
#include "SkPicture.h"
#include "SkString.h"
#include "picture_utils.h"

namespace sk_tools {

PictureBenchmark::PictureBenchmark()
    : fRepeats(1)
    , fRenderer(NULL)
    , fTimerResult(TimerData::kAvg_Result)
    , fTimerTypes(0)
    , fTimeIndividualTiles(false)
    , fPurgeDecodedTex(false)
    , fWriter(NULL) {
}

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

Timer* PictureBenchmark::setupTimer(bool useGLTimer) {
#if SK_SUPPORT_GPU
    if (useGLTimer && fRenderer != NULL && fRenderer->isUsingGpuDevice()) {
        return SkNEW_ARGS(Timer, (fRenderer->getGLContext()));
    }
#endif
    return SkNEW_ARGS(Timer, (NULL));
}

PictureRenderer* PictureBenchmark::setRenderer(sk_tools::PictureRenderer* renderer) {
    SkRefCnt_SafeAssign(fRenderer, renderer);
    return renderer;
}

void PictureBenchmark::run(SkPicture* pict, bool useMultiPictureDraw) {
    SkASSERT(pict);
    if (NULL == pict) {
        return;
    }

    SkASSERT(fRenderer != NULL);
    if (NULL == fRenderer) {
        return;
    }

    fRenderer->init(pict, NULL, NULL, NULL, false, useMultiPictureDraw);

    // We throw this away to remove first time effects (such as paging in this program)
    fRenderer->setup();

    fRenderer->render(NULL);
    fRenderer->resetState(true);   // flush, swapBuffers and Finish

    if (fPurgeDecodedTex) {
        fRenderer->purgeTextures();
    }

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

    static const int kNumInnerLoops = 10;
    int numOuterLoops = 1;
    int numInnerLoops = fRepeats;

    if (TimerData::kPerIter_Result == fTimerResult && fRepeats > 1) {
        // interpret this flag combination to mean: generate 'fRepeats'
        // numbers by averaging each rendering 'kNumInnerLoops' times
        numOuterLoops = fRepeats;
        numInnerLoops = kNumInnerLoops;
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

        int x, y;
        while (tiledRenderer->nextTile(x, y)) {
            // There are two timers, which will behave slightly differently:
            // 1) longRunningTimer, along with perTileTimerData, will time how long it takes to draw
            // one tile fRepeats times, and take the average. As such, it will not respect the
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
            SkAutoTDelete<Timer> longRunningTimer(this->setupTimer());
            TimerData longRunningTimerData(numOuterLoops);

            for (int outer = 0; outer < numOuterLoops; ++outer) {
                SkAutoTDelete<Timer> perTileTimer(this->setupTimer(false));
                TimerData perTileTimerData(numInnerLoops);

                longRunningTimer->start();
                for (int inner = 0; inner < numInnerLoops; ++inner) {
                    perTileTimer->start();
                    tiledRenderer->drawCurrentTile();
                    perTileTimer->truncatedEnd();
                    tiledRenderer->resetState(false);  // flush & swapBuffers, but don't Finish
                    perTileTimer->end();
                    SkAssertResult(perTileTimerData.appendTimes(perTileTimer.get()));

                    if (fPurgeDecodedTex) {
                        fRenderer->purgeTextures();
                    }
                }
                longRunningTimer->truncatedEnd();
                tiledRenderer->resetState(true);       // flush, swapBuffers and Finish
                longRunningTimer->end();
                SkAssertResult(longRunningTimerData.appendTimes(longRunningTimer.get()));
            }

            fWriter->logRenderer(tiledRenderer);
            fWriter->tileMeta(x, y, xTiles, yTiles);

            // TODO(borenet): Turn off per-iteration tile time reporting for now.
            // Avoiding logging the time for every iteration for each tile cuts
            // down on data file size by a significant amount. Re-enable this once
            // we're loading the bench data directly into a data store and are no
            // longer generating SVG graphs.
#if 0
            fWriter->tileData(
                    &perTileTimerData,
                    timeFormat.c_str(),
                    fTimerResult,
                    timerTypes);
#endif

            if (fPurgeDecodedTex) {
                fWriter->addTileFlag(PictureResultsWriter::kPurging);
            }
            fWriter->addTileFlag(PictureResultsWriter::kAvg);
            fWriter->tileData(
                &longRunningTimerData,
                tiledRenderer->getNormalTimeFormat().c_str(),
                TimerData::kAvg_Result,
                timerTypes,
                numInnerLoops);
        }
    } else {
        SkAutoTDelete<Timer> longRunningTimer(this->setupTimer());
        TimerData longRunningTimerData(numOuterLoops);

        for (int outer = 0; outer < numOuterLoops; ++outer) {
            SkAutoTDelete<Timer> perRunTimer(this->setupTimer(false));
            TimerData perRunTimerData(numInnerLoops);

            longRunningTimer->start();
            for (int inner = 0; inner < numInnerLoops; ++inner) {
                fRenderer->setup();

                perRunTimer->start();
                fRenderer->render(NULL);
                perRunTimer->truncatedEnd();
                fRenderer->resetState(false);   // flush & swapBuffers, but don't Finish
                perRunTimer->end();

                SkAssertResult(perRunTimerData.appendTimes(perRunTimer.get()));

                if (fPurgeDecodedTex) {
                    fRenderer->purgeTextures();
                }
            }
            longRunningTimer->truncatedEnd();
            fRenderer->resetState(true);        // flush, swapBuffers and Finish
            longRunningTimer->end();
            SkAssertResult(longRunningTimerData.appendTimes(longRunningTimer.get()));
        }

        fWriter->logRenderer(fRenderer);
        if (fPurgeDecodedTex) {
            fWriter->addTileFlag(PictureResultsWriter::kPurging);
        }

        // Beware - since the per-run-timer doesn't ever include a glFinish it can
        // report a lower time then the long-running-timer
#if 0
        fWriter->tileData(
                &perRunTimerData,
                timeFormat.c_str(),
                fTimerResult,
                timerTypes);
#else
        fWriter->tileData(
                &longRunningTimerData,
                timeFormat.c_str(),
                fTimerResult,
                timerTypes,
                numInnerLoops);
#endif
    }

    fRenderer->end();
}

}
