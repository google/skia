//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChain11.cpp: Implements a back-end specific class for the D3D11 swap chain.

#include "libANGLE/renderer/d3d/d3d11/SwapChain11.h"

#include "libANGLE/features.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "third_party/trace_event/trace_event.h"

// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough2d11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba2d11ps.h"

#ifdef ANGLE_ENABLE_KEYEDMUTEX
#define ANGLE_RESOURCE_SHARE_TYPE D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
#else
#define ANGLE_RESOURCE_SHARE_TYPE D3D11_RESOURCE_MISC_SHARED
#endif

namespace rx
{

SwapChain11::SwapChain11(Renderer11 *renderer, NativeWindow nativeWindow, HANDLE shareHandle,
                         GLenum backBufferFormat, GLenum depthBufferFormat)
    : SwapChainD3D(nativeWindow, shareHandle, backBufferFormat, depthBufferFormat),
      mRenderer(renderer),
      mPassThroughResourcesInit(false),
      mColorRenderTarget(this, renderer, false),
      mDepthStencilRenderTarget(this, renderer, true)
{
    mHeight = -1;
    mWidth = -1;
    mAppCreatedShareHandle = mShareHandle != NULL;
    mSwapInterval = 0;

    mSwapChain = NULL;
    mSwapChain1 = nullptr;

    mKeyedMutex = nullptr;

    mBackBufferTexture = NULL;
    mBackBufferRTView = NULL;

    mOffscreenTexture = NULL;
    mOffscreenRTView = NULL;
    mOffscreenSRView = NULL;

    mDepthStencilTexture = NULL;
    mDepthStencilDSView = NULL;
    mDepthStencilSRView = NULL;

    mQuadVB = NULL;
    mPassThroughSampler = NULL;
    mPassThroughIL = NULL;
    mPassThroughVS = NULL;
    mPassThroughPS = NULL;
}

SwapChain11::~SwapChain11()
{
    release();
}

void SwapChain11::release()
{
    SafeRelease(mSwapChain1);
    SafeRelease(mSwapChain);
    SafeRelease(mKeyedMutex);
    SafeRelease(mBackBufferTexture);
    SafeRelease(mBackBufferRTView);
    SafeRelease(mOffscreenTexture);
    SafeRelease(mOffscreenRTView);
    SafeRelease(mOffscreenSRView);
    SafeRelease(mDepthStencilTexture);
    SafeRelease(mDepthStencilDSView);
    SafeRelease(mDepthStencilSRView);
    SafeRelease(mQuadVB);
    SafeRelease(mPassThroughSampler);
    SafeRelease(mPassThroughIL);
    SafeRelease(mPassThroughVS);
    SafeRelease(mPassThroughPS);

    if (!mAppCreatedShareHandle)
    {
        mShareHandle = NULL;
    }
}

void SwapChain11::releaseOffscreenTexture()
{
    SafeRelease(mOffscreenTexture);
    SafeRelease(mOffscreenRTView);
    SafeRelease(mOffscreenSRView);
    SafeRelease(mDepthStencilTexture);
    SafeRelease(mDepthStencilDSView);
    SafeRelease(mDepthStencilSRView);
}

EGLint SwapChain11::resetOffscreenTexture(int backbufferWidth, int backbufferHeight)
{
    TRACE_EVENT0("gpu.angle", "SwapChain11::resetOffscreenTexture");
    ID3D11Device *device = mRenderer->getDevice();

    ASSERT(device != NULL);

    // D3D11 does not allow zero size textures
    ASSERT(backbufferWidth >= 1);
    ASSERT(backbufferHeight >= 1);

    // Preserve the render target content
    ID3D11Texture2D *previousOffscreenTexture = mOffscreenTexture;
    if (previousOffscreenTexture)
    {
        previousOffscreenTexture->AddRef();
    }
    const int previousWidth = mWidth;
    const int previousHeight = mHeight;

    releaseOffscreenTexture();

    const d3d11::TextureFormat &backbufferFormatInfo = d3d11::GetTextureFormatInfo(mOffscreenRenderTargetFormat, mRenderer->getRenderer11DeviceCaps());

    // If the app passed in a share handle, open the resource
    // See EGL_ANGLE_d3d_share_handle_client_buffer
    if (mAppCreatedShareHandle)
    {
        ID3D11Resource *tempResource11;
        HRESULT result = device->OpenSharedResource(mShareHandle, __uuidof(ID3D11Resource), (void**)&tempResource11);

        if (FAILED(result))
        {
            ERR("Failed to open the swap chain pbuffer share handle: %08lX", result);
            release();
            return EGL_BAD_PARAMETER;
        }

        result = tempResource11->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&mOffscreenTexture);
        SafeRelease(tempResource11);

        if (FAILED(result))
        {
            ERR("Failed to query texture2d interface in pbuffer share handle: %08lX", result);
            release();
            return EGL_BAD_PARAMETER;
        }

        // Validate offscreen texture parameters
        D3D11_TEXTURE2D_DESC offscreenTextureDesc = {0};
        mOffscreenTexture->GetDesc(&offscreenTextureDesc);

        if (offscreenTextureDesc.Width != (UINT)backbufferWidth ||
            offscreenTextureDesc.Height != (UINT)backbufferHeight ||
            offscreenTextureDesc.Format != backbufferFormatInfo.texFormat ||
            offscreenTextureDesc.MipLevels != 1 ||
            offscreenTextureDesc.ArraySize != 1)
        {
            ERR("Invalid texture parameters in the shared offscreen texture pbuffer");
            release();
            return EGL_BAD_PARAMETER;
        }
    }
    else
    {
        const bool useSharedResource = !mNativeWindow.getNativeWindow() && mRenderer->getShareHandleSupport();

        D3D11_TEXTURE2D_DESC offscreenTextureDesc = {0};
        offscreenTextureDesc.Width = backbufferWidth;
        offscreenTextureDesc.Height = backbufferHeight;
        offscreenTextureDesc.Format = backbufferFormatInfo.texFormat;
        offscreenTextureDesc.MipLevels = 1;
        offscreenTextureDesc.ArraySize = 1;
        offscreenTextureDesc.SampleDesc.Count = 1;
        offscreenTextureDesc.SampleDesc.Quality = 0;
        offscreenTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        offscreenTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        offscreenTextureDesc.CPUAccessFlags = 0;
        offscreenTextureDesc.MiscFlags = useSharedResource ? ANGLE_RESOURCE_SHARE_TYPE : 0;

        HRESULT result = device->CreateTexture2D(&offscreenTextureDesc, NULL, &mOffscreenTexture);

        if (FAILED(result))
        {
            ERR("Could not create offscreen texture: %08lX", result);
            release();

            if (d3d11::isDeviceLostError(result))
            {
                return EGL_CONTEXT_LOST;
            }
            else
            {
                return EGL_BAD_ALLOC;
            }
        }

        d3d11::SetDebugName(mOffscreenTexture, "Offscreen back buffer texture");

        // EGL_ANGLE_surface_d3d_texture_2d_share_handle requires that we store a share handle for the client
        if (useSharedResource)
        {
            IDXGIResource *offscreenTextureResource = NULL;
            result = mOffscreenTexture->QueryInterface(__uuidof(IDXGIResource), (void**)&offscreenTextureResource);

            // Fall back to no share handle on failure
            if (FAILED(result))
            {
                ERR("Could not query offscreen texture resource: %08lX", result);
            }
            else
            {
                result = offscreenTextureResource->GetSharedHandle(&mShareHandle);
                SafeRelease(offscreenTextureResource);

                if (FAILED(result))
                {
                    mShareHandle = NULL;
                    ERR("Could not get offscreen texture shared handle: %08lX", result);
                }
            }
        }
        mKeyedMutex = d3d11::DynamicCastComObject<IDXGIKeyedMutex>(mOffscreenTexture);
    }


