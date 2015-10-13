//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Blit11.cpp: Texture copy utility class.

#include "libANGLE/renderer/d3d/d3d11/Blit11.h"

#include <float.h>

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "third_party/trace_event/trace_event.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough2d11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughdepth2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba2dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba2di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb2dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb2di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg2dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg2di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr2dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr2di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughlum2d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughlumalpha2d11ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough3d11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough3d11gs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba3d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba3dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba3di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb3d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb3dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgb3di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg3d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg3dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrg3di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr3d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr3dui11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughr3di11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughlum3d11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughlumalpha3d11ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui2dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef3dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei3dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui3dps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlef2darrayps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzlei2darrayps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/swizzleui2darrayps.h"

namespace rx
{

namespace
{

DXGI_FORMAT GetTextureFormat(ID3D11Resource *resource)
{
    ID3D11Texture2D *texture = d3d11::DynamicCastComObject<ID3D11Texture2D>(resource);
    if (!texture)
    {
        return DXGI_FORMAT_UNKNOWN;
    }

    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);

    SafeRelease(texture);

    return desc.Format;
}

ID3D11Resource *CreateStagingTexture(ID3D11Device *device, ID3D11DeviceContext *context,
                                     ID3D11Resource *source, unsigned int subresource,
                                     const gl::Extents &size, unsigned int cpuAccessFlags)
{
    D3D11_TEXTURE2D_DESC stagingDesc;
    stagingDesc.Width = size.width;
    stagingDesc.Height = size.height;
    stagingDesc.MipLevels = 1;
    stagingDesc.ArraySize = 1;
    stagingDesc.Format = GetTextureFormat(source);
    stagingDesc.SampleDesc.Count = 1;
    stagingDesc.SampleDesc.Quality = 0;
    stagingDesc.Usage = D3D11_USAGE_STAGING;
    stagingDesc.CPUAccessFlags = cpuAccessFlags;
    stagingDesc.MiscFlags = 0;
    stagingDesc.BindFlags = 0;

    ID3D11Texture2D *stagingTexture = nullptr;
    HRESULT result = device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
    if (FAILED(result))
    {
        ERR("Failed to create staging texture for depth stencil blit. HRESULT: 0x%X.", result);
        return nullptr;
    }

    context->CopySubresourceRegion(stagingTexture, 0, 0, 0, 0, source, subresource, nullptr);

    return stagingTexture;
}

inline void GenerateVertexCoords(const gl::Box &sourceArea, const gl::Extents &sourceSize,
                                 const gl::Box &destArea, const gl::Extents &destSize,
                                 float *x1, float *y1, float *x2, float *y2,
                                 float *u1, float *v1, float *u2, float *v2)
{
    *x1 = (destArea.x / float(destSize.width)) * 2.0f - 1.0f;
    *y1 = ((destSize.height - destArea.y - destArea.height) / float(destSize.height)) * 2.0f - 1.0f;
    *x2 = ((destArea.x + destArea.width) / float(destSize.width)) * 2.0f - 1.0f;
    *y2 = ((destSize.height - destArea.y) / float(destSize.height)) * 2.0f - 1.0f;

    *u1 = sourceArea.x / float(sourceSize.width);
    *v1 = sourceArea.y / float(sourceSize.height);
    *u2 = (sourceArea.x + sourceArea.width) / float(sourceSize.width);
    *v2 = (sourceArea.y + sourceArea.height) / float(sourceSize.height);
}

void Write2DVertices(const gl::Box &sourceArea, const gl::Extents &sourceSize,
                     const gl::Box &destArea, const gl::Extents &destSize,
                     void *outVertices, unsigned int *outStride, unsigned int *outVertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY *outTopology)
{
    float x1, y1, x2, y2, u1, v1, u2, v2;
    GenerateVertexCoords(sourceArea, sourceSize, destArea, destSize, &x1, &y1, &x2, &y2, &u1, &v1, &u2, &v2);

    d3d11::PositionTexCoordVertex *vertices = static_cast<d3d11::PositionTexCoordVertex*>(outVertices);

    d3d11::SetPositionTexCoordVertex(&vertices[0], x1, y1, u1, v2);
    d3d11::SetPositionTexCoordVertex(&vertices[1], x1, y2, u1, v1);
    d3d11::SetPositionTexCoordVertex(&vertices[2], x2, y1, u2, v2);
    d3d11::SetPositionTexCoordVertex(&vertices[3], x2, y2, u2, v1);

    *outStride = sizeof(d3d11::PositionTexCoordVertex);
    *outVertexCount = 4;
    *outTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
}

void Write3DVertices(const gl::Box &sourceArea, const gl::Extents &sourceSize,
                     const gl::Box &destArea, const gl::Extents &destSize,
                     void *outVertices, unsigned int *outStride, unsigned int *outVertexCount,
                     D3D11_PRIMITIVE_TOPOLOGY *outTopology)
{
    ASSERT(sourceSize.depth > 0 && destSize.depth > 0);

    float x1, y1, x2, y2, u1, v1, u2, v2;
    GenerateVertexCoords(sourceArea, sourceSize, destArea, destSize, &x1, &y1, &x2, &y2, &u1, &v1, &u2, &v2);

    d3d11::PositionLayerTexCoord3DVertex *vertices = static_cast<d3d11::PositionLayerTexCoord3DVertex*>(outVertices);

    for (int i = 0; i < destSize.depth; i++)
    {
        float readDepth = (float)i / std::max(destSize.depth - 1, 1);

        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 0], x1, y1, i, u1, v2, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 1], x1, y2, i, u1, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 2], x2, y1, i, u2, v2, readDepth);

        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 3], x1, y2, i, u1, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 4], x2, y2, i, u2, v1, readDepth);
        d3d11::SetPositionLayerTexCoord3DVertex(&vertices[i * 6 + 5], x2, y1, i, u2, v2, readDepth);
    }

    *outStride = sizeof(d3d11::PositionLayerTexCoord3DVertex);
    *outVertexCount = destSize.depth * 6;
    *outTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
}

