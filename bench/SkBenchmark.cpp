#include "SkBenchmark.h"

const char* SkBenchmark::getName() {
    return this->onGetName();
}

SkIPoint SkBenchmark::getSize() {
    return this->onGetSize();
}

void SkBenchmark::draw(SkCanvas* canvas) {
    this->onDraw(canvas);
}

