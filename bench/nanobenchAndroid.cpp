/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "nanobenchAndroid.h"

/* These functions are only compiled in the Android Framework. */

HWUITarget::HWUITarget(const Config& c, Benchmark* bench) : Target(c) { }

void HWUITarget::setup() {
    this->renderer.fence();
}

SkCanvas* HWUITarget::beginTiming(SkCanvas* canvas) {
    SkCanvas* targetCanvas = this->renderer.prepareToDraw();
    if (targetCanvas) {
        this->fc.reset(targetCanvas);
        canvas = &this->fc;
        // This might minimally distort timing, but canvas isn't valid outside the timer.
        canvas->clear(SK_ColorWHITE);
    }

    return canvas;
}

void HWUITarget::endTiming() {
    this->renderer.finishDrawing();
}

void HWUITarget::fence() {
    this->renderer.fence();
}

bool HWUITarget::needsFrameTiming(int* frameLag) const {
    extern int FLAGS_gpuFrameLag;
    *frameLag = FLAGS_gpuFrameLag;
    return true;
}

bool HWUITarget::init(SkImageInfo info, Benchmark* bench) {
    this->renderer.initialize(bench->getSize().x(), bench->getSize().y());
    return true;
}

bool HWUITarget::capturePixels(SkBitmap* bmp) {
    return this->renderer.capturePixels(bmp);
}


