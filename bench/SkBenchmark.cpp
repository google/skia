
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

SK_DEFINE_INST_COUNT(SkBenchmark)

template BenchRegistry* BenchRegistry::gHead;

SkBenchmark::SkBenchmark() {
    fForceAlpha = 0xFF;
    fForceAA = true;
    fDither = SkTriState::kDefault;
    fIsRendering = true;
    fOrMask = fClearMask = 0;
    fLoops = 1;
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

void SkBenchmark::draw(SkCanvas* canvas) {
    this->onDraw(canvas);
}

void SkBenchmark::postDraw() {
    this->onPostDraw();
}

void SkBenchmark::setupPaint(SkPaint* paint) {
    paint->setAlpha(fForceAlpha);
    paint->setAntiAlias(fForceAA);
    paint->setFilterBitmap(fForceFilter);

    paint->setFlags((paint->getFlags() & ~fClearMask) | fOrMask);

    if (SkTriState::kDefault != fDither) {
        paint->setDither(SkTriState::kTrue == fDither);
    }
}


///////////////////////////////////////////////////////////////////////////////

SkIPoint SkBenchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
