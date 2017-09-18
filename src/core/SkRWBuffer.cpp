/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRWBuffer.h"

#include "SkAtomics.h"
#include "SkMalloc.h"
#include "SkMakeUnique.h"
#include "SkStream.h"

// Force small chunks to be a page's worth
static const size_t kMinAllocSize = 4096;

struct SkBufferBlock {
    SkBufferBlock*  fNext;      // updated by the writer
    size_t          fUsed;      // updated by the writer
    const size_t    fCapacity;

    SkBufferBlock(size_t capacity) : fNext(nullptr), fUsed(0), fCapacity(capacity) {}

    const void* startData() const { return this + 1; }

    size_t avail() const { return fCapacity - fUsed; }
    void* availData() { return (char*)this->startData() + fUsed; }

    static SkBufferBlock* Alloc(size_t length) {
        size_t capacity = LengthToCapacity(length);
        void* buffer = sk_malloc_throw(sizeof(SkBufferBlock) + capacity);
        return new (buffer) SkBufferBlock(capacity);
    }

    // Return number of bytes actually appended. Important that we always completely this block
    // before spilling into the next, since the reader uses fCapacity to know how many it can read.
    //
    size_t append(const void* src, size_t length) {
        this->validate();
        size_t amount = SkTMin(this->avail(), length);
        memcpy(this->availData(), src, amount);
        fUsed += amount;
        this->validate();
        return amount;
    }

    // Do not call in the reader thread, since the writer may be updating fUsed.
    // (The assertion is still true, but TSAN still may complain about its raciness.)
    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fCapacity > 0);
        SkASSERT(fUsed <= fCapacity);
#endif
    }

private:
    static size_t LengthToCapacity(size_t length) {
        const size_t minSize = kMinAllocSize - sizeof(SkBufferBlock);
        return SkTMax(length, minSize);
    }
};

struct SkBufferHead {
    mutable int32_t fRefCnt;
    SkBufferBlock   fBlock;

    SkBufferHead(size_t capacity) : fRefCnt(1), fBlock(capacity) {}

    static size_t LengthToCapacity(size_t length) {
        const size_t minSize = kMinAllocSize - sizeof(SkBufferHead);
        return SkTMax(length, minSize);
    }

    static SkBufferHead* Alloc(size_t length) {
        size_t capacity = LengthToCapacity(length);
        size_t size = sizeof(SkBufferHead) + capacity;
        void* buffer = sk_malloc_throw(size);
        return new (buffer) SkBufferHead(capacity);
    }

    void ref() const {
        SkASSERT(fRefCnt > 0);
        sk_atomic_inc(&fRefCnt);
    }

    void unref() const {
        SkASSERT(fRefCnt > 0);
        // A release here acts in place of all releases we "should" have been doing in ref().
        if (1 == sk_atomic_fetch_add(&fRefCnt, -1, sk_memory_order_acq_rel)) {
            // Like unique(), the acquire is only needed on success.
            SkBufferBlock* block = fBlock.fNext;
            sk_free((void*)this);
            while (block) {
                SkBufferBlock* next = block->fNext;
                sk_free(block);
                block = next;
            }
        }
    }