inline unsigned int GetSwizzleIndex(GLenum swizzle)
{
    unsigned int colorIndex = 0;

    switch (swizzle)
    {
      case GL_RED:   colorIndex = 0; break;
      case GL_GREEN: colorIndex = 1; break;
      case GL_BLUE:  colorIndex = 2; break;
      case GL_ALPHA: colorIndex = 3; break;
      case GL_ZERO:  colorIndex = 4; break;
      case GL_ONE:   colorIndex = 5; break;
      default:       UNREACHABLE();  break;
    }

    return colorIndex;
}

D3D11_INPUT_ELEMENT_DESC quad2DLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

D3D11_INPUT_ELEMENT_DESC quad3DLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "LAYER", 0, DXGI_FORMAT_R32_UINT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

} // namespace

Blit11::Blit11(Renderer11 *renderer)
    : mRenderer(renderer),
      mVertexBuffer(nullptr),
      mPointSampler(nullptr),
      mLinearSampler(nullptr),
      mScissorEnabledRasterizerState(nullptr),
      mScissorDisabledRasterizerState(nullptr),
      mDepthStencilState(nullptr),
      mQuad2DIL(quad2DLayout, ArraySize(quad2DLayout), g_VS_Passthrough2D, ArraySize(g_VS_Passthrough2D), "Blit11 2D input layout"),
      mQuad2DVS(g_VS_Passthrough2D, ArraySize(g_VS_Passthrough2D), "Blit11 2D vertex shader"),
      mDepthPS(g_PS_PassthroughDepth2D, ArraySize(g_PS_PassthroughDepth2D), "Blit11 2D depth pixel shader"),
      mQuad3DIL(quad3DLayout, ArraySize(quad3DLayout), g_VS_Passthrough3D, ArraySize(g_VS_Passthrough3D), "Blit11 3D input layout"),
      mQuad3DVS(g_VS_Passthrough3D, ArraySize(g_VS_Passthrough3D), "Blit11 3D vertex shader"),
      mQuad3DGS(g_GS_Passthrough3D, ArraySize(g_GS_Passthrough3D), "Blit11 3D geometry shader"),
      mSwizzleCB(nullptr)
{
    TRACE_EVENT0("gpu.angle", "Blit11::Blit11");

    HRESULT result;
    ID3D11Device *device = mRenderer->getDevice();

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth =
        static_cast<unsigned int>(std::max(sizeof(d3d11::PositionLayerTexCoord3DVertex),
                                           sizeof(d3d11::PositionTexCoordVertex)) *
                                  6 * renderer->getRendererCaps().max3DTextureSize);
    vbDesc.Usage = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    vbDesc.MiscFlags = 0;
    vbDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&vbDesc, nullptr, &mVertexBuffer);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mVertexBuffer, "Blit11 vertex buffer");

    D3D11_SAMPLER_DESC pointSamplerDesc;
    pointSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
    pointSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    pointSamplerDesc.MipLODBias = 0.0f;
    pointSamplerDesc.MaxAnisotropy = 0;
    pointSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    pointSamplerDesc.BorderColor[0] = 0.0f;
    pointSamplerDesc.BorderColor[1] = 0.0f;
    pointSamplerDesc.BorderColor[2] = 0.0f;
    pointSamplerDesc.BorderColor[3] = 0.0f;
    pointSamplerDesc.MinLOD = 0.0f;
    pointSamplerDesc.MaxLOD = FLT_MAX;

    result = device->CreateSamplerState(&pointSamplerDesc, &mPointSampler);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mPointSampler, "Blit11 point sampler");

    D3D11_SAMPLER_DESC linearSamplerDesc;
    linearSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    linearSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    linearSamplerDesc.MipLODBias = 0.0f;
    linearSamplerDesc.MaxAnisotropy = 0;
    linearSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    linearSamplerDesc.BorderColor[0] = 0.0f;
    linearSamplerDesc.BorderColor[1] = 0.0f;
    linearSamplerDesc.BorderColor[2] = 0.0f;
    linearSamplerDesc.BorderColor[3] = 0.0f;
    linearSamplerDesc.MinLOD = 0.0f;
    linearSamplerDesc.MaxLOD = FLT_MAX;

    result = device->CreateSamplerState(&linearSamplerDesc, &mLinearSampler);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mLinearSampler, "Blit11 linear sampler");

    // Use a rasterizer state that will not cull so that inverted quads will not be culled
    D3D11_RASTERIZER_DESC rasterDesc;
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthBias = 0;
    rasterDesc.SlopeScaledDepthBias = 0.0f;
    rasterDesc.DepthBiasClamp = 0.0f;
    rasterDesc.DepthClipEnable = TRUE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.AntialiasedLineEnable = FALSE;

    rasterDesc.ScissorEnable = TRUE;
    result = device->CreateRasterizerState(&rasterDesc, &mScissorEnabledRasterizerState);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mScissorEnabledRasterizerState, "Blit11 scissoring rasterizer state");

    rasterDesc.ScissorEnable = FALSE;
    result = device->CreateRasterizerState(&rasterDesc, &mScissorDisabledRasterizerState);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mScissorDisabledRasterizerState, "Blit11 no scissoring rasterizer state");

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.StencilEnable = FALSE;
    depthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
    depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    result = device->CreateDepthStencilState(&depthStencilDesc, &mDepthStencilState);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mDepthStencilState, "Blit11 depth stencil state");

    D3D11_BUFFER_DESC swizzleBufferDesc;
    swizzleBufferDesc.ByteWidth = sizeof(unsigned int) * 4;
    swizzleBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    swizzleBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    swizzleBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    swizzleBufferDesc.MiscFlags = 0;
    swizzleBufferDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&swizzleBufferDesc, nullptr, &mSwizzleCB);
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mSwizzleCB, "Blit11 swizzle constant buffer");
}

