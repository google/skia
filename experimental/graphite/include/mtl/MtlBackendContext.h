/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_MtlBackendContext_DEFINED
#define skgpu_MtlBackendContext_DEFINED

#include "experimental/graphite/include/mtl/MtlTypes.h"

namespace skgpu::mtl {

// The BackendContext contains all of the base Metal objects needed by the MtlGpu. The assumption
// is that the client will set these up and pass them to the MtlGpu constructor.
struct SK_API BackendContext {
    sk_cfp<CFTypeRef> fDevice;
    sk_cfp<CFTypeRef> fQueue;
};

} // namespace skgpu::mtl

#endif // skgpu_MtlBackendContext_DEFINED
