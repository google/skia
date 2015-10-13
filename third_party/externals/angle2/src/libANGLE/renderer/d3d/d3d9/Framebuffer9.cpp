//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Framebuffer9.cpp: Implements the Framebuffer9 class.

#include "libANGLE/renderer/d3d/d3d9/Framebuffer9.h"
#include "libANGLE/renderer/d3d/d3d9/formatutils9.h"
#include "libANGLE/renderer/d3d/d3d9/TextureStorage9.h"
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#include "libANGLE/renderer/d3d/d3d9/renderer9_utils.h"
#include "libANGLE/renderer/d3d/d3d9/RenderTarget9.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Texture.h"

namespace rx
{

Framebuffer9::Framebuffer9(const gl::Framebuffer::Data &data, Renderer9 *renderer)
    : FramebufferD3D(data),
      mRenderer(renderer)
{
    ASSERT(mRenderer != nullptr);
}

Framebuffer9::~Framebuffer9()
{
}

gl::Error Framebuffer9::discard(size_t, const GLenum *)
{
    // Extension not implemented in D3D9 renderer
    UNREACHABLE();
    return gl::Error(GL_NO_ERROR);
}

gl::Error Framebuffer9::invalidate(size_t, const GLenum *)
{
    // Shouldn't ever reach here in D3D9
    UNREACHABLE();
    return gl::Error(GL_NO_ERROR);
}

gl::Error Framebuffer9::invalidateSub(size_t, const GLenum *, const gl::Rectangle &)
{
    // Shouldn't ever reach here in D3D9
    UNREACHABLE();
    return gl::Error(GL_NO_ERROR);
}

gl::Error Framebuffer9::clear(const gl::State &state, const ClearParameters &clearParams)
{
    const gl::FramebufferAttachment *colorAttachment = mData.getColorAttachment(0);
    const gl::FramebufferAttachment *depthStencilAttachment = mData.getDepthOrStencilAttachment();

    gl::Error error = mRenderer->applyRenderTarget(colorAttachment, depthStencilAttachment);
    if (error.isError())
    {
        return error;
    }

    float nearZ = state.getNearPlane();
    float farZ = state.getFarPlane();
    mRenderer->setViewport(state.getViewport(), nearZ, farZ, GL_TRIANGLES, state.getRasterizerState().frontFace, true);

    mRenderer->setScissorRectangle(state.getScissor(), state.isScissorTestEnabled());

    return mRenderer->clear(clearParams, colorAttachment, depthStencilAttachment);
}

gl::Error Framebuffer9::readPixels(const gl::Rectangle &area, GLenum format, GLenum type, size_t outputPitch, const gl::PixelPackState &pack, uint8_t *pixels) const
{
    ASSERT(pack.pixelBuffer.get() == nullptr);

    const gl::FramebufferAttachment *colorbuffer = mData.getColorAttachment(0);
    ASSERT(colorbuffer);

    RenderTarget9 *renderTarget = nullptr;
    gl::Error error = colorbuffer->getRenderTarget(&renderTarget);
    if (error.isError())
    {
        return error;
    }
    ASSERT(renderTarget);

    IDirect3DSurface9 *surface = renderTarget->getSurface();
    ASSERT(surface);

    D3DSURFACE_DESC desc;
    surface->GetDesc(&desc);

    if (desc.MultiSampleType != D3DMULTISAMPLE_NONE)
    {
        UNIMPLEMENTED();   // FIXME: Requires resolve using StretchRect into non-multisampled render target
        SafeRelease(surface);
        return gl::Error(GL_OUT_OF_MEMORY, "ReadPixels is unimplemented for multisampled framebuffer attachments.");
    }

    IDirect3DDevice9 *device = mRenderer->getDevice();
    ASSERT(device);

    HRESULT result;
    IDirect3DSurface9 *systemSurface = nullptr;
    bool directToPixels = !pack.reverseRowOrder && pack.alignment <= 4 && mRenderer->getShareHandleSupport() &&
                          area.x == 0 && area.y == 0 &&
                          static_cast<UINT>(area.width) == desc.Width && static_cast<UINT>(area.height) == desc.Height &&
                          desc.Format == D3DFMT_A8R8G8B8 && format == GL_BGRA_EXT && type == GL_UNSIGNED_BYTE;
    if (directToPixels)
    {
        // Use the pixels ptr as a shared handle to write directly into client's memory
        result = device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                     D3DPOOL_SYSTEMMEM, &systemSurface, reinterpret_cast<void**>(&pixels));
        if (FAILED(result))
        {
            // Try again without the shared handle
            directToPixels = false;
        }
    }