    D3D11_RENDER_TARGET_VIEW_DESC offscreenRTVDesc;
    offscreenRTVDesc.Format = backbufferFormatInfo.rtvFormat;
    offscreenRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    offscreenRTVDesc.Texture2D.MipSlice = 0;

    HRESULT result = device->CreateRenderTargetView(mOffscreenTexture, &offscreenRTVDesc, &mOffscreenRTView);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mOffscreenRTView, "Offscreen back buffer render target");

    D3D11_SHADER_RESOURCE_VIEW_DESC offscreenSRVDesc;
    offscreenSRVDesc.Format = backbufferFormatInfo.srvFormat;
    offscreenSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    offscreenSRVDesc.Texture2D.MostDetailedMip = 0;
    offscreenSRVDesc.Texture2D.MipLevels = static_cast<UINT>(-1);

    result = device->CreateShaderResourceView(mOffscreenTexture, &offscreenSRVDesc, &mOffscreenSRView);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mOffscreenSRView, "Offscreen back buffer shader resource");

    const d3d11::TextureFormat &depthBufferFormatInfo = d3d11::GetTextureFormatInfo(mDepthBufferFormat, mRenderer->getRenderer11DeviceCaps());

    if (mDepthBufferFormat != GL_NONE)
    {
        D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
        depthStencilTextureDesc.Width = backbufferWidth;
        depthStencilTextureDesc.Height = backbufferHeight;
        depthStencilTextureDesc.Format = depthBufferFormatInfo.texFormat;
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = 1;
        depthStencilTextureDesc.SampleDesc.Count = 1;
        depthStencilTextureDesc.SampleDesc.Quality = 0;
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            depthStencilTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }

        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;

        result = device->CreateTexture2D(&depthStencilTextureDesc, NULL, &mDepthStencilTexture);
        if (FAILED(result))
        {
            ERR("Could not create depthstencil surface for new swap chain: 0x%08X", result);
            release();

            if (d3d11::isDeviceLostError(result))
            {
                return EGL_CONTEXT_LOST;
            }
            else
            {
                return EGL_BAD_ALLOC;
            }
        }
        d3d11::SetDebugName(mDepthStencilTexture, "Offscreen depth stencil texture");

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
        depthStencilDesc.Format = depthBufferFormatInfo.dsvFormat;
        depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = 0;
        depthStencilDesc.Texture2D.MipSlice = 0;

        result = device->CreateDepthStencilView(mDepthStencilTexture, &depthStencilDesc, &mDepthStencilDSView);
        ASSERT(SUCCEEDED(result));
        d3d11::SetDebugName(mDepthStencilDSView, "Offscreen depth stencil view");

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc;
            depthStencilSRVDesc.Format = depthBufferFormatInfo.srvFormat;
            depthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            depthStencilSRVDesc.Texture2D.MostDetailedMip = 0;
            depthStencilSRVDesc.Texture2D.MipLevels = static_cast<UINT>(-1);

            result = device->CreateShaderResourceView(mDepthStencilTexture, &depthStencilSRVDesc, &mDepthStencilSRView);
            ASSERT(SUCCEEDED(result));
            d3d11::SetDebugName(mDepthStencilSRView, "Offscreen depth stencil shader resource");
        }
    }

    mWidth = backbufferWidth;
    mHeight = backbufferHeight;

    if (previousOffscreenTexture != NULL)
    {
        D3D11_BOX sourceBox = {0};
        sourceBox.left = 0;
        sourceBox.right = std::min(previousWidth, mWidth);
        sourceBox.top = std::max(previousHeight - mHeight, 0);
        sourceBox.bottom = previousHeight;
        sourceBox.front = 0;
        sourceBox.back = 1;

        ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
        const int yoffset = std::max(mHeight - previousHeight, 0);
        deviceContext->CopySubresourceRegion(mOffscreenTexture, 0, 0, yoffset, 0, previousOffscreenTexture, 0, &sourceBox);

        SafeRelease(previousOffscreenTexture);

        if (mSwapChain)
        {
            swapRect(0, 0, mWidth, mHeight);
        }
    }

    return EGL_SUCCESS;
}

