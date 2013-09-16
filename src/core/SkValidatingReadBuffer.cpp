/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkErrorInternals.h"
#include "SkValidatingReadBuffer.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkValidatingReadBuffer::SkValidatingReadBuffer() : INHERITED() {
    fMemoryPtr = NULL;

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    fDecodedBitmapIndex = -1;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT

    setFlags(SkFlattenableReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::SkValidatingReadBuffer(const void* data, size_t size) : INHERITED()  {
    this->setMemory(data, size);
    fMemoryPtr = NULL;

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    fDecodedBitmapIndex = -1;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT

    setFlags(SkFlattenableReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::SkValidatingReadBuffer(SkStream* stream) {
    const size_t length = stream->getLength();
    fMemoryPtr = sk_malloc_throw(length);
    stream->read(fMemoryPtr, length);
    this->setMemory(fMemoryPtr, length);

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    fDecodedBitmapIndex = -1;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT

    setFlags(SkFlattenableReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::~SkValidatingReadBuffer() {
    sk_free(fMemoryPtr);
    SkSafeUnref(fBitmapStorage);
}

void SkValidatingReadBuffer::setMemory(const void* data, size_t size) {
    fError |= (!ptr_align_4(data) || (SkAlign4(size) != size));
    if (!fError) {
        fReader.setMemory(data, size);
    }
}

const void* SkValidatingReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    const void* addr = fReader.peek();
    fError |= !ptr_align_4(addr) || !fReader.isAvailable(inc);
    if (!fError) {
        fReader.skip(size);
    }
    return addr;
}

bool SkValidatingReadBuffer::readBool() {
    return this->readInt() != 0;
}

SkColor SkValidatingReadBuffer::readColor() {
    return this->readInt();
}

SkFixed SkValidatingReadBuffer::readFixed() {
    return this->readInt();
}

int32_t SkValidatingReadBuffer::readInt() {
    size_t inc = sizeof(int32_t);
    fError |= !ptr_align_4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : fReader.readInt();
}

SkScalar SkValidatingReadBuffer::readScalar() {
    size_t inc = sizeof(SkScalar);
    fError |= !ptr_align_4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : fReader.readScalar();
}

uint32_t SkValidatingReadBuffer::readUInt() {
    return this->readInt();
}

int32_t SkValidatingReadBuffer::read32() {
    return this->readInt();
}

void SkValidatingReadBuffer::readString(SkString* string) {
    size_t len = this->readInt();
    const void* ptr = fReader.peek();

    // skip over the string + '\0' and then pad to a multiple of 4
    size_t alignedSize = SkAlign4(len + 1);
    this->skip(alignedSize);
    if (!fError) {
        string->set((const char*)ptr, len);
    }
}

void* SkValidatingReadBuffer::readEncodedString(size_t* length, SkPaint::TextEncoding encoding) {
    int32_t encodingType = fReader.readInt();
    if (encodingType == encoding) {
        fError = true;
    }
    *length = this->readInt();
    const void* ptr = this->skip(SkAlign4(*length));
    void* data = NULL;
    if (!fError) {
        data = sk_malloc_throw(*length);
        memcpy(data, ptr, *length);
    }
    return data;
}

void SkValidatingReadBuffer::readPoint(SkPoint* point) {
    point->fX = fReader.readScalar();
    point->fY = fReader.readScalar();
}

void SkValidatingReadBuffer::readMatrix(SkMatrix* matrix) {
    size_t size = matrix->readFromMemory(fReader.peek());
    fError |= (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

void SkValidatingReadBuffer::readIRect(SkIRect* rect) {
    memcpy(rect, this->skip(sizeof(SkIRect)), sizeof(SkIRect));
}

void SkValidatingReadBuffer::readRect(SkRect* rect) {
    memcpy(rect, this->skip(sizeof(SkRect)), sizeof(SkRect));
}

void SkValidatingReadBuffer::readRegion(SkRegion* region) {
    size_t size = region->readFromMemory(fReader.peek());
    fError |= (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

void SkValidatingReadBuffer::readPath(SkPath* path) {
    size_t size = path->readFromMemory(fReader.peek());
    fError |= (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

uint32_t SkValidatingReadBuffer::readByteArray(void* value) {
    const uint32_t length = this->readUInt();
    memcpy(value, this->skip(SkAlign4(length)), length);
    return fError ? 0 : length;
}

uint32_t SkValidatingReadBuffer::readColorArray(SkColor* colors) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkColor);
    memcpy(colors, this->skip(SkAlign4(byteLength)), byteLength);
    return fError ? 0 : count;
}

uint32_t SkValidatingReadBuffer::readIntArray(int32_t* values) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(int32_t);
    memcpy(values, this->skip(SkAlign4(byteLength)), byteLength);
    return fError ? 0 : count;
}

uint32_t SkValidatingReadBuffer::readPointArray(SkPoint* points) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkPoint);
    memcpy(points, this->skip(SkAlign4(byteLength)), byteLength);
    return fError ? 0 : count;
}

uint32_t SkValidatingReadBuffer::readScalarArray(SkScalar* values) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkScalar);
    memcpy(values, this->skip(SkAlign4(byteLength)), byteLength);
    return fError ? 0 : count;
}

uint32_t SkValidatingReadBuffer::getArrayCount() {
    return *(uint32_t*)fReader.peek();
}

void SkValidatingReadBuffer::readBitmap(SkBitmap* bitmap) {
    const int width = this->readInt();
    const int height = this->readInt();
    const size_t length = this->readUInt();
    // A size of zero means the SkBitmap was simply flattened.
    if (length != 0) {
        fError = true;
    }
    if (fError) {
        return;
    }
    bitmap->unflatten(*this);
    if ((bitmap->width() != width) || (bitmap->height() != height)) {
        fError = true;
    }
}

SkTypeface* SkValidatingReadBuffer::readTypeface() {

    uint32_t index = this->readUInt();
    if (0 == index || index > (unsigned)fTFCount || fError) {
        if (index) {
            SkDebugf("====== typeface index %d\n", index);
        }
        return NULL;
    } else {
        SkASSERT(fTFArray);
        return fTFArray[index - 1];
    }
}

SkFlattenable* SkValidatingReadBuffer::readFlattenable() {
    SkString string;
    this->readString(&string);
    if (fError) {
        return NULL;
    }
    SkFlattenable::Factory factory = SkFlattenable::NameToFactory(string.c_str());
    if (NULL == factory) {
        return NULL; // writer failed to give us the flattenable
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    SkFlattenable* obj = NULL;
    uint32_t sizeRecorded = this->readUInt();
    if (factory) {
        uint32_t offset = fReader.offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        uint32_t sizeRead = fReader.offset() - offset;
        if (sizeRecorded != sizeRead) {
            // we could try to fix up the offset...
            fError = true;
            delete obj;
            obj = NULL;
        }
    } else {
        // we must skip the remaining data
        this->skip(sizeRecorded);
    }
    return obj;
}
