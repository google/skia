
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBitmap.h"
#include "SkOrderedReadBuffer.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkOrderedReadBuffer::SkOrderedReadBuffer() : INHERITED() {
    fMemoryPtr = NULL;

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
}

SkOrderedReadBuffer::SkOrderedReadBuffer(const void* data, size_t size) : INHERITED()  {
    fReader.setMemory(data, size);
    fMemoryPtr = NULL;

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
}

SkOrderedReadBuffer::SkOrderedReadBuffer(SkStream* stream) {
    const size_t length = stream->getLength();
    fMemoryPtr = sk_malloc_throw(length);
    stream->read(fMemoryPtr, length);
    fReader.setMemory(fMemoryPtr, length);

    fBitmapStorage = NULL;
    fTFArray = NULL;
    fTFCount = 0;

    fFactoryTDArray = NULL;
    fFactoryArray = NULL;
    fFactoryCount = 0;
    fBitmapDecoder = NULL;
}

SkOrderedReadBuffer::~SkOrderedReadBuffer() {
    sk_free(fMemoryPtr);
    SkSafeUnref(fBitmapStorage);
}

bool SkOrderedReadBuffer::readBool() {
    return fReader.readBool();
}

SkColor SkOrderedReadBuffer::readColor() {
    return fReader.readInt();
}

SkFixed SkOrderedReadBuffer::readFixed() {
    return fReader.readS32();
}

int32_t SkOrderedReadBuffer::readInt() {
    return fReader.readInt();
}

SkScalar SkOrderedReadBuffer::readScalar() {
    return fReader.readScalar();
}

uint32_t SkOrderedReadBuffer::readUInt() {
    return fReader.readU32();
}

int32_t SkOrderedReadBuffer::read32() {
    return fReader.readInt();
}

char* SkOrderedReadBuffer::readString() {
    const char* string = fReader.readString();
    const size_t length = strlen(string);
    char* value = (char*)sk_malloc_throw(length + 1);
    strcpy(value, string);
    return value;
}

void* SkOrderedReadBuffer::readEncodedString(size_t* length, SkPaint::TextEncoding encoding) {
    SkDEBUGCODE(int32_t encodingType = ) fReader.readInt();
    SkASSERT(encodingType == encoding);
    *length =  fReader.readInt();
    void* data = sk_malloc_throw(*length);
    memcpy(data, fReader.skip(SkAlign4(*length)), *length);
    return data;
}

void SkOrderedReadBuffer::readPoint(SkPoint* point) {
    point->fX = fReader.readScalar();
    point->fY = fReader.readScalar();
}

void SkOrderedReadBuffer::readMatrix(SkMatrix* matrix) {
    fReader.readMatrix(matrix);
}

void SkOrderedReadBuffer::readIRect(SkIRect* rect) {
    memcpy(rect, fReader.skip(sizeof(SkIRect)), sizeof(SkIRect));
}

void SkOrderedReadBuffer::readRect(SkRect* rect) {
    memcpy(rect, fReader.skip(sizeof(SkRect)), sizeof(SkRect));
}

void SkOrderedReadBuffer::readRegion(SkRegion* region) {
    fReader.readRegion(region);
}

void SkOrderedReadBuffer::readPath(SkPath* path) {
    fReader.readPath(path);
}

uint32_t SkOrderedReadBuffer::readByteArray(void* value) {
    const uint32_t length = fReader.readU32();
    memcpy(value, fReader.skip(SkAlign4(length)), length);
    return length;
}

uint32_t SkOrderedReadBuffer::readColorArray(SkColor* colors) {
    const uint32_t count = fReader.readU32();
    const uint32_t byteLength = count * sizeof(SkColor);
    memcpy(colors, fReader.skip(SkAlign4(byteLength)), byteLength);
    return count;
}

