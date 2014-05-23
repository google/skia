/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unistd.h>
#include <sys/mman.h>
#include "SkDiscardableMemory.h"
#include "SkDiscardableMemoryPool.h"
#include "SkTypes.h"
#include "android/ashmem.h"

////////////////////////////////////////////////////////////////////////////////
namespace {
/**
 *  DiscardableMemory implementation that uses the Android kernel's
 *  ashmem (Android shared memory).
 */
class SkAshmemDiscardableMemory : public SkDiscardableMemory {
public:
    SkAshmemDiscardableMemory(int fd, void* address, size_t size);
    virtual ~SkAshmemDiscardableMemory();
    virtual bool lock() SK_OVERRIDE;
    virtual void* data() SK_OVERRIDE;
    virtual void unlock() SK_OVERRIDE;
    static SkAshmemDiscardableMemory* Create(size_t bytes);

private:
    bool         fLocked;
    int          fFd;
    void*        fMemory;
    const size_t fSize;
};

SkAshmemDiscardableMemory::SkAshmemDiscardableMemory(int fd,
                                                     void* address,
                                                     size_t size)
    : fLocked(true)  // Ashmem pages are pinned by default.
    , fFd(fd)
    , fMemory(address)
    , fSize(size) {
    SkASSERT(fFd >= 0);
    SkASSERT(fMemory != NULL);
    SkASSERT(fSize > 0);
}

SkAshmemDiscardableMemory::~SkAshmemDiscardableMemory() {
    SkASSERT(!fLocked);
    if (NULL != fMemory) {
        munmap(fMemory, fSize);
    }
    if (fFd != -1) {
        close(fFd);
    }
}

bool SkAshmemDiscardableMemory::lock() {
    SkASSERT(!fLocked);
    if (-1 == fFd) {
        fLocked = false;
        return false;
    }
    SkASSERT(fMemory != NULL);
    if (fLocked || (ASHMEM_NOT_PURGED == ashmem_pin_region(fFd, 0, 0))) {
        fLocked = true;
        return true;
    } else {
        munmap(fMemory, fSize);
        fMemory = NULL;

        close(fFd);
        fFd = -1;
        fLocked = false;
        return false;
    }
}

void* SkAshmemDiscardableMemory::data() {
    SkASSERT(fLocked);
    return fLocked ? fMemory : NULL;
}

void SkAshmemDiscardableMemory::unlock() {
    SkASSERT(fLocked);
    if (fLocked && (fFd != -1)) {
        ashmem_unpin_region(fFd, 0, 0);
    }
    fLocked = false;
}

SkAshmemDiscardableMemory* SkAshmemDiscardableMemory::Create(size_t bytes) {
    // ashmem likes lengths on page boundaries.
    const size_t mask = getpagesize() - 1;
    size_t size = (bytes + mask) & ~mask;

    static const char name[] = "Skia_Ashmem_Discardable_Memory";
    int fd = ashmem_create_region(name, size);
    if (fd < 0) {
        return NULL;
    }
    if (0 != ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE)) {
        close(fd);
        return NULL;
    }
    void* addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if ((MAP_FAILED == addr) || (NULL == addr)) {
        close(fd);
        return NULL;
    }

    return SkNEW_ARGS(SkAshmemDiscardableMemory, (fd, addr, size));
}
}  // namespace
////////////////////////////////////////////////////////////////////////////////

#ifndef SK_ASHMEM_MINIMUM_MEMORY_SIZE
// number taken from android/graphics/BitmapFactory.cpp
#define SK_ASHMEM_MINIMUM_MEMORY_SIZE (32 * 1024)
#endif  // SK_ASHMEM_MINIMUM_MEMORY_SIZE
SkDiscardableMemory* SkDiscardableMemory::Create(size_t bytes) {
    if (bytes < SK_ASHMEM_MINIMUM_MEMORY_SIZE) {
        return SkGetGlobalDiscardableMemoryPool()->create(bytes);
    } else {
        return SkAshmemDiscardableMemory::Create(bytes);
    }
}