    void validate(size_t minUsed, const SkBufferBlock* tail = nullptr) const {
#ifdef SK_DEBUG
        SkASSERT(fRefCnt > 0);
        size_t totalUsed = 0;
        const SkBufferBlock* block = &fBlock;
        const SkBufferBlock* lastBlock = block;
        while (block) {
            block->validate();
            totalUsed += block->fUsed;
            lastBlock = block;
            block = block->fNext;
        }
        SkASSERT(minUsed <= totalUsed);
        if (tail) {
            SkASSERT(tail == lastBlock);
        }
#endif
    }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// The reader can only access block.fCapacity (which never changes), and cannot access
// block.fUsed, which may be updated by the writer.
//
SkROBuffer::SkROBuffer(const SkBufferHead* head, size_t available, const SkBufferBlock* tail)
    : fHead(head), fAvailable(available), fTail(tail)
{
    if (head) {
        fHead->ref();
        SkASSERT(available > 0);
        head->validate(available, tail);
    } else {
        SkASSERT(0 == available);
        SkASSERT(!tail);
    }
}

SkROBuffer::~SkROBuffer() {
    if (fHead) {
        fHead->unref();
    }
}

SkROBuffer::Iter::Iter(const SkROBuffer* buffer) {
    this->reset(buffer);
}

SkROBuffer::Iter::Iter(const sk_sp<SkROBuffer>& buffer) {
    this->reset(buffer.get());
}

void SkROBuffer::Iter::reset(const SkROBuffer* buffer) {
    fBuffer = buffer;
    if (buffer && buffer->fHead) {
        fBlock = &buffer->fHead->fBlock;
        fRemaining = buffer->fAvailable;
    } else {
        fBlock = nullptr;
        fRemaining = 0;
    }
}

const void* SkROBuffer::Iter::data() const {
    return fRemaining ? fBlock->startData() : nullptr;
}

size_t SkROBuffer::Iter::size() const {
    if (!fBlock) {
        return 0;
    }
    return SkTMin(fBlock->fCapacity, fRemaining);
}

bool SkROBuffer::Iter::next() {
    if (fRemaining) {
        fRemaining -= this->size();
        if (fBuffer->fTail == fBlock) {
            // There are more blocks, but fBuffer does not know about them.
            SkASSERT(0 == fRemaining);
            fBlock = nullptr;
        } else {
            fBlock = fBlock->fNext;
        }
    }
    return fRemaining != 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////

SkRWBuffer::SkRWBuffer(size_t initialCapacity) : fHead(nullptr), fTail(nullptr), fTotalUsed(0) {
    if (initialCapacity) {
        fHead = SkBufferHead::Alloc(initialCapacity);
        fTail = &fHead->fBlock;
    }
}

SkRWBuffer::~SkRWBuffer() {
    this->validate();
    if (fHead) {
        fHead->unref();
    }
}

// It is important that we always completely fill the current block before spilling over to the
// next, since our reader will be using fCapacity (min'd against its total available) to know how
// many bytes to read from a given block.
//
void SkRWBuffer::append(const void* src, size_t length, size_t reserve) {
    this->validate();
    if (0 == length) {
        return;
    }

    fTotalUsed += length;

    if (nullptr == fHead) {
        fHead = SkBufferHead::Alloc(length + reserve);
        fTail = &fHead->fBlock;
    }

    size_t written = fTail->append(src, length);
    SkASSERT(written <= length);
    src = (const char*)src + written;
    length -= written;

    if (length) {
        SkBufferBlock* block = SkBufferBlock::Alloc(length + reserve);
        fTail->fNext = block;
        fTail = block;
        written = fTail->append(src, length);
        SkASSERT(written == length);
    }
    this->validate();
}

#ifdef SK_DEBUG
void SkRWBuffer::validate() const {
    if (fHead) {
        fHead->validate(fTotalUsed, fTail);
    } else {
        SkASSERT(nullptr == fTail);
        SkASSERT(0 == fTotalUsed);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////

class SkROBufferStreamAsset : public SkStreamAsset {
    void validate() const {
#ifdef SK_DEBUG
        SkASSERT(fGlobalOffset <= fBuffer->size());
        SkASSERT(fLocalOffset <= fIter.size());
        SkASSERT(fLocalOffset <= fGlobalOffset);
#endif
    }

#ifdef SK_DEBUG
    class AutoValidate {
        SkROBufferStreamAsset* fStream;
    public:
        AutoValidate(SkROBufferStreamAsset* stream) : fStream(stream) { stream->validate(); }
        ~AutoValidate() { fStream->validate(); }
    };
    #define AUTO_VALIDATE   AutoValidate av(this);
#else
    #define AUTO_VALIDATE
#endif

public:
    SkROBufferStreamAsset(sk_sp<SkROBuffer> buffer) : fBuffer(std::move(buffer)), fIter(fBuffer) {
        fGlobalOffset = fLocalOffset = 0;
    }

    size_t getLength() const override { return fBuffer->size(); }

    bool rewind() override {
        AUTO_VALIDATE
        fIter.reset(fBuffer.get());
        fGlobalOffset = fLocalOffset = 0;
        return true;
    }

    size_t read(void* dst, size_t request) override {
        AUTO_VALIDATE
        size_t bytesRead = 0;
        for (;;) {
            size_t size = fIter.size();
            SkASSERT(fLocalOffset <= size);
            size_t avail = SkTMin(size - fLocalOffset, request - bytesRead);
            if (dst) {
                memcpy(dst, (const char*)fIter.data() + fLocalOffset, avail);
                dst = (char*)dst + avail;
            }
            bytesRead += avail;
            fLocalOffset += avail;
            SkASSERT(bytesRead <= request);
            if (bytesRead == request) {
                break;
            }
            // If we get here, we've exhausted the current iter
            SkASSERT(fLocalOffset == size);
            fLocalOffset = 0;
            if (!fIter.next()) {
                break;   // ran out of data
            }
        }
        fGlobalOffset += bytesRead;
        SkASSERT(fGlobalOffset <= fBuffer->size());
        return bytesRead;
    }

    bool isAtEnd() const override {
        return fBuffer->size() == fGlobalOffset;
    }

    SkStreamAsset* duplicate() const override { return new SkROBufferStreamAsset(fBuffer); }

    size_t getPosition() const override {
        return fGlobalOffset;
    }

    bool seek(size_t position) override {
        AUTO_VALIDATE
        if (position < fGlobalOffset) {
            this->rewind();
        }
        (void)this->skip(position - fGlobalOffset);
        return true;
    }

    bool move(long offset)  override{
        AUTO_VALIDATE
        offset += fGlobalOffset;
        if (offset <= 0) {
            this->rewind();
        } else {
            (void)this->seek(SkToSizeT(offset));
        }
        return true;
    }

    SkStreamAsset* fork() const override {
        SkStreamAsset* clone = this->duplicate();
        clone->seek(this->getPosition());
        return clone;
    }


private:
    sk_sp<SkROBuffer> fBuffer;
    SkROBuffer::Iter  fIter;
    size_t            fLocalOffset;
    size_t            fGlobalOffset;
};

std::unique_ptr<SkStreamAsset> SkRWBuffer::makeStreamSnapshot() const {
    return skstd::make_unique<SkROBufferStreamAsset>(this->makeROBufferSnapshot());
}