EGLint SwapChain11::resize(EGLint backbufferWidth, EGLint backbufferHeight)
{
    TRACE_EVENT0("gpu.angle", "SwapChain11::resize");
    ID3D11Device *device = mRenderer->getDevice();

    if (device == NULL)
    {
        return EGL_BAD_ACCESS;
    }

    // EGL allows creating a surface with 0x0 dimension, however, DXGI does not like 0x0 swapchains
    if (backbufferWidth < 1 || backbufferHeight < 1)
    {
        return EGL_SUCCESS;
    }

    // Can only call resize if we have already created our swap buffer and resources
    ASSERT(mSwapChain && mBackBufferTexture && mBackBufferRTView);

    SafeRelease(mBackBufferTexture);
    SafeRelease(mBackBufferRTView);

    // Resize swap chain
    DXGI_SWAP_CHAIN_DESC desc;
    HRESULT result = mSwapChain->GetDesc(&desc);
    if (FAILED(result))
    {
        ERR("Error reading swap chain description: 0x%08X", result);
        release();
        return EGL_BAD_ALLOC;
    }

    result = mSwapChain->ResizeBuffers(desc.BufferCount, backbufferWidth, backbufferHeight, getSwapChainNativeFormat(), 0);

    if (FAILED(result))
    {
        ERR("Error resizing swap chain buffers: 0x%08X", result);
        release();

        if (d3d11::isDeviceLostError(result))
        {
            return EGL_CONTEXT_LOST;
        }
        else
        {
            return EGL_BAD_ALLOC;
        }
    }

    result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBufferTexture);
    ASSERT(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        d3d11::SetDebugName(mBackBufferTexture, "Back buffer texture");
        result = device->CreateRenderTargetView(mBackBufferTexture, NULL, &mBackBufferRTView);
        ASSERT(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        d3d11::SetDebugName(mBackBufferRTView, "Back buffer render target");
    }

    return resetOffscreenTexture(backbufferWidth, backbufferHeight);
}

