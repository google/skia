/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DResourceProvider_DEFINED
#define GrD3DResourceProvider_DEFINED

#include "src/gpu/d3d/GrD3D12.h"

#include <memory>

class GrD3DDirectCommandList;
class GrD3DGpu;

class GrD3DResourceProvider {
public:
    GrD3DResourceProvider(GrD3DGpu*);

    std::unique_ptr<GrD3DDirectCommandList> findOrCreateDirectCommandList();

private:
    gr_cp<ID3D12CommandAllocator> fDirectCommandAllocator;

    GrD3DGpu* fGpu;
};

#endif
