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

void PipePictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (pict == NULL) {
        return;
    }

    renderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    renderer.render();

    BenchTimer timer = BenchTimer(NULL);
    timer.start();
    for (int i = 0; i < fRepeats; ++i) {
        renderer.render();
    }
    timer.end();

    renderer.end();

    SkDebugf("pipe: msecs = %6.2f\n", timer.fWall / fRepeats);
}

void RecordPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (pict == NULL) {
        return;
    }

    BenchTimer timer = BenchTimer(NULL);
    double wall_time = 0;

    for (int i = 0; i < fRepeats + 1; ++i) {
        SkPicture replayer;
        SkCanvas* recorder = replayer.beginRecording(pict->width(), pict->height());

        timer.start();
        recorder->drawPicture(*pict);
        timer.end();

        // We want to ignore first time effects
        if (i > 0) {
            wall_time += timer.fWall;
        }
    }

    SkDebugf("record: msecs = %6.5f\n", wall_time / fRepeats);
}

void SimplePictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (pict == NULL) {
        return;
    }

    renderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    renderer.render();

    BenchTimer timer = BenchTimer(NULL);
    timer.start();
    for (int i = 0; i < fRepeats; ++i) {
        renderer.render();
    }
    timer.end();

    renderer.end();

    printf("simple: msecs = %6.2f\n", timer.fWall / fRepeats);
}

void TiledPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (pict == NULL) {
        return;
    }

    renderer.init(pict);

    // We throw this away to remove first time effects (such as paging in this
    // program)
    renderer.drawTiles();

    BenchTimer timer = BenchTimer(NULL);
    timer.start();
    for (int i = 0; i < fRepeats; ++i) {
        renderer.drawTiles();
    }
    timer.end();

    renderer.end();

    SkDebugf("%i_tiles_%ix%i: msecs = %6.2f\n", renderer.numTiles(), renderer.getTileWidth(),
            renderer.getTileHeight(), timer.fWall / fRepeats);
}

void UnflattenPictureBenchmark::run(SkPicture* pict) {
    SkASSERT(pict);
    if (pict == NULL) {
        return;
    }

    BenchTimer timer = BenchTimer(NULL);
    double wall_time = 0;

    for (int i = 0; i < fRepeats + 1; ++i) {
        SkPicture replayer;
        SkCanvas* recorder = replayer.beginRecording(pict->width(), pict->height());

        recorder->drawPicture(*pict);

        timer.start();
        replayer.endRecording();
        timer.end();

        // We want to ignore first time effects
        if (i > 0) {
            wall_time += timer.fWall;
        }
    }

    SkDebugf("unflatten: msecs = %6.4f\n", wall_time / fRepeats);
}

}
