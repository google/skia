/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GMBench.h"

GMBench::GMBench(std::unique_ptr<skiagm::GM> gm) : fGM(std::move(gm)) {
    fGM->setMode(skiagm::GM::kBench_Mode);

    fName.printf("GM_%s", fGM->getName());
}

const char* GMBench::onGetName() {
    return fName.c_str();
}

bool GMBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend != backend;
}

void GMBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    fGM->onceBeforeDraw();
}

void GMBench::onPerCanvasPostDraw(SkCanvas*) {}

void GMBench::onDraw(int loops, SkCanvas* canvas) {
    fGM->drawBackground(canvas);
    for (int i = 0; i < loops; ++i) {
        fGM->drawContent(canvas);
    }
}

SkIPoint GMBench::onGetSize() {
    SkISize size = fGM->getISize();
    return SkIPoint::Make(size.fWidth, size.fHeight);
}
