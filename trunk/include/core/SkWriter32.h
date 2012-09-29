
/*
 * Copyright 2008 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkWriter32_DEFINED
#define SkWriter32_DEFINED

#include "SkTypes.h"

#include "SkScalar.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkRect.h"
#include "SkMatrix.h"
#include "SkRegion.h"

class SkStream;
class SkWStream;

class SkWriter32 : SkNoncopyable {
public:
    /**
     *  The caller can specify an initial block of storage, which the caller manages.
     *  SkWriter32 will not attempt to free this in its destructor. It is up to the
     *  implementation to decide if, and how much, of the storage to utilize, and it
     *  is possible that it may be ignored entirely.
     */
    SkWriter32(size_t minSize, void* initialStorage, size_t storageSize);

    SkWriter32(size_t minSize)
        : fMinSize(minSize),
          fSize(0),
          fSingleBlock(NULL),
          fSingleBlockSize(0),
          fHead(NULL),
          fTail(NULL),
          fHeadIsExternalStorage(false) {}

    ~SkWriter32();

    /**
     *  Returns the single block backing the writer, or NULL if the memory is
     *  to be dynamically allocated.
     */
    void* getSingleBlock() const { return fSingleBlock; }

    // return the current offset (will always be a multiple of 4)
    uint32_t bytesWritten() const { return fSize; }
    // DEPRECATED: use byetsWritten instead
    uint32_t  size() const { return this->bytesWritten(); }

    void      reset();
    uint32_t* reserve(size_t size); // size MUST be multiple of 4

    /**
     *  Specify the single block to back the writer, rathern than dynamically
     *  allocating the memory. If block == NULL, then the writer reverts to
     *  dynamic allocation (and resets).
     */
    void reset(void* block, size_t size);

    bool writeBool(bool value) {
        this->writeInt(value);
        return value;
    }

    void writeInt(int32_t value) {
        *(int32_t*)this->reserve(sizeof(value)) = value;
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

    void writePtr(void* ptr) {
        // Since we "know" that we're always 4-byte aligned, we can tell the
        // compiler that here, by assigning to an int32 ptr.
        int32_t* addr = (int32_t*)this->reserve(sizeof(void*));
        if (4 == sizeof(void*)) {
            *(void**)addr = ptr;
        } else {
            memcpy(addr, &ptr, sizeof(void*));
        }
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
        // if we could query how much is avail in the current block, we might
        // copy that much, and then alloc the rest. That would reduce the waste
        // in the current block
        memcpy(this->reserve(size), values, size);
    }

    void writePad(const void* src, size_t size);

    /**
     *  Writes a string to the writer, which can be retrieved with
     *  SkReader32::readString().
     *  The length can be specified, or if -1 is passed, it will be computed by
     *  calling strlen(). The length must be < 0xFFFF
     */
    void writeString(const char* str, size_t len = (size_t)-1);

    /**
     *  Computes the size (aligned to multiple of 4) need to write the string
     *  in a call to writeString(). If the length is not specified, it will be
     *  computed by calling strlen().
     */
    static size_t WriteStringSize(const char* str, size_t len = (size_t)-1);

    // return the address of the 4byte int at the specified offset (which must
    // be a multiple of 4. This does not allocate any new space, so the returned
    // address is only valid for 1 int.
    uint32_t* peek32(size_t offset);

    /**
     *  Move the cursor back to offset bytes from the beginning.
     *  This has the same restrictions as peek32: offset must be <= size() and
     *  offset must be a multiple of 4.
     */
    void rewindToOffset(size_t offset);

    // copy into a single buffer (allocated by caller). Must be at least size()
    void flatten(void* dst) const;

    // read from the stream, and write up to length bytes. Return the actual
    // number of bytes written.
    size_t readFromStream(SkStream*, size_t length);

    bool writeToStream(SkWStream*);

private:
    size_t      fMinSize;
    uint32_t    fSize;

    char*       fSingleBlock;
    uint32_t    fSingleBlockSize;

    struct Block;
    Block*  fHead;
    Block*  fTail;

    bool fHeadIsExternalStorage;

    Block* newBlock(size_t bytes);

    SkDEBUGCODE(void validate() const;)
};

/**
 *  Helper class to allocated SIZE bytes as part of the writer, and to provide
 *  that storage to the constructor as its initial storage buffer.
 *
 *  This wrapper ensures proper alignment rules are met for the storage.
 */
template <size_t SIZE> class SkSWriter32 : public SkWriter32 {
public:
    SkSWriter32(size_t minSize) : SkWriter32(minSize, fData.fStorage, SIZE) {}

private:
    union {
        void*   fPtrAlignment;
        double  fDoubleAlignment;
        char    fStorage[SIZE];
    } fData;
};

#endif
