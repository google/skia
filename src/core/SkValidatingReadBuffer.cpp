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

SkValidatingReadBuffer::SkValidatingReadBuffer(const void* data, size_t size) {
    this->setMemory(data, size);
    fError = false;

    this->setFlags(SkFlattenableReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::~SkValidatingReadBuffer() {
}

void SkValidatingReadBuffer::setMemory(const void* data, size_t size) {
    fError = fError || !IsPtrAlign4(data) || (SkAlign4(size) != size);
    if (!fError) {
        fReader.setMemory(data, size);
    }
}

const void* SkValidatingReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    const void* addr = fReader.peek();
    fError = fError || !IsPtrAlign4(addr) || !fReader.isAvailable(inc);
    if (!fError) {
        fReader.skip(size);
    }
    return addr;
}

// All the methods in this file funnel down into either readInt(), readScalar() or skip(),
// followed by a memcpy. So we've got all our validation in readInt(), readScalar() and skip();
// if they fail they'll return a zero value or skip nothing, respectively, and set fError to
// true, which the caller should check to see if an error occurred during the read operation.

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
    const size_t inc = sizeof(int32_t);
    fError = fError || !IsPtrAlign4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : fReader.readInt();
}

SkScalar SkValidatingReadBuffer::readScalar() {
    const size_t inc = sizeof(SkScalar);
    fError = fError || !IsPtrAlign4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : fReader.readScalar();
}

uint32_t SkValidatingReadBuffer::readUInt() {
    return this->readInt();
}

int32_t SkValidatingReadBuffer::read32() {
    return this->readInt();
}

void SkValidatingReadBuffer::readString(SkString* string) {
    const size_t len = this->readInt();
    const void* ptr = fReader.peek();
    const char* cptr = (const char*)ptr;

    // skip over the string + '\0' and then pad to a multiple of 4
    const size_t alignedSize = SkAlign4(len + 1);
    this->skip(alignedSize);
    fError = fError || (cptr[len] != '\0');
    if (!fError) {
        string->set(cptr, len);
    }
}

void* SkValidatingReadBuffer::readEncodedString(size_t* length, SkPaint::TextEncoding encoding) {
    const int32_t encodingType = fReader.readInt();
    fError = fError || (encodingType != encoding);
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
    const size_t size = matrix->readFromMemory(fReader.peek());
    fError = fError || (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

void SkValidatingReadBuffer::readIRect(SkIRect* rect) {
    const void* ptr = this->skip(sizeof(SkIRect));
    if (!fError) {
        memcpy(rect, ptr, sizeof(SkIRect));
    }
}

void SkValidatingReadBuffer::readRect(SkRect* rect) {
    const void* ptr = this->skip(sizeof(SkRect));
    if (!fError) {
        memcpy(rect, ptr, sizeof(SkRect));
    }
}

void SkValidatingReadBuffer::readRegion(SkRegion* region) {
    const size_t size = region->readFromMemory(fReader.peek());
    fError = fError || (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

void SkValidatingReadBuffer::readPath(SkPath* path) {
    const size_t size = path->readFromMemory(fReader.peek());
    fError = fError || (SkAlign4(size) != size);
    if (!fError) {
        (void)this->skip(size);
    }
}

uint32_t SkValidatingReadBuffer::readByteArray(void* value) {
    const uint32_t length = this->readUInt();
    const void* ptr = this->skip(SkAlign4(length));
    if (!fError) {
        memcpy(value, ptr, length);
        return length;
    }
    return 0;
}

uint32_t SkValidatingReadBuffer::readColorArray(SkColor* colors) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkColor);
    const void* ptr = this->skip(SkAlign4(byteLength));
    if (!fError) {
        memcpy(colors, ptr, byteLength);
        return count;
    }
    return 0;
}

uint32_t SkValidatingReadBuffer::readIntArray(int32_t* values) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(int32_t);
    const void* ptr = this->skip(SkAlign4(byteLength));
    if (!fError) {
        memcpy(values, ptr, byteLength);
        return count;
    }
    return 0;
}

uint32_t SkValidatingReadBuffer::readPointArray(SkPoint* points) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkPoint);
    const void* ptr = this->skip(SkAlign4(byteLength));
    if (!fError) {
        memcpy(points, ptr, byteLength);
        return count;
    }
    return 0;
}

uint32_t SkValidatingReadBuffer::readScalarArray(SkScalar* values) {
    const uint32_t count = this->readUInt();
    const uint32_t byteLength = count * sizeof(SkScalar);
    const void* ptr = this->skip(SkAlign4(byteLength));
    if (!fError) {
        memcpy(values, ptr, byteLength);
        return count;
    }
    return 0;
}

uint32_t SkValidatingReadBuffer::getArrayCount() {
    return *(uint32_t*)fReader.peek();
}

void SkValidatingReadBuffer::readBitmap(SkBitmap* bitmap) {
    const int width = this->readInt();
    const int height = this->readInt();
    const size_t length = this->readUInt();
    // A size of zero means the SkBitmap was simply flattened.
    fError = fError || (length != 0);
    if (fError) {
        return;
    }
    bitmap->unflatten(*this);
    fError = fError || (bitmap->width() != width) || (bitmap->height() != height);
}

SkFlattenable* SkValidatingReadBuffer::readFlattenable(SkFlattenable::Type type) {
    SkString name;
    this->readString(&name);
    if (fError) {
        return NULL;
    }

    // Is this the type we wanted ?
    const char* cname = name.c_str();
    SkFlattenable::Type baseType;
    if (!SkFlattenable::NameToType(cname, &baseType) || (baseType != type)) {
        return NULL;
    }

    SkFlattenable::Factory factory = SkFlattenable::NameToFactory(cname);
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
        fError = fError || (sizeRecorded != sizeRead);
        if (fError) {
            // we could try to fix up the offset...
            delete obj;
            obj = NULL;
        }
    } else {
        // we must skip the remaining data
        this->skip(sizeRecorded);
        SkASSERT(false);
    }
    return obj;
}
