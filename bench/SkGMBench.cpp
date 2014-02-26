/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkGMBench.h"

SkGMBench::SkGMBench(skiagm::GM* gm) : fGM(gm) {
    fName.printf("GM:%s", gm->getName());
}

SkGMBench::~SkGMBench() { delete fGM; }

const char* SkGMBench::onGetName() {
    return fName.c_str();
}

bool SkGMBench::isSuitableFor(Backend backend) {
    uint32_t flags = fGM->getFlags();
    switch (backend) {
        case kGPU_Backend:
            return !(skiagm::GM::kSkipGPU_Flag & flags);
        case kPDF_Backend:
            return !(skiagm::GM::kSkipPDF_Flag & flags);
        case kRaster_Backend:
            // GM doesn't have an equivalent flag. If the GM has known issues with 565 then
            // we skip it for ALL raster configs in bench.
            return !(skiagm::GM::kSkip565_Flag & flags);
        case kNonRendering_Backend:
            return false;
        default:
            SkDEBUGFAIL("Unexpected backend type.");
            return false;
    }
}

void SkGMBench::onDraw(const int loops, SkCanvas* canvas) {
    // Do we care about timing the draw of the background (once)?
    // Does the GM ever rely on drawBackground to lazily compute something?
    fGM->drawBackground(canvas);
    for (int i = 0; i < loops; ++i) {
        fGM->drawContent(canvas);
    }
}

SkIPoint SkGMBench::onGetSize() {
    SkISize size = fGM->getISize();
    return SkIPoint::Make(size.fWidth, size.fHeight);
}
