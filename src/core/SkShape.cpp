#include "SkCanvas.h"
#include "SkShape.h"
#include "SkMatrix.h"

#if 0
static int gShapeCounter;
static void inc_shape(const SkShape* s) {
    SkDebugf("inc %d\n", gShapeCounter);
    gShapeCounter += 1;
}
static void dec_shape(const SkShape* s) {
    --gShapeCounter;
    SkDebugf("dec %d\n", gShapeCounter);
}
#else
#define inc_shape(s)
#define dec_shape(s)
#endif

///////////////////////////////////////////////////////////////////////////////

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

SkShape::SkShape() {
    inc_shape(this);
}

SkShape::~SkShape() {
    dec_shape(this);
}

SkShape::SkShape(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    inc_shape(this);
}

SkFlattenable* SkShape::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkShape, (buffer));
}

SkFlattenable::Factory SkShape::getFactory() {
    return CreateProc;
}

void SkShape::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);
}

void SkShape::onDraw(SkCanvas*) {}

static SkFlattenable::Registrar gReg("SkShape", SkShape::CreateProc);
