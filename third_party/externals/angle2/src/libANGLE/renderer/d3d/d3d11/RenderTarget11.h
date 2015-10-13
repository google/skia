//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderTarget11.h: Defines a DX11-specific wrapper for ID3D11View pointers
// retained by Renderbuffers.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"

namespace rx
{
class SwapChain11;
class Renderer11;

class RenderTarget11 : public RenderTargetD3D
{
  public:
    RenderTarget11() { }
    virtual ~RenderTarget11() { }

    virtual ID3D11Resource *getTexture() const = 0;
    virtual ID3D11RenderTargetView *getRenderTargetView() const = 0;
    virtual ID3D11DepthStencilView *getDepthStencilView() const = 0;
    virtual ID3D11ShaderResourceView *getShaderResourceView() const = 0;

    virtual unsigned int getSubresourceIndex() const = 0;

    virtual DXGI_FORMAT getDXGIFormat() const = 0;
};

class TextureRenderTarget11 : public RenderTarget11
{
  public:
    // TextureRenderTarget11 takes ownership of any D3D11 resources it is given and will AddRef them
    TextureRenderTarget11(ID3D11RenderTargetView *rtv, ID3D11Resource *resource, ID3D11ShaderResourceView *srv,
                           GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei samples);
    TextureRenderTarget11(ID3D11DepthStencilView *dsv, ID3D11Resource *resource, ID3D11ShaderResourceView *srv,
                           GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei samples);
    virtual ~TextureRenderTarget11();

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    ID3D11Resource *getTexture() const override;
    ID3D11RenderTargetView *getRenderTargetView() const override;
    ID3D11DepthStencilView *getDepthStencilView() const override;
    ID3D11ShaderResourceView *getShaderResourceView() const override;

    unsigned int getSubresourceIndex() const override;

    DXGI_FORMAT getDXGIFormat() const override;

  private:
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    DXGI_FORMAT mDXGIFormat;
    GLsizei mSamples;

    unsigned int mSubresourceIndex;
    ID3D11Resource *mTexture;
    ID3D11RenderTargetView *mRenderTarget;
    ID3D11DepthStencilView *mDepthStencil;
    ID3D11ShaderResourceView *mShaderResource;
};

class SurfaceRenderTarget11 : public RenderTarget11
{
  public:
    SurfaceRenderTarget11(SwapChain11 *swapChain, Renderer11 *renderer, bool depth);
    virtual ~SurfaceRenderTarget11();

    GLsizei getWidth() const override;
    GLsizei getHeight() const override;
    GLsizei getDepth() const override;
    GLenum getInternalFormat() const override;
    GLsizei getSamples() const override;

    ID3D11Resource *getTexture() const override;
    ID3D11RenderTargetView *getRenderTargetView() const override;
    ID3D11DepthStencilView *getDepthStencilView() const override;
    ID3D11ShaderResourceView *getShaderResourceView() const override;

    unsigned int getSubresourceIndex() const override;

    DXGI_FORMAT getDXGIFormat() const override;

  private:
    SwapChain11 *mSwapChain;
    Renderer11 *mRenderer;
    bool mDepth;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_RENDERTARGET11_H_
