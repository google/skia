
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWriteBuffer.h"
#include "SkBitmap.h"
#include "SkBitmapHeap.h"
#include "SkData.h"
#include "SkPixelRef.h"
#include "SkPtrRecorder.h"
#include "SkStream.h"
#include "SkTypeface.h"

SkWriteBuffer::SkWriteBuffer(uint32_t flags)
    : fFlags(flags)
    , fFactorySet(NULL)
    , fNamedFactorySet(NULL)
    , fBitmapHeap(NULL)
    , fTFSet(NULL) {
}

SkWriteBuffer::SkWriteBuffer(void* storage, size_t storageSize, uint32_t flags)
    : fFlags(flags)
    , fFactorySet(NULL)
    , fNamedFactorySet(NULL)
    , fWriter(storage, storageSize)
    , fBitmapHeap(NULL)
    , fTFSet(NULL) {
}

SkWriteBuffer::~SkWriteBuffer() {
    SkSafeUnref(fFactorySet);
    SkSafeUnref(fNamedFactorySet);
    SkSafeUnref(fBitmapHeap);
    SkSafeUnref(fTFSet);
}

void SkWriteBuffer::writeByteArray(const void* data, size_t size) {
    fWriter.write32(SkToU32(size));
    fWriter.writePad(data, size);
}

void SkWriteBuffer::writeBool(bool value) {
    fWriter.writeBool(value);
}

void SkWriteBuffer::writeFixed(SkFixed value) {
    fWriter.write32(value);
}

void SkWriteBuffer::writeScalar(SkScalar value) {
    fWriter.writeScalar(value);
}

void SkWriteBuffer::writeScalarArray(const SkScalar* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(SkScalar));
}

