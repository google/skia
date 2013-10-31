
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOrderedReadBuffer_DEFINED
#define SkOrderedReadBuffer_DEFINED

#include "SkRefCnt.h"
#include "SkBitmapHeap.h"
#include "SkFlattenableBuffers.h"
#include "SkPath.h"
#include "SkPicture.h"
#include "SkReader32.h"

class SkBitmap;

#if defined(SK_DEBUG) && defined(SK_BUILD_FOR_MAC)
    #define DEBUG_NON_DETERMINISTIC_ASSERT
#endif

class SkOrderedReadBuffer : public SkFlattenableReadBuffer {
public:
    SkOrderedReadBuffer();
    SkOrderedReadBuffer(const void* data, size_t size);
    SkOrderedReadBuffer(SkStream* stream);
    virtual ~SkOrderedReadBuffer();

    virtual SkOrderedReadBuffer* getOrderedBinaryBuffer() SK_OVERRIDE { return this; }

    SkReader32* getReader32() { return &fReader; }

    uint32_t size() { return fReader.size(); }
    uint32_t offset() { return fReader.offset(); }
    bool eof() { return fReader.eof(); }
    const void* skip(size_t size) { return fReader.skip(size); }

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
    virtual SkFlattenable* readFlattenable(SkFlattenable::Type) SK_OVERRIDE;
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
    virtual SkTypeface* readTypeface() SK_OVERRIDE;

    void setBitmapStorage(SkBitmapHeapReader* bitmapStorage) {
        SkRefCnt_SafeAssign(fBitmapStorage, bitmapStorage);
    }

    void setTypefaceArray(SkTypeface* array[], int count) {
        fTFArray = array;
        fTFCount = count;
    }

    /**
     *  Call this with a pre-loaded array of Factories, in the same order as
     *  were created/written by the writer. SkPicture uses this.
     */
    void setFactoryPlayback(SkFlattenable::Factory array[], int count) {
        fFactoryTDArray = NULL;
        fFactoryArray = array;
        fFactoryCount = count;
    }

    /**
     *  Call this with an initially empty array, so the reader can cache each
     *  factory it sees by name. Used by the pipe code in conjunction with
     *  SkOrderedWriteBuffer::setNamedFactoryRecorder.
     */
    void setFactoryArray(SkTDArray<SkFlattenable::Factory>* array) {
        fFactoryTDArray = array;
        fFactoryArray = NULL;
        fFactoryCount = 0;
    }

    /**
     *  Provide a function to decode an SkBitmap from encoded data. Only used if the writer
     *  encoded the SkBitmap. If the proper decoder cannot be used, a red bitmap with the
     *  appropriate size will be used.
     */
    void setBitmapDecoder(SkPicture::InstallPixelRefProc bitmapDecoder) {
        fBitmapDecoder = bitmapDecoder;
    }

private:
    bool readArray(void* value, size_t size, size_t elementSize);

    SkReader32 fReader;
    void* fMemoryPtr;

    SkBitmapHeapReader* fBitmapStorage;
    SkTypeface** fTFArray;
    int        fTFCount;

    SkTDArray<SkFlattenable::Factory>* fFactoryTDArray;
    SkFlattenable::Factory* fFactoryArray;
    int                     fFactoryCount;

    SkPicture::InstallPixelRefProc fBitmapDecoder;

#ifdef DEBUG_NON_DETERMINISTIC_ASSERT
    // Debugging counter to keep track of how many bitmaps we
    // have decoded.
    int fDecodedBitmapIndex;
#endif // DEBUG_NON_DETERMINISTIC_ASSERT

    typedef SkFlattenableReadBuffer INHERITED;
};

#endif // SkOrderedReadBuffer_DEFINED
