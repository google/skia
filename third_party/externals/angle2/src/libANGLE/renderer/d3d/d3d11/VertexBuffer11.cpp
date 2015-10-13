//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexBuffer11.cpp: Defines the D3D11 VertexBuffer implementation.

#include "libANGLE/renderer/d3d/d3d11/VertexBuffer11.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Buffer11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{

VertexBuffer11::VertexBuffer11(Renderer11 *const renderer) : mRenderer(renderer)
{
    mBuffer = NULL;
    mBufferSize = 0;
    mDynamicUsage = false;
    mMappedResourceData = NULL;
}

VertexBuffer11::~VertexBuffer11()
{
    ASSERT(mMappedResourceData == NULL);
    SafeRelease(mBuffer);
}

gl::Error VertexBuffer11::initialize(unsigned int size, bool dynamicUsage)
{
    SafeRelease(mBuffer);

    updateSerial();

    if (size > 0)
    {
        ID3D11Device* dxDevice = mRenderer->getDevice();

        D3D11_BUFFER_DESC bufferDesc;
        bufferDesc.ByteWidth = size;
        bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bufferDesc.MiscFlags = 0;
        bufferDesc.StructureByteStride = 0;

        HRESULT result = dxDevice->CreateBuffer(&bufferDesc, NULL, &mBuffer);
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal vertex buffer of size, %lu.", size);
        }

        if (dynamicUsage)
        {
            d3d11::SetDebugName(mBuffer, "VertexBuffer11 (dynamic)");
        }
        else
        {
            d3d11::SetDebugName(mBuffer, "VertexBuffer11 (static)");
        }
    }

    mBufferSize = size;
    mDynamicUsage = dynamicUsage;

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexBuffer11::mapResource()
{
    if (mMappedResourceData == NULL)
    {
        ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();

        D3D11_MAPPED_SUBRESOURCE mappedResource;

        HRESULT result = dxContext->Map(mBuffer, 0, D3D11_MAP_WRITE_NO_OVERWRITE, 0, &mappedResource);
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal vertex buffer, HRESULT: 0x%08x.", result);
        }

        mMappedResourceData = reinterpret_cast<uint8_t*>(mappedResource.pData);
    }

    return gl::Error(GL_NO_ERROR);
}

void VertexBuffer11::hintUnmapResource()
{
    if (mMappedResourceData != NULL)
    {
        ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();
        dxContext->Unmap(mBuffer, 0);

        mMappedResourceData = NULL;
    }
}

gl::Error VertexBuffer11::storeVertexAttributes(const gl::VertexAttribute &attrib,
                                                GLenum currentValueType,
                                                GLint start,
                                                GLsizei count,
                                                GLsizei instances,
                                                unsigned int offset,
                                                const uint8_t *sourceData)
{
    if (!mBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal vertex buffer is not initialized.");
    }

    int inputStride = static_cast<int>(ComputeVertexAttributeStride(attrib));

    // This will map the resource if it isn't already mapped.
    gl::Error error = mapResource();
    if (error.isError())
    {
        return error;
    }

    uint8_t *output = mMappedResourceData + offset;

    const uint8_t *input = sourceData;

    if (instances == 0 || attrib.divisor == 0)
    {
        input += inputStride * start;
    }

    gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib, currentValueType);
    const D3D_FEATURE_LEVEL featureLevel = mRenderer->getRenderer11DeviceCaps().featureLevel;
    const d3d11::VertexFormat &vertexFormatInfo = d3d11::GetVertexFormatInfo(vertexFormatType, featureLevel);
    ASSERT(vertexFormatInfo.copyFunction != NULL);
    vertexFormatInfo.copyFunction(input, inputStride, count, output);

    return gl::Error(GL_NO_ERROR);
}

gl::Error VertexBuffer11::getSpaceRequired(const gl::VertexAttribute &attrib, GLsizei count,
                                           GLsizei instances, unsigned int *outSpaceRequired) const
{
    unsigned int elementCount = 0;
    if (attrib.enabled)
    {
        if (instances == 0 || attrib.divisor == 0)
        {
            elementCount = count;
        }
        else
        {
            // Round up to divisor, if possible
            elementCount = UnsignedCeilDivide(static_cast<unsigned int>(instances), attrib.divisor);
        }

        gl::VertexFormatType formatType = gl::GetVertexFormatType(attrib);
        const D3D_FEATURE_LEVEL featureLevel = mRenderer->getRenderer11DeviceCaps().featureLevel;
        const d3d11::VertexFormat &vertexFormatInfo = d3d11::GetVertexFormatInfo(formatType, featureLevel);
        const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(vertexFormatInfo.nativeFormat);
        unsigned int elementSize = dxgiFormatInfo.pixelBytes;
        if (elementSize <= std::numeric_limits<unsigned int>::max() / elementCount)
        {
            if (outSpaceRequired)
            {
                *outSpaceRequired = elementSize * elementCount;
            }
            return gl::Error(GL_NO_ERROR);
        }
        else
        {
            return gl::Error(GL_OUT_OF_MEMORY, "New vertex buffer size would result in an overflow.");
        }
    }
    else
    {
        const unsigned int elementSize = 4;
        if (outSpaceRequired)
        {
            *outSpaceRequired = elementSize * 4;
        }
        return gl::Error(GL_NO_ERROR);
    }
}

unsigned int VertexBuffer11::getBufferSize() const
{
    return mBufferSize;
}

gl::Error VertexBuffer11::setBufferSize(unsigned int size)
{
    if (size > mBufferSize)
    {
        return initialize(size, mDynamicUsage);
    }
    else
    {
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error VertexBuffer11::discard()
{
    if (!mBuffer)
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Internal vertex buffer is not initialized.");
    }

    ID3D11DeviceContext *dxContext = mRenderer->getDeviceContext();

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT result = dxContext->Map(mBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to map internal buffer for discarding, HRESULT: 0x%08x", result);
    }

    dxContext->Unmap(mBuffer, 0);

    return gl::Error(GL_NO_ERROR);
}

ID3D11Buffer *VertexBuffer11::getBuffer() const
{
    return mBuffer;
}

}
