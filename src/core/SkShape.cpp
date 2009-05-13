#include "SkCanvas.h"
#include "SkShape.h"
#include "SkMatrix.h"

SkShape::~SkShape() {
    if (fMatrix) {
        SkDELETE(fMatrix);
    }
}

void SkShape::getMatrix(SkMatrix* matrix) const {
    if (matrix) {
        if (fMatrix) {
            *matrix = *fMatrix;
        } else {
            matrix->reset();
        }
    }
}

void SkShape::setMatrix(const SkMatrix& matrix) {
    if (matrix.isIdentity()) {
        this->resetMatrix();
    } else {
        if (NULL == fMatrix) {
            fMatrix = SkNEW(SkMatrix);
        }
        *fMatrix = matrix;
    }
}

void SkShape::resetMatrix() {
    if (fMatrix) {
        SkDELETE(fMatrix);
        fMatrix = NULL;
    }
}

void SkShape::draw(SkCanvas* canvas) {
    int saveCount = canvas->getSaveCount();
    if (fMatrix) {
        canvas->save(SkCanvas::kMatrix_SaveFlag);
        canvas->concat(*fMatrix);
    }
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

void SkShape::drawXY(SkCanvas* canvas, SkScalar dx, SkScalar dy) {
    int saveCount = canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->translate(dx, dy);
    if (fMatrix) {
        canvas->concat(*fMatrix);
    }
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

void SkShape::drawMatrix(SkCanvas* canvas, const SkMatrix& matrix) {
    int saveCount = canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->concat(matrix);
    if (fMatrix) {
        canvas->concat(*fMatrix);
    }
    this->onDraw(canvas);
    canvas->restoreToCount(saveCount);
}

///////////////////////////////////////////////////////////////////////////////

void SkShape::flatten(SkFlattenableWriteBuffer& buffer) {
    buffer.writeBool(fMatrix != NULL);
    if (fMatrix) {
        *(SkMatrix*)buffer.reserve(sizeof(SkMatrix)) = *fMatrix;
    }
}

SkShape::SkShape(SkFlattenableReadBuffer& buffer) {
    fMatrix = NULL;
    if (buffer.readBool()) {
        fMatrix = SkNEW(SkMatrix);
        buffer.read(fMatrix, sizeof(*fMatrix));
    }
}

