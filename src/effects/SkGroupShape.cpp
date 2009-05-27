#include "SkGroupShape.h"

SkGroupShape::SkGroupShape() {}

SkGroupShape::~SkGroupShape() {
    this->removeAllShapes();
}

int SkGroupShape::countShapes() const {
    return fList.count();
}

SkShape* SkGroupShape::getShape(int index, SkMatrixRef** mr) const {
    if ((unsigned)index < (unsigned)fList.count()) {
        const Rec& rec = fList[index];
        if (mr) {
            *mr = rec.fMatrixRef;
        }
        return rec.fShape;
    }
    return NULL;
}

void SkGroupShape::addShape(int index, SkShape* shape, SkMatrixRef* mr) {
    int count = fList.count();
    if (NULL == shape || index < 0 || index > count) {
        return;
    }

    shape->ref();
    SkMatrixRef::SafeRef(mr);

    Rec* rec;
    if (index == count) {
        rec = fList.append();
    } else {
        rec = fList.insert(index);
    }
    rec->fShape = shape;
    rec->fMatrixRef = mr;
}

void SkGroupShape::removeShape(int index) {
    if ((unsigned)index < (unsigned)fList.count()) {
        Rec& rec = fList[index];
        rec.fShape->unref();
        SkMatrixRef::SafeUnref(rec.fMatrixRef);
        fList.remove(index);
    }
}

void SkGroupShape::removeAllShapes() {
    Rec* rec = fList.begin();
    Rec* stop = fList.end();
    while (rec < stop) {
        rec->fShape->unref();
        SkMatrixRef::SafeUnref(rec->fMatrixRef);
        rec++;
    }
    fList.reset();
}

///////////////////////////////////////////////////////////////////////////////

void SkGroupShape::onDraw(SkCanvas* canvas) {
    const Rec* rec = fList.begin();
    const Rec* stop = fList.end();
    while (rec < stop) {
        SkShape* shape = rec->fShape;
        if (rec->fMatrixRef) {
            shape->drawMatrix(canvas, *rec->fMatrixRef);
        } else {
            shape->draw(canvas);
        }
        rec++;
    }
}

SkFlattenable::Factory SkGroupShape::getFactory() {
    return CreateProc;
}

void SkGroupShape::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);

    int count = fList.count();
    buffer.write32(count);
    const Rec* rec = fList.begin();
    const Rec* stop = fList.end();
    while (rec < stop) {
        SkShape* shape = rec->fShape;
        buffer.writeFunctionPtr((void*)shape->getFactory());
        shape->flatten(buffer);
        // todo: flatten the matrixref if present
    }
}

SkGroupShape::SkGroupShape(SkFlattenableReadBuffer& buffer) : INHERITED(buffer){
    int count = buffer.readS32();
    for (int i = 0; i < count; i++) {
        SkFlattenable::Factory fact =
                            (SkFlattenable::Factory)buffer.readFunctionPtr();
        this->appendShape((SkShape*)fact(buffer))->unref();
        // todo: unflatten the matrixref if present
    }
}

SkFlattenable* SkGroupShape::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkGroupShape, (buffer));
}

