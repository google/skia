/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_MtlTrampoline_DEFINED
#define skgpu_graphite_MtlTrampoline_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu::graphite {
class Gpu;
struct MtlBackendContext;

/*
 * This class is used to hold functions which trampoline from the Graphite cpp code
 * to the Mtl Objective-C files.
 */
class MtlTrampoline {
public:
    static sk_sp<skgpu::graphite::Gpu> MakeGpu(const MtlBackendContext&);
};

} // namespace skgpu::graphite

#endif // skgpu_graphite_MtlTrampoline_DEFINED

