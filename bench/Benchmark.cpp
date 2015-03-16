/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Benchmark.h"

#include "SkCanvas.h"
#include "SkPaint.h"
#include "SkParse.h"

const char* SkTriState::Name[] = { "default", "true", "false" };

template BenchRegistry* BenchRegistry::gHead;

Benchmark::Benchmark() {
    fForceAlpha = 0xFF;
    fDither = SkTriState::kDefault;
    fOrMask = fClearMask = 0;
}

const char* Benchmark::getName() {
    return this->onGetName();
}

const char* Benchmark::getUniqueName() {
    return this->onGetUniqueName();
}

SkIPoint Benchmark::getSize() {
    return this->onGetSize();
}

void Benchmark::preDraw() {
    this->onPreDraw();
}

void Benchmark::perCanvasPreDraw(SkCanvas* canvas) {
    this->onPerCanvasPreDraw(canvas);
}

void Benchmark::perCanvasPostDraw(SkCanvas* canvas) {
    this->onPerCanvasPostDraw(canvas);
}

void Benchmark::draw(const int loops, SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, true/*save now*/);
    this->onDraw(loops, canvas);
}

void Benchmark::setupPaint(SkPaint* paint) {
    paint->setAlpha(fForceAlpha);
    paint->setAntiAlias(true);
    paint->setFilterQuality(kNone_SkFilterQuality);

    paint->setFlags((paint->getFlags() & ~fClearMask) | fOrMask);

    if (SkTriState::kDefault != fDither) {
        paint->setDither(SkTriState::kTrue == fDither);
    }
}

SkIPoint Benchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
