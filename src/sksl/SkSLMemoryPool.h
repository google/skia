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

#if SK_SUPPORT_GPU

#include "src/gpu/GrMemoryPool.h"

namespace SkSL {
using MemoryPool = ::GrMemoryPool;
}

#else

// When Ganesh is disabled, GrMemoryPool is not linked in. We include a minimal class which mimics
// the GrMemoryPool interface but simply redirects to the system allocator.
namespace SkSL {

class MemoryPool {
public:
    static std::unique_ptr<MemoryPool> Make(size_t, size_t) {
        return std::make_unique<MemoryPool>();
    }
    void resetScratchSpace() {}
    void reportLeaks() const {}
    bool isEmpty() const { return true; }
    void* allocate(size_t size) { return ::operator new(size); }
    void release(void* p) { ::operator delete(p); }
};

}  // namespace SkSL

#endif // SK_SUPPORT_GPU
#endif // SKSL_MEMORYPOOL
