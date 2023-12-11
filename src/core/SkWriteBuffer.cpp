/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkWriteBuffer.h"

#include "include/core/SkAlphaType.h"
#include "include/core/SkData.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImage.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRect.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkMatrixPriv.h"
#include "src/core/SkMipmap.h"
#include "src/core/SkPaintPriv.h"
#include "src/core/SkPtrRecorder.h"
#include "src/image/SkImage_Base.h"

#if !defined(SK_DISABLE_LEGACY_PNG_WRITEBUFFER)
#include "include/core/SkBitmap.h"
#include "include/core/SkStream.h"
#include "include/encode/SkPngEncoder.h"
#endif

#include <cstring>
#include <utility>

class SkMatrix;
class SkPaint;
class SkRegion;

///////////////////////////////////////////////////////////////////////////////////////////////////

SkBinaryWriteBuffer::SkBinaryWriteBuffer(const SkSerialProcs& p)
        : SkWriteBuffer(p), fFactorySet(nullptr), fTFSet(nullptr) {}

SkBinaryWriteBuffer::SkBinaryWriteBuffer(void* storage, size_t storageSize, const SkSerialProcs& p)
        : SkWriteBuffer(p), fFactorySet(nullptr), fTFSet(nullptr), fWriter(storage, storageSize) {}

SkBinaryWriteBuffer::~SkBinaryWriteBuffer() {}

