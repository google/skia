#include "SkGroupShape.h"

SkGroupShape::SkGroupShape() {}

SkGroupShape::~SkGroupShape() {
    this->removeAllShapes();
}

int SkGroupShape::countShapes() const {
    return fList.count();
}

SkShape* SkGroupShape::getShape(int index) const {
    if ((unsigned)index < (unsigned)fList.count()) {
        return fList[index];
    }
    return NULL;
}

SkShape* SkGroupShape::addShape(int index, SkShape* shape) {
    int count = fList.count();
    if (NULL == shape || index < 0 || index > count) {
        return shape;
    }

    shape->ref();
    SkShape** spot;
    if (index == count) {
        spot = fList.append();
    } else {
        spot = fList.insert(index);
    }
    *spot = shape;
    return shape;
}

void SkGroupShape::removeShape(int index) {
    if ((unsigned)index < (unsigned)fList.count()) {
        fList[index]->unref();
        fList.remove(index);
    }
}

void SkGroupShape::removeAllShapes() {
    fList.unrefAll();
    fList.reset();
}

///////////////////////////////////////////////////////////////////////////////

void SkGroupShape::onDraw(SkCanvas* canvas) {
    SkShape** iter = fList.begin();
    SkShape** stop = fList.end();
    while (iter < stop) {
        (*iter)->draw(canvas);
        iter++;
    }
}

SkFlattenable::Factory SkGroupShape::getFactory() {
    return CreateProc;
}

void SkGroupShape::flatten(SkFlattenableWriteBuffer& buffer) {
    this->INHERITED::flatten(buffer);

    int count = fList.count();
    buffer.write32(count);
    for (int i = 0; i < count; i++) {
        buffer.writeFunctionPtr((void*)fList[i]->getFactory());
        fList[i]->flatten(buffer);
    }
}

SkGroupShape::SkGroupShape(SkFlattenableReadBuffer& buffer) : INHERITED(buffer){
    int count = buffer.readS32();
    for (int i = 0; i < count; i++) {
        SkFlattenable::Factory fact =
                            (SkFlattenable::Factory)buffer.readFunctionPtr();
        this->appendShape((SkShape*)fact(buffer))->unref();
    }
}

SkFlattenable* SkGroupShape::CreateProc(SkFlattenableReadBuffer& buffer) {
    return SkNEW_ARGS(SkGroupShape, (buffer));
}

