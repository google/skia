/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "bench/Benchmark.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/utils/SkParse.h"

template BenchRegistry* BenchRegistry::gHead;

Benchmark::Benchmark() {}

const char* Benchmark::getName() {
    return this->onGetName();
}

const char* Benchmark::getUniqueName() {
    return this->onGetUniqueName();
}

SkIPoint Benchmark::getSize() {
    return this->onGetSize();
}

void Benchmark::delayedSetup() {
    this->onDelayedSetup();
}

void Benchmark::perCanvasPreDraw(SkCanvas* canvas) {
    this->onPerCanvasPreDraw(canvas);
}

void Benchmark::preDraw(SkCanvas* canvas) {
    this->onPreDraw(canvas);
}

void Benchmark::postDraw(SkCanvas* canvas) {
    this->onPostDraw(canvas);
}

void Benchmark::perCanvasPostDraw(SkCanvas* canvas) {
    this->onPerCanvasPostDraw(canvas);
}

void Benchmark::draw(int loops, SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, true/*save now*/);
    this->onDraw(loops, canvas);
}

void Benchmark::setupPaint(SkPaint* paint) {
    paint->setAntiAlias(true);
}

SkIPoint Benchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