bool SkBinaryWriteBuffer::usingInitialStorage() const {
    return fWriter.usingInitialStorage();
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

void SkBinaryWriteBuffer::writeString(std::string_view value) {
    fWriter.writeString(value.data(), value.size());
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

void SkBinaryWriteBuffer::writePoint3(const SkPoint3& point) {
    this->writePad32(&point, sizeof(SkPoint3));
}

void SkBinaryWriteBuffer::writePointArray(const SkPoint* point, uint32_t count) {
    fWriter.write32(count);
    fWriter.write(point, count * sizeof(SkPoint));
}

void SkBinaryWriteBuffer::write(const SkM44& matrix) {
    fWriter.write(SkMatrixPriv::M44ColMajor(matrix), sizeof(float) * 16);
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

void SkBinaryWriteBuffer::writeSampling(const SkSamplingOptions& sampling) {
    fWriter.writeSampling(sampling);
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

bool SkBinaryWriteBuffer::writeToStream(SkWStream* stream) const {
    return fWriter.writeToStream(stream);
}

static sk_sp<SkData> serialize_image(const SkImage* image, SkSerialProcs procs) {
    sk_sp<SkData> data;
    if (procs.fImageProc) {
        data = procs.fImageProc(const_cast<SkImage*>(image), procs.fImageCtx);
    }
    if (data) {
        return data;
    }
    // Check to see if the image's source was an encoded block of data.
    // If so, just use that.
    data = image->refEncodedData();
    if (data) {
        return data;
    }
#if !defined(SK_DISABLE_LEGACY_PNG_WRITEBUFFER)
    SkBitmap bm;
    auto ib = as_IB(image);
    if (!ib->getROPixels(ib->directContext(), &bm)) {
        return nullptr;
    }
    SkDynamicMemoryWStream stream;
    if (SkPngEncoder::Encode(&stream, bm.pixmap(), SkPngEncoder::Options())) {
        return stream.detachAsData();
    }
#endif
    return nullptr;
}

static sk_sp<SkData> serialize_mipmap(const SkMipmap* mipmap, SkSerialProcs procs) {
    /*  Format
        count_levels:32
        for each level, starting with the biggest (index 0 in our iterator)
            encoded_size:32
            encoded_data (padded)
    */
    const int count = mipmap->countLevels();

    // This buffer does not need procs because it is just writing SkDatas
    SkBinaryWriteBuffer buffer({});
    buffer.write32(count);
    for (int i = 0; i < count; ++i) {
        SkMipmap::Level level;
        if (mipmap->getLevel(i, &level)) {
            sk_sp<SkImage> levelImage = SkImages::RasterFromPixmap(level.fPixmap, nullptr, nullptr);
            sk_sp<SkData> levelData = serialize_image(levelImage.get(), procs);
            buffer.writeDataAsByteArray(levelData.get());
        } else {
            return nullptr;
        }
    }
    return buffer.snapshotAsData();
}

/*  Format:
 *      flags: U32
 *      encoded : size_32 + data[]
 *      [subset: IRect]
 *      [mips]  : size_32 + data[]
 */
void SkBinaryWriteBuffer::writeImage(const SkImage* image) {
    uint32_t flags = 0;
    const SkMipmap* mips = as_IB(image)->onPeekMips();
    if (mips) {
        flags |= SkWriteBufferImageFlags::kHasMipmap;
    }
    if (image->alphaType() == kUnpremul_SkAlphaType) {
        flags |= SkWriteBufferImageFlags::kUnpremul;
    }

    this->write32(flags);

    sk_sp<SkData> data = serialize_image(image, fProcs);
    SkASSERT(data);
    this->writeDataAsByteArray(data.get());

    if (flags & SkWriteBufferImageFlags::kHasMipmap) {
        sk_sp<SkData> mipData = serialize_mipmap(mips, fProcs);
        this->writeDataAsByteArray(mipData.get());
    }
}

void SkBinaryWriteBuffer::writeTypeface(SkTypeface* obj) {
    // Write 32 bits (signed)
    //   0 -- empty font
    //  >0 -- index
    //  <0 -- custom (serial procs)

    if (obj == nullptr) {
        fWriter.write32(0);
    } else if (fProcs.fTypefaceProc) {
        auto data = fProcs.fTypefaceProc(obj, fProcs.fTypefaceCtx);
        if (data) {
            size_t size = data->size();
            if (!SkTFitsIn<int32_t>(size)) {
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
    SkPaintPriv::Flatten(paint, *this);
}

void SkBinaryWriteBuffer::setFactoryRecorder(sk_sp<SkFactorySet> rec) {
    fFactorySet = std::move(rec);
}

void SkBinaryWriteBuffer::setTypefaceRecorder(sk_sp<SkRefCntSet> rec) {
    fTFSet = std::move(rec);
}

void SkBinaryWriteBuffer::writeFlattenable(const SkFlattenable* flattenable) {
    if (nullptr == flattenable) {
        this->write32(0);
        return;
    }

    /*
     *  We can write 1 of 2 versions of the flattenable:
     *
     *  1. index into fFactorySet: This assumes the writer will later resolve the function-ptrs
     *     into strings for its reader. SkPicture does exactly this, by writing a table of names
     *     (matching the indices) up front in its serialized form.
     *
     *  2. string name of the flattenable or index into fFlattenableDict:  We store the string to
     *     allow the reader to specify its own factories after write time. In order to improve
     *     compression, if we have already written the string, we write its index instead.
     */

    if (SkFlattenable::Factory factory = flattenable->getFactory(); factory && fFactorySet) {
        this->write32(fFactorySet->add(factory));
    } else {
        const char* name = flattenable->getTypeName();
        SkASSERT(name);
        SkASSERT(0 != strcmp("", name));

        if (uint32_t* indexPtr = fFlattenableDict.find(name)) {
            // We will write the index as a 32-bit int.  We want the first byte
            // that we send to be zero - this will act as a sentinel that we
            // have an index (not a string).  This means that we will send the
            // the index shifted left by 8.  The remaining 24-bits should be
            // plenty to store the index.  Note that this strategy depends on
            // being little endian, and type names being non-empty.
            SkASSERT(0 == *indexPtr >> 24);
            this->write32(*indexPtr << 8);
        } else {
            this->writeString(name);
            fFlattenableDict.set(name, fFlattenableDict.count() + 1);
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