    if (!directToPixels)
    {
        result = device->CreateOffscreenPlainSurface(desc.Width, desc.Height, desc.Format,
                                                     D3DPOOL_SYSTEMMEM, &systemSurface, nullptr);
        if (FAILED(result))
        {
            ASSERT(result == D3DERR_OUTOFVIDEOMEMORY || result == E_OUTOFMEMORY);
            SafeRelease(surface);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal texture for ReadPixels.");
        }
    }

    result = device->GetRenderTargetData(surface, systemSurface);
    SafeRelease(surface);

    if (FAILED(result))
    {
        SafeRelease(systemSurface);

        // It turns out that D3D will sometimes produce more error
        // codes than those documented.
        if (d3d9::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
        }
        else
        {
            UNREACHABLE();
        }

        return gl::Error(GL_OUT_OF_MEMORY, "Failed to read internal render target data.");
    }

    if (directToPixels)
    {
        SafeRelease(systemSurface);
        return gl::Error(GL_NO_ERROR);
    }

    RECT rect;
    rect.left = gl::clamp(area.x, 0L, static_cast<LONG>(desc.Width));
    rect.top = gl::clamp(area.y, 0L, static_cast<LONG>(desc.Height));
    rect.right = gl::clamp(area.x + area.width, 0L, static_cast<LONG>(desc.Width));
    rect.bottom = gl::clamp(area.y + area.height, 0L, static_cast<LONG>(desc.Height));

    D3DLOCKED_RECT lock;
    result = systemSurface->LockRect(&lock, &rect, D3DLOCK_READONLY);

    if (FAILED(result))
    {
        UNREACHABLE();
        SafeRelease(systemSurface);

        return gl::Error(GL_OUT_OF_MEMORY, "Failed to lock internal render target.");
    }

    uint8_t *source;
    int inputPitch;
    if (pack.reverseRowOrder)
    {
        source = reinterpret_cast<uint8_t*>(lock.pBits) + lock.Pitch * (rect.bottom - rect.top - 1);
        inputPitch = -lock.Pitch;
    }
    else
    {
        source = reinterpret_cast<uint8_t*>(lock.pBits);
        inputPitch = lock.Pitch;
    }

