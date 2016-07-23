/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWriteBuffer.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkPixelRef.h"
#include "SkPtrRecorder.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkBinaryWriteBuffer::SkBinaryWriteBuffer(uint32_t flags)
    : fFlags(flags)
    , fFactorySet(nullptr)
    , fTFSet(nullptr) {
}

SkBinaryWriteBuffer::SkBinaryWriteBuffer(void* storage, size_t storageSize, uint32_t flags)
    : fFlags(flags)
    , fFactorySet(nullptr)
    , fWriter(storage, storageSize)
    , fTFSet(nullptr) {
}

SkBinaryWriteBuffer::~SkBinaryWriteBuffer() {
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fTFSet);
}

void SkBinaryWriteBuffer::writeByteArray(const void* data, size_t size) {
    fWriter.write32(SkToU32(size));
    fWriter.writePad(data, size);
}

void SkBinaryWriteBuffer::writeBool(bool value) {
    fWriter.writeBool(value);
}

void SkBinaryWriteBuffer::writeScalar(SkScalar value) {
    fWriter.writeScalar(value);
}

void SkBinaryWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(SkScalar));
}

void SkBinaryWriteBuffer::writeInt(int32_t value) {
    fWriter.write32(value);
}

void SkBinaryWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(int32_t));
}

void SkBinaryWriteBuffer::writeUInt(uint32_t value) {
    fWriter.write32(value);
}

void SkBinaryWriteBuffer::writeString(const char* value) {
    fWriter.writeString(value);
}

void SkBinaryWriteBuffer::writeColor(SkColor color) {
    fWriter.write32(color);
}

void SkBinaryWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(color, count * sizeof(SkColor));
}

void SkBinaryWriteBuffer::writePoint(const SkPoint& point) {
    fWriter.writeScalar(point.fX);
    fWriter.writeScalar(point.fY);
}

void SkBinaryWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(point, count * sizeof(SkPoint));
}

void SkBinaryWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkBinaryWriteBuffer::writeIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(SkIRect));
}

void SkBinaryWriteBuffer::writeRect(const SkRect& rect) {
    fWriter.writeRect(rect);
}

void SkBinaryWriteBuffer::writeRegion(const SkRegion& region) {
    fWriter.writeRegion(region);
}

void SkBinaryWriteBuffer::writePath(const SkPath& path) {
    fWriter.writePath(path);
}

size_t SkBinaryWriteBuffer::writeStream(SkStream* stream, size_t length) {
    fWriter.write32(SkToU32(length));
    size_t bytesWritten = fWriter.readFromStream(stream, length);
    if (bytesWritten < length) {
        fWriter.reservePad(length - bytesWritten);
    }
    return bytesWritten;
}

bool SkBinaryWriteBuffer::writeToStream(SkWStream* stream) {
    return fWriter.writeToStream(stream);
}

static void write_encoded_bitmap(SkBinaryWriteBuffer* buffer, SkData* data,
                                 const SkIPoint& origin) {
    buffer->writeDataAsByteArray(data);
    buffer->write32(origin.fX);
    buffer->write32(origin.fY);
}

void SkBinaryWriteBuffer::writeBitmap(const SkBitmap& bitmap) {
    // Record the width and height. This way if readBitmap fails a dummy bitmap can be drawn at the
    // right size.
    this->writeInt(bitmap.width());
    this->writeInt(bitmap.height());

    // Record information about the bitmap in one of two ways, in order of priority:
    // 1. If there is a function for encoding bitmaps, use it to write an encoded version of the
    //    bitmap. After writing a boolean value of false, signifying that a heap was not used, write
    //    the size of the encoded data. A non-zero size signifies that encoded data was written.
    // 2. Call SkBitmap::flatten. After writing a boolean value of false, signifying that a heap was
    //    not used, write a zero to signify that the data was not encoded.

    // Write a bool to indicate that we did not use an SkBitmapHeap. That feature is deprecated.
    this->writeBool(false);

    SkPixelRef* pixelRef = bitmap.pixelRef();
    if (pixelRef) {
        // see if the pixelref already has an encoded version
        SkAutoDataUnref existingData(pixelRef->refEncodedData());
        if (existingData.get() != nullptr) {
            // Assumes that if the client did not set a serializer, they are
            // happy to get the encoded data.
            if (!fPixelSerializer || fPixelSerializer->useEncodedData(existingData->data(),
                                                                      existingData->size())) {
                write_encoded_bitmap(this, existingData, bitmap.pixelRefOrigin());
                return;
            }
        }

        // see if the caller wants to manually encode
        SkAutoPixmapUnlock result;
        if (fPixelSerializer && bitmap.requestLock(&result)) {
            SkAutoDataUnref data(fPixelSerializer->encode(result.pixmap()));
            if (data.get() != nullptr) {
                // if we have to "encode" the bitmap, then we assume there is no
                // offset to share, since we are effectively creating a new pixelref
                write_encoded_bitmap(this, data, SkIPoint::Make(0, 0));
                return;
            }
        }
    }

    this->writeUInt(0); // signal raw pixels
    SkBitmap::WriteRawPixels(this, bitmap);
}

