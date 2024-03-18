/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_SwizzlePriv_DEFINED
#define skgpu_SwizzlePriv_DEFINED

#include "src/gpu/Swizzle.h"

#include <cstdint>

namespace skgpu {

// This class is friended by Swizzle and allows other functions to trampoline through this
// to call the private Swizzle ctor.
class SwizzleCtorAccessor {
public:
    static Swizzle Make(uint16_t key) { return Swizzle(key); }
};

} // namespace skgpu

#endif // skgpu_SwizzlePriv_DEFINED
