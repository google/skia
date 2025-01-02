/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/MSKPBench.h"
#include "include/core/SkCanvas.h"
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/GrRecordingContext.h"
#include "tools/MSKPPlayer.h"

#if defined(SK_GRAPHITE)
#include "include/gpu/graphite/Context.h"
#include "include/gpu/graphite/Recorder.h"
#include "src/gpu/graphite/RecorderPriv.h"
#endif

MSKPBench::MSKPBench(SkString name, std::unique_ptr<MSKPPlayer> player)
        : fName(name), fPlayer(std::move(player)) {}

MSKPBench::~MSKPBench() = default;

void MSKPBench::onDrawFrame(int loops, SkCanvas* canvas, std::function<void()> submitFrame) {
    for (int i = 0; i < loops; ++i) {
        for (int f = 0; f < fPlayer->numFrames(); ++f) {
            canvas->save();
            canvas->clipIRect(SkIRect::MakeSize(fPlayer->frameDimensions(f)));
            fPlayer->playFrame(canvas, f);
            canvas->restore();
            if (submitFrame) {
                submitFrame();
            }
        }
        // Ensure each loop replays all offscreen layer draws from scratch.
        fPlayer->rewindLayers();
    }
}

const char* MSKPBench::onGetName() { return fName.c_str(); }

SkISize MSKPBench::onGetSize() {
    auto dims = fPlayer->maxDimensions();
    return {dims.width(), dims.height()};
}

void MSKPBench::onPreDraw(SkCanvas* canvas) {
    // We don't benchmark creation of the backing stores for layers so ensure they're all created.
    fPlayer->allocateLayers(canvas);
}

void MSKPBench::onPostDraw(SkCanvas*) {
    // nanobench can tear down the 3D API context/device before destroying the benchmarks.
    fPlayer->resetLayers();
}