uint32_t SkOrderedReadBuffer::readIntArray(int32_t* values) {
    const uint32_t count = fReader.readU32();
    const uint32_t byteLength = count * sizeof(int32_t);
    memcpy(values, fReader.skip(SkAlign4(byteLength)), byteLength);
    return count;
}

uint32_t SkOrderedReadBuffer::readPointArray(SkPoint* points) {
    const uint32_t count = fReader.readU32();
    const uint32_t byteLength = count * sizeof(SkPoint);
    memcpy(points, fReader.skip(SkAlign4(byteLength)), byteLength);
    return count;
}

uint32_t SkOrderedReadBuffer::readScalarArray(SkScalar* values) {
    const uint32_t count = fReader.readU32();
    const uint32_t byteLength = count * sizeof(SkScalar);
    memcpy(values, fReader.skip(SkAlign4(byteLength)), byteLength);
    return count;
}

uint32_t SkOrderedReadBuffer::getArrayCount() {
    return *(uint32_t*)fReader.peek();
}

void SkOrderedReadBuffer::readBitmap(SkBitmap* bitmap) {
    const size_t length = this->readUInt();
    if (length > 0) {
        // Bitmap was encoded.
        const void* data = this->skip(length);
        const int width = this->readInt();
        const int height = this->readInt();
        if (fBitmapDecoder != NULL && fBitmapDecoder(data, length, bitmap)) {
            SkASSERT(bitmap->width() == width && bitmap->height() == height);
        } else {
            // This bitmap was encoded when written, but we are unable to decode, possibly due to
            // not having a decoder. Use a placeholder bitmap.
            SkDebugf("Could not decode bitmap. Resulting bitmap will be red.\n");
            bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
            bitmap->allocPixels();
            bitmap->eraseColor(SK_ColorRED);
        }
    } else {
        if (fBitmapStorage) {
            const uint32_t index = fReader.readU32();
            fReader.readU32(); // bitmap generation ID (see SkOrderedWriteBuffer::writeBitmap)
            *bitmap = *fBitmapStorage->getBitmap(index);
            fBitmapStorage->releaseRef(index);
        } else {
            bitmap->unflatten(*this);
        }
    }
}

SkTypeface* SkOrderedReadBuffer::readTypeface() {

    uint32_t index = fReader.readU32();
    if (0 == index || index > (unsigned)fTFCount) {
        if (index) {
            SkDebugf("====== typeface index %d\n", index);
        }
        return NULL;
    } else {
        SkASSERT(fTFArray);
        return fTFArray[index - 1];
    }
}

SkFlattenable* SkOrderedReadBuffer::readFlattenable() {
    SkFlattenable::Factory factory = NULL;

    if (fFactoryCount > 0) {
        int32_t index = fReader.readU32();
        if (0 == index) {
            return NULL; // writer failed to give us the flattenable
        }
        index -= 1;     // we stored the index-base-1
        SkASSERT(index < fFactoryCount);
        factory = fFactoryArray[index];
    } else if (fFactoryTDArray) {
        int32_t index = fReader.readU32();
        if (0 == index) {
            return NULL; // writer failed to give us the flattenable
        }
        index -= 1;     // we stored the index-base-1
        factory = (*fFactoryTDArray)[index];
    } else {
        factory = (SkFlattenable::Factory)readFunctionPtr();
        if (NULL == factory) {
            return NULL; // writer failed to give us the flattenable
        }
    }

    // if we get here, factory may still be null, but if that is the case, the
    // failure was ours, not the writer.
    SkFlattenable* obj = NULL;
    uint32_t sizeRecorded = fReader.readU32();
    if (factory) {
        uint32_t offset = fReader.offset();
        obj = (*factory)(*this);
        // check that we read the amount we expected
        uint32_t sizeRead = fReader.offset() - offset;
        if (sizeRecorded != sizeRead) {
            // we could try to fix up the offset...
            sk_throw();
        }
    } else {
        // we must skip the remaining data
        fReader.skip(sizeRecorded);
    }
    return obj;
}
