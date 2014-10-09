/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef PictureBenchmark_DEFINED
#define PictureBenchmark_DEFINED

#include "PictureRenderer.h"
#include "PictureResultsWriter.h"
#include "SkTypes.h"
#include "TimerData.h"

class SkPicture;
class Timer;

namespace sk_tools {

class PictureBenchmark {
public:
    PictureBenchmark();

    ~PictureBenchmark();

    /**
     * Draw the provided SkPicture fRepeats times while collecting timing data, and log the output
     * via fWriter.
     */
    void run(SkPicture* pict, bool useMultiPictureDraw);

    void setRepeats(int repeats) {
        fRepeats = repeats;
    }

    /**
     * If true, tells run to log separate timing data for each individual tile. Each tile will be
     * drawn fRepeats times. Requires the PictureRenderer set by setRenderer to be a
     * TiledPictureRenderer.
     */
    void setTimeIndividualTiles(bool indiv) { fTimeIndividualTiles = indiv; }
    bool timeIndividualTiles() const { return fTimeIndividualTiles; }

    void setPurgeDecodedTex(bool purgeDecodedTex) { fPurgeDecodedTex = purgeDecodedTex; }
    bool purgeDecodedText() const { return fPurgeDecodedTex; }

    PictureRenderer* setRenderer(PictureRenderer*);
    PictureRenderer* renderer() { return fRenderer; }

    void setTimerResultType(TimerData::Result resultType) { fTimerResult = resultType; }

    void setTimersToShow(bool wall, bool truncatedWall, bool cpu, bool truncatedCpu, bool gpu);

    void setWriter(PictureResultsWriter* writer) { fWriter = writer; }

private:
    int               fRepeats;
    PictureRenderer*  fRenderer;
    TimerData::Result fTimerResult;
    uint32_t          fTimerTypes; // bitfield of TimerData::TimerFlags values
    bool              fTimeIndividualTiles;
    bool              fPurgeDecodedTex;

    PictureResultsWriter* fWriter;

    Timer* setupTimer(bool useGLTimer = true);
};

}

#endif  // PictureBenchmark_DEFINED
