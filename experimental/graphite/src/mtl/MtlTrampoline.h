/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlTrampoline_DEFINED
#define skgpu_MtlTrampoline_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {
class Gpu;
}

namespace skgpu::mtl {
struct BackendContext;

/*
 * This class is used to hold functions which trampoline from the Graphite cpp code
 * to the Mtl Objective-C files.
 */
class Trampoline {
public:
    static sk_sp<skgpu::Gpu> MakeGpu(const BackendContext&);
};

} // namespace skgpu::mtl

#endif // skgpu_MtlTrampoline_DEFINED