DXGI_FORMAT SwapChain11::getSwapChainNativeFormat() const
{
    // Return a render target format for offscreen rendering is supported by IDXGISwapChain.
    // MSDN https://msdn.microsoft.com/en-us/library/windows/desktop/bb173064(v=vs.85).aspx
    return (mOffscreenRenderTargetFormat == GL_BGRA8_EXT) ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
}

EGLint SwapChain11::reset(int backbufferWidth, int backbufferHeight, EGLint swapInterval)
{
    TRACE_EVENT0("gpu.angle", "SwapChain11::reset");
    ID3D11Device *device = mRenderer->getDevice();

    if (device == NULL)
    {
        return EGL_BAD_ACCESS;
    }

    // Release specific resources to free up memory for the new render target, while the
    // old render target still exists for the purpose of preserving its contents.
    SafeRelease(mSwapChain1);
    SafeRelease(mSwapChain);
    SafeRelease(mBackBufferTexture);
    SafeRelease(mBackBufferRTView);

    mSwapInterval = static_cast<unsigned int>(swapInterval);
    if (mSwapInterval > 4)
    {
        // IDXGISwapChain::Present documentation states that valid sync intervals are in the [0,4] range
        return EGL_BAD_PARAMETER;
    }

    // EGL allows creating a surface with 0x0 dimension, however, DXGI does not like 0x0 swapchains
    if (backbufferWidth < 1 || backbufferHeight < 1)
    {
        releaseOffscreenTexture();
        return EGL_SUCCESS;
    }

    if (mNativeWindow.getNativeWindow())
    {
        HRESULT result = mNativeWindow.createSwapChain(device, mRenderer->getDxgiFactory(),
                                               getSwapChainNativeFormat(),
                                               backbufferWidth, backbufferHeight, &mSwapChain);

        if (FAILED(result))
        {
            ERR("Could not create additional swap chains or offscreen surfaces: %08lX", result);
            release();

            if (d3d11::isDeviceLostError(result))
            {
                return EGL_CONTEXT_LOST;
            }
            else
            {
                return EGL_BAD_ALLOC;
            }
        }

        if (mRenderer->getRenderer11DeviceCaps().supportsDXGI1_2)
        {
            mSwapChain1 = d3d11::DynamicCastComObject<IDXGISwapChain1>(mSwapChain);
        }

        result = mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&mBackBufferTexture);
        ASSERT(SUCCEEDED(result));
        d3d11::SetDebugName(mBackBufferTexture, "Back buffer texture");

        result = device->CreateRenderTargetView(mBackBufferTexture, NULL, &mBackBufferRTView);
        ASSERT(SUCCEEDED(result));
        d3d11::SetDebugName(mBackBufferRTView, "Back buffer render target");
    }

    return resetOffscreenTexture(backbufferWidth, backbufferHeight);
}

