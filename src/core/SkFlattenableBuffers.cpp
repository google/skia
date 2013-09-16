
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkFlattenableBuffers.h"
#include "SkPaint.h"
#include "SkTypeface.h"

SkFlattenableReadBuffer::SkFlattenableReadBuffer() {
    // Set default values. These should be explicitly set by our client
    // via setFlags() if the buffer came from serialization.
    fFlags = 0;
#ifdef SK_SCALAR_IS_FLOAT
    fFlags |= kScalarIsFloat_Flag;
#endif
    if (8 == sizeof(void*)) {
        fFlags |= kPtrIs64Bit_Flag;
    }
}

SkFlattenableReadBuffer::~SkFlattenableReadBuffer() { }

void* SkFlattenableReadBuffer::readFunctionPtr() {
    void* proc;
    SkASSERT(sizeof(void*) == this->getArrayCount());
    this->readByteArray(&proc);
    return proc;
}

void SkFlattenableReadBuffer::readPaint(SkPaint* paint) {
    paint->unflatten(*this);
}

///////////////////////////////////////////////////////////////////////////////

SkFlattenableWriteBuffer::SkFlattenableWriteBuffer() {
    fFlags = (Flags)0;
}

SkFlattenableWriteBuffer::~SkFlattenableWriteBuffer() { }

void SkFlattenableWriteBuffer::writeFunctionPtr(void* ptr) {
    void* ptrStorage[] = { ptr };
    this->writeByteArray(ptrStorage, sizeof(void*));
}

void SkFlattenableWriteBuffer::writePaint(const SkPaint& paint) {
    paint.flatten(*this);
}

void SkFlattenableWriteBuffer::flattenObject(SkFlattenable* obj, SkFlattenableWriteBuffer& buffer) {
    obj->flatten(buffer);
}
