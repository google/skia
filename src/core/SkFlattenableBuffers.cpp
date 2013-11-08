
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkFlattenableBuffers.h"
#include "SkPaint.h"
#include "SkTypeface.h"

#include "SkColorFilter.h"
#include "SkDrawLooper.h"
#include "SkImageFilter.h"
#include "SkMaskFilter.h"
#include "SkPathEffect.h"
#include "SkPixelRef.h"
#include "SkRasterizer.h"
#include "SkShader.h"
#include "SkUnitMapper.h"
#include "SkXfermode.h"

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
    this->readByteArray(&proc, sizeof(void*));
    return proc;
}

void SkFlattenableReadBuffer::readPaint(SkPaint* paint) {
    paint->unflatten(*this);
}

template <typename T> T* SkFlattenableReadBuffer::readFlattenableT() {
    return static_cast<T*>(this->readFlattenable(T::GetFlattenableType()));
}

SkColorFilter* SkFlattenableReadBuffer::readColorFilter() {
    return this->readFlattenableT<SkColorFilter>();
}

SkDrawLooper* SkFlattenableReadBuffer::readDrawLooper() {
    return this->readFlattenableT<SkDrawLooper>();
}

SkImageFilter* SkFlattenableReadBuffer::readImageFilter() {
    return this->readFlattenableT<SkImageFilter>();
}

SkMaskFilter* SkFlattenableReadBuffer::readMaskFilter() {
    return this->readFlattenableT<SkMaskFilter>();
}

SkPathEffect* SkFlattenableReadBuffer::readPathEffect() {
    return this->readFlattenableT<SkPathEffect>();
}

SkPixelRef* SkFlattenableReadBuffer::readPixelRef() {
    return this->readFlattenableT<SkPixelRef>();
}

SkRasterizer* SkFlattenableReadBuffer::readRasterizer() {
    return this->readFlattenableT<SkRasterizer>();
}

SkShader* SkFlattenableReadBuffer::readShader() {
    return this->readFlattenableT<SkShader>();
}

SkUnitMapper* SkFlattenableReadBuffer::readUnitMapper() {
    return this->readFlattenableT<SkUnitMapper>();
}

SkXfermode* SkFlattenableReadBuffer::readXfermode() {
    return this->readFlattenableT<SkXfermode>();
}

bool SkFlattenableReadBuffer::validate(bool isValid) {
    return true;
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

void SkFlattenableWriteBuffer::flattenObject(const SkFlattenable* obj,
                                             SkFlattenableWriteBuffer& buffer) {
    obj->flatten(buffer);
}