void SwapChain11::initPassThroughResources()
{
    if (mPassThroughResourcesInit)
    {
        return;
    }

    TRACE_EVENT0("gpu.angle", "SwapChain11::initPassThroughResources");
    ID3D11Device *device = mRenderer->getDevice();

    ASSERT(device != NULL);

    // Make sure our resources are all not allocated, when we create
    ASSERT(mQuadVB == NULL && mPassThroughSampler == NULL);
    ASSERT(mPassThroughIL == NULL && mPassThroughVS == NULL && mPassThroughPS == NULL);

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth = sizeof(d3d11::PositionTexCoordVertex) * 4;
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = 0;

    HRESULT result = device->CreateBuffer(&vbDesc, NULL, &mQuadVB);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mQuadVB, "Swap chain quad vertex buffer");

    D3D11_SAMPLER_DESC samplerDesc;
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    result = device->CreateSamplerState(&samplerDesc, &mPassThroughSampler);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mPassThroughSampler, "Swap chain pass through sampler");

    D3D11_INPUT_ELEMENT_DESC quadLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    result = device->CreateInputLayout(quadLayout, 2, g_VS_Passthrough2D, sizeof(g_VS_Passthrough2D), &mPassThroughIL);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mPassThroughIL, "Swap chain pass through layout");

    result = device->CreateVertexShader(g_VS_Passthrough2D, sizeof(g_VS_Passthrough2D), NULL, &mPassThroughVS);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mPassThroughVS, "Swap chain pass through vertex shader");

    result = device->CreatePixelShader(g_PS_PassthroughRGBA2D, sizeof(g_PS_PassthroughRGBA2D), NULL, &mPassThroughPS);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mPassThroughPS, "Swap chain pass through pixel shader");

    mPassThroughResourcesInit = true;
}

