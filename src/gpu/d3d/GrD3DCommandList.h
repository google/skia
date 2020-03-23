/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrD3DCommandList_DEFINED
#define GrD3DCommandList_DEFINED

#include "include/gpu/GrTypes.h"
#include "src/gpu/d3d/GrD3D12.h"

#include <memory>

class GrD3DGpu;

class GrD3DCommandList {
public:

protected:
    GrD3DCommandList(gr_cp<ID3D12GraphicsCommandList> commandList);

private:
    gr_cp<ID3D12CommandList> fCommandList;
};

class GrD3DDirectCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DDirectCommandList> Make(ID3D12Device* device,
                                                        ID3D12CommandAllocator* cmdAllocator);

private:
    GrD3DDirectCommandList(gr_cp<ID3D12GraphicsCommandList> commandList);
};

class GrD3DCopyCommandList : public GrD3DCommandList {
public:
    static std::unique_ptr<GrD3DCopyCommandList> Make(ID3D12Device* device,
                                                      ID3D12CommandAllocator* cmdAllocator);

private:
    GrD3DCopyCommandList(gr_cp<ID3D12GraphicsCommandList> commandList);
};
#endif
