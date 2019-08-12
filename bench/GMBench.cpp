/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GMBench.h"

GMBench::GMBench(std::unique_ptr<skiagm::GM> gm) : fGM(std::move(gm)) {
    fName.printf("GM_%s", fGM->getName());
}

const char* GMBench::onGetName() {
    return fName.c_str();
}

bool GMBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend != backend;
}

void GMBench::onDraw(int loops, SkCanvas* canvas) {
    fGM->setMode(skiagm::GM::kBench_Mode);
    // Do we care about timing the draw of the background (once)?
    // Does the GM ever rely on drawBackground to lazily compute something?
    fGM->drawBackground(canvas);
    for (int i = 0; i < loops; ++i) {
        SkAutoCanvasRestore acr(canvas, true);
        fGM->drawContent(canvas);
    }
}

SkIPoint GMBench::onGetSize() {
    SkISize size = fGM->getISize();
    return SkIPoint::Make(size.fWidth, size.fHeight);
}