void SkWriteBuffer::writeInt(int32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::writeIntArray(const int32_t* value, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(value, count * sizeof(int32_t));
}

void SkWriteBuffer::writeUInt(uint32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::write32(int32_t value) {
    fWriter.write32(value);
}

void SkWriteBuffer::writeString(const char* value) {
    fWriter.writeString(value);
}

void SkWriteBuffer::writeEncodedString(const void* value, size_t byteLength,
                                              SkPaint::TextEncoding encoding) {
    fWriter.writeInt(encoding);
    fWriter.writeInt(SkToU32(byteLength));
    fWriter.write(value, byteLength);
}


void SkWriteBuffer::writeColor(const SkColor& color) {
    fWriter.write32(color);
}

void SkWriteBuffer::writeColorArray(const SkColor* color, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(color, count * sizeof(SkColor));
}

void SkWriteBuffer::writePoint(const SkPoint& point) {
    fWriter.writeScalar(point.fX);
    fWriter.writeScalar(point.fY);
}

void SkWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(point, count * sizeof(SkPoint));
}

void SkWriteBuffer::writeMatrix(const SkMatrix& matrix) {
    fWriter.writeMatrix(matrix);
}

void SkWriteBuffer::writeIRect(const SkIRect& rect) {
    fWriter.write(&rect, sizeof(SkIRect));
}

void SkWriteBuffer::writeRect(const SkRect& rect) {
    fWriter.writeRect(rect);
}

void SkWriteBuffer::writeRegion(const SkRegion& region) {
    fWriter.writeRegion(region);
}

void SkWriteBuffer::writePath(const SkPath& path) {
    fWriter.writePath(path);
}

size_t SkWriteBuffer::writeStream(SkStream* stream, size_t length) {
    fWriter.write32(SkToU32(length));
    size_t bytesWritten = fWriter.readFromStream(stream, length);
    if (bytesWritten < length) {
        fWriter.reservePad(length - bytesWritten);
    }
    return bytesWritten;
}

bool SkWriteBuffer::writeToStream(SkWStream* stream) {
    return fWriter.writeToStream(stream);
}

static void write_encoded_bitmap(SkWriteBuffer* buffer, SkData* data,
                                 const SkIPoint& origin) {
    buffer->writeUInt(SkToU32(data->size()));
    buffer->getWriter32()->writePad(data->data(), data->size());
    buffer->write32(origin.fX);
    buffer->write32(origin.fY);
}

void SkWriteBuffer::writeBitmap(const SkBitmap& bitmap) {
    // Record the width and height. This way if readBitmap fails a dummy bitmap can be drawn at the
    // right size.
    this->writeInt(bitmap.width());
    this->writeInt(bitmap.height());

    // Record information about the bitmap in one of three ways, in order of priority:
    // 1. If there is an SkBitmapHeap, store it in the heap. The client can avoid serializing the
    //    bitmap entirely or serialize it later as desired. A boolean value of true will be written
    //    to the stream to signify that a heap was used.
    // 2. If there is a function for encoding bitmaps, use it to write an encoded version of the
    //    bitmap. After writing a boolean value of false, signifying that a heap was not used, write
    //    the size of the encoded data. A non-zero size signifies that encoded data was written.
    // 3. Call SkBitmap::flatten. After writing a boolean value of false, signifying that a heap was
    //    not used, write a zero to signify that the data was not encoded.
    bool useBitmapHeap = fBitmapHeap != NULL;
    // Write a bool: true if the SkBitmapHeap is to be used, in which case the reader must use an
    // SkBitmapHeapReader to read the SkBitmap. False if the bitmap was serialized another way.
    this->writeBool(useBitmapHeap);
    if (useBitmapHeap) {
        SkASSERT(NULL == fPixelSerializer);
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

    SkPixelRef* pixelRef = bitmap.pixelRef();
    if (pixelRef) {
        // see if the pixelref already has an encoded version
        SkAutoDataUnref existingData(pixelRef->refEncodedData());
        if (existingData.get() != NULL) {
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
            const SkPixmap& pmap = result.pixmap();
            SkASSERT(NULL == fBitmapHeap);
            SkAutoDataUnref data(fPixelSerializer->encodePixels(pmap.info(),
                                                                pmap.addr(),
                                                                pmap.rowBytes()));
            if (data.get() != NULL) {
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

static bool try_write_encoded(SkWriteBuffer* buffer, SkData* encoded) {
    SkPixelSerializer* ps = buffer->getPixelSerializer();
    // Assumes that if the client did not set a serializer, they are
    // happy to get the encoded data.
    if (!ps || ps->useEncodedData(encoded->data(), encoded->size())) {
        write_encoded_bitmap(buffer, encoded, SkIPoint::Make(0, 0));
        return true;
    }
    return false;
}

void SkWriteBuffer::writeImage(const SkImage* image) {
    this->writeInt(image->width());
    this->writeInt(image->height());

    SkAutoTUnref<SkData> encoded(image->refEncoded());
    if (encoded && try_write_encoded(this, encoded)) {
        return;
    }

    encoded.reset(image->encode(SkImageEncoder::kPNG_Type, 100));
    if (encoded && try_write_encoded(this, encoded)) {
        return;
    }
    
    this->writeUInt(0); // signal no pixels (in place of the size of the encoded data)
}

void SkWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (NULL == obj || NULL == fTFSet) {
        fWriter.write32(0);
    } else {
        fWriter.write32(fTFSet->add(obj));
    }
}

SkFactorySet* SkWriteBuffer::setFactoryRecorder(SkFactorySet* rec) {
    SkRefCnt_SafeAssign(fFactorySet, rec);
    if (fNamedFactorySet != NULL) {
        fNamedFactorySet->unref();
        fNamedFactorySet = NULL;
    }
    return rec;
}

SkNamedFactorySet* SkWriteBuffer::setNamedFactoryRecorder(SkNamedFactorySet* rec) {
    SkRefCnt_SafeAssign(fNamedFactorySet, rec);
    if (fFactorySet != NULL) {
        fFactorySet->unref();
        fFactorySet = NULL;
    }
    return rec;
}

SkRefCntSet* SkWriteBuffer::setTypefaceRecorder(SkRefCntSet* rec) {
    SkRefCnt_SafeAssign(fTFSet, rec);
    return rec;
}

void SkWriteBuffer::setBitmapHeap(SkBitmapHeap* bitmapHeap) {
    SkRefCnt_SafeAssign(fBitmapHeap, bitmapHeap);
    if (bitmapHeap != NULL) {
        SkASSERT(NULL == fPixelSerializer);
        fPixelSerializer.reset(NULL);
    }
}

void SkWriteBuffer::setPixelSerializer(SkPixelSerializer* serializer) {
    fPixelSerializer.reset(serializer);
    if (serializer) {
        serializer->ref();
        SkASSERT(NULL == fBitmapHeap);
        SkSafeUnref(fBitmapHeap);
        fBitmapHeap = NULL;
    }
}

void SkWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
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
    if (NULL == flattenable) {
        if (this->isValidating()) {
            this->writeString("");
        } else if (fFactorySet != NULL || fNamedFactorySet != NULL) {
            this->write32(0);
        } else {
            this->writeFunctionPtr(NULL);
        }
        return;
    }

    SkFlattenable::Factory factory = flattenable->getFactory();
    SkASSERT(factory != NULL);

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
    if (this->isValidating()) {
        this->writeString(flattenable->getTypeName());
    } else if (fFactorySet) {
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
    size_t offset = fWriter.bytesWritten();
    // now flatten the object
    flattenable->flatten(*this);
    size_t objSize = fWriter.bytesWritten() - offset;
    // record the obj's size
    fWriter.overwriteTAt(offset - sizeof(uint32_t), SkToU32(objSize));
}