Blit11::~Blit11()
{
    SafeRelease(mVertexBuffer);
    SafeRelease(mPointSampler);
    SafeRelease(mLinearSampler);
    SafeRelease(mScissorEnabledRasterizerState);
    SafeRelease(mScissorDisabledRasterizerState);
    SafeRelease(mDepthStencilState);

    mQuad2DIL.release();
    mQuad2DVS.release();
    mDepthPS.release();

    mQuad3DIL.release();
    mQuad3DVS.release();
    mQuad3DGS.release();

    SafeRelease(mSwizzleCB);

    clearShaderMap();
}

// static
Blit11::BlitShaderType Blit11::GetBlitShaderType(GLenum destinationFormat, bool isSigned, ShaderDimension dimension)
{
    if (dimension == SHADER_3D)
    {
        if (isSigned)
        {
            switch (destinationFormat)
            {
              case GL_RGBA_INTEGER: return BLITSHADER_3D_RGBAI;
              case GL_RGB_INTEGER:  return BLITSHADER_3D_RGBI;
              case GL_RG_INTEGER:   return BLITSHADER_3D_RGI;
              case GL_RED_INTEGER:  return BLITSHADER_3D_RI;
              default:
                UNREACHABLE();
                return BLITSHADER_INVALID;
            }
        }
        else
        {
            switch (destinationFormat)
            {
              case GL_RGBA:             return BLITSHADER_3D_RGBAF;
              case GL_RGBA_INTEGER:     return BLITSHADER_3D_RGBAUI;
              case GL_BGRA_EXT:         return BLITSHADER_3D_BGRAF;
              case GL_RGB:              return BLITSHADER_3D_RGBF;
              case GL_RGB_INTEGER:      return BLITSHADER_3D_RGBUI;
              case GL_RG:               return BLITSHADER_3D_RGF;
              case GL_RG_INTEGER:       return BLITSHADER_3D_RGUI;
              case GL_RED:              return BLITSHADER_3D_RF;
              case GL_RED_INTEGER:      return BLITSHADER_3D_RUI;
              case GL_ALPHA:            return BLITSHADER_3D_ALPHA;
              case GL_LUMINANCE:        return BLITSHADER_3D_LUMA;
              case GL_LUMINANCE_ALPHA:  return BLITSHADER_3D_LUMAALPHA;
              default:
                UNREACHABLE();
                return BLITSHADER_INVALID;
            }
        }
    }
    else if (isSigned)
    {
        switch (destinationFormat)
        {
          case GL_RGBA_INTEGER: return BLITSHADER_2D_RGBAI;
          case GL_RGB_INTEGER:  return BLITSHADER_2D_RGBI;
          case GL_RG_INTEGER:   return BLITSHADER_2D_RGI;
          case GL_RED_INTEGER:  return BLITSHADER_2D_RI;
          default:
            UNREACHABLE();
            return BLITSHADER_INVALID;
        }
    }
    else
    {
        switch (destinationFormat)
        {
          case GL_RGBA:             return BLITSHADER_2D_RGBAF;
          case GL_RGBA_INTEGER:     return BLITSHADER_2D_RGBAUI;
          case GL_BGRA_EXT:         return BLITSHADER_2D_BGRAF;
          case GL_RGB:              return BLITSHADER_2D_RGBF;
          case GL_RGB_INTEGER:      return BLITSHADER_2D_RGBUI;
          case GL_RG:               return BLITSHADER_2D_RGF;
          case GL_RG_INTEGER:       return BLITSHADER_2D_RGUI;
          case GL_RED:              return BLITSHADER_2D_RF;
          case GL_RED_INTEGER:      return BLITSHADER_2D_RUI;
          case GL_ALPHA:            return BLITSHADER_2D_ALPHA;
          case GL_LUMINANCE:        return BLITSHADER_2D_LUMA;
          case GL_LUMINANCE_ALPHA:  return BLITSHADER_2D_LUMAALPHA;
          default:
            UNREACHABLE();
            return BLITSHADER_INVALID;
        }
    }
}

// static
Blit11::SwizzleShaderType Blit11::GetSwizzleShaderType(GLenum type, D3D11_SRV_DIMENSION dimensionality)
{
    switch (dimensionality)
    {
      case D3D11_SRV_DIMENSION_TEXTURE2D:
        switch (type)
        {
          case GL_FLOAT:        return SWIZZLESHADER_2D_FLOAT;
          case GL_UNSIGNED_INT: return SWIZZLESHADER_2D_UINT;
          case GL_INT:          return SWIZZLESHADER_2D_INT;
          default:
            UNREACHABLE();
            return SWIZZLESHADER_INVALID;
        }
      case D3D11_SRV_DIMENSION_TEXTURECUBE:
        switch (type)
        {
          case GL_FLOAT:        return SWIZZLESHADER_CUBE_FLOAT;
          case GL_UNSIGNED_INT: return SWIZZLESHADER_CUBE_UINT;
          case GL_INT:          return SWIZZLESHADER_CUBE_INT;
          default:
            UNREACHABLE();
            return SWIZZLESHADER_INVALID;
        }
      case D3D11_SRV_DIMENSION_TEXTURE3D:
        switch (type)
        {
          case GL_FLOAT:        return SWIZZLESHADER_3D_FLOAT;
          case GL_UNSIGNED_INT: return SWIZZLESHADER_3D_UINT;
          case GL_INT:          return SWIZZLESHADER_3D_INT;
          default:
            UNREACHABLE();
            return SWIZZLESHADER_INVALID;
        }
      case D3D11_SRV_DIMENSION_TEXTURE2DARRAY:
        switch (type)
        {
          case GL_FLOAT:        return SWIZZLESHADER_ARRAY_FLOAT;
          case GL_UNSIGNED_INT: return SWIZZLESHADER_ARRAY_UINT;
          case GL_INT:          return SWIZZLESHADER_ARRAY_INT;
          default:
            UNREACHABLE();
            return SWIZZLESHADER_INVALID;
        }
      default:
        UNREACHABLE();
        return SWIZZLESHADER_INVALID;
    }
}

