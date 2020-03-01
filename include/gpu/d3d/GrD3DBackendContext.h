/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DBackendContext_DEFINED
#define GrD3DBackendContext_DEFINED

#include "include/gpu/d3d/GrD3DTypes.h"

// The BackendContext contains all of the base D3D objects needed by the GrD3DGpu. The assumption
// is that the client will set these up and pass them to the GrD3DGpu constructor.
struct SK_API GrD3DBackendContext {
    Microsoft::WRL::ComPtr<ID3D12Device> fDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> fQueue;
    GrProtected fProtectedContext = GrProtected::kNo;
};

#endif
