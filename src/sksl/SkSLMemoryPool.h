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
        return fArena.makeBytesAlignedTo(size, kAlignment);
    }
    void release(void*) {
        // SkArenaAlloc does not ever attempt to reclaim space.
    }

private:
#ifdef SK_FORCE_8_BYTE_ALIGNMENT
    // https://github.com/emscripten-core/emscripten/issues/10072
    // Since Skia does not use "long double" (16 bytes), we should be ok to force it back to 8 bytes
    // until emscripten is fixed.
    static constexpr size_t kAlignment = 8;
#else
    // Guaranteed alignment of pointer returned by allocate().
    static constexpr size_t kAlignment = alignof(std::max_align_t);
#endif

    SkSTArenaAlloc<65536> fArena{/*firstHeapAllocation=*/32768};
};

}  // namespace SkSL

#endif // SKSL_MEMORYPOOL