    const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(desc.Format);
    const gl::InternalFormat &sourceFormatInfo = gl::GetInternalFormatInfo(d3dFormatInfo.internalFormat);
    if (sourceFormatInfo.format == format && sourceFormatInfo.type == type)
    {
        // Direct copy possible
        for (int y = 0; y < rect.bottom - rect.top; y++)
        {
            memcpy(pixels + y * outputPitch, source + y * inputPitch, (rect.right - rect.left) * sourceFormatInfo.pixelBytes);
        }
    }
    else
    {
        const d3d9::D3DFormat &sourceD3DFormatInfo = d3d9::GetD3DFormatInfo(desc.Format);
        ColorCopyFunction fastCopyFunc = sourceD3DFormatInfo.getFastCopyFunction(format, type);

        GLenum sizedDestInternalFormat = gl::GetSizedInternalFormat(format, type);
        const gl::InternalFormat &destFormatInfo = gl::GetInternalFormatInfo(sizedDestInternalFormat);

        if (fastCopyFunc)
        {
            // Fast copy is possible through some special function
            for (int y = 0; y < rect.bottom - rect.top; y++)
            {
                for (int x = 0; x < rect.right - rect.left; x++)
                {
                    uint8_t *dest = pixels + y * outputPitch + x * destFormatInfo.pixelBytes;
                    const uint8_t *src = source + y * inputPitch + x * sourceFormatInfo.pixelBytes;

                    fastCopyFunc(src, dest);
                }
            }
        }
        else
        {
            ColorReadFunction colorReadFunction = sourceD3DFormatInfo.colorReadFunction;
            ColorWriteFunction colorWriteFunction = GetColorWriteFunction(format, type);

            uint8_t temp[sizeof(gl::ColorF)];
            for (int y = 0; y < rect.bottom - rect.top; y++)
            {
                for (int x = 0; x < rect.right - rect.left; x++)
                {
                    uint8_t *dest = pixels + y * outputPitch + x * destFormatInfo.pixelBytes;
                    const uint8_t *src = source + y * inputPitch + x * sourceFormatInfo.pixelBytes;

                    // readFunc and writeFunc will be using the same type of color, CopyTexImage
                    // will not allow the copy otherwise.
                    colorReadFunction(src, temp);
                    colorWriteFunction(temp, dest);
                }
            }
        }
    }

    systemSurface->UnlockRect();
    SafeRelease(systemSurface);

    return gl::Error(GL_NO_ERROR);
}

