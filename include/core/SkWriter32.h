
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
#include "SkRRect.h"
#include "SkMatrix.h"
#include "SkRegion.h"

class SkStream;
class SkWStream;

class SkWriter32 : SkNoncopyable {
    struct BlockHeader;
public:
    /**
     *  The caller can specify an initial block of storage, which the caller manages.
     *  SkWriter32 will not attempt to free this in its destructor. It is up to the
     *  implementation to decide if, and how much, of the storage to utilize, and it
     *  is possible that it may be ignored entirely.
     */
    SkWriter32(size_t minSize, void* initialStorage, size_t storageSize);

    SkWriter32(size_t minSize)
        : fHead(NULL)
        , fTail(NULL)
        , fMinSize(minSize)
        , fSize(0)
        , fWrittenBeforeLastBlock(0)
        {}

    ~SkWriter32();

    // return the current offset (will always be a multiple of 4)
    uint32_t bytesWritten() const { return fSize; }
    // DEPRECATED: use bytesWritten instead  TODO(mtklein): clean up
    uint32_t  size() const { return this->bytesWritten(); }

    // Returns true if we've written only into the storage passed into constructor or reset.
    // (You may be able to use this to avoid a call to flatten.)
    bool wroteOnlyToStorage() const {
        return fHead == &fExternalBlock && this->bytesWritten() <= fExternalBlock.fSizeOfBlock;
    }

    void reset();
    void reset(void* storage, size_t size);

    // size MUST be multiple of 4
    uint32_t* reserve(size_t size) {
        SkASSERT(SkAlign4(size) == size);

        Block* block = fTail;
        if (NULL == block || block->available() < size) {
            block = this->doReserve(size);
        }
        fSize += size;
        return block->alloc(size);
    }

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
        // if we could query how much is avail in the current block, we might
        // copy that much, and then alloc the rest. That would reduce the waste
        // in the current block
        memcpy(this->reserve(size), values, size);
    }

    /**
     *  Reserve size bytes. Does not need to be 4 byte aligned. The remaining space (if any) will be
     *  filled in with zeroes.
     */
    uint32_t* reservePad(size_t size);

    /**
     *  Write size bytes from src, and pad to 4 byte alignment with zeroes.
     */
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
    struct Block {
        Block*  fNext;
        char*   fBasePtr;
        size_t  fSizeOfBlock;      // total space allocated (after this)
        size_t  fAllocatedSoFar;    // space used so far

        size_t  available() const { return fSizeOfBlock - fAllocatedSoFar; }
        char*   base() { return fBasePtr; }
        const char* base() const { return fBasePtr; }

        uint32_t* alloc(size_t size) {
            SkASSERT(SkAlign4(size) == size);
            SkASSERT(this->available() >= size);
            void* ptr = this->base() + fAllocatedSoFar;
            fAllocatedSoFar += size;
            SkASSERT(fAllocatedSoFar <= fSizeOfBlock);
            return (uint32_t*)ptr;
        }

        uint32_t* peek32(size_t offset) {
            SkASSERT(offset <= fAllocatedSoFar + 4);
            void* ptr = this->base() + offset;
            return (uint32_t*)ptr;
        }

        void rewind() {
            fNext = NULL;
            fAllocatedSoFar = 0;
            // keep fSizeOfBlock as is
        }

        static Block* Create(size_t size) {
            SkASSERT(SkIsAlign4(size));
            Block* block = (Block*)sk_malloc_throw(sizeof(Block) + size);
            block->fNext = NULL;
            block->fBasePtr = (char*)(block + 1);
            block->fSizeOfBlock = size;
            block->fAllocatedSoFar = 0;
            return block;
        }

        Block* initFromStorage(void* storage, size_t size) {
            SkASSERT(SkIsAlign4((intptr_t)storage));
            SkASSERT(SkIsAlign4(size));
            Block* block = this;
            block->fNext = NULL;
            block->fBasePtr = (char*)storage;
            block->fSizeOfBlock = size;
            block->fAllocatedSoFar = 0;
            return block;
        }
    };

    enum {
        MIN_BLOCKSIZE = sizeof(SkWriter32::Block) + sizeof(intptr_t)
    };

    Block       fExternalBlock;
    Block*      fHead;
    Block*      fTail;
    size_t      fMinSize;
    uint32_t    fSize;
    // sum of bytes written in all blocks *before* fTail
    uint32_t    fWrittenBeforeLastBlock;

    bool isHeadExternallyAllocated() const {
        return fHead == &fExternalBlock;
    }

    Block* newBlock(size_t bytes);

    // only call from reserve()
    Block* doReserve(size_t bytes);

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
