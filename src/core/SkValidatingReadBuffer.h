/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkValidatingReadBuffer_DEFINED
#define SkValidatingReadBuffer_DEFINED

#include "SkRefCnt.h"
#include "SkBitmapHeap.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkReader32.h"

class SkBitmap;

class SkValidatingReadBuffer : public SkReadBuffer {
public:
    SkValidatingReadBuffer(const void* data, size_t size);
    virtual ~SkValidatingReadBuffer();

    const void* skip(size_t size) SK_OVERRIDE;

    // primitives
    bool readBool() SK_OVERRIDE;
    SkColor readColor() SK_OVERRIDE;
    SkFixed readFixed() SK_OVERRIDE;
    int32_t readInt() SK_OVERRIDE;
    SkScalar readScalar() SK_OVERRIDE;
    uint32_t readUInt() SK_OVERRIDE;
    int32_t read32() SK_OVERRIDE;

    // strings -- the caller is responsible for freeing the string contents
    void readString(SkString* string) SK_OVERRIDE;
    void* readEncodedString(size_t* length, SkPaint::TextEncoding encoding) SK_OVERRIDE;

    // common data structures
    SkFlattenable* readFlattenable(SkFlattenable::Type type) SK_OVERRIDE;
    void skipFlattenable() SK_OVERRIDE;
    void readPoint(SkPoint* point) SK_OVERRIDE;
    void readMatrix(SkMatrix* matrix) SK_OVERRIDE;
    void readIRect(SkIRect* rect) SK_OVERRIDE;
    void readRect(SkRect* rect) SK_OVERRIDE;
    void readRegion(SkRegion* region) SK_OVERRIDE;
    void readPath(SkPath* path) SK_OVERRIDE;

    // binary data and arrays
    bool readByteArray(void* value, size_t size) SK_OVERRIDE;
    bool readColorArray(SkColor* colors, size_t size) SK_OVERRIDE;
    bool readIntArray(int32_t* values, size_t size) SK_OVERRIDE;
    bool readPointArray(SkPoint* points, size_t size) SK_OVERRIDE;
    bool readScalarArray(SkScalar* values, size_t size) SK_OVERRIDE;

    // helpers to get info about arrays and binary data
    uint32_t getArrayCount() SK_OVERRIDE;

    // TODO: Implement this (securely) when needed
    SkTypeface* readTypeface() SK_OVERRIDE;

    bool validate(bool isValid) SK_OVERRIDE;
    bool isValid() const SK_OVERRIDE;

    bool validateAvailable(size_t size) SK_OVERRIDE;

private:
    bool readArray(void* value, size_t size, size_t elementSize);

    void setMemory(const void* data, size_t size);

    static bool IsPtrAlign4(const void* ptr) {
        return SkIsAlign4((uintptr_t)ptr);
    }

    bool fError;

    typedef SkReadBuffer INHERITED;
};

#endif // SkValidatingReadBuffer_DEFINED
