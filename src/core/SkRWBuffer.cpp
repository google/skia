/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRWBuffer.h"
#include "SkStream.h"

// Force small chunks to be a page's worth
static const size_t kMinAllocSize = 4096;

struct SkBufferBlock {
    SkBufferBlock*  fNext;
    size_t          fUsed;
    size_t          fCapacity;

    const void* startData() const { return this + 1; };

    size_t avail() const { return fCapacity - fUsed; }
    void* availData() { return (char*)this->startData() + fUsed; }

    static SkBufferBlock* Alloc(size_t length) {
        size_t capacity = LengthToCapacity(length);
        SkBufferBlock* block = (SkBufferBlock*)sk_malloc_throw(sizeof(SkBufferBlock) + capacity);
        block->fNext = nullptr;
        block->fUsed = 0;
        block->fCapacity = capacity;
        return block;
    }

    // Return number of bytes actually appended
    size_t append(const void* src, size_t length) {
        this->validate();
        size_t amount = SkTMin(this->avail(), length);
        memcpy(this->availData(), src, amount);
        fUsed += amount;
        this->validate();
        return amount;
    }

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

    static size_t LengthToCapacity(size_t length) {
        const size_t minSize = kMinAllocSize - sizeof(SkBufferHead);
        return SkTMax(length, minSize);
    }

    static SkBufferHead* Alloc(size_t length) {
        size_t capacity = LengthToCapacity(length);
        size_t size = sizeof(SkBufferHead) + capacity;
        SkBufferHead* head = (SkBufferHead*)sk_malloc_throw(size);
        head->fRefCnt = 1;
        head->fBlock.fNext = nullptr;
        head->fBlock.fUsed = 0;
        head->fBlock.fCapacity = capacity;
        return head;
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

    void validate(size_t minUsed, SkBufferBlock* tail = nullptr) const {
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

SkROBuffer::SkROBuffer(const SkBufferHead* head, size_t used) : fHead(head), fUsed(used) {
    if (head) {
        fHead->ref();
        SkASSERT(used > 0);
        head->validate(used);
    } else {
        SkASSERT(0 == used);
    }
}

SkROBuffer::~SkROBuffer() {
    if (fHead) {
        fHead->validate(fUsed);
        fHead->unref();
    }
}

SkROBuffer::Iter::Iter(const SkROBuffer* buffer) {
    this->reset(buffer);
}

void SkROBuffer::Iter::reset(const SkROBuffer* buffer) {
    if (buffer) {
        fBlock = &buffer->fHead->fBlock;
        fRemaining = buffer->fUsed;
    } else {
        fBlock = nullptr;
        fRemaining = 0;
    }
}

const void* SkROBuffer::Iter::data() const {
    return fRemaining ? fBlock->startData() : nullptr;
}

size_t SkROBuffer::Iter::size() const {
    return SkTMin(fBlock->fUsed, fRemaining);
}

bool SkROBuffer::Iter::next() {
    if (fRemaining) {
        fRemaining -= this->size();
        fBlock = fBlock->fNext;
    }
    return fRemaining != 0;
}

SkRWBuffer::SkRWBuffer(size_t initialCapacity) : fHead(nullptr), fTail(nullptr), fTotalUsed(0) {}

SkRWBuffer::~SkRWBuffer() {
    this->validate();
    fHead->unref();
}

void SkRWBuffer::append(const void* src, size_t length) {
    this->validate();
    if (0 == length) {
        return;
    }

    fTotalUsed += length;

    if (nullptr == fHead) {
        fHead = SkBufferHead::Alloc(length);
        fTail = &fHead->fBlock;
    }

    size_t written = fTail->append(src, length);
    SkASSERT(written <= length);
    src = (const char*)src + written;
    length -= written;

    if (length) {
        SkBufferBlock* block = SkBufferBlock::Alloc(length);
        fTail->fNext = block;
        fTail = block;
        written = fTail->append(src, length);
        SkASSERT(written == length);
    }
    this->validate();
}

void* SkRWBuffer::append(size_t length) {
    this->validate();
    if (0 == length) {
        return nullptr;
    }

    fTotalUsed += length;

    if (nullptr == fHead) {
        fHead = SkBufferHead::Alloc(length);
        fTail = &fHead->fBlock;
    } else if (fTail->avail() < length) {
        SkBufferBlock* block = SkBufferBlock::Alloc(length);
        fTail->fNext = block;
        fTail = block;
    }

    fTail->fUsed += length;
    this->validate();
    return (char*)fTail->availData() - length;
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

SkROBuffer* SkRWBuffer::newRBufferSnapshot() const { return new SkROBuffer(fHead, fTotalUsed); }

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
    SkROBufferStreamAsset(const SkROBuffer* buffer) : fBuffer(SkRef(buffer)), fIter(buffer) {
        fGlobalOffset = fLocalOffset = 0;
    }

    virtual ~SkROBufferStreamAsset() { fBuffer->unref(); }

    size_t getLength() const override { return fBuffer->size(); }

    bool rewind() override {
        AUTO_VALIDATE
        fIter.reset(fBuffer);
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
    const SkROBuffer*   fBuffer;
    SkROBuffer::Iter    fIter;
    size_t              fLocalOffset;
    size_t              fGlobalOffset;
};

SkStreamAsset* SkRWBuffer::newStreamSnapshot() const {
    SkAutoTUnref<SkROBuffer> buffer(this->newRBufferSnapshot());
    return new SkROBufferStreamAsset(buffer);
}
