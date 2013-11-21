/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDiscardableMemory.h"
#include "SkTypes.h"

namespace {
////////////////////////////////////////////////////////////////////////////////
/**
 *  Always successful, never purges.  Useful for testing.
 */
class SkMockDiscardableMemory : public SkDiscardableMemory {
public:
    SkMockDiscardableMemory(void*);
    virtual ~SkMockDiscardableMemory();
    virtual bool lock() SK_OVERRIDE;
    virtual void* data() SK_OVERRIDE;
    virtual void unlock() SK_OVERRIDE;
private:
    bool fLocked;
    void* fPointer;
};

////////////////////////////////////////////////////////////////////////////////

SkMockDiscardableMemory::SkMockDiscardableMemory(void* ptr)
    : fLocked(true)
    , fPointer(ptr) {  // Takes ownership of ptr.
    SkASSERT(fPointer != NULL);
}

SkMockDiscardableMemory::~SkMockDiscardableMemory() {
    SkASSERT(!fLocked);
    sk_free(fPointer);
}

bool SkMockDiscardableMemory::lock() {
    SkASSERT(!fLocked);
    return fLocked = true;
}

void* SkMockDiscardableMemory::data() {
    SkASSERT(fLocked);
    return fLocked ? fPointer : NULL;
}

void SkMockDiscardableMemory::unlock() {
    SkASSERT(fLocked);
    fLocked = false;
}
////////////////////////////////////////////////////////////////////////////////
}  // namespace

SkDiscardableMemory* SkDiscardableMemory::Create(size_t bytes) {
    void* ptr = sk_malloc_throw(bytes);
    return (ptr != NULL) ? SkNEW_ARGS(SkMockDiscardableMemory, (ptr)) : NULL;
}