Blit11::ShaderSupport Blit11::getShaderSupport(const Shader &shader)
{
    ID3D11Device *device = mRenderer->getDevice();
    ShaderSupport support;

    if (shader.dimension == SHADER_2D)
    {
        support.inputLayout = mQuad2DIL.resolve(device);
        support.vertexShader = mQuad2DVS.resolve(device);
        support.geometryShader = nullptr;
        support.vertexWriteFunction = Write2DVertices;
    }
    else
    {
        ASSERT(shader.dimension == SHADER_3D);
        support.inputLayout = mQuad3DIL.resolve(device);
        support.vertexShader = mQuad3DVS.resolve(device);
        support.geometryShader = mQuad3DGS.resolve(device);
        support.vertexWriteFunction = Write3DVertices;
    }

    return support;
}

gl::Error Blit11::swizzleTexture(ID3D11ShaderResourceView *source,
                                 ID3D11RenderTargetView *dest,
                                 const gl::Extents &size,
                                 GLenum swizzleRed,
                                 GLenum swizzleGreen,
                                 GLenum swizzleBlue,
                                 GLenum swizzleAlpha)
{
    HRESULT result;
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    D3D11_SHADER_RESOURCE_VIEW_DESC sourceSRVDesc;
    source->GetDesc(&sourceSRVDesc);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(sourceSRVDesc.Format);
    const gl::InternalFormat &sourceFormatInfo = gl::GetInternalFormatInfo(dxgiFormatInfo.internalFormat);

    GLenum shaderType = GL_NONE;
    switch (sourceFormatInfo.componentType)
    {
      case GL_UNSIGNED_NORMALIZED:
      case GL_SIGNED_NORMALIZED:
      case GL_FLOAT:
        shaderType = GL_FLOAT;
        break;
      case GL_INT:
        shaderType = GL_INT;
        break;
      case GL_UNSIGNED_INT:
        shaderType = GL_UNSIGNED_INT;
        break;
      default:
        UNREACHABLE();
        break;
    }

    const Shader *shader = nullptr;
    gl::Error error = getSwizzleShader(shaderType, sourceSRVDesc.ViewDimension, &shader);
    if (error.isError())
    {
        return error;
    }

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = deviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal vertex buffer for swizzle, HRESULT: 0x%X.", result);
    }

    const ShaderSupport &support = getShaderSupport(*shader);

    UINT stride = 0;
    UINT startIdx = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    gl::Box area(0, 0, 0, size.width, size.height, size.depth);
    support.vertexWriteFunction(area, size, area, size, mappedResource.pData, &stride, &drawCount, &topology);

    deviceContext->Unmap(mVertexBuffer, 0);

    // Set constant buffer
    result = deviceContext->Map(mSwizzleCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal constant buffer for swizzle, HRESULT: 0x%X.", result);
    }

    unsigned int *swizzleIndices = reinterpret_cast<unsigned int*>(mappedResource.pData);
    swizzleIndices[0] = GetSwizzleIndex(swizzleRed);
    swizzleIndices[1] = GetSwizzleIndex(swizzleGreen);
    swizzleIndices[2] = GetSwizzleIndex(swizzleBlue);
    swizzleIndices[3] = GetSwizzleIndex(swizzleAlpha);

    deviceContext->Unmap(mSwizzleCB, 0);

    // Apply vertex buffer
    deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &startIdx);

    // Apply constant buffer
    deviceContext->PSSetConstantBuffers(0, 1, &mSwizzleCB);

    // Apply state
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFF);
    deviceContext->OMSetDepthStencilState(nullptr, 0xFFFFFFFF);
    deviceContext->RSSetState(mScissorDisabledRasterizerState);

    // Apply shaders
    deviceContext->IASetInputLayout(support.inputLayout);
    deviceContext->IASetPrimitiveTopology(topology);
    deviceContext->VSSetShader(support.vertexShader, nullptr, 0);

    deviceContext->PSSetShader(shader->pixelShader, nullptr, 0);
    deviceContext->GSSetShader(support.geometryShader, nullptr, 0);

    // Unset the currently bound shader resource to avoid conflicts
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    // Apply render target
    mRenderer->setOneTimeRenderTarget(dest);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(size.width);
    viewport.Height = static_cast<FLOAT>(size.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Apply textures
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, source);

    // Apply samplers
    deviceContext->PSSetSamplers(0, 1, &mPointSampler);

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    // Unbind textures and render targets and vertex buffer
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    mRenderer->unapplyRenderTargets();

    UINT zero = 0;
    ID3D11Buffer *const nullBuffer = nullptr;
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);

    mRenderer->markAllStateDirty();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Blit11::copyTexture(ID3D11ShaderResourceView *source, const gl::Box &sourceArea, const gl::Extents &sourceSize,
                              ID3D11RenderTargetView *dest, const gl::Box &destArea, const gl::Extents &destSize,
                              const gl::Rectangle *scissor, GLenum destFormat, GLenum filter)
{
    HRESULT result;
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Determine if the source format is a signed integer format, the destFormat will already
    // be GL_XXXX_INTEGER but it does not tell us if it is signed or unsigned.
    D3D11_SHADER_RESOURCE_VIEW_DESC sourceSRVDesc;
    source->GetDesc(&sourceSRVDesc);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(sourceSRVDesc.Format);
    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(dxgiFormatInfo.internalFormat);

    bool isSigned = (internalFormatInfo.componentType == GL_INT);
    ShaderDimension dimension = (sourceSRVDesc.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE3D) ? SHADER_3D : SHADER_2D;

    const Shader *shader = nullptr;
    gl::Error error = getBlitShader(destFormat, isSigned, dimension, &shader);
    if (error.isError())
    {
        return error;
    }

    const ShaderSupport &support = getShaderSupport(*shader);

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = deviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal vertex buffer for texture copy, HRESULT: 0x%X.", result);
    }

    UINT stride = 0;
    UINT startIdx = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    support.vertexWriteFunction(sourceArea, sourceSize, destArea, destSize, mappedResource.pData,
                                &stride, &drawCount, &topology);

    deviceContext->Unmap(mVertexBuffer, 0);

    // Apply vertex buffer
    deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &startIdx);

    // Apply state
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFF);
    deviceContext->OMSetDepthStencilState(nullptr, 0xFFFFFFFF);

    if (scissor)
    {
        D3D11_RECT scissorRect;
        scissorRect.left = scissor->x;
        scissorRect.right = scissor->x + scissor->width;
        scissorRect.top = scissor->y;
        scissorRect.bottom = scissor->y + scissor->height;

        deviceContext->RSSetScissorRects(1, &scissorRect);
        deviceContext->RSSetState(mScissorEnabledRasterizerState);
    }
    else
    {
        deviceContext->RSSetState(mScissorDisabledRasterizerState);
    }

    // Apply shaders
    deviceContext->IASetInputLayout(support.inputLayout);
    deviceContext->IASetPrimitiveTopology(topology);
    deviceContext->VSSetShader(support.vertexShader, nullptr, 0);

    deviceContext->PSSetShader(shader->pixelShader, nullptr, 0);
    deviceContext->GSSetShader(support.geometryShader, nullptr, 0);

    // Unset the currently bound shader resource to avoid conflicts
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    // Apply render target
    mRenderer->setOneTimeRenderTarget(dest);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(destSize.width);
    viewport.Height = static_cast<FLOAT>(destSize.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Apply textures
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, source);

    // Apply samplers
    ID3D11SamplerState *sampler = nullptr;
    switch (filter)
    {
      case GL_NEAREST: sampler = mPointSampler;  break;
      case GL_LINEAR:  sampler = mLinearSampler; break;

      default:
        UNREACHABLE();
        return gl::Error(GL_OUT_OF_MEMORY, "Internal error, unknown blit filter mode.");
    }
    deviceContext->PSSetSamplers(0, 1, &sampler);

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    // Unbind textures and render targets and vertex buffer
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    mRenderer->unapplyRenderTargets();

    UINT zero = 0;
    ID3D11Buffer *const nullBuffer = nullptr;
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);

    mRenderer->markAllStateDirty();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Blit11::copyStencil(ID3D11Resource *source, unsigned int sourceSubresource, const gl::Box &sourceArea, const gl::Extents &sourceSize,
                              ID3D11Resource *dest, unsigned int destSubresource, const gl::Box &destArea, const gl::Extents &destSize,
                              const gl::Rectangle *scissor)
{
    return copyDepthStencil(source, sourceSubresource, sourceArea, sourceSize,
                            dest, destSubresource, destArea, destSize,
                            scissor, true);
}

