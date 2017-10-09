/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRWBuffer_DEFINED
#define SkRWBuffer_DEFINED

#include "SkRefCnt.h"

struct SkBufferBlock;
struct SkBufferHead;
class SkRWBuffer;
class SkStreamAsset;

/**
 *  Contains a read-only, thread-sharable block of memory. To access the memory, the caller must
 *  instantiate a local iterator, as the memory is stored in 1 or more contiguous blocks.
 */
class SK_API SkROBuffer : public SkRefCnt {
public:
    /**
     *  Return the logical length of the data owned/shared by this buffer. It may be stored in
     *  multiple contiguous blocks, accessible via the iterator.
     */
    size_t size() const { return fAvailable; }

    class SK_API Iter {
    public:
        Iter(const SkROBuffer*);
        Iter(const sk_sp<SkROBuffer>&);

        void reset(const SkROBuffer*);

        /**
         *  Return the current continuous block of memory, or nullptr if the iterator is exhausted
         */
        const void* data() const;

        /**
         *  Returns the number of bytes in the current continguous block of memory, or 0 if the
         *  iterator is exhausted.
         */
        size_t size() const;

        /**
         *  Advance to the next contiguous block of memory, returning true if there is another
         *  block, or false if the iterator is exhausted.
         */
        bool next();

    private:
        const SkBufferBlock* fBlock;
        size_t               fRemaining;
        const SkROBuffer*    fBuffer;
    };

private:
    SkROBuffer(const SkBufferHead* head, size_t available, const SkBufferBlock* fTail);
    virtual ~SkROBuffer();

    const SkBufferHead*     fHead;
    const size_t            fAvailable;
    const SkBufferBlock*    fTail;

    friend class SkRWBuffer;
};

/**
 *  Accumulates bytes of memory that are "appended" to it, growing internal storage as needed.
 *  The growth is done such that at any time in the writer's thread, an RBuffer or StreamAsset
 *  can be snapped off (and safely passed to another thread). The RBuffer/StreamAsset snapshot
 *  can see the previously stored bytes, but will be unaware of any future writes.
 */
class SK_API SkRWBuffer {
public:
    SkRWBuffer(size_t initialCapacity = 0);
    ~SkRWBuffer();

    size_t size() const { return fTotalUsed; }

    /**
     *  Append |length| bytes from |buffer|.
     *
     *  If the caller knows in advance how much more data they are going to append, they can
     *  pass a |reserve| hint (representing the number of upcoming bytes *in addition* to the
     *  current append), to minimize the number of internal allocations.
     */
    void append(const void* buffer, size_t length, size_t reserve = 0);

    sk_sp<SkROBuffer> makeROBufferSnapshot() const {
        return sk_sp<SkROBuffer>(new SkROBuffer(fHead, fTotalUsed, fTail));
    }

    std::unique_ptr<SkStreamAsset> makeStreamSnapshot() const;

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif

private:
    SkBufferHead*   fHead;
    SkBufferBlock*  fTail;
    size_t          fTotalUsed;
};

#endif
