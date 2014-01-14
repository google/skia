
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkWriter32_DEFINED
#define SkWriter32_DEFINED

#include "SkMatrix.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRRect.h"
#include "SkRect.h"
#include "SkRegion.h"
#include "SkScalar.h"
#include "SkStream.h"
#include "SkTDArray.h"
#include "SkTypes.h"

class SkWriter32 : SkNoncopyable {
public:
    /**
     *  The caller can specify an initial block of storage, which the caller manages.
     *
     *  SkWriter32 will try to back reserve and write calls with this external storage until the
     *  first time an allocation doesn't fit.  From then it will use dynamically allocated storage.
     *  This used to be optional behavior, but pipe now relies on it.
     */
    SkWriter32(void* external = NULL, size_t externalBytes = 0) {
        this->reset(external, externalBytes);
    }

    // return the current offset (will always be a multiple of 4)
    size_t bytesWritten() const { return fCount * 4; }

    SK_ATTR_DEPRECATED("use bytesWritten")
    size_t size() const { return this->bytesWritten(); }

    void reset(void* external = NULL, size_t externalBytes = 0) {
        SkASSERT(SkIsAlign4((uintptr_t)external));
        SkASSERT(SkIsAlign4(externalBytes));
        fExternal = (uint32_t*)external;
        fExternalLimit = externalBytes/4;
        fCount = 0;
        fInternal.rewind();
    }

    // If all data written is contiguous, then this returns a pointer to it, otherwise NULL.
    // This will work if we've only written to the externally supplied block of storage, or if we've
    // only written to our internal dynamic storage, but will fail if we have written into both.
    const uint32_t* contiguousArray() const {
        if (this->externalCount()  == 0) {
            return fInternal.begin();
        } else if (fInternal.isEmpty()) {
            return fExternal;
        }
        return NULL;
    }

    // size MUST be multiple of 4
    uint32_t* reserve(size_t size) {
        SkASSERT(SkAlign4(size) == size);
        const int count = size/4;

        uint32_t* p;
        // Once we start writing to fInternal, we never write to fExternal again.
        // This simplifies tracking what data is where.
        if (fInternal.isEmpty() && this->externalCount() + count <= fExternalLimit) {
            p = fExternal + fCount;
        } else {
            p = fInternal.append(count);
        }

        fCount += count;
        return p;
    }

    // return the address of the 4byte int at the specified offset (which must
    // be a multiple of 4. This does not allocate any new space, so the returned
    // address is only valid for 1 int.
    uint32_t* peek32(size_t offset) {
        SkASSERT(SkAlign4(offset) == offset);
        const int count = offset/4;
        SkASSERT(count < fCount);

        if (count < this->externalCount()) {
            return fExternal + count;
        }
        return &fInternal[count - this->externalCount()];
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

    void writePtr(void* value) {
        *(void**)this->reserve(sizeof(value)) = value;
    }

    void writeScalar(SkScalar value) {
        *(SkScalar*)this->reserve(sizeof(value)) = value;
    }

    void writePoint(const SkPoint& pt) {
        *(SkPoint*)this->reserve(sizeof(pt)) = pt;
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
        size_t size = path.writeToMemory(NULL);
        SkASSERT(SkAlign4(size) == size);
        path.writeToMemory(this->reserve(size));
    }

    void writeMatrix(const SkMatrix& matrix) {
        size_t size = matrix.writeToMemory(NULL);
        SkASSERT(SkAlign4(size) == size);
        matrix.writeToMemory(this->reserve(size));
    }

    void writeRegion(const SkRegion& rgn) {
        size_t size = rgn.writeToMemory(NULL);
        SkASSERT(SkAlign4(size) == size);
        rgn.writeToMemory(this->reserve(size));
    }

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
        // TODO: If we're going to spill from fExternal to fInternal, we might want to fill
        // fExternal as much as possible before writing to fInternal.
        memcpy(this->reserve(size), values, size);
    }

    /**
     *  Reserve size bytes. Does not need to be 4 byte aligned. The remaining space (if any) will be
     *  filled in with zeroes.
     */
    uint32_t* reservePad(size_t size) {
        uint32_t* p = this->reserve(SkAlign4(size));
        uint8_t* tail = (uint8_t*)p + size;
        switch (SkAlign4(size) - size) {
            default: SkDEBUGFAIL("SkAlign4(x) - x should always be 0, 1, 2, or 3.");
            case 3: *tail++ = 0x00;  // fallthrough is intentional
            case 2: *tail++ = 0x00;  // fallthrough is intentional
            case 1: *tail++ = 0x00;
            case 0: ;/*nothing to do*/
        }
        return p;
    }

    /**
     *  Write size bytes from src, and pad to 4 byte alignment with zeroes.
     */
    void writePad(const void* src, size_t size) {
        memcpy(this->reservePad(size), src, size);
    }

    /**
     *  Writes a string to the writer, which can be retrieved with
     *  SkReader32::readString().
     *  The length can be specified, or if -1 is passed, it will be computed by
     *  calling strlen(). The length must be < max size_t.
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

    /**
     *  Move the cursor back to offset bytes from the beginning.
     *  This has the same restrictions as peek32: offset must be <= size() and
     *  offset must be a multiple of 4.
     */
    void rewindToOffset(size_t offset) {
        SkASSERT(SkAlign4(offset) == offset);
        const int count = offset/4;
        if (count < this->externalCount()) {
            fInternal.setCount(0);
        } else {
            fInternal.setCount(count - this->externalCount());
        }
        fCount = count;
    }

    // copy into a single buffer (allocated by caller). Must be at least size()
    void flatten(void* dst) const {
        const size_t externalBytes = this->externalCount()*4;
        memcpy(dst, fExternal, externalBytes);
        dst = (uint8_t*)dst + externalBytes;
        memcpy(dst, fInternal.begin(), fInternal.bytes());
    }

    bool writeToStream(SkWStream* stream) const {
        return stream->write(fExternal, this->externalCount()*4)
            && stream->write(fInternal.begin(), fInternal.bytes());
    }

    // read from the stream, and write up to length bytes. Return the actual
    // number of bytes written.
    size_t readFromStream(SkStream* stream, size_t length) {
        return stream->read(this->reservePad(length), length);
    }

private:
    // Number of uint32_t written into fExternal.  <= fExternalLimit.
    int externalCount() const { return fCount - fInternal.count(); }

    int fCount;                     // Total number of uint32_t written.
    int fExternalLimit;             // Number of uint32_t we can write to fExternal.
    uint32_t* fExternal;            // Unmanaged memory block.
    SkTDArray<uint32_t> fInternal;  // Managed memory block.
};

/**
 *  Helper class to allocated SIZE bytes as part of the writer, and to provide
 *  that storage to the constructor as its initial storage buffer.
 *
 *  This wrapper ensures proper alignment rules are met for the storage.
 */
template <size_t SIZE> class SkSWriter32 : public SkWriter32 {
public:
    SkSWriter32() : SkWriter32(fData.fStorage, SIZE) {}

private:
    union {
        void*   fPtrAlignment;
        double  fDoubleAlignment;
        char    fStorage[SIZE];
    } fData;
};

#endif
