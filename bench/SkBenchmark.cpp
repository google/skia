#include "SkBenchmark.h"
#include "SkPaint.h"

template BenchRegistry* BenchRegistry::gHead;

SkBenchmark::SkBenchmark(void* defineDict) {
    fDict = reinterpret_cast<const SkTDict<const char*>*>(defineDict);
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
    paint->setFilterBitmap(fForceFilter);
}

const char* SkBenchmark::findDefine(const char* key) const {
    if (fDict) {
        const char* value;
        if (fDict->find(key, &value)) {
            return value;
        }
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////

SkIPoint SkBenchmark::onGetSize() {
    return SkMakeIPoint(640, 480);
}