// parameters should be validated/clamped by caller
EGLint SwapChain11::swapRect(EGLint x, EGLint y, EGLint width, EGLint height)
{
    if (!mSwapChain)
    {
        return EGL_SUCCESS;
    }

    initPassThroughResources();

    ID3D11Device *device = mRenderer->getDevice();
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = deviceContext->Map(mQuadVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return EGL_BAD_ACCESS;
    }

    d3d11::PositionTexCoordVertex *vertices = static_cast<d3d11::PositionTexCoordVertex*>(mappedResource.pData);

    // Create a quad in homogeneous coordinates
    float x1 = (x / float(mWidth)) * 2.0f - 1.0f;
    float y1 = (y / float(mHeight)) * 2.0f - 1.0f;
    float x2 = ((x + width) / float(mWidth)) * 2.0f - 1.0f;
    float y2 = ((y + height) / float(mHeight)) * 2.0f - 1.0f;

    float u1 = x / float(mWidth);
    float v1 = y / float(mHeight);
    float u2 = (x + width) / float(mWidth);
    float v2 = (y + height) / float(mHeight);

    d3d11::SetPositionTexCoordVertex(&vertices[0], x1, y1, u1, v1);
    d3d11::SetPositionTexCoordVertex(&vertices[1], x1, y2, u1, v2);
    d3d11::SetPositionTexCoordVertex(&vertices[2], x2, y1, u2, v1);
    d3d11::SetPositionTexCoordVertex(&vertices[3], x2, y2, u2, v2);

    deviceContext->Unmap(mQuadVB, 0);

    static UINT stride = sizeof(d3d11::PositionTexCoordVertex);
    static UINT startIdx = 0;
    deviceContext->IASetVertexBuffers(0, 1, &mQuadVB, &stride, &startIdx);

    // Apply state
    deviceContext->OMSetDepthStencilState(NULL, 0xFFFFFFFF);

    static const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    deviceContext->OMSetBlendState(NULL, blendFactor, 0xFFFFFFF);

    deviceContext->RSSetState(NULL);

    // Apply shaders
    deviceContext->IASetInputLayout(mPassThroughIL);
    deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    deviceContext->VSSetShader(mPassThroughVS, NULL, 0);
    deviceContext->PSSetShader(mPassThroughPS, NULL, 0);
    deviceContext->GSSetShader(NULL, NULL, 0);

    // Apply render targets
    mRenderer->setOneTimeRenderTarget(mBackBufferRTView);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(mWidth);
    viewport.Height = static_cast<FLOAT>(mHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Apply textures
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, mOffscreenSRView);
    deviceContext->PSSetSamplers(0, 1, &mPassThroughSampler);

    // Draw
    deviceContext->Draw(4, 0);

    // Rendering to the swapchain is now complete. Now we can call Present().
    // Before that, we perform any cleanup on the D3D device. We do this before Present() to make sure the
    // cleanup is caught under the current eglSwapBuffers() PIX/Graphics Diagnostics call rather than the next one.
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, NULL);

    mRenderer->unapplyRenderTargets();
    mRenderer->markAllStateDirty();

#if ANGLE_VSYNC == ANGLE_DISABLED
    result = mSwapChain->Present(0, 0);
#else
    // Use IDXGISwapChain1::Present1 with a dirty rect if DXGI 1.2 is available.
    if (mSwapChain1 != nullptr)
    {
        RECT rect =
        {
            static_cast<LONG>(x), static_cast<LONG>(mHeight - y - height),
            static_cast<LONG>(x + width), static_cast<LONG>(mHeight - y)
        };
        DXGI_PRESENT_PARAMETERS params = { 1, &rect, nullptr, nullptr };
        result = mSwapChain1->Present1(mSwapInterval, 0, &params);
    }
    else
    {
        result = mSwapChain->Present(mSwapInterval, 0);
    }
#endif

    if (result == DXGI_ERROR_DEVICE_REMOVED)
    {
        HRESULT removedReason = device->GetDeviceRemovedReason();
        UNUSED_TRACE_VARIABLE(removedReason);
        ERR("Present failed: the D3D11 device was removed: 0x%08X", removedReason);
        return EGL_CONTEXT_LOST;
    }
    else if (result == DXGI_ERROR_DEVICE_RESET)
    {
        ERR("Present failed: the D3D11 device was reset from a bad command.");
        return EGL_CONTEXT_LOST;
    }
    else if (FAILED(result))
    {
        ERR("Present failed with error code 0x%08X", result);
    }

    mRenderer->onSwap();

    return EGL_SUCCESS;
}

ID3D11Texture2D *SwapChain11::getOffscreenTexture()
{
    return mOffscreenTexture;
}

ID3D11RenderTargetView *SwapChain11::getRenderTarget()
{
    return mOffscreenRTView;
}

ID3D11ShaderResourceView *SwapChain11::getRenderTargetShaderResource()
{
    return mOffscreenSRView;
}

ID3D11DepthStencilView *SwapChain11::getDepthStencil()
{
    return mDepthStencilDSView;
}

ID3D11ShaderResourceView * SwapChain11::getDepthStencilShaderResource()
{
    return mDepthStencilSRView;
}

ID3D11Texture2D *SwapChain11::getDepthStencilTexture()
{
    return mDepthStencilTexture;
}

void SwapChain11::recreate()
{
    // possibly should use this method instead of reset
}

}
