/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkWriter32_DEFINED
#define SkWriter32_DEFINED

#include "include/core/SkData.h"
#include "include/core/SkPath.h"
#include "include/core/SkPoint.h"
#include "include/core/SkPoint3.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"

#include <cstdint>
#include <cstring>

class SkMatrix;
struct SkSamplingOptions;

class SkWriter32 : SkNoncopyable {
public:
    /**
     *  The caller can specify an initial block of storage, which the caller manages.
     *
     *  SkWriter32 will try to back reserve and write calls with this external storage until the
     *  first time an allocation doesn't fit.  From then it will use dynamically allocated storage.
     *  This used to be optional behavior, but pipe now relies on it.
     */
    SkWriter32(void* external = nullptr, size_t externalBytes = 0) {
        this->reset(external, externalBytes);
    }

    // return the current offset (will always be a multiple of 4)
    size_t bytesWritten() const { return fUsed; }

    // Returns true iff all of the bytes written so far are stored in the initial storage
    // buffer provided in the constructor or the most recent call to reset.
    bool usingInitialStorage() const { return fData == fExternal; }

    void reset(void* external = nullptr, size_t externalBytes = 0) {
        // we cast this pointer to int* and float* at times, so assert that it is aligned.
        SkASSERT(SkIsAlign4((uintptr_t)external));
        // we always write multiples of 4-bytes, so truncate down the size to match that
        externalBytes &= ~3;

        fData = (uint8_t*)external;
        fCapacity = externalBytes;
        fUsed = 0;
        fExternal = external;
    }

    // size MUST be multiple of 4
    uint32_t* reserve(size_t size) {
        SkASSERT(SkAlign4(size) == size);
        size_t offset = fUsed;
        size_t totalRequired = fUsed + size;
        if (totalRequired > fCapacity) {
            this->growToAtLeast(totalRequired);
        }
        fUsed = totalRequired;
        return (uint32_t*)(fData + offset);
    }

    /**
     *  Read a T record at offset, which must be a multiple of 4. Only legal if the record
     *  was written atomically using the write methods below.
     */
    template<typename T>
    const T& readTAt(size_t offset) const {
        SkASSERT(SkAlign4(offset) == offset);
        SkASSERT(offset < fUsed);
        return *(T*)(fData + offset);
    }

    /**
     *  Overwrite a T record at offset, which must be a multiple of 4. Only legal if the record
     *  was written atomically using the write methods below.
     */
    template<typename T>
    void overwriteTAt(size_t offset, const T& value) {
        SkASSERT(SkAlign4(offset) == offset);
        SkASSERT(offset < fUsed);
        *(T*)(fData + offset) = value;
    }

    bool writeBool(bool value) {
        this->write32(value);
        return value;
    }

    void writeInt(int32_t value) {
        this->write32(value);
    }

    void write8(int32_t value) {
        *(int32_t*)this->reserve(sizeof(value)) = value & 0xFF;
    }

    void write16(int32_t value) {
        *(int32_t*)this->reserve(sizeof(value)) = value & 0xFFFF;
    }

    void write32(int32_t value) {
        *(int32_t*)this->reserve(sizeof(value)) = value;
    }

    void writeScalar(SkScalar value) {
        *(SkScalar*)this->reserve(sizeof(value)) = value;
    }

    void writePoint(const SkPoint& pt) {
        *(SkPoint*)this->reserve(sizeof(pt)) = pt;
    }

    void writePoint3(const SkPoint3& pt) {
        *(SkPoint3*)this->reserve(sizeof(pt)) = pt;
    }

    void writeRect(const SkRect& rect) {
        *(SkRect*)this->reserve(sizeof(rect)) = rect;
    }

    void writeIRect(const SkIRect& rect) {
        *(SkIRect*)this->reserve(sizeof(rect)) = rect;
    }

    void writeRRect(const SkRRect& rrect) {
        rrect.writeToMemory(this->reserve(SkRRect::kSizeInMemory));
    }

    void writePath(const SkPath& path) {
        size_t size = path.writeToMemory(nullptr);
        SkASSERT(SkAlign4(size) == size);
        path.writeToMemory(this->reserve(size));
    }

    void writeMatrix(const SkMatrix& matrix);

