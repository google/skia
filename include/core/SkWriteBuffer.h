
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWriteBuffer_DEFINED
#define SkWriteBuffer_DEFINED

#include "SkData.h"
#include "SkImage.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkPixelSerializer.h"
#include "SkRefCnt.h"
#include "SkWriter32.h"
#include "../private/SkTHash.h"

class SkBitmap;
class SkBitmapHeap;
class SkFactorySet;
class SkFlattenable;
class SkRefCntSet;

class SkWriteBuffer {
public:
    enum Flags {
        kCrossProcess_Flag  = 1 << 0,
    };

    SkWriteBuffer(uint32_t flags = 0);
    SkWriteBuffer(void* initialStorage, size_t storageSize, uint32_t flags = 0);
    ~SkWriteBuffer();

    bool isCrossProcess() const {
        return SkToBool(fFlags & kCrossProcess_Flag);
    }

    void reset(void* storage = NULL, size_t storageSize = 0) {
        fWriter.reset(storage, storageSize);
    }

    size_t bytesWritten() const { return fWriter.bytesWritten(); }

    void writeByteArray(const void* data, size_t size);
    void writeDataAsByteArray(SkData* data) { this->writeByteArray(data->data(), data->size()); }
    void writeBool(bool value);
    void writeScalar(SkScalar value);
    void writeScalarArray(const SkScalar* value, uint32_t count);
    void writeInt(int32_t value);
    void writeIntArray(const int32_t* value, uint32_t count);
    void writeUInt(uint32_t value);
    void write32(int32_t value);
    void writeString(const char* value);

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
    void writeImage(const SkImage*);
    void writeTypeface(SkTypeface* typeface);
    void writePaint(const SkPaint& paint) { paint.flatten(*this); }

    bool writeToStream(SkWStream*);
    void writeToMemory(void* dst) { fWriter.flatten(dst); }

    SkFactorySet* setFactoryRecorder(SkFactorySet*);

    SkRefCntSet* getTypefaceRecorder() const { return fTFSet; }
    SkRefCntSet* setTypefaceRecorder(SkRefCntSet*);

    /**
     * Set an SkBitmapHeap to store bitmaps rather than flattening.
     *
     * Incompatible with an SkPixelSerializer. If an SkPixelSerializer is set,
     * setting an SkBitmapHeap will set the SkPixelSerializer to NULL in release
     * and crash in debug.
     */
    void setBitmapHeap(SkBitmapHeap*);

    /**
     * Set an SkPixelSerializer to store an encoded representation of pixels,
     * e.g. SkBitmaps.
     *
     * Calls ref() on the serializer.
     *
     * TODO: Encode SkImage pixels as well.
     *
     * Incompatible with the SkBitmapHeap. If an encoder is set fBitmapHeap will
     * be set to NULL in release and crash in debug.
     */
    void setPixelSerializer(SkPixelSerializer*);
    SkPixelSerializer* getPixelSerializer() const { return fPixelSerializer; }

private:
    const uint32_t fFlags;
    SkFactorySet* fFactorySet;
    SkWriter32 fWriter;

    SkBitmapHeap* fBitmapHeap;
    SkRefCntSet* fTFSet;

    SkAutoTUnref<SkPixelSerializer> fPixelSerializer;

    // Only used if we do not have an fFactorySet
    SkTHashMap<SkString, uint32_t> fFlattenableDict;
};

#endif // SkWriteBuffer_DEFINED
