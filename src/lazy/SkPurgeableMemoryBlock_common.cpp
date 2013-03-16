/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPurgeableMemoryBlock.h"

SkPurgeableMemoryBlock* SkPurgeableMemoryBlock::Create(size_t size) {
    SkASSERT(IsSupported());
    if (!IsSupported()) {
        return NULL;
    }
    return SkNEW_ARGS(SkPurgeableMemoryBlock, (size));
}