gl::Error Framebuffer9::blit(const gl::Rectangle &sourceArea, const gl::Rectangle &destArea, const gl::Rectangle *scissor,
                             bool blitRenderTarget, bool blitDepth, bool blitStencil, GLenum filter,
                             const gl::Framebuffer *sourceFramebuffer)
{
    ASSERT(filter == GL_NEAREST);

    IDirect3DDevice9 *device = mRenderer->getDevice();
    ASSERT(device);

    mRenderer->endScene();

    if (blitRenderTarget)
    {
        const gl::FramebufferAttachment *readBuffer = sourceFramebuffer->getColorbuffer(0);
        ASSERT(readBuffer);

        RenderTarget9 *readRenderTarget = nullptr;
        gl::Error error = readBuffer->getRenderTarget(&readRenderTarget);
        if (error.isError())
        {
            return error;
        }
        ASSERT(readRenderTarget);

        const gl::FramebufferAttachment *drawBuffer = mData.getColorAttachment(0);
        ASSERT(drawBuffer);

        RenderTarget9 *drawRenderTarget = nullptr;
        error = drawBuffer->getRenderTarget(&drawRenderTarget);
        if (error.isError())
        {
            return error;
        }
        ASSERT(drawRenderTarget);

        // The getSurface calls do an AddRef so save them until after no errors are possible
        IDirect3DSurface9* readSurface = readRenderTarget->getSurface();
        ASSERT(readSurface);

        IDirect3DSurface9* drawSurface = drawRenderTarget->getSurface();
        ASSERT(drawSurface);

        gl::Extents srcSize(readRenderTarget->getWidth(), readRenderTarget->getHeight(), 1);
        gl::Extents dstSize(drawRenderTarget->getWidth(), drawRenderTarget->getHeight(), 1);

        RECT srcRect;
        srcRect.left = sourceArea.x;
        srcRect.right = sourceArea.x + sourceArea.width;
        srcRect.top = sourceArea.y;
        srcRect.bottom = sourceArea.y + sourceArea.height;

        RECT dstRect;
        dstRect.left = destArea.x;
        dstRect.right = destArea.x + destArea.width;
        dstRect.top = destArea.y;
        dstRect.bottom = destArea.y + destArea.height;

        // Clip the rectangles to the scissor rectangle
        if (scissor)
        {
            if (dstRect.left < scissor->x)
            {
                srcRect.left += (scissor->x - dstRect.left);
                dstRect.left = scissor->x;
            }
            if (dstRect.top < scissor->y)
            {
                srcRect.top += (scissor->y - dstRect.top);
                dstRect.top = scissor->y;
            }
            if (dstRect.right > scissor->x + scissor->width)
            {
                srcRect.right -= (dstRect.right - (scissor->x + scissor->width));
                dstRect.right = scissor->x + scissor->width;
            }
            if (dstRect.bottom > scissor->y + scissor->height)
            {
                srcRect.bottom -= (dstRect.bottom - (scissor->y + scissor->height));
                dstRect.bottom = scissor->y + scissor->height;
            }
        }

        // Clip the rectangles to the destination size
        if (dstRect.left < 0)
        {
            srcRect.left += -dstRect.left;
            dstRect.left = 0;
        }
        if (dstRect.right > dstSize.width)
        {
            srcRect.right -= (dstRect.right - dstSize.width);
            dstRect.right = dstSize.width;
        }
        if (dstRect.top < 0)
        {
            srcRect.top += -dstRect.top;
            dstRect.top = 0;
        }
        if (dstRect.bottom > dstSize.height)
        {
            srcRect.bottom -= (dstRect.bottom - dstSize.height);
            dstRect.bottom = dstSize.height;
        }

        // Clip the rectangles to the source size
        if (srcRect.left < 0)
        {
            dstRect.left += -srcRect.left;
            srcRect.left = 0;
        }
        if (srcRect.right > srcSize.width)
        {
            dstRect.right -= (srcRect.right - srcSize.width);
            srcRect.right = srcSize.width;
        }
        if (srcRect.top < 0)
        {
            dstRect.top += -srcRect.top;
            srcRect.top = 0;
        }
        if (srcRect.bottom > srcSize.height)
        {
            dstRect.bottom -= (srcRect.bottom - srcSize.height);
            srcRect.bottom = srcSize.height;
        }

        HRESULT result = device->StretchRect(readSurface, &srcRect, drawSurface, &dstRect, D3DTEXF_NONE);

        SafeRelease(readSurface);
        SafeRelease(drawSurface);

        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Internal blit failed, StretchRect returned 0x%X.", result);
        }
    }

    if (blitDepth || blitStencil)
    {
        const gl::FramebufferAttachment *readBuffer = sourceFramebuffer->getDepthOrStencilbuffer();
        ASSERT(readBuffer);

        RenderTarget9 *readDepthStencil = nullptr;
        gl::Error error = readBuffer->getRenderTarget(&readDepthStencil);
        if (error.isError())
        {
            return error;
        }
        ASSERT(readDepthStencil);

        const gl::FramebufferAttachment *drawBuffer = mData.getDepthOrStencilAttachment();
        ASSERT(drawBuffer);

        RenderTarget9 *drawDepthStencil = nullptr;
        error = drawBuffer->getRenderTarget(&drawDepthStencil);
        if (error.isError())
        {
            return error;
        }
        ASSERT(drawDepthStencil);

        // The getSurface calls do an AddRef so save them until after no errors are possible
        IDirect3DSurface9* readSurface = readDepthStencil->getSurface();
        ASSERT(readDepthStencil);

        IDirect3DSurface9* drawSurface = drawDepthStencil->getSurface();
        ASSERT(drawDepthStencil);

        HRESULT result = device->StretchRect(readSurface, nullptr, drawSurface, nullptr, D3DTEXF_NONE);

        SafeRelease(readSurface);
        SafeRelease(drawSurface);

        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Internal blit failed, StretchRect returned 0x%X.", result);
        }
    }

    return gl::Error(GL_NO_ERROR);
}

GLenum Framebuffer9::getRenderTargetImplementationFormat(RenderTargetD3D *renderTarget) const
{
    RenderTarget9 *renderTarget9 = GetAs<RenderTarget9>(renderTarget);
    const d3d9::D3DFormat &d3dFormatInfo = d3d9::GetD3DFormatInfo(renderTarget9->getD3DFormat());
    return d3dFormatInfo.internalFormat;
}

}
