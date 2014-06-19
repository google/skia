/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkPaint.h"
#include "SkParse.h"

const char* SkTriState::Name[] = { "default", "true", "false" };

template BenchRegistry* BenchRegistry::gHead;

Benchmark::Benchmark() {
    fForceAlpha = 0xFF;
    fForceAA = true;
    fForceFilter = false;
    fDither = SkTriState::kDefault;
    fOrMask = fClearMask = 0;
}

const char* Benchmark::getName() {
    return this->onGetName();
}

SkIPoint Benchmark::getSize() {
    return this->onGetSize();
}

void Benchmark::preDraw() {
    this->onPreDraw();
}

void Benchmark::draw(const int loops, SkCanvas* canvas) {
    this->onDraw(loops, canvas);
}

void Benchmark::setupPaint(SkPaint* paint) {
    paint->setAlpha(fForceAlpha);
    paint->setAntiAlias(fForceAA);
    paint->setFilterLevel(fForceFilter ? SkPaint::kLow_FilterLevel
                                       : SkPaint::kNone_FilterLevel);

    paint->setFlags((paint->getFlags() & ~fClearMask) | fOrMask);

    if (SkTriState::kDefault != fDither) {
        paint->setDither(SkTriState::kTrue == fDither);
    }
}

SkIPoint Benchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
