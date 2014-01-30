
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWriteBuffer_DEFINED
#define SkWriteBuffer_DEFINED

#include "SkBitmapHeap.h"
#include "SkData.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkRefCnt.h"
#include "SkWriter32.h"

class SkBitmap;
class SkFactorySet;
class SkFlattenable;
class SkNamedFactorySet;
class SkRefCntSet;

class SkWriteBuffer {
public:
    enum Flags {
        kCrossProcess_Flag  = 1 << 0,
        kValidation_Flag    = 1 << 1,
    };

    SkWriteBuffer(uint32_t flags = 0);
    SkWriteBuffer(void* initialStorage, size_t storageSize, uint32_t flags = 0);
    ~SkWriteBuffer();

    bool isCrossProcess() const {
        return this->isValidating() || SkToBool(fFlags & kCrossProcess_Flag);
    }

    SkWriter32* getWriter32() { return &fWriter; }
    void reset(void* storage = NULL, size_t storageSize = 0) {
        fWriter.reset(storage, storageSize);
    }

    uint32_t* reserve(size_t size) { return fWriter.reserve(size); }

    size_t bytesWritten() const { return fWriter.bytesWritten(); }

    void writeByteArray(const void* data, size_t size);
    void writeDataAsByteArray(SkData* data) { this->writeByteArray(data->data(), data->size()); }
    void writeBool(bool value);
    void writeFixed(SkFixed value);
    void writeScalar(SkScalar value);
    void writeScalarArray(const SkScalar* value, uint32_t count);
    void writeInt(int32_t value);
    void writeIntArray(const int32_t* value, uint32_t count);
    void writeUInt(uint32_t value);
    void write32(int32_t value);
    void writeString(const char* value);
    void writeEncodedString(const void* value, size_t byteLength, SkPaint::TextEncoding encoding);
    void writeFunctionPtr(void* ptr) { this->writeByteArray(&ptr, sizeof(ptr)); }

    void writeFlattenable(const SkFlattenable* flattenable);
    void writeColor(const SkColor& color);
    void writeColorArray(const SkColor* color, uint32_t count);
    void writePoint(const SkPoint& point);
    void writePointArray(const SkPoint* point, uint32_t count);
    void writeMatrix(const SkMatrix& matrix);
    void writeIRect(const SkIRect& rect);
    void writeRect(const SkRect& rect);
    void writeRegion(const SkRegion& region);
    void writePath(const SkPath& path);
    size_t writeStream(SkStream* stream, size_t length);
    void writeBitmap(const SkBitmap& bitmap);
    void writeTypeface(SkTypeface* typeface);
    void writePaint(const SkPaint& paint) { paint.flatten(*this); }

    bool writeToStream(SkWStream*);
    void writeToMemory(void* dst) { fWriter.flatten(dst); }

    SkFactorySet* setFactoryRecorder(SkFactorySet*);
    SkNamedFactorySet* setNamedFactoryRecorder(SkNamedFactorySet*);

    SkRefCntSet* getTypefaceRecorder() const { return fTFSet; }
    SkRefCntSet* setTypefaceRecorder(SkRefCntSet*);

    /**
     * Set an SkBitmapHeap to store bitmaps rather than flattening.
     *
     * Incompatible with an EncodeBitmap function. If an EncodeBitmap function is set, setting an
     * SkBitmapHeap will set the function to NULL in release mode and crash in debug.
     */
    void setBitmapHeap(SkBitmapHeap*);

    /**
     * Provide a function to encode an SkBitmap to an SkData. writeBitmap will attempt to use
     * bitmapEncoder to store the SkBitmap. If the reader does not provide a function to decode, it
     * will not be able to restore SkBitmaps, but will still be able to read the rest of the stream.
     * bitmapEncoder will never be called with a NULL pixelRefOffset.
     *
     * Incompatible with the SkBitmapHeap. If an encoder is set fBitmapHeap will be set to NULL in
     * release and crash in debug.
     */
    void setBitmapEncoder(SkPicture::EncodeBitmap bitmapEncoder);

private:
    bool isValidating() const { return SkToBool(fFlags & kValidation_Flag); }

    const uint32_t fFlags;
    SkFactorySet* fFactorySet;
    SkNamedFactorySet* fNamedFactorySet;
    SkWriter32 fWriter;

    SkBitmapHeap* fBitmapHeap;
    SkRefCntSet* fTFSet;

    SkPicture::EncodeBitmap fBitmapEncoder;
};

#endif // SkWriteBuffer_DEFINED
