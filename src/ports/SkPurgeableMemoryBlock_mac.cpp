/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPurgeableMemoryBlock.h"

#include <mach/mach.h>

bool SkPurgeableMemoryBlock::IsSupported() {
    return true;
}

#ifdef SK_DEBUG
bool SkPurgeableMemoryBlock::PlatformSupportsPurgingAllUnpinnedBlocks() {
    return true;
}

bool SkPurgeableMemoryBlock::PurgeAllUnpinnedBlocks() {
    // Unused.
    int state = 0;
    kern_return_t ret = vm_purgable_control(mach_task_self(), 0, VM_PURGABLE_PURGE_ALL, &state);
    return ret == KERN_SUCCESS;
}

bool SkPurgeableMemoryBlock::purge() {
    return false;
}
#endif

static size_t round_to_page_size(size_t size) {
    const size_t mask = 4096 - 1;
    return (size + mask) & ~mask;
}

SkPurgeableMemoryBlock::SkPurgeableMemoryBlock(size_t size)
    : fAddr(NULL)
    , fSize(round_to_page_size(size))
    , fPinned(false) {
}

SkPurgeableMemoryBlock::~SkPurgeableMemoryBlock() {
    SkDEBUGCODE(kern_return_t ret =) vm_deallocate(mach_task_self(),
                                                   reinterpret_cast<vm_address_t>(fAddr),
                                                   static_cast<vm_size_t>(fSize));
#ifdef SK_DEBUG
    if (ret != KERN_SUCCESS) {
        SkDebugf("SkPurgeableMemoryBlock destructor failed to deallocate.\n");
    }
#endif
}

void* SkPurgeableMemoryBlock::pin(SkPurgeableMemoryBlock::PinResult* pinResult) {
    SkASSERT(!fPinned);
    SkASSERT(pinResult != NULL);
    if (NULL == fAddr) {
        vm_address_t addr = 0;
        kern_return_t ret = vm_allocate(mach_task_self(), &addr, static_cast<vm_size_t>(fSize),
                                        VM_FLAGS_PURGABLE | VM_FLAGS_ANYWHERE);
        if (KERN_SUCCESS == ret) {
            fAddr = reinterpret_cast<void*>(addr);
            *pinResult = kUninitialized_PinResult;
            fPinned = true;
        } else {
            fAddr = NULL;
        }
    } else {
        int state = VM_PURGABLE_NONVOLATILE;
        kern_return_t ret = vm_purgable_control(mach_task_self(),
                                                reinterpret_cast<vm_address_t>(fAddr),
                                                VM_PURGABLE_SET_STATE, &state);
        if (ret != KERN_SUCCESS) {
            fAddr = NULL;
            fPinned = false;
            return NULL;
        }

        fPinned = true;

        if (state & VM_PURGABLE_EMPTY) {
            *pinResult = kUninitialized_PinResult;
        } else {
            *pinResult = kRetained_PinResult;
        }
    }
    return fAddr;
}

void SkPurgeableMemoryBlock::unpin() {
    SkASSERT(fPinned);
    int state = VM_PURGABLE_VOLATILE | VM_VOLATILE_GROUP_DEFAULT;
    SkDEBUGCODE(kern_return_t ret =) vm_purgable_control(mach_task_self(),
                                                         reinterpret_cast<vm_address_t>(fAddr),
                                                         VM_PURGABLE_SET_STATE, &state);
    fPinned = false;

#ifdef SK_DEBUG
    if (ret != KERN_SUCCESS) {
        SkDebugf("SkPurgeableMemoryBlock::unpin() failed.\n");
    }
#endif
}