gl::Error Blit11::copyDepth(ID3D11ShaderResourceView *source, const gl::Box &sourceArea, const gl::Extents &sourceSize,
                            ID3D11DepthStencilView *dest, const gl::Box &destArea, const gl::Extents &destSize,
                            const gl::Rectangle *scissor)
{
    HRESULT result;
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result = deviceContext->Map(mVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal vertex buffer for texture copy, HRESULT: 0x%X.", result);
    }

    UINT stride = 0;
    UINT startIdx = 0;
    UINT drawCount = 0;
    D3D11_PRIMITIVE_TOPOLOGY topology;

    Write2DVertices(sourceArea, sourceSize, destArea, destSize, mappedResource.pData,
                    &stride, &drawCount, &topology);

    deviceContext->Unmap(mVertexBuffer, 0);

    // Apply vertex buffer
    deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &startIdx);

    // Apply state
    deviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFF);
    deviceContext->OMSetDepthStencilState(mDepthStencilState, 0xFFFFFFFF);

    if (scissor)
    {
        D3D11_RECT scissorRect;
        scissorRect.left = scissor->x;
        scissorRect.right = scissor->x + scissor->width;
        scissorRect.top = scissor->y;
        scissorRect.bottom = scissor->y + scissor->height;

        deviceContext->RSSetScissorRects(1, &scissorRect);
        deviceContext->RSSetState(mScissorEnabledRasterizerState);
    }
    else
    {
        deviceContext->RSSetState(mScissorDisabledRasterizerState);
    }

    ID3D11Device *device = mRenderer->getDevice();
    ID3D11VertexShader *quad2DVS = mQuad2DVS.resolve(device);
    if (quad2DVS == nullptr)
    {
        return gl::Error(GL_INVALID_OPERATION, "Error compiling internal 2D blit vertex shader");
    }

    // Apply shaders
    deviceContext->IASetInputLayout(mQuad2DIL.resolve(device));
    deviceContext->IASetPrimitiveTopology(topology);
    deviceContext->VSSetShader(quad2DVS, nullptr, 0);

    deviceContext->PSSetShader(mDepthPS.resolve(device), nullptr, 0);
    deviceContext->GSSetShader(nullptr, nullptr, 0);

    // Unset the currently bound shader resource to avoid conflicts
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    // Apply render target
    deviceContext->OMSetRenderTargets(0, nullptr, dest);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<FLOAT>(destSize.width);
    viewport.Height = static_cast<FLOAT>(destSize.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Apply textures
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, source);

    // Apply samplers
    deviceContext->PSSetSamplers(0, 1, &mPointSampler);

    // Draw the quad
    deviceContext->Draw(drawCount, 0);

    // Unbind textures and render targets and vertex buffer
    mRenderer->setShaderResource(gl::SAMPLER_PIXEL, 0, nullptr);

    mRenderer->unapplyRenderTargets();

    UINT zero = 0;
    ID3D11Buffer *const nullBuffer = nullptr;
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);

    mRenderer->markAllStateDirty();

    return gl::Error(GL_NO_ERROR);
}

