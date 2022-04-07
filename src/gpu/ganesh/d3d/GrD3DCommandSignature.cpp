/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/ganesh/d3d/GrD3DCommandSignature.h"

#include "src/gpu/ganesh/d3d/GrD3DGpu.h"

sk_sp<GrD3DCommandSignature> GrD3DCommandSignature::Make(GrD3DGpu* gpu, ForIndexed forIndexed,
                                                         unsigned int slot) {
    bool indexed = (forIndexed == ForIndexed::kYes);
    D3D12_INDIRECT_ARGUMENT_DESC argumentDesc = {};
    argumentDesc.Type = indexed ? D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED
                                : D3D12_INDIRECT_ARGUMENT_TYPE_DRAW;
    argumentDesc.VertexBuffer.Slot = slot;

    D3D12_COMMAND_SIGNATURE_DESC commandSigDesc = {};
    commandSigDesc.ByteStride = indexed ? sizeof(D3D12_DRAW_INDEXED_ARGUMENTS)
                                        : sizeof(D3D12_DRAW_ARGUMENTS);
    commandSigDesc.NumArgumentDescs = 1;
    commandSigDesc.pArgumentDescs = &argumentDesc;
    commandSigDesc.NodeMask = 0;

    gr_cp<ID3D12CommandSignature> commandSig;
    HRESULT hr = gpu->device()->CreateCommandSignature(&commandSigDesc, nullptr,
                                                       IID_PPV_ARGS(&commandSig));
    if (!SUCCEEDED(hr)) {
        SkDebugf("Failed to create command signature.\n");
        return nullptr;
    }

    return sk_sp<GrD3DCommandSignature>(new GrD3DCommandSignature(std::move(commandSig),
                                                                  forIndexed, slot));
}
