/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkValidatingReadBuffer.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkValidatingReadBuffer::SkValidatingReadBuffer(const void* data, size_t size) :
    fError(false) {
    this->setMemory(data, size);
    this->setFlags(SkReadBuffer::kValidation_Flag);
}

SkValidatingReadBuffer::~SkValidatingReadBuffer() {
}

bool SkValidatingReadBuffer::validate(bool isValid) {
    if (!fError && !isValid) {
        // When an error is found, send the read cursor to the end of the stream
        fReader.skip(fReader.available());
        fError = true;
    }
    return !fError;
}

bool SkValidatingReadBuffer::isValid() const {
    return !fError;
}

void SkValidatingReadBuffer::setMemory(const void* data, size_t size) {
    this->validate(IsPtrAlign4(data) && (SkAlign4(size) == size));
    if (!fError) {
        fReader.setMemory(data, size);
    }
}

const void* SkValidatingReadBuffer::skip(size_t size) {
    size_t inc = SkAlign4(size);
    this->validate(inc >= size);
    const void* addr = fReader.peek();
    this->validate(IsPtrAlign4(addr) && fReader.isAvailable(inc));
    if (fError) {
        return nullptr;
    }

    fReader.skip(size);
    return addr;
}

// All the methods in this file funnel down into either readInt(), readScalar() or skip(),
// followed by a memcpy. So we've got all our validation in readInt(), readScalar() and skip();
// if they fail they'll return a zero value or skip nothing, respectively, and set fError to
// true, which the caller should check to see if an error occurred during the read operation.

bool SkValidatingReadBuffer::readBool() {
    uint32_t value = this->readInt();
    // Boolean value should be either 0 or 1
    this->validate(!(value & ~1));
    return value != 0;
}

SkColor SkValidatingReadBuffer::readColor() {
    return this->readInt();
}

int32_t SkValidatingReadBuffer::readInt() {
    const size_t inc = sizeof(int32_t);
    this->validate(IsPtrAlign4(fReader.peek()) && fReader.isAvailable(inc));
    return fError ? 0 : fReader.readInt();
}

SkScalar SkValidatingReadBuffer::readScalar() {
    const size_t inc = sizeof(SkScalar);
    this->validate(IsPtrAlign4(fReader.peek()) && fReader.isAvailable(inc));
    return fError ? 0 : fReader.readScalar();
}

uint32_t SkValidatingReadBuffer::readUInt() {
    return this->readInt();
}

int32_t SkValidatingReadBuffer::read32() {
    return this->readInt();
}

uint8_t SkValidatingReadBuffer::peekByte() {
    if (fReader.available() <= 0) {
        fError = true;
        return 0;
    }
    return *((uint8_t*) fReader.peek());
}

void SkValidatingReadBuffer::readString(SkString* string) {
    const size_t len = this->readUInt();
    const void* ptr = fReader.peek();
    const char* cptr = (const char*)ptr;

    // skip over the string + '\0' and then pad to a multiple of 4
    const size_t alignedSize = SkAlign4(len + 1);
    this->skip(alignedSize);
    if (!fError) {
        this->validate(cptr[len] == '\0');
    }
    if (!fError) {
        string->set(cptr, len);
    }
}

void SkValidatingReadBuffer::readColor4f(SkColor4f* color) {
    const void* ptr = this->skip(sizeof(SkColor4f));
    if (!fError) {
        memcpy(color, ptr, sizeof(SkColor4f));
    }
}

void SkValidatingReadBuffer::readPoint(SkPoint* point) {
    point->fX = this->readScalar();
    point->fY = this->readScalar();
}

