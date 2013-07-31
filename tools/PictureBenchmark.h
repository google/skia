/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureBenchmark_DEFINED
#define PictureBenchmark_DEFINED

#include "SkTypes.h"
#include "PictureRenderer.h"
#include "TimerData.h"

class BenchTimer;
class SkBenchLogger;
class SkPicture;
class SkString;

namespace sk_tools {

class PictureBenchmark {
public:
    PictureBenchmark();

    ~PictureBenchmark();

    /**
     * Draw the provided SkPicture fRepeats times while collecting timing data, and log the output
     * via fLogger.
     */
    void run(SkPicture* pict);

    void setRepeats(int repeats) {
        fRepeats = repeats;
    }

    /**
     * If true, tells run to log separate timing data for each individual tile. Each tile will be
     * drawn fRepeats times. Requires the PictureRenderer set by setRenderer to be a
     * TiledPictureRenderer.
     */
    void setTimeIndividualTiles(bool indiv) { fTimeIndividualTiles = indiv; }

    bool timeIndividualTiles() { return fTimeIndividualTiles; }

    PictureRenderer* setRenderer(PictureRenderer*);

    void setTimerResultType(TimerData::Result resultType) { fTimerResult = resultType; }

    void setTimersToShow(bool wall, bool truncatedWall, bool cpu, bool truncatedCpu, bool gpu);

    void setLogger(SkBenchLogger* logger) { fLogger = logger; }

private:
    int               fRepeats;
    SkBenchLogger*    fLogger;
    PictureRenderer*  fRenderer;
    TimerData::Result fTimerResult;
    uint32_t          fTimerTypes; // bitfield of TimerData::TimerFlags values
    bool              fTimeIndividualTiles;

    void logProgress(const char msg[]);

    BenchTimer* setupTimer(bool useGLTimer = true);
};

}

#endif  // PictureBenchmark_DEFINED