    void writeRegion(const SkRegion& rgn) {
        size_t size = rgn.writeToMemory(nullptr);
        SkASSERT(SkAlign4(size) == size);
        rgn.writeToMemory(this->reserve(size));
    }

    void writeSampling(const SkSamplingOptions& sampling);

    // write count bytes (must be a multiple of 4)
    void writeMul4(const void* values, size_t size) {
        this->write(values, size);
    }

    /**
     *  Write size bytes from values. size must be a multiple of 4, though
     *  values need not be 4-byte aligned.
     */
    void write(const void* values, size_t size) {
        SkASSERT(SkAlign4(size) == size);
        sk_careful_memcpy(this->reserve(size), values, size);
    }

    /**
     *  Reserve size bytes. Does not need to be 4 byte aligned. The remaining space (if any) will be
     *  filled in with zeroes.
     */
    uint32_t* reservePad(size_t size) {
        size_t alignedSize = SkAlign4(size);
        uint32_t* p = this->reserve(alignedSize);
        if (alignedSize != size) {
            SkASSERT(alignedSize >= 4);
            p[alignedSize / 4 - 1] = 0;
        }
        return p;
    }

    /**
     *  Write size bytes from src, and pad to 4 byte alignment with zeroes.
     */
    void writePad(const void* src, size_t size) {
        sk_careful_memcpy(this->reservePad(size), src, size);
    }

    /**
     *  Writes a string to the writer, which can be retrieved with SkReadBuffer::readString().
     *  The length can be specified, or if -1 is passed, it will be computed by calling strlen().
     *  The length must be < max size_t.
     *
     *  If you write NULL, it will be read as "".
     */
    void writeString(const char* str, size_t len = (size_t)-1);

    /**
     *  Computes the size (aligned to multiple of 4) need to write the string
     *  in a call to writeString(). If the length is not specified, it will be
     *  computed by calling strlen().
     */
    static size_t WriteStringSize(const char* str, size_t len = (size_t)-1);

    void writeData(const SkData* data) {
        uint32_t len = data ? SkToU32(data->size()) : 0;
        this->write32(len);
        if (data) {
            this->writePad(data->data(), len);
        }
    }

    static size_t WriteDataSize(const SkData* data) {
        return 4 + SkAlign4(data ? data->size() : 0);
    }

    /**
     *  Move the cursor back to offset bytes from the beginning.
     *  offset must be a multiple of 4 no greater than size().
     */
    void rewindToOffset(size_t offset) {
        SkASSERT(SkAlign4(offset) == offset);
        SkASSERT(offset <= bytesWritten());
        fUsed = offset;
    }

    // copy into a single buffer (allocated by caller). Must be at least size()
    void flatten(void* dst) const {
        memcpy(dst, fData, fUsed);
    }

    bool writeToStream(SkWStream* stream) const {
        return stream->write(fData, fUsed);
    }

    // read from the stream, and write up to length bytes. Return the actual
    // number of bytes written.
    size_t readFromStream(SkStream* stream, size_t length) {
        return stream->read(this->reservePad(length), length);
    }

    /**
     *  Captures a snapshot of the data as it is right now, and return it.
     */
    sk_sp<SkData> snapshotAsData() const;
private:
    void growToAtLeast(size_t size);

    uint8_t* fData;                    // Points to either fInternal or fExternal.
    size_t fCapacity;                  // Number of bytes we can write to fData.
    size_t fUsed;                      // Number of bytes written.
    void* fExternal;                   // Unmanaged memory block.
    skia_private::AutoTMalloc<uint8_t> fInternal;  // Managed memory block.
};

/**
 *  Helper class to allocated SIZE bytes as part of the writer, and to provide
 *  that storage to the constructor as its initial storage buffer.
 *
 *  This wrapper ensures proper alignment rules are met for the storage.
 */
template <size_t SIZE> class SkSWriter32 : public SkWriter32 {
public:
    SkSWriter32() { this->reset(); }

    void reset() {this->INHERITED::reset(fData.fStorage, SIZE); }

private:
    union {
        void*   fPtrAlignment;
        double  fDoubleAlignment;
        char    fStorage[SIZE];
    } fData;

    using INHERITED = SkWriter32;
};

#endif