void SkBinaryWriteBuffer::writeImage(const SkImage* image) {
    this->writeInt(image->width());
    this->writeInt(image->height());

    SkAutoTUnref<SkData> encoded(image->encode(this->getPixelSerializer()));
    if (encoded && encoded->size() > 0) {
        write_encoded_bitmap(this, encoded, SkIPoint::Make(0, 0));
        return;
    }

    SkBitmap bm;
    if (image->asLegacyBitmap(&bm, SkImage::kRO_LegacyBitmapMode)) {
        this->writeUInt(1);  // signal raw pixels.
        SkBitmap::WriteRawPixels(this, bm);
        return;
    }

    this->writeUInt(0); // signal no pixels (in place of the size of the encoded data)
}

void SkBinaryWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (nullptr == obj || nullptr == fTFSet) {
        fWriter.write32(0);
    } else {
        fWriter.write32(fTFSet->add(obj));
    }
}

void SkBinaryWriteBuffer::writePaint(const SkPaint& paint) {
    paint.flatten(*this);
}

SkFactorySet* SkBinaryWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    return rec;
}

SkRefCntSet* SkBinaryWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

void SkBinaryWriteBuffer::setPixelSerializer(SkPixelSerializer* serializer) {
    fPixelSerializer.reset(serializer);
    if (serializer) {
        serializer->ref();
    }
}

void SkBinaryWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    /*
     *  The first 32 bits tell us...
     *       0: failure to write the flattenable
     *      >0: index (1-based) into fFactorySet or fFlattenableDict or
     *          the first character of a string
     */
    if (nullptr == flattenable) {
        this->write32(0);
        return;
    }

    /*
     *  We can write 1 of 2 versions of the flattenable:
     *  1.  index into fFactorySet : This assumes the writer will later
     *      resolve the function-ptrs into strings for its reader. SkPicture
     *      does exactly this, by writing a table of names (matching the indices)
     *      up front in its serialized form.
     *  2.  string name of the flattenable or index into fFlattenableDict:  We
     *      store the string to allow the reader to specify its own factories
     *      after write time.  In order to improve compression, if we have
     *      already written the string, we write its index instead.
     */
    if (fFactorySet) {
        SkFlattenable::Factory factory = flattenable->getFactory();
        SkASSERT(factory);
        this->write32(fFactorySet->add(factory));
    } else {
        const char* name = flattenable->getTypeName();
        SkASSERT(name);
        SkString key(name);
        if (uint32_t* indexPtr = fFlattenableDict.find(key)) {
            // We will write the index as a 32-bit int.  We want the first byte
            // that we send to be zero - this will act as a sentinel that we
            // have an index (not a string).  This means that we will send the
            // the index shifted left by 8.  The remaining 24-bits should be
            // plenty to store the index.  Note that this strategy depends on
            // being little endian.
            SkASSERT(0 == *indexPtr >> 24);
            this->write32(*indexPtr << 8);
        } else {
            // Otherwise write the string.  Clients should not use the empty
            // string as a name, or we will have a problem.
            SkASSERT(strcmp("", name));
            this->writeString(name);

            // Add key to dictionary.
            fFlattenableDict.set(key, fFlattenableDict.count() + 1);
        }
    }

    // make room for the size of the flattened object
    (void)fWriter.reserve(sizeof(uint32_t));
    // record the current size, so we can subtract after the object writes.
    size_t offset = fWriter.bytesWritten();
    // now flatten the object
    flattenable->flatten(*this);
    size_t objSize = fWriter.bytesWritten() - offset;
    // record the obj's size
    fWriter.overwriteTAt(offset - sizeof(uint32_t), SkToU32(objSize));
}
