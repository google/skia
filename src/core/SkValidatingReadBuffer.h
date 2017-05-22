/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkValidatingReadBuffer_DEFINED
#define SkValidatingReadBuffer_DEFINED

#include "SkRefCnt.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkReader32.h"

class SkBitmap;

class SkValidatingReadBuffer : public SkReadBuffer {
public:
    SkValidatingReadBuffer(const void* data, size_t size);
    ~SkValidatingReadBuffer() override;

    SkReadBuffer* clone(const void* data, size_t size) const override {
        return new SkValidatingReadBuffer(data, size);
    }

    const void* skip(size_t size) override;

    // primitives
    bool readBool() override;
    SkColor readColor() override;
    int32_t readInt() override;
    SkScalar readScalar() override;
    uint32_t readUInt() override;
    int32_t read32() override;

    // peek
    uint8_t peekByte() override;

    // strings -- the caller is responsible for freeing the string contents
    void readString(SkString* string) override;

    // common data structures
    SkFlattenable* readFlattenable(SkFlattenable::Type type) override;
    void readColor4f(SkColor4f* color) override;
    void readPoint(SkPoint* point) override;
    void readPoint3(SkPoint3* point) override;
    void readMatrix(SkMatrix* matrix) override;
    void readIRect(SkIRect* rect) override;
    void readRect(SkRect* rect) override;
    void readRRect(SkRRect* rrect) override;
    void readRegion(SkRegion* region) override;
    void readPath(SkPath* path) override;

    // binary data and arrays
    bool readByteArray(void* value, size_t size) override;
    bool readColorArray(SkColor* colors, size_t size) override;
    bool readColor4fArray(SkColor4f* colors, size_t size) override;
    bool readIntArray(int32_t* values, size_t size) override;
    bool readPointArray(SkPoint* points, size_t size) override;
    bool readScalarArray(SkScalar* values, size_t size) override;

    // helpers to get info about arrays and binary data
    uint32_t getArrayCount() override;

    bool validate(bool isValid) override;
    bool isValid() const override;

    bool validateAvailable(size_t size) override;

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
