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
#include "SkFlattenableBuffers.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkReader32.h"

class SkBitmap;

class SkValidatingReadBuffer : public SkFlattenableReadBuffer {
public:
    SkValidatingReadBuffer(const void* data, size_t size);
    virtual ~SkValidatingReadBuffer();

    const void* skip(size_t size);

    // primitives
    virtual bool readBool() SK_OVERRIDE;
    virtual SkColor readColor() SK_OVERRIDE;
    virtual SkFixed readFixed() SK_OVERRIDE;
    virtual int32_t readInt() SK_OVERRIDE;
    virtual SkScalar readScalar() SK_OVERRIDE;
    virtual uint32_t readUInt() SK_OVERRIDE;
    virtual int32_t read32() SK_OVERRIDE;

    // strings -- the caller is responsible for freeing the string contents
    virtual void readString(SkString* string) SK_OVERRIDE;
    virtual void* readEncodedString(size_t* length, SkPaint::TextEncoding encoding) SK_OVERRIDE;

    // common data structures
    virtual SkFlattenable* readFlattenable(SkFlattenable::Type type) SK_OVERRIDE;
    virtual void readPoint(SkPoint* point) SK_OVERRIDE;
    virtual void readMatrix(SkMatrix* matrix) SK_OVERRIDE;
    virtual void readIRect(SkIRect* rect) SK_OVERRIDE;
    virtual void readRect(SkRect* rect) SK_OVERRIDE;
    virtual void readRegion(SkRegion* region) SK_OVERRIDE;
    virtual void readPath(SkPath* path) SK_OVERRIDE;

    // binary data and arrays
    virtual bool readByteArray(void* value, size_t size) SK_OVERRIDE;
    virtual bool readColorArray(SkColor* colors, size_t size) SK_OVERRIDE;
    virtual bool readIntArray(int32_t* values, size_t size) SK_OVERRIDE;
    virtual bool readPointArray(SkPoint* points, size_t size) SK_OVERRIDE;
    virtual bool readScalarArray(SkScalar* values, size_t size) SK_OVERRIDE;

    // helpers to get info about arrays and binary data
    virtual uint32_t getArrayCount() SK_OVERRIDE;

    virtual void readBitmap(SkBitmap* bitmap) SK_OVERRIDE;
    // TODO: Implement this (securely) when needed
    virtual SkTypeface* readTypeface() SK_OVERRIDE;

    virtual bool validate(bool isValid) SK_OVERRIDE;
    virtual bool isValid() const SK_OVERRIDE;

private:
    bool readArray(void* value, size_t size, size_t elementSize);

    void setMemory(const void* data, size_t size);

    static bool IsPtrAlign4(const void* ptr) {
        return SkIsAlign4((uintptr_t)ptr);
    }

    SkReader32 fReader;
    bool fError;

    typedef SkFlattenableReadBuffer INHERITED;
};

#endif // SkValidatingReadBuffer_DEFINED
