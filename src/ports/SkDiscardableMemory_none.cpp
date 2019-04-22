/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDiscardableMemory.h"
#include "src/lazy/SkDiscardableMemoryPool.h"
#include "include/core/SkTypes.h"

SkDiscardableMemory* SkDiscardableMemory::Create(size_t bytes) {
    return SkGetGlobalDiscardableMemoryPool()->create(bytes);
}