gl::Error Blit11::copyDepthStencil(ID3D11Resource *source, unsigned int sourceSubresource, const gl::Box &sourceArea, const gl::Extents &sourceSize,
                                   ID3D11Resource *dest, unsigned int destSubresource, const gl::Box &destArea, const gl::Extents &destSize,
                                   const gl::Rectangle *scissor)
{
    return copyDepthStencil(source, sourceSubresource, sourceArea, sourceSize,
                            dest, destSubresource, destArea, destSize,
                            scissor, false);
}

gl::Error Blit11::copyDepthStencil(ID3D11Resource *source, unsigned int sourceSubresource, const gl::Box &sourceArea, const gl::Extents &sourceSize,
                                   ID3D11Resource *dest, unsigned int destSubresource, const gl::Box &destArea, const gl::Extents &destSize,
                                   const gl::Rectangle *scissor, bool stencilOnly)
{
    ID3D11Device *device = mRenderer->getDevice();
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();

    ID3D11Resource *sourceStaging = CreateStagingTexture(device, deviceContext, source, sourceSubresource, sourceSize, D3D11_CPU_ACCESS_READ);
    // HACK: Create the destination staging buffer as a read/write texture so ID3D11DevicContext::UpdateSubresource can be called
    //       using it's mapped data as a source
    ID3D11Resource *destStaging = CreateStagingTexture(device, deviceContext, dest, destSubresource, destSize, D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE);

    if (!sourceStaging || !destStaging)
    {
        SafeRelease(sourceStaging);
        SafeRelease(destStaging);
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal staging textures for depth stencil blit.");
    }

    DXGI_FORMAT format = GetTextureFormat(source);
    ASSERT(format == GetTextureFormat(dest));

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(format);
    unsigned int pixelSize = dxgiFormatInfo.pixelBytes;
    unsigned int copyOffset = 0;
    unsigned int copySize = pixelSize;
    if (stencilOnly)
    {
        copyOffset = dxgiFormatInfo.depthBits / 8;
        copySize = dxgiFormatInfo.stencilBits / 8;

        // It would be expensive to have non-byte sized stencil sizes since it would
        // require reading from the destination, currently there aren't any though.
        ASSERT(dxgiFormatInfo.stencilBits % 8 == 0 &&
               dxgiFormatInfo.depthBits   % 8 == 0);
    }

    D3D11_MAPPED_SUBRESOURCE sourceMapping;
    HRESULT result = deviceContext->Map(sourceStaging, 0, D3D11_MAP_READ, 0, &sourceMapping);
    if (FAILED(result))
    {
        SafeRelease(sourceStaging);
        SafeRelease(destStaging);
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal source staging texture for depth stencil blit, HRESULT: 0x%X.", result);
    }

    D3D11_MAPPED_SUBRESOURCE destMapping;
    result = deviceContext->Map(destStaging, 0, D3D11_MAP_WRITE, 0, &destMapping);
    if (FAILED(result))
    {
        deviceContext->Unmap(sourceStaging, 0);
        SafeRelease(sourceStaging);
        SafeRelease(destStaging);
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal destination staging texture for depth stencil blit, HRESULT: 0x%X.", result);
    }

    gl::Rectangle clippedDestArea(destArea.x, destArea.y, destArea.width, destArea.height);

    // Clip dest area to the destination size
    gl::ClipRectangle(clippedDestArea, gl::Rectangle(0, 0, destSize.width, destSize.height), &clippedDestArea);

    // Clip dest area to the scissor
    if (scissor)
    {
        gl::ClipRectangle(clippedDestArea, *scissor, &clippedDestArea);
    }

    // Determine if entire rows can be copied at once instead of each individual pixel, requires that there is
    // no out of bounds lookups required, the entire pixel is copied and no stretching
    bool wholeRowCopy = sourceArea.width == clippedDestArea.width &&
                        sourceArea.x >= 0 && sourceArea.x + sourceArea.width <= sourceSize.width &&
                        copySize == pixelSize;

    for (int y = clippedDestArea.y; y < clippedDestArea.y + clippedDestArea.height; y++)
    {
        float yPerc = static_cast<float>(y - destArea.y) / (destArea.height - 1);

        // Interpolate using the original source rectangle to determine which row to sample from while clamping to the edges
        unsigned int readRow = static_cast<unsigned int>(gl::clamp(sourceArea.y + floor(yPerc * (sourceArea.height - 1) + 0.5f), 0, sourceSize.height - 1));
        unsigned int writeRow = y;

        if (wholeRowCopy)
        {
            void *sourceRow = reinterpret_cast<char*>(sourceMapping.pData) +
                              readRow * sourceMapping.RowPitch +
                              sourceArea.x * pixelSize;

            void *destRow = reinterpret_cast<char*>(destMapping.pData) +
                            writeRow * destMapping.RowPitch +
                            destArea.x * pixelSize;

            memcpy(destRow, sourceRow, pixelSize * destArea.width);
        }
        else
        {
            for (int x = clippedDestArea.x; x < clippedDestArea.x + clippedDestArea.width; x++)
            {
                float xPerc = static_cast<float>(x - destArea.x) / (destArea.width - 1);

                // Interpolate the original source rectangle to determine which column to sample from while clamping to the edges
                unsigned int readColumn = static_cast<unsigned int>(gl::clamp(sourceArea.x + floor(xPerc * (sourceArea.width - 1) + 0.5f), 0, sourceSize.width - 1));
                unsigned int writeColumn = x;

                void *sourcePixel = reinterpret_cast<char*>(sourceMapping.pData) +
                                    readRow * sourceMapping.RowPitch +
                                    readColumn * pixelSize +
                                    copyOffset;

                void *destPixel = reinterpret_cast<char*>(destMapping.pData) +
                                  writeRow * destMapping.RowPitch +
                                  writeColumn * pixelSize +
                                  copyOffset;

                memcpy(destPixel, sourcePixel, copySize);
            }
        }
    }

    // HACK: Use ID3D11DevicContext::UpdateSubresource which causes an extra copy compared to ID3D11DevicContext::CopySubresourceRegion
    //       according to MSDN.
    deviceContext->UpdateSubresource(dest, destSubresource, nullptr, destMapping.pData, destMapping.RowPitch, destMapping.DepthPitch);

    deviceContext->Unmap(sourceStaging, 0);
    deviceContext->Unmap(destStaging, 0);

    // TODO: Determine why this call to ID3D11DevicContext::CopySubresourceRegion causes a TDR timeout on some
    //       systems when called repeatedly.
    // deviceContext->CopySubresourceRegion(dest, destSubresource, 0, 0, 0, destStaging, 0, nullptr);

    SafeRelease(sourceStaging);
    SafeRelease(destStaging);

    return gl::Error(GL_NO_ERROR);
}

