/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/base/SkTime.h"
#include "tools/flags/CommandLineFlags.h"
#include "tools/testrunners/benchmark/target/BenchmarkTarget.h"

DECLARE_int(loops);
DECLARE_int(maxLoops);

void BenchmarkTarget::setup() const { fBenchmark->perCanvasPreDraw(getCanvas()); }

double BenchmarkTarget::time(int loops) const {
    SkCanvas* canvas = getCanvas();
    if (canvas) {
        canvas->clear(SK_ColorWHITE);
    }
    fBenchmark->preDraw(canvas);
    double start = nowMs();
    canvas = onBeforeDraw(canvas);

    fBenchmark->draw(loops, canvas);

    onAfterDraw();
    double elapsed = nowMs() - start;
    fBenchmark->postDraw(canvas);
    return elapsed;
}

void BenchmarkTarget::tearDown() const { fBenchmark->perCanvasPostDraw(getCanvas()); }

SkCanvas* BenchmarkTarget::getCanvas() const {
    if (!fSurfaceManager || !fSurfaceManager->getSurface()) {
        return nullptr;
    }
    return fSurfaceManager->getSurface()->getCanvas();
}

Benchmark* BenchmarkTarget::getBenchmark() const { return fBenchmark; }

double BenchmarkTarget::nowMs() const { return SkTime::GetNSecs() * 1e-6; }
