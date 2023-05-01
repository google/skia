/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_MEMORYPOOL
#define SKSL_MEMORYPOOL

#include <memory>

#include "include/core/SkTypes.h"
#include "src/base/SkArenaAlloc.h"

namespace SkSL {

class MemoryPool {
public:
    static std::unique_ptr<MemoryPool> Make() {
        return std::make_unique<MemoryPool>();
    }
    void* allocate(size_t size) {
        return fArena.makeBytesAlignedTo(size, sizeof(void*));
    }
    void release(void*) {
        // SkArenaAlloc does not ever attempt to reclaim space.
    }

private:
    SkSTArenaAlloc<65536> fArena{/*firstHeapAllocation=*/32768};
};

}  // namespace SkSL

#endif // SKSL_MEMORYPOOL