void Blit11::addBlitShaderToMap(BlitShaderType blitShaderType, ShaderDimension dimension, ID3D11PixelShader *ps)
{
    ASSERT(mBlitShaderMap.find(blitShaderType) == mBlitShaderMap.end());
    ASSERT(ps);

    Shader shader;
    shader.dimension = dimension;
    shader.pixelShader = ps;

    mBlitShaderMap[blitShaderType] = shader;
}

void Blit11::addSwizzleShaderToMap(SwizzleShaderType swizzleShaderType, ShaderDimension dimension, ID3D11PixelShader *ps)
{
    ASSERT(mSwizzleShaderMap.find(swizzleShaderType) == mSwizzleShaderMap.end());
    ASSERT(ps);

    Shader shader;
    shader.dimension = dimension;
    shader.pixelShader = ps;

    mSwizzleShaderMap[swizzleShaderType] = shader;
}

void Blit11::clearShaderMap()
{
    for (auto &blitShader : mBlitShaderMap)
    {
        SafeRelease(blitShader.second.pixelShader);
    }
    mBlitShaderMap.clear();

    for (auto &swizzleShader : mSwizzleShaderMap)
    {
        SafeRelease(swizzleShader.second.pixelShader);
    }
    mSwizzleShaderMap.clear();
}

gl::Error Blit11::getBlitShader(GLenum destFormat, bool isSigned, ShaderDimension dimension, const Shader **shader)
{
    BlitShaderType blitShaderType = GetBlitShaderType(destFormat, isSigned, dimension);

    if (blitShaderType == BLITSHADER_INVALID)
    {
        return gl::Error(GL_INVALID_OPERATION, "Internal blit shader type mismatch");
    }

    auto blitShaderIt = mBlitShaderMap.find(blitShaderType);
    if (blitShaderIt != mBlitShaderMap.end())
    {
        *shader = &blitShaderIt->second;
        return gl::Error(GL_NO_ERROR);
    }

    ASSERT(dimension == SHADER_2D || mRenderer->isES3Capable());

    ID3D11Device *device = mRenderer->getDevice();

    switch (blitShaderType)
    {
      case BLITSHADER_2D_RGBAF:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGBA2D, "Blit11 2D RGBA pixel shader"));
        break;
      case BLITSHADER_2D_BGRAF:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGBA2D, "Blit11 2D BGRA pixel shader"));
        break;
      case BLITSHADER_2D_RGBF:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGB2D, "Blit11 2D RGB pixel shader"));
        break;
      case BLITSHADER_2D_RGF:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRG2D, "Blit11 2D RG pixel shader"));
        break;
      case BLITSHADER_2D_RF:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughR2D, "Blit11 2D R pixel shader"));
        break;
      case BLITSHADER_2D_ALPHA:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGBA2D, "Blit11 2D alpha pixel shader"));
        break;
      case BLITSHADER_2D_LUMA:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughLum2D, "Blit11 2D lum pixel shader"));
        break;
      case BLITSHADER_2D_LUMAALPHA:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughLumAlpha2D, "Blit11 2D luminance alpha pixel shader"));
        break;
      case BLITSHADER_2D_RGBAUI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGBA2DUI, "Blit11 2D RGBA UI pixel shader"));
        break;
      case BLITSHADER_2D_RGBAI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGBA2DI, "Blit11 2D RGBA I pixel shader"));
        break;
      case BLITSHADER_2D_RGBUI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGB2DUI, "Blit11 2D RGB UI pixel shader"));
        break;
      case BLITSHADER_2D_RGBI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRGB2DI, "Blit11 2D RGB I pixel shader"));
        break;
      case BLITSHADER_2D_RGUI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRG2DUI, "Blit11 2D RG UI pixel shader"));
        break;
      case BLITSHADER_2D_RGI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughRG2DI, "Blit11 2D RG I pixel shader"));
        break;
      case BLITSHADER_2D_RUI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughR2DUI, "Blit11 2D R UI pixel shader"));
        break;
      case BLITSHADER_2D_RI:
        addBlitShaderToMap(blitShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_PassthroughR2DI, "Blit11 2D R I pixel shader"));
        break;
      case BLITSHADER_3D_RGBAF:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGBA3D, "Blit11 3D RGBA pixel shader"));
        break;
      case BLITSHADER_3D_RGBAUI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGBA3DUI, "Blit11 3D UI RGBA pixel shader"));
        break;
      case BLITSHADER_3D_RGBAI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGBA3DI, "Blit11 3D I RGBA pixel shader"));
        break;
      case BLITSHADER_3D_BGRAF:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGBA3D, "Blit11 3D BGRA pixel shader"));
        break;
      case BLITSHADER_3D_RGBF:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGB3D, "Blit11 3D RGB pixel shader"));
        break;
      case BLITSHADER_3D_RGBUI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGB3DUI, "Blit11 3D RGB UI pixel shader"));
        break;
      case BLITSHADER_3D_RGBI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGB3DI, "Blit11 3D RGB I pixel shader"));
        break;
      case BLITSHADER_3D_RGF:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRG3D, "Blit11 3D RG pixel shader"));
        break;
      case BLITSHADER_3D_RGUI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRG3DUI, "Blit11 3D RG UI pixel shader"));
        break;
      case BLITSHADER_3D_RGI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRG3DI, "Blit11 3D RG I pixel shader"));
        break;
      case BLITSHADER_3D_RF:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughR3D, "Blit11 3D R pixel shader"));
        break;
      case BLITSHADER_3D_RUI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughR3DUI, "Blit11 3D R UI pixel shader"));
        break;
      case BLITSHADER_3D_RI:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughR3DI, "Blit11 3D R I pixel shader"));
        break;
      case BLITSHADER_3D_ALPHA:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughRGBA3D, "Blit11 3D alpha pixel shader"));
        break;
      case BLITSHADER_3D_LUMA:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughLum3D, "Blit11 3D luminance pixel shader"));
        break;
      case BLITSHADER_3D_LUMAALPHA:
        addBlitShaderToMap(blitShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_PassthroughLumAlpha3D, "Blit11 3D luminance alpha pixel shader"));
        break;
      default:
        UNREACHABLE();
        return gl::Error(GL_INVALID_OPERATION, "Internal error");
    }

    blitShaderIt = mBlitShaderMap.find(blitShaderType);
    ASSERT(blitShaderIt != mBlitShaderMap.end());
    *shader = &blitShaderIt->second;
    return gl::Error(GL_NO_ERROR);
}

