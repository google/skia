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
class SkROBuffer : public SkRefCnt {
public:
    /**
     *  Return the logical length of the data owned/shared by this buffer. It may be stored in
     *  multiple contiguous blocks, accessible via the iterator.
     */
    size_t size() const { return fUsed; }
    
    class Iter {
    public:
        Iter(const SkROBuffer*);

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
    };

private:
    SkROBuffer(const SkBufferHead* head, size_t used);
    virtual ~SkROBuffer();
    
    const SkBufferHead* fHead;
    const size_t        fUsed;
    
    friend class SkRWBuffer;
};

/**
 *  Accumulates bytes of memory that are "appended" to it, growing internal storage as needed.
 *  The growth is done such that at any time, a RBuffer or StreamAsset can be snapped off, which
 *  can see the previously stored bytes, but which will be unaware of any future writes.
 */
class SkRWBuffer {
public:
    SkRWBuffer(size_t initialCapacity = 0);
    ~SkRWBuffer();
    
    size_t size() const { return fTotalUsed; }
    void append(const void* buffer, size_t length);
    void* append(size_t length);

    SkROBuffer* newRBufferSnapshot() const;
    SkStreamAsset* newStreamSnapshot() const;
    
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
