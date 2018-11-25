/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkWriteBuffer.h"
#include "SkBitmap.h"
#include "SkData.h"
#include "SkDeduper.h"
#include "SkPaint.h"
#include "SkPixelRef.h"
#include "SkPtrRecorder.h"
#include "SkStream.h"
#include "SkTypeface.h"

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBinaryWriteBuffer::SkBinaryWriteBuffer()
    : fFactorySet(nullptr)
    , fTFSet(nullptr) {
}

SkBinaryWriteBuffer::SkBinaryWriteBuffer(void* storage, size_t storageSize)
    : fFactorySet(nullptr)
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

void SkBinaryWriteBuffer::writeColor4f(const SkColor4f& color) {
    fWriter.write(&color, sizeof(SkColor4f));
}

void SkBinaryWriteBuffer::writeColor4fArray(const SkColor4f* color, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(color, count * sizeof(SkColor4f));
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

void SkBinaryWriteBuffer::writeImage(const SkImage* image) {
    if (fDeduper) {
        this->write32(fDeduper->findOrDefineImage(const_cast<SkImage*>(image)));
        return;
    }

    this->writeInt(image->width());
    this->writeInt(image->height());

    auto write_data = [this](sk_sp<SkData> data, int sign) {
        size_t size = data ? data->size() : 0;
        if (!sk_64_isS32(size)) {
            size = 0;   // too big to store
        }
        if (size) {
            this->write32(SkToS32(size) * sign);
            this->writePad32(data->data(), size);    // does nothing if size == 0
            this->write32(0);   // origin-x
            this->write32(0);   // origin-y
        } else {
            this->write32(0);   // signal no image
        }
    };

    /*
     *  What follows is a 32bit encoded size.
     *   0 : failure, nothing else to do
     *  <0 : negative (int32_t) of a custom encoded blob using SerialProcs
     *  >0 : standard encoded blob size (use MakeFromEncoded)
     */
    sk_sp<SkData> data;
    int sign = 1;   // +1 signals standard encoder
    if (fProcs.fImageProc) {
        data = fProcs.fImageProc(const_cast<SkImage*>(image), fProcs.fImageCtx);
        sign = -1;  // +1 signals custom encoder
    }
    // We check data, since a custom proc can return nullptr, in which case we behave as if
    // there was no custom proc.
    if (!data) {
        data = image->encodeToData();
        sign = 1;
    }
    write_data(std::move(data), sign);
}

void SkBinaryWriteBuffer::writeTypeface(SkTypeface* obj) {
    if (fDeduper) {
        this->write32(fDeduper->findOrDefineTypeface(obj));
        return;
    }

    // Write 32 bits (signed)
    //   0 -- default font
    //  >0 -- index
    //  <0 -- custom (serial procs)

    if (obj == nullptr) {
        fWriter.write32(0);
    } else if (fProcs.fTypefaceProc) {
        auto data = fProcs.fTypefaceProc(obj, fProcs.fTypefaceCtx);
        if (data) {
            size_t size = data->size();
            if (!sk_64_isS32(size)) {
                size = 0;               // fall back to default font
            }
            int32_t ssize = SkToS32(size);
            fWriter.write32(-ssize);    // negative to signal custom
            if (size) {
                this->writePad32(data->data(), size);
            }
            return;
        }
        // no data means fall through for std behavior
    }
    fWriter.write32(fTFSet ? fTFSet->add(obj) : 0);
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

void SkBinaryWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    if (nullptr == flattenable) {
        this->write32(0);
        return;
    }

    if (fDeduper) {
        this->write32(fDeduper->findOrDefineFactory(const_cast<SkFlattenable*>(flattenable)));
    } else {
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
