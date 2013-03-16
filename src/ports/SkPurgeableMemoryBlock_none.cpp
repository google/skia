/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPurgeableMemoryBlock.h"

bool SkPurgeableMemoryBlock::IsSupported() {
    return false;
}

#ifdef SK_DEBUG
bool SkPurgeableMemoryBlock::PlatformSupportsPurgingAllUnpinnedBlocks() {
    return false;
}

bool SkPurgeableMemoryBlock::PurgeAllUnpinnedBlocks() {
    return false;
}

bool SkPurgeableMemoryBlock::purge() {
    return false;
}
#endif

SkPurgeableMemoryBlock::SkPurgeableMemoryBlock(size_t size) {
    SkASSERT(false);
}

SkPurgeableMemoryBlock::~SkPurgeableMemoryBlock() {
}

void* SkPurgeableMemoryBlock::pin(SkPurgeableMemoryBlock::PinResult*) {
    return NULL;
}

void SkPurgeableMemoryBlock::unpin() {
}
