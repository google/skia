/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/gpu/d3d/D3DTestUtils.h"

#ifdef SK_DIRECT3D
#include <d3d12sdklayers.h>

#include "include/gpu/d3d/GrD3DBackendContext.h"

namespace sk_gpu_test {

void get_hardware_adapter(IDXGIFactory4* pFactory, IDXGIAdapter1** ppAdapter) {
    *ppAdapter = nullptr;
    for (UINT adapterIndex = 0; ; ++adapterIndex) {
        IDXGIAdapter1* pAdapter = nullptr;
        if (DXGI_ERROR_NOT_FOUND == pFactory->EnumAdapters1(adapterIndex, &pAdapter)) {
            // No more adapters to enumerate.
            break;
        }

        // Check to see if the adapter supports Direct3D 12, but don't create the
        // actual device yet.
        if (SUCCEEDED(D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device),
                                        nullptr))) {
            *ppAdapter = pAdapter;
            return;
        }
        pAdapter->Release();
    }
}

bool CreateD3DBackendContext(GrD3DBackendContext* ctx,
                             bool isProtected) {
#if defined(SK_ENABLE_D3D_DEBUG_LAYER)
    // Enable the D3D12 debug layer.
    {
        gr_cp<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif
    // Create the device
    gr_cp<IDXGIFactory4> factory;
    if (!SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        return false;
    }

    gr_cp<IDXGIAdapter1> hardwareAdapter;
    get_hardware_adapter(factory.get(), &hardwareAdapter);

    gr_cp<ID3D12Device> device;
    if (!SUCCEEDED(D3D12CreateDevice(hardwareAdapter.get(),
                                     D3D_FEATURE_LEVEL_11_0,
                                     IID_PPV_ARGS(&device)))) {
        return false;
    }

    // Create the command queue
    gr_cp<ID3D12CommandQueue> queue;
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    if (!SUCCEEDED(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&queue)))) {
        return false;
    }

    ctx->fAdapter = hardwareAdapter;
    ctx->fDevice = device;
    ctx->fQueue = queue;
    // TODO: set up protected memory
    ctx->fProtectedContext = /*isProtected ? GrProtected::kYes :*/ GrProtected::kNo;

    return true;
}

}

#endif
