#include "SkBenchmark.h"
#include "SkPaint.h"
#include "SkParse.h"

template BenchRegistry* BenchRegistry::gHead;

SkBenchmark::SkBenchmark(void* defineDict) {
    fDict = reinterpret_cast<const SkTDict<const char*>*>(defineDict);
    fForceAlpha = 0xFF;
    fForceAA = true;
    fDither = SkTriState::kDefault;
    fHasStrokeWidth = false;
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

    if (SkTriState::kDefault != fDither) {
        paint->setDither(SkTriState::kTrue == fDither);
    }
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

bool SkBenchmark::findDefine32(const char* key, int32_t* value) const {
    const char* valueStr = this->findDefine(key);
    if (valueStr) {
        SkParse::FindS32(valueStr, value);
        return true;
    }
    return false;
}

bool SkBenchmark::findDefineScalar(const char* key, SkScalar* value) const {
    const char* valueStr = this->findDefine(key);
    if (valueStr) {
        SkParse::FindScalar(valueStr, value);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

SkIPoint SkBenchmark::onGetSize() {
    return SkIPoint::Make(640, 480);
}