gl::Error Blit11::getSwizzleShader(GLenum type, D3D11_SRV_DIMENSION viewDimension, const Shader **shader)
{
    SwizzleShaderType swizzleShaderType = GetSwizzleShaderType(type, viewDimension);

    if (swizzleShaderType == SWIZZLESHADER_INVALID)
    {
        return gl::Error(GL_INVALID_OPERATION, "Swizzle shader type not found");
    }

    auto swizzleShaderIt = mSwizzleShaderMap.find(swizzleShaderType);
    if (swizzleShaderIt != mSwizzleShaderMap.end())
    {
        *shader = &swizzleShaderIt->second;
        return gl::Error(GL_NO_ERROR);
    }

    // Swizzling shaders (OpenGL ES 3+)
    ASSERT(mRenderer->isES3Capable());

    ID3D11Device *device = mRenderer->getDevice();

    switch (swizzleShaderType)
    {
      case SWIZZLESHADER_2D_FLOAT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_SwizzleF2D, "Blit11 2D F swizzle pixel shader"));
        break;
      case SWIZZLESHADER_2D_UINT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_SwizzleUI2D, "Blit11 2D UI swizzle pixel shader"));
        break;
      case SWIZZLESHADER_2D_INT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_2D, d3d11::CompilePS(device, g_PS_SwizzleI2D, "Blit11 2D I swizzle pixel shader"));
        break;
      case SWIZZLESHADER_CUBE_FLOAT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleF2DArray, "Blit11 2D Cube F swizzle pixel shader"));
        break;
      case SWIZZLESHADER_CUBE_UINT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleUI2DArray, "Blit11 2D Cube UI swizzle pixel shader"));
        break;
      case SWIZZLESHADER_CUBE_INT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleI2DArray, "Blit11 2D Cube I swizzle pixel shader"));
        break;
      case SWIZZLESHADER_3D_FLOAT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleF3D, "Blit11 3D F swizzle pixel shader"));
        break;
      case SWIZZLESHADER_3D_UINT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleUI3D, "Blit11 3D UI swizzle pixel shader"));
        break;
      case SWIZZLESHADER_3D_INT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleI3D, "Blit11 3D I swizzle pixel shader"));
        break;
      case SWIZZLESHADER_ARRAY_FLOAT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleF2DArray, "Blit11 2D Array F swizzle pixel shader"));
        break;
      case SWIZZLESHADER_ARRAY_UINT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleUI2DArray, "Blit11 2D Array UI swizzle pixel shader"));
        break;
      case SWIZZLESHADER_ARRAY_INT:
        addSwizzleShaderToMap(swizzleShaderType, SHADER_3D, d3d11::CompilePS(device, g_PS_SwizzleI2DArray, "Blit11 2D Array I swizzle pixel shader"));
        break;
      default:
        UNREACHABLE();
        return gl::Error(GL_INVALID_OPERATION, "Internal error");
    }

    swizzleShaderIt = mSwizzleShaderMap.find(swizzleShaderType);
    ASSERT(swizzleShaderIt != mSwizzleShaderMap.end());
    *shader = &swizzleShaderIt->second;
    return gl::Error(GL_NO_ERROR);
}

}
