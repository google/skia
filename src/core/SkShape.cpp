#include "SkCanvas.h"
#include "SkShape.h"
#include "SkMatrix.h"

void SkShape::draw(SkCanvas* canvas) {
    int saveCount = canvas->getSaveCount();
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

void SkShape::drawXY(SkCanvas* canvas, SkScalar dx, SkScalar dy) {
    int saveCount = canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->translate(dx, dy);
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

void SkShape::drawMatrix(SkCanvas* canvas, const SkMatrix& matrix) {
    int saveCount = canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->concat(matrix);
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

///////////////////////////////////////////////////////////////////////////////

void SkShape::flatten(SkFlattenableWriteBuffer& buffer) {}

