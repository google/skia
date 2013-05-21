
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkFlattenableBuffers_DEFINED
#define SkFlattenableBuffers_DEFINED

#include "SkColor.h"
#include "SkPaint.h"
#include "SkPoint.h"

class SkBitmap;
class SkFlattenable;
struct SkIRect;
class SkMatrix;
class SkOrderedReadBuffer;
class SkOrderedWriteBuffer;
class SkPath;
class SkPixelRef;
struct SkRect;
class SkRefCnt;
class SkRegion;
class SkStream;
class SkString;
class SkTypeface;
class SkWStream;

class SkFlattenableReadBuffer {
public:
    SkFlattenableReadBuffer();
    virtual ~SkFlattenableReadBuffer();

    bool isOrderedBinaryBuffer() { return NULL != getOrderedBinaryBuffer(); }
    virtual SkOrderedReadBuffer* getOrderedBinaryBuffer() { return NULL; }

    enum Flags {
        kCrossProcess_Flag      = 1 << 0,
        kScalarIsFloat_Flag     = 1 << 1,
        kPtrIs64Bit_Flag        = 1 << 2,
    };

    void setFlags(uint32_t flags) { fFlags = flags; }
    uint32_t getFlags() const { return fFlags; }

    bool isCrossProcess() const { return SkToBool(fFlags & kCrossProcess_Flag); }
    bool isScalarFloat() const { return SkToBool(fFlags & kScalarIsFloat_Flag); }
    bool isPtr64Bit() const { return SkToBool(fFlags & kPtrIs64Bit_Flag); }

    // primitives
    virtual bool readBool() = 0;
    virtual SkColor readColor() = 0;
    virtual SkFixed readFixed() = 0;
    virtual int32_t readInt() = 0;
    virtual SkScalar readScalar() = 0;
    virtual uint32_t readUInt() = 0;
    virtual int32_t read32() = 0;

    // strings -- the caller is responsible for freeing the string contents
    virtual void readString(SkString* string) = 0;
    virtual void* readEncodedString(size_t* length, SkPaint::TextEncoding encoding) = 0;

    // common data structures
    virtual SkFlattenable* readFlattenable() = 0;
    virtual void readPoint(SkPoint* point) = 0;
    virtual void readMatrix(SkMatrix* matrix) = 0;
    virtual void readIRect(SkIRect* rect) = 0;
    virtual void readRect(SkRect* rect) = 0;
    virtual void readRegion(SkRegion* region) = 0;
    virtual void readPath(SkPath* path) = 0;

    // binary data and arrays
    virtual uint32_t readByteArray(void* value) = 0;
    virtual uint32_t readColorArray(SkColor* colors) = 0;
    virtual uint32_t readIntArray(int32_t* values) = 0;
    virtual uint32_t readPointArray(SkPoint* points) = 0;
    virtual uint32_t readScalarArray(SkScalar* values) = 0;

    /** This helper peeks into the buffer and reports back the length of the next array in
     *  the buffer but does not change the state of the buffer.
     */
    virtual uint32_t getArrayCount() = 0;

    // helper functions
    virtual void* readFunctionPtr();
    virtual void readPaint(SkPaint* paint);

    virtual void readBitmap(SkBitmap* bitmap) = 0;
    virtual SkTypeface* readTypeface() = 0;

    // helper function for classes with const SkPoint members
    SkPoint readPoint() {
        SkPoint point;
        this->readPoint(&point);
        return point;
    }

    template <typename T> T* readFlattenableT() {
        return static_cast<T*>(this->readFlattenable());
    }

private:
    uint32_t fFlags;
};

///////////////////////////////////////////////////////////////////////////////

class SkFlattenableWriteBuffer {
public:
    SkFlattenableWriteBuffer();
    virtual ~SkFlattenableWriteBuffer();

    virtual bool isOrderedBinaryBuffer() { return false; }
    virtual SkOrderedWriteBuffer* getOrderedBinaryBuffer() { sk_throw(); return NULL; }

    // primitives
    virtual void writeByteArray(const void* data, size_t size) = 0;
    virtual void writeBool(bool value) = 0;
    virtual void writeFixed(SkFixed value) = 0;
    virtual void writeScalar(SkScalar value) = 0;
    virtual void writeScalarArray(const SkScalar* value, uint32_t count) = 0;
    virtual void writeInt(int32_t value) = 0;
    virtual void writeIntArray(const int32_t* value, uint32_t count) = 0;
    virtual void writeUInt(uint32_t value) = 0;
    virtual void write32(int32_t value) = 0; // printf in hex
    virtual void writeString(const char* value) = 0;
    virtual void writeEncodedString(const void* value, size_t byteLength,
                                    SkPaint::TextEncoding encoding) = 0;

    // common data structures
    virtual void writeFlattenable(SkFlattenable* flattenable) = 0;
    virtual void writeColor(const SkColor& color) = 0;
    virtual void writeColorArray(const SkColor* color, uint32_t count) = 0;
    virtual void writePoint(const SkPoint& point) = 0;
    virtual void writePointArray(const SkPoint* points, uint32_t count) = 0;
    virtual void writeMatrix(const SkMatrix& matrix) = 0;
    virtual void writeIRect(const SkIRect& rect) = 0;
    virtual void writeRect(const SkRect& rect) = 0;
    virtual void writeRegion(const SkRegion& region) = 0;
    virtual void writePath(const SkPath& path) = 0;
    virtual size_t writeStream(SkStream* stream, size_t length) = 0;

    // helper functions
    virtual void writeFunctionPtr(void* ptr);
    virtual void writePaint(const SkPaint& paint);

    virtual void writeBitmap(const SkBitmap& bitmap) = 0;
    virtual void writeTypeface(SkTypeface* typeface) = 0;

    virtual bool writeToStream(SkWStream*) = 0;

    enum Flags {
        kCrossProcess_Flag               = 0x01,
    };

    uint32_t getFlags() const { return fFlags; }
    void setFlags(uint32_t flags) { fFlags = flags; }

    bool isCrossProcess() const {
        return SkToBool(fFlags & kCrossProcess_Flag);
    }

    bool persistTypeface() const { return (fFlags & kCrossProcess_Flag) != 0; }

protected:
    // A helper function so that each subclass does not have to be a friend of SkFlattenable
    void flattenObject(SkFlattenable* obj, SkFlattenableWriteBuffer& buffer);

    uint32_t fFlags;
};

#endif
