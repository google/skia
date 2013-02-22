
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOrderedWriteBuffer.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkPixelRef.h"
#include "SkPtrRecorder.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkOrderedWriteBuffer::SkOrderedWriteBuffer(size_t minSize)
    : INHERITED()
    , fFactorySet(NULL)
    , fNamedFactorySet(NULL)
    , fWriter(minSize)
    , fBitmapHeap(NULL)
    , fTFSet(NULL)
    , fBitmapEncoder(NULL) {
}

SkOrderedWriteBuffer::SkOrderedWriteBuffer(size_t minSize, void* storage, size_t storageSize)
    : INHERITED()
    , fFactorySet(NULL)
    , fNamedFactorySet(NULL)
    , fWriter(minSize, storage, storageSize)
    , fBitmapHeap(NULL)
    , fTFSet(NULL)
    , fBitmapEncoder(NULL) {
}

SkOrderedWriteBuffer::~SkOrderedWriteBuffer() {
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fNamedFactorySet);
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fTFSet);
}

void SkOrderedWriteBuffer::writeByteArray(const void* data, size_t size) {
    fWriter.write32(size);
    fWriter.writePad(data, size);
}

void SkOrderedWriteBuffer::writeBool(bool value) {
    fWriter.writeBool(value);
}

void SkOrderedWriteBuffer::writeFixed(SkFixed value) {
    fWriter.write32(value);
}

void SkOrderedWriteBuffer::writeScalar(SkScalar value) {
    fWriter.writeScalar(value);
}

void SkOrderedWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(SkScalar));
}

void SkOrderedWriteBuffer::writeInt(int32_t value) {
    fWriter.write32(value);
}

void SkOrderedWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(int32_t));
}

void SkOrderedWriteBuffer::writeUInt(uint32_t value) {
    fWriter.write32(value);
}

void SkOrderedWriteBuffer::write32(int32_t value) {
    fWriter.write32(value);
}

void SkOrderedWriteBuffer::writeString(const char* value) {
    fWriter.writeString(value);
}

void SkOrderedWriteBuffer::writeEncodedString(const void* value, size_t byteLength,
                                              SkPaint::TextEncoding encoding) {
    fWriter.writeInt(encoding);
    fWriter.writeInt(byteLength);
    fWriter.write(value, byteLength);
}


void SkOrderedWriteBuffer::writeColor(const SkColor& color) {
    fWriter.write32(color);
}

void SkOrderedWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(color, count * sizeof(SkColor));
}

void SkOrderedWriteBuffer::writePoint(const SkPoint& point) {
    fWriter.writeScalar(point.fX);
    fWriter.writeScalar(point.fY);
}

void SkOrderedWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(point, count * sizeof(SkPoint));
}

void SkOrderedWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkOrderedWriteBuffer::writeIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(SkIRect));
}

void SkOrderedWriteBuffer::writeRect(const SkRect& rect) {
    fWriter.writeRect(rect);
}

void SkOrderedWriteBuffer::writeRegion(const SkRegion& region) {
    fWriter.writeRegion(region);
}

void SkOrderedWriteBuffer::writePath(const SkPath& path) {
    fWriter.writePath(path);
}

size_t SkOrderedWriteBuffer::writeStream(SkStream* stream, size_t length) {
    fWriter.write32(length);
    size_t bytesWritten = fWriter.readFromStream(stream, length);
    if (bytesWritten < length) {
        fWriter.reservePad(length - bytesWritten);
    }
    return bytesWritten;
}

bool SkOrderedWriteBuffer::writeToStream(SkWStream* stream) {
    return fWriter.writeToStream(stream);
}

void SkOrderedWriteBuffer::writeBitmap(const SkBitmap& bitmap) {
    // Record information about the bitmap in one of three ways, in order of priority:
    // 1. If there is an SkBitmapHeap, store it in the heap. The client can avoid serializing the
    //    bitmap entirely or serialize it later as desired.
    // 2. Write an encoded version of the bitmap. Afterwards the width and height are written, so
    //    a reader without a decoder can draw a dummy bitmap of the right size.
    //    A. If the bitmap has an encoded representation, write it to the stream.
    //    B. If there is a function for encoding bitmaps, use it.
    // 3. Call SkBitmap::flatten.
    // For an encoded bitmap, write the size first. Otherwise store a 0 so the reader knows not to
    // decode.
    if (fBitmapHeap != NULL) {
        SkASSERT(NULL == fBitmapEncoder);
        // Bitmap was not encoded. Record a zero, implying that the reader need not decode.
        this->writeUInt(0);
        int32_t slot = fBitmapHeap->insert(bitmap);
        fWriter.write32(slot);
        // crbug.com/155875
        // The generation ID is not required information. We write it to prevent collisions
        // in SkFlatDictionary.  It is possible to get a collision when a previously
        // unflattened (i.e. stale) instance of a similar flattenable is in the dictionary
        // and the instance currently being written is re-using the same slot from the
        // bitmap heap.
        fWriter.write32(bitmap.getGenerationID());
        return;
    }
    bool encoded = false;
    // Before attempting to encode the SkBitmap, check to see if there is already an encoded
    // version.
    SkPixelRef* ref = bitmap.pixelRef();
    if (ref != NULL) {
        SkAutoDataUnref data(ref->refEncodedData());
        if (data.get() != NULL) {
            // Write the length to indicate that the bitmap was encoded successfully, followed
            // by the actual data. This must match the case where fBitmapEncoder is used so the
            // reader need not know the difference.
            this->writeUInt(data->size());
            fWriter.writePad(data->data(), data->size());
            encoded = true;
        }
    }
    if (fBitmapEncoder != NULL && !encoded) {
        SkASSERT(NULL == fBitmapHeap);
        SkDynamicMemoryWStream stream;
        if (fBitmapEncoder(&stream, bitmap)) {
            uint32_t offset = fWriter.bytesWritten();
            // Write the length to indicate that the bitmap was encoded successfully, followed
            // by the actual data. This must match the case where the original data is used so the
            // reader need not know the difference.
            size_t length = stream.getOffset();
            this->writeUInt(length);
            if (stream.read(fWriter.reservePad(length), 0, length)) {
                encoded = true;
            } else {
                // Writing the stream failed, so go back to original state to store another way.
                fWriter.rewindToOffset(offset);
            }
        }
    }
    if (encoded) {
        // Write the width and height in case the reader does not have a decoder.
        this->writeInt(bitmap.width());
        this->writeInt(bitmap.height());
    } else {
        // Bitmap was not encoded. Record a zero, implying that the reader need not decode.
        this->writeUInt(0);
        bitmap.flatten(*this);
    }
}

void SkOrderedWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (NULL == obj || NULL == fTFSet) {
        fWriter.write32(0);
    } else {
        fWriter.write32(fTFSet->add(obj));
    }
}

SkFactorySet* SkOrderedWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    if (fNamedFactorySet != NULL) {
        fNamedFactorySet->unref();
        fNamedFactorySet = NULL;
    }
    return rec;
}

SkNamedFactorySet* SkOrderedWriteBuffer::setNamedFactoryRecorder(SkNamedFactorySet* rec) {
    SkRefCnt_SafeAssign(fNamedFactorySet, rec);
    if (fFactorySet != NULL) {
        fFactorySet->unref();
        fFactorySet = NULL;
    }
    return rec;
}

SkRefCntSet* SkOrderedWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

void SkOrderedWriteBuffer::setBitmapHeap(SkBitmapHeap* bitmapHeap) {
    SkRefCnt_SafeAssign(fBitmapHeap, bitmapHeap);
    if (bitmapHeap != NULL) {
        SkASSERT(NULL == fBitmapEncoder);
        fBitmapEncoder = NULL;
    }
}

void SkOrderedWriteBuffer::setBitmapEncoder(SkPicture::EncodeBitmap bitmapEncoder) {
    fBitmapEncoder = bitmapEncoder;
    if (bitmapEncoder != NULL) {
        SkASSERT(NULL == fBitmapHeap);
        SkSafeUnref(fBitmapHeap);
        fBitmapHeap = NULL;
    }
}

void SkOrderedWriteBuffer::writeFlattenable(SkFlattenable* flattenable) {
    /*
     *  If we have a factoryset, then the first 32bits tell us...
     *       0: failure to write the flattenable
     *      >0: (1-based) index into the SkFactorySet or SkNamedFactorySet
     *  If we don't have a factoryset, then the first "ptr" is either the
     *  factory, or null for failure.
     *
     *  The distinction is important, since 0-index is 32bits (always), but a
     *  0-functionptr might be 32 or 64 bits.
     */

    SkFlattenable::Factory factory = NULL;
    if (flattenable) {
        factory = flattenable->getFactory();
    }
    if (NULL == factory) {
        if (fFactorySet != NULL || fNamedFactorySet != NULL) {
            this->write32(0);
        } else {
            this->writeFunctionPtr(NULL);
        }
        return;
    }

    /*
     *  We can write 1 of 3 versions of the flattenable:
     *  1.  function-ptr : this is the fastest for the reader, but assumes that
     *      the writer and reader are in the same process.
     *  2.  index into fFactorySet : This is assumes the writer will later
     *      resolve the function-ptrs into strings for its reader. SkPicture
     *      does exactly this, by writing a table of names (matching the indices)
     *      up front in its serialized form.
     *  3.  index into fNamedFactorySet. fNamedFactorySet will also store the
     *      name. SkGPipe uses this technique so it can write the name to its
     *      stream before writing the flattenable.
     */
    if (fFactorySet) {
        this->write32(fFactorySet->add(factory));
    } else if (fNamedFactorySet) {
        int32_t index = fNamedFactorySet->find(factory);
        this->write32(index);
        if (0 == index) {
            return;
        }
    } else {
        this->writeFunctionPtr((void*)factory);
    }

    // make room for the size of the flattened object
    (void)fWriter.reserve(sizeof(uint32_t));
    // record the current size, so we can subtract after the object writes.
    uint32_t offset = fWriter.size();
    // now flatten the object
    flattenObject(flattenable, *this);
    uint32_t objSize = fWriter.size() - offset;
    // record the obj's size
    *fWriter.peek32(offset - sizeof(uint32_t)) = objSize;
}
