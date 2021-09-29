/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_Context_DEFINED
#define skgpu_Context_DEFINED

#include "include/core/SkRefCnt.h"

namespace skgpu {

class Gpu;
namespace mtl { struct BackendContext; }

class Context final : public SkRefCnt {
public:
    ~Context() override;

#ifdef SK_METAL
    static sk_sp<Context> MakeMetal(const skgpu::mtl::BackendContext&);
#endif

protected:
    Context(sk_sp<Gpu>);

private:
    sk_sp<Gpu> fGpu;
};

} // namespace skgpu

#endif // skgpu_Context_DEFINED

