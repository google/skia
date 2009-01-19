#include "SkBenchmark.h"
#include "SkPaint.h"

SkBenchmark::SkBenchmark() {
    fForceAlpha = 0xFF;
    fForceAA = true;
}

const char* SkBenchmark::getName() {
    return this->onGetName();
}

SkIPoint SkBenchmark::getSize() {
    return this->onGetSize();
}

void SkBenchmark::draw(SkCanvas* canvas) {
    this->onDraw(canvas);
}

void SkBenchmark::setupPaint(SkPaint* paint) {
    paint->setAlpha(fForceAlpha);
    paint->setAntiAlias(fForceAA);
}