void SkValidatingReadBuffer::readMatrix(SkMatrix* matrix) {
    size_t size = 0;
    if (!fError) {
        size = matrix->readFromMemory(fReader.peek(), fReader.available());
        this->validate((SkAlign4(size) == size) && (0 != size));
    }
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

void SkValidatingReadBuffer::readRRect(SkRRect* rrect) {
    const void* ptr = this->skip(sizeof(SkRRect));
    if (!fError) {
        memcpy(rrect, ptr, sizeof(SkRRect));
        this->validate(rrect->isValid());
    }

    if (fError) {
        rrect->setEmpty();
    }
}

void SkValidatingReadBuffer::readRegion(SkRegion* region) {
    size_t size = 0;
    if (!fError) {
        size = region->readFromMemory(fReader.peek(), fReader.available());
        this->validate((SkAlign4(size) == size) && (0 != size));
    }
    if (!fError) {
        (void)this->skip(size);
    }
}

void SkValidatingReadBuffer::readPath(SkPath* path) {
    size_t size = 0;
    if (!fError) {
        size = path->readFromMemory(fReader.peek(), fReader.available());
        this->validate((SkAlign4(size) == size) && (0 != size));
    }
    if (!fError) {
        (void)this->skip(size);
    }
}

bool SkValidatingReadBuffer::readArray(void* value, size_t size, size_t elementSize) {
    const uint32_t count = this->getArrayCount();
    this->validate(size == count);
    (void)this->skip(sizeof(uint32_t)); // Skip array count
    const uint64_t byteLength64 = sk_64_mul(count, elementSize);
    const size_t byteLength = count * elementSize;
    this->validate(byteLength == byteLength64);
    const void* ptr = this->skip(SkAlign4(byteLength));
    if (!fError) {
        memcpy(value, ptr, byteLength);
        return true;
    }
    return false;
}

bool SkValidatingReadBuffer::readByteArray(void* value, size_t size) {
    return this->readArray(static_cast<unsigned char*>(value), size, sizeof(unsigned char));
}

bool SkValidatingReadBuffer::readColorArray(SkColor* colors, size_t size) {
    return this->readArray(colors, size, sizeof(SkColor));
}

bool SkValidatingReadBuffer::readColor4fArray(SkColor4f* colors, size_t size) {
    return this->readArray(colors, size, sizeof(SkColor4f));
}

bool SkValidatingReadBuffer::readIntArray(int32_t* values, size_t size) {
    return this->readArray(values, size, sizeof(int32_t));
}

bool SkValidatingReadBuffer::readPointArray(SkPoint* points, size_t size) {
    return this->readArray(points, size, sizeof(SkPoint));
}

bool SkValidatingReadBuffer::readScalarArray(SkScalar* values, size_t size) {
    return this->readArray(values, size, sizeof(SkScalar));
}

uint32_t SkValidatingReadBuffer::getArrayCount() {
    const size_t inc = sizeof(uint32_t);
    fError = fError || !IsPtrAlign4(fReader.peek()) || !fReader.isAvailable(inc);
    return fError ? 0 : *(uint32_t*)fReader.peek();
}

bool SkValidatingReadBuffer::validateAvailable(size_t size) {
    return this->validate((size <= SK_MaxU32) && fReader.isAvailable(static_cast<uint32_t>(size)));
}

SkFlattenable* SkValidatingReadBuffer::readFlattenable(SkFlattenable::Type type) {
    // The validating read buffer always uses strings and string-indices for unflattening.
    SkASSERT(0 == this->factoryCount());

    uint8_t firstByte = this->peekByte();
    if (fError) {
        return nullptr;
    }

    SkString name;
    if (firstByte) {
        // If the first byte is non-zero, the flattenable is specified by a string.
        this->readString(&name);
        if (fError) {
            return nullptr;
        }

        // Add the string to the dictionary.
        fFlattenableDict.set(fFlattenableDict.count() + 1, name);
    } else {
        // Read the index.  We are guaranteed that the first byte
        // is zeroed, so we must shift down a byte.
        uint32_t index = this->readUInt() >> 8;
        if (0 == index) {
            return nullptr; // writer failed to give us the flattenable
        }

        SkString* namePtr = fFlattenableDict.find(index);
        if (!namePtr) {
            return nullptr;
        }
        name = *namePtr;
    }

    // Is this the type we wanted ?
    const char* cname = name.c_str();
    SkFlattenable::Type baseType;
    if (!SkFlattenable::NameToType(cname, &baseType) || (baseType != type)) {
        return nullptr;
    }

    // Get the factory for this flattenable.
    SkFlattenable::Factory factory = this->getCustomFactory(name);
    if (!factory) {
        factory = SkFlattenable::NameToFactory(cname);
        if (!factory) {
            return nullptr; // writer failed to give us the flattenable
        }
    }

    // If we get here, the factory is non-null.
    sk_sp<SkFlattenable> obj;
    uint32_t sizeRecorded = this->readUInt();
    size_t offset = fReader.offset();
    obj = (*factory)(*this);
    // check that we read the amount we expected
    size_t sizeRead = fReader.offset() - offset;
    this->validate(sizeRecorded == sizeRead);
    if (fError) {
        obj = nullptr;
    }
    return obj.release();
}
