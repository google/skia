/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkRasterPipelineContextUtils_DEFINED
#define SkRasterPipelineContextUtils_DEFINED

#include "src/base/SkArenaAlloc.h"
#include "src/base/SkUtils.h"

#include <cstring>
#include <type_traits>

namespace SkRPCtxUtils {

template <typename T>
using UnpackedType = typename std::conditional<sizeof(T) <= sizeof(void*), T, const T&>::type;

/**
 * SkRPCtxUtils::Pack will check if the passed-in struct is small enough to fit directly in the
 * context field. If so, it will return the data bit-casted into a void pointer. If not, it
 * allocates a copy of the struct inside the alloc and then returns a pointer to the copy.
 */
template <typename T>
[[maybe_unused]] static void* Pack(const T& ctx, SkArenaAlloc* alloc) {
    // If the context is small enough to fit in a pointer, bit-cast it; if not, alloc a copy.
    if constexpr (sizeof(T) <= sizeof(void*)) {
        return sk_bit_cast<void*>(ctx);
    } else {
        return alloc->make<T>(ctx);
    }
}

/**
 * SkRPCtxUtils::Unpack performs the reverse operation: either un-bitcasting the object back to its
 * original form, or returning the pointer as-is, depending on the size of the type.
 */
template <typename T>
[[maybe_unused]] static UnpackedType<T> Unpack(const T* ctx) {
    // If the context struct fits in a pointer, reinterpret the bits; if it doesn't, return
    // a reference. This can vary based on the architecture (32-bit pointers vs 64-bit) so we
    // let the compiler decide which is right, rather than hard-coding it.
    if constexpr (sizeof(T) <= sizeof(void*)) {
        return sk_bit_cast<T>(ctx);
    } else {
        return *ctx;
    }
}

}  // namespace SkRPCtxUtils

#endif  // SkRasterPipelineContextUtils_DEFINED
