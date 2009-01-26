#include "SkBenchmark.h"
#include "SkPaint.h"

template BenchRegistry* BenchRegistry::gHead;

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

///////////////////////////////////////////////////////////////////////////////

SkIPoint SkBenchmark::onGetSize() {
    return SkMakeIPoint(640, 480);
}
