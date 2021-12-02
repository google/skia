/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWriteBuffer_DEFINED
#define SkWriteBuffer_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkSerialProcs.h"
#include "include/private/SkTHash.h"
#include "src/core/SkWriter32.h"

class SkFactorySet;
class SkImage;
class SkM44;
class SkPath;
class SkRefCntSet;

class SkWriteBuffer {
public:
    SkWriteBuffer() {}
    virtual ~SkWriteBuffer() {}

    virtual void writePad32(const void* buffer, size_t bytes) = 0;

    virtual void writeByteArray(const void* data, size_t size) = 0;
    void writeDataAsByteArray(SkData* data) {
        if (!data) {
            this->write32(0);
        } else {
            this->writeByteArray(data->data(), data->size());
        }
    }

    virtual void writeBool(bool value) = 0;
    virtual void writeScalar(SkScalar value) = 0;
    virtual void writeScalarArray(const SkScalar* value, uint32_t count) = 0;
    virtual void writeInt(int32_t value) = 0;
    virtual void writeIntArray(const int32_t* value, uint32_t count) = 0;
    virtual void writeUInt(uint32_t value) = 0;
    void write32(int32_t value) {
        this->writeInt(value);
    }
    virtual void writeString(const char* value) = 0;

    virtual void writeFlattenable(const SkFlattenable* flattenable) = 0;
    virtual void writeColor(SkColor color) = 0;
    virtual void writeColorArray(const SkColor* color, uint32_t count) = 0;
    virtual void writeColor4f(const SkColor4f& color) = 0;
    virtual void writeColor4fArray(const SkColor4f* color, uint32_t count) = 0;
    virtual void writePoint(const SkPoint& point) = 0;
    virtual void writePointArray(const SkPoint* point, uint32_t count) = 0;
    virtual void writePoint3(const SkPoint3& point) = 0;
    virtual void write(const SkM44&) = 0;
    virtual void writeMatrix(const SkMatrix& matrix) = 0;
    virtual void writeIRect(const SkIRect& rect) = 0;
    virtual void writeRect(const SkRect& rect) = 0;
    virtual void writeRegion(const SkRegion& region) = 0;
    virtual void writePath(const SkPath& path) = 0;
    virtual size_t writeStream(SkStream* stream, size_t length) = 0;
    virtual void writeImage(const SkImage*) = 0;
    virtual void writeTypeface(SkTypeface* typeface) = 0;
    virtual void writePaint(const SkPaint& paint) = 0;

    void setSerialProcs(const SkSerialProcs& procs) { fProcs = procs; }

protected:
    SkSerialProcs   fProcs;

    friend class SkPicturePriv; // fProcs
};

/**
 * Concrete implementation that serializes to a flat binary blob.
 */
class SkBinaryWriteBuffer : public SkWriteBuffer {
public:
    SkBinaryWriteBuffer();
    SkBinaryWriteBuffer(void* initialStorage, size_t storageSize);
    ~SkBinaryWriteBuffer() override;

    void write(const void* buffer, size_t bytes) {
        fWriter.write(buffer, bytes);
    }
    void writePad32(const void* buffer, size_t bytes) override {
        fWriter.writePad(buffer, bytes);
    }

    void reset(void* storage = nullptr, size_t storageSize = 0) {
        fWriter.reset(storage, storageSize);
    }

    size_t bytesWritten() const { return fWriter.bytesWritten(); }

    // Returns true iff all of the bytes written so far are stored in the initial storage
    // buffer provided in the constructor or the most recent call to reset.
    bool usingInitialStorage() const;

    void writeByteArray(const void* data, size_t size) override;
    void writeBool(bool value) override;
    void writeScalar(SkScalar value) override;
    void writeScalarArray(const SkScalar* value, uint32_t count) override;
    void writeInt(int32_t value) override;
    void writeIntArray(const int32_t* value, uint32_t count) override;
    void writeUInt(uint32_t value) override;
    void writeString(const char* value) override;

    void writeFlattenable(const SkFlattenable* flattenable) override;
    void writeColor(SkColor color) override;
    void writeColorArray(const SkColor* color, uint32_t count) override;
    void writeColor4f(const SkColor4f& color) override;
    void writeColor4fArray(const SkColor4f* color, uint32_t count) override;
    void writePoint(const SkPoint& point) override;
    void writePointArray(const SkPoint* point, uint32_t count) override;
    void writePoint3(const SkPoint3& point) override;
    void write(const SkM44&) override;
    void writeMatrix(const SkMatrix& matrix) override;
    void writeIRect(const SkIRect& rect) override;
    void writeRect(const SkRect& rect) override;
    void writeRegion(const SkRegion& region) override;
    void writePath(const SkPath& path) override;
    size_t writeStream(SkStream* stream, size_t length) override;
    void writeImage(const SkImage*) override;
    void writeTypeface(SkTypeface* typeface) override;
    void writePaint(const SkPaint& paint) override;

    bool writeToStream(SkWStream*) const;
    void writeToMemory(void* dst) const { fWriter.flatten(dst); }
    sk_sp<SkData> snapshotAsData() const { return fWriter.snapshotAsData(); }

    void setFactoryRecorder(sk_sp<SkFactorySet>);
    void setTypefaceRecorder(sk_sp<SkRefCntSet>);

private:
    sk_sp<SkFactorySet> fFactorySet;
    sk_sp<SkRefCntSet> fTFSet;

    SkWriter32 fWriter;

    // Only used if we do not have an fFactorySet
    SkTHashMap<const char*, uint32_t> fFlattenableDict;
};

enum SkWriteBufferImageFlags {
    kVersion_bits   = 8,
    kCurrVersion    = 0,

    kHasSubsetRect  = 1 << 8,
    kHasMipmap      = 1 << 9,
    kUnpremul       = 1 << 10,
};


#endif // SkWriteBuffer_DEFINED
