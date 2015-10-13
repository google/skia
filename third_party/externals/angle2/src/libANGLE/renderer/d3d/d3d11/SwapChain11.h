//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SwapChain11.h: Defines a back-end specific class for the D3D11 swap chain.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_

#include "common/angleutils.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"

namespace rx
{
class Renderer11;

class SwapChain11 : public SwapChainD3D
{
  public:
    SwapChain11(Renderer11 *renderer, NativeWindow nativeWindow, HANDLE shareHandle,
                GLenum backBufferFormat, GLenum depthBufferFormat);
    virtual ~SwapChain11();

    EGLint resize(EGLint backbufferWidth, EGLint backbufferHeight);
    virtual EGLint reset(EGLint backbufferWidth, EGLint backbufferHeight, EGLint swapInterval);
    virtual EGLint swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
    virtual void recreate();

    RenderTargetD3D *getColorRenderTarget() override { return &mColorRenderTarget; }
    RenderTargetD3D *getDepthStencilRenderTarget() override { return &mDepthStencilRenderTarget; }

    virtual ID3D11Texture2D *getOffscreenTexture();
    virtual ID3D11RenderTargetView *getRenderTarget();
    virtual ID3D11ShaderResourceView *getRenderTargetShaderResource();

    virtual ID3D11Texture2D *getDepthStencilTexture();
    virtual ID3D11DepthStencilView *getDepthStencil();
    virtual ID3D11ShaderResourceView *getDepthStencilShaderResource();

    EGLint getWidth() const { return mWidth; }
    EGLint getHeight() const { return mHeight; }
    void *getKeyedMutex() override { return mKeyedMutex; }

  private:
    void release();
    void initPassThroughResources();
    void releaseOffscreenTexture();
    EGLint resetOffscreenTexture(int backbufferWidth, int backbufferHeight);
    DXGI_FORMAT getSwapChainNativeFormat() const;

    Renderer11 *mRenderer;
    EGLint mHeight;
    EGLint mWidth;
    bool mAppCreatedShareHandle;
    unsigned int mSwapInterval;
    bool mPassThroughResourcesInit;

    DXGISwapChain *mSwapChain;
    IDXGISwapChain1 *mSwapChain1;
    IDXGIKeyedMutex *mKeyedMutex;

    ID3D11Texture2D *mBackBufferTexture;
    ID3D11RenderTargetView *mBackBufferRTView;

    ID3D11Texture2D *mOffscreenTexture;
    ID3D11RenderTargetView *mOffscreenRTView;
    ID3D11ShaderResourceView *mOffscreenSRView;

    ID3D11Texture2D *mDepthStencilTexture;
    ID3D11DepthStencilView *mDepthStencilDSView;
    ID3D11ShaderResourceView *mDepthStencilSRView;

    ID3D11Buffer *mQuadVB;
    ID3D11SamplerState *mPassThroughSampler;
    ID3D11InputLayout *mPassThroughIL;
    ID3D11VertexShader *mPassThroughVS;
    ID3D11PixelShader *mPassThroughPS;

    SurfaceRenderTarget11 mColorRenderTarget;
    SurfaceRenderTarget11 mDepthStencilRenderTarget;
};

}
#endif // LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
