/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPurgeableMemoryBlock.h"

#include "android/ashmem.h"
#include <sys/mman.h>
#include <unistd.h>

bool SkPurgeableMemoryBlock::IsSupported() {
    return true;
}

#ifdef SK_DEBUG
bool SkPurgeableMemoryBlock::PlatformSupportsPurgingAllUnpinnedBlocks() {
    return false;
}

bool SkPurgeableMemoryBlock::PurgeAllUnpinnedBlocks() {
    return false;
}

bool SkPurgeableMemoryBlock::purge() {
    SkASSERT(!fPinned);
    if (-1 != fFD) {
        ashmem_purge_all_caches(fFD);
        return true;
    } else {
        return false;
    }
}
#endif

// ashmem likes lengths on page boundaries.
static size_t round_to_page_size(size_t size) {
    const size_t mask = getpagesize() - 1;
    size_t newSize = (size + mask) & ~mask;
    return newSize;
}

SkPurgeableMemoryBlock::SkPurgeableMemoryBlock(size_t size)
    : fAddr(NULL)
    , fSize(round_to_page_size(size))
    , fPinned(false)
    , fFD(-1) {
}

SkPurgeableMemoryBlock::~SkPurgeableMemoryBlock() {
    if (-1 != fFD) {
        munmap(fAddr, fSize);
        close(fFD);
    }
}

void* SkPurgeableMemoryBlock::pin(SkPurgeableMemoryBlock::PinResult* pinResult) {
    SkASSERT(!fPinned);
    if (-1 == fFD) {
        int fd = ashmem_create_region(NULL, fSize);
        if (-1 == fd) {
            SkDebugf("ashmem_create_region failed\n");
            return NULL;
        }

        int err = ashmem_set_prot_region(fd, PROT_READ | PROT_WRITE);
        if (err != 0) {
            SkDebugf("ashmem_set_prot_region failed\n");
            close(fd);
            return NULL;
        }

        void* addr = mmap(NULL, fSize, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
        if (-1 == (long) addr) {
            SkDebugf("mmap failed\n");
            close(fd);
            return NULL;
        }
        fAddr = addr;
        fFD = fd;
        (void) ashmem_pin_region(fd, 0, 0);
        *pinResult = kUninitialized_PinResult;
        fPinned = true;
    } else {
        int pin = ashmem_pin_region(fFD, 0, 0);
        if (ASHMEM_NOT_PURGED == pin) {
            fPinned = true;
            *pinResult = kRetained_PinResult;
        } else if (ASHMEM_WAS_PURGED == pin) {
            fPinned = true;
            *pinResult = kUninitialized_PinResult;
        } else {
            // Failed.
            munmap(fAddr, fSize);
            close(fFD);
            fFD = -1;
            fAddr = NULL;
        }
    }
    return fAddr;
}

void SkPurgeableMemoryBlock::unpin() {
    if (-1 != fFD) {
        ashmem_unpin_region(fFD, 0, 0);
        fPinned = false;
    }
}
