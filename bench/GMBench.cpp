/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/GMBench.h"

#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/ganesh/GrRecordingContextPriv.h"

GMBench::GMBench(std::unique_ptr<skiagm::GM> gm) : fGM(std::move(gm)) {
    fGM->setMode(skiagm::GM::kBench_Mode);

    fName.printf("GM_%s", fGM->getName().c_str());
}

const char* GMBench::onGetName() {
    return fName.c_str();
}

bool GMBench::isSuitableFor(Backend backend) {
    return kNonRendering_Backend != backend;
}

void GMBench::onPerCanvasPreDraw(SkCanvas* canvas) {
    if (fGM->gpuSetup(canvas) != skiagm::DrawResult::kOk) {
        fGpuSetupFailed = true;
    }

    fGM->onceBeforeDraw();
}

void GMBench::onPerCanvasPostDraw(SkCanvas*) {
    fGM->gpuTeardown();

    // The same GM will be reused with multiple GrContexts. Let the next GrContext start
    // afresh.
    fGpuSetupFailed = false;
}

void GMBench::onDraw(int loops, SkCanvas* canvas) {
    if (fGpuSetupFailed) {
        return;
    }

    fGM->drawBackground(canvas);
    for (int i = 0; i < loops; ++i) {
        fGM->drawContent(canvas);
    }
}

SkISize GMBench::onGetSize() {
    return fGM->getISize();
}
