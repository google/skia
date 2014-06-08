/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkBenchmark.h"
#include "SkPaint.h"
#include "SkParse.h"

const char* SkTriState::Name[] = { "default", "true", "false" };

template BenchRegistry* BenchRegistry::gHead;

SkString SkBenchmark::gResourcePath;

SkBenchmark::SkBenchmark() {
    fForceAlpha = 0xFF;
    fForceAA = true;
    fForceFilter = false;
    fDither = SkTriState::kDefault;
    fOrMask = fClearMask = 0;
}

const char* SkBenchmark::getName() {
    return this->onGetName();
}

SkIPoint SkBenchmark::getSize() {
    return this->onGetSize();
}

void SkBenchmark::preDraw() {
    this->onPreDraw();
}

void SkBenchmark::draw(const int loops, SkCanvas* canvas) {
    this->onDraw(loops, canvas);
}

void SkBenchmark::postDraw() {
    this->onPostDraw();
}

void SkBenchmark::setupPaint(SkPaint* paint) {
    paint->setAlpha(fForceAlpha);
    paint->setAntiAlias(fForceAA);
    paint->setFilterLevel(fForceFilter ? SkPaint::kLow_FilterLevel
                                       : SkPaint::kNone_FilterLevel);

    paint->setFlags((paint->getFlags() & ~fClearMask) | fOrMask);

    if (SkTriState::kDefault != fDither) {
        paint->setDither(SkTriState::kTrue == fDither);
    }
}


///////////////////////////////////////////////////////////////////////////////

SkIPoint SkBenchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
