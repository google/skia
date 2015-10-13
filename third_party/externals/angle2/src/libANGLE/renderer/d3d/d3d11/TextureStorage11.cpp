//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureStorage11.cpp: Implements the abstract rx::TextureStorage11 class and its concrete derived
// classes TextureStorage11_2D and TextureStorage11_Cube, which act as the interface to the D3D11 texture.

#include "libANGLE/renderer/d3d/d3d11/TextureStorage11.h"

#include <tuple>

#include "common/MemoryBuffer.h"
#include "common/utilities.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/renderer/d3d/d3d11/Blit11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/Image11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/SwapChain11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/TextureD3D.h"

namespace rx
{

TextureStorage11::SwizzleCacheValue::SwizzleCacheValue()
    : swizzleRed(GL_NONE), swizzleGreen(GL_NONE), swizzleBlue(GL_NONE), swizzleAlpha(GL_NONE)
{
}

TextureStorage11::SwizzleCacheValue::SwizzleCacheValue(GLenum red, GLenum green, GLenum blue, GLenum alpha)
    : swizzleRed(red), swizzleGreen(green), swizzleBlue(blue), swizzleAlpha(alpha)
{
}

bool TextureStorage11::SwizzleCacheValue::operator==(const SwizzleCacheValue &other) const
{
    return swizzleRed == other.swizzleRed &&
           swizzleGreen == other.swizzleGreen &&
           swizzleBlue == other.swizzleBlue &&
           swizzleAlpha == other.swizzleAlpha;
}

bool TextureStorage11::SwizzleCacheValue::operator!=(const SwizzleCacheValue &other) const
{
    return !(*this == other);
}

TextureStorage11::SRVKey::SRVKey(int baseLevel, int mipLevels, bool swizzle)
    : baseLevel(baseLevel), mipLevels(mipLevels), swizzle(swizzle)
{
}

bool TextureStorage11::SRVKey::operator<(const SRVKey &rhs) const
{
    return std::tie(baseLevel, mipLevels, swizzle) < std::tie(rhs.baseLevel, rhs.mipLevels, rhs.swizzle);
}

TextureStorage11::TextureStorage11(Renderer11 *renderer, UINT bindFlags, UINT miscFlags)
    : mRenderer(renderer),
      mTopLevel(0),
      mMipLevels(0),
      mInternalFormat(GL_NONE),
      mTextureFormat(DXGI_FORMAT_UNKNOWN),
      mShaderResourceFormat(DXGI_FORMAT_UNKNOWN),
      mRenderTargetFormat(DXGI_FORMAT_UNKNOWN),
      mDepthStencilFormat(DXGI_FORMAT_UNKNOWN),
      mTextureWidth(0),
      mTextureHeight(0),
      mTextureDepth(0),
      mBindFlags(bindFlags),
      mMiscFlags(miscFlags)
{
    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mLevelSRVs[i] = nullptr;
    }
}

TextureStorage11::~TextureStorage11()
{
    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        SafeRelease(mLevelSRVs[level]);
    }

    for (SRVCache::iterator i = mSrvCache.begin(); i != mSrvCache.end(); i++)
    {
        SafeRelease(i->second);
    }
    mSrvCache.clear();
}

DWORD TextureStorage11::GetTextureBindFlags(GLenum internalFormat, const Renderer11DeviceCaps &renderer11DeviceCaps, bool renderTarget)
{
    UINT bindFlags = 0;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalFormat, renderer11DeviceCaps);
    if (formatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
    {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (formatInfo.dsvFormat != DXGI_FORMAT_UNKNOWN)
    {
        bindFlags |= D3D11_BIND_DEPTH_STENCIL;
    }
    if (formatInfo.rtvFormat != DXGI_FORMAT_UNKNOWN && renderTarget)
    {
        bindFlags |= D3D11_BIND_RENDER_TARGET;
    }

    return bindFlags;
}

DWORD TextureStorage11::GetTextureMiscFlags(GLenum internalFormat, const Renderer11DeviceCaps &renderer11DeviceCaps, bool renderTarget, int levels)
{
    UINT miscFlags = 0;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalFormat, renderer11DeviceCaps);
    if (renderTarget && levels > 1)
    {
        const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(formatInfo.texFormat);

        if (dxgiFormatInfo.nativeMipmapSupport(renderer11DeviceCaps.featureLevel))
        {
            miscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
        }
    }

    return miscFlags;
}

UINT TextureStorage11::getBindFlags() const
{
    return mBindFlags;
}

UINT TextureStorage11::getMiscFlags() const
{
    return mMiscFlags;
}

int TextureStorage11::getTopLevel() const
{
    return mTopLevel;
}

bool TextureStorage11::isRenderTarget() const
{
    return (mBindFlags & (D3D11_BIND_RENDER_TARGET | D3D11_BIND_DEPTH_STENCIL)) != 0;
}

bool TextureStorage11::isManaged() const
{
    return false;
}

bool TextureStorage11::supportsNativeMipmapFunction() const
{
    return (mMiscFlags & D3D11_RESOURCE_MISC_GENERATE_MIPS) != 0;
}

int TextureStorage11::getLevelCount() const
{
    return mMipLevels - mTopLevel;
}

int TextureStorage11::getLevelWidth(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureWidth) >> mipLevel, 1);
}

int TextureStorage11::getLevelHeight(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureHeight) >> mipLevel, 1);
}

int TextureStorage11::getLevelDepth(int mipLevel) const
{
    return std::max(static_cast<int>(mTextureDepth) >> mipLevel, 1);
}

UINT TextureStorage11::getSubresourceIndex(const gl::ImageIndex &index) const
{
    UINT mipSlice = static_cast<UINT>(index.mipIndex + mTopLevel);
    UINT arraySlice = static_cast<UINT>(index.hasLayer() ? index.layerIndex : 0);
    UINT subresource = D3D11CalcSubresource(mipSlice, arraySlice, mMipLevels);
    ASSERT(subresource != std::numeric_limits<UINT>::max());
    return subresource;
}

gl::Error TextureStorage11::getSRV(const gl::TextureState &textureState,
                                   ID3D11ShaderResourceView **outSRV)
{
    bool swizzleRequired   = textureState.swizzleRequired();
    bool mipmapping        = gl::IsMipmapFiltered(textureState.samplerState);
    unsigned int mipLevels = mipmapping ? (textureState.maxLevel - textureState.baseLevel + 1) : 1;

    // Make sure there's 'mipLevels' mipmap levels below the base level (offset by the top level, which corresponds to GL level 0)
    mipLevels = std::min(mipLevels, mMipLevels - mTopLevel - textureState.baseLevel);

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        ASSERT(!swizzleRequired);
        ASSERT(mipLevels == 1 || mipLevels == mMipLevels);
    }

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        // We must ensure that the level zero texture is in sync with mipped texture.
        gl::Error error = useLevelZeroWorkaroundTexture(mipLevels == 1);
        if (error.isError())
        {
            return error;
        }
    }

    if (swizzleRequired)
    {
        verifySwizzleExists(textureState.swizzleRed, textureState.swizzleGreen,
                            textureState.swizzleBlue, textureState.swizzleAlpha);
    }

    SRVKey key(textureState.baseLevel, mipLevels, swizzleRequired);
    auto iter = mSrvCache.find(key);
    if (iter != mSrvCache.end())
    {
        *outSRV = iter->second;
        return gl::Error(GL_NO_ERROR);
    }

    ID3D11Resource *texture = nullptr;
    if (swizzleRequired)
    {
        gl::Error error = getSwizzleTexture(&texture);
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        gl::Error error = getResource(&texture);
        if (error.isError())
        {
            return error;
        }
    }

    ID3D11ShaderResourceView *srv = nullptr;
    DXGI_FORMAT format = (swizzleRequired ? mSwizzleShaderResourceFormat : mShaderResourceFormat);
    gl::Error error = createSRV(textureState.baseLevel, mipLevels, format, texture, &srv);
    if (error.isError())
    {
        return error;
    }

    mSrvCache.insert(std::make_pair(key, srv));
    *outSRV = srv;

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11::getSRVLevel(int mipLevel, ID3D11ShaderResourceView **outSRV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    if (!mLevelSRVs[mipLevel])
    {
        ID3D11Resource *resource = NULL;
        gl::Error error = getResource(&resource);
        if (error.isError())
        {
            return error;
        }

        error = createSRV(mipLevel, 1, mShaderResourceFormat, resource, &mLevelSRVs[mipLevel]);
        if (error.isError())
        {
            return error;
        }
    }

    *outSRV = mLevelSRVs[mipLevel];

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11::getSRVLevels(GLint baseLevel, GLint maxLevel, ID3D11ShaderResourceView **outSRV)
{
    unsigned int mipLevels = maxLevel - baseLevel + 1;

    // Make sure there's 'mipLevels' mipmap levels below the base level (offset by the top level, which corresponds to GL level 0)
    mipLevels = std::min(mipLevels, mMipLevels - mTopLevel - baseLevel);

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        ASSERT(mipLevels == 1 || mipLevels == mMipLevels);
    }

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        // We must ensure that the level zero texture is in sync with mipped texture.
        gl::Error error = useLevelZeroWorkaroundTexture(mipLevels == 1);
        if (error.isError())
        {
            return error;
        }
    }

    SRVKey key(baseLevel, mipLevels, false);
    auto iter = mSrvCache.find(key);
    if (iter != mSrvCache.end())
    {
        *outSRV = iter->second;
        return gl::Error(GL_NO_ERROR);
    }

    ID3D11Resource *texture = nullptr;
    gl::Error error = getResource(&texture);
    if (error.isError())
    {
        return error;
    }

    ID3D11ShaderResourceView *srv = nullptr;
    error = createSRV(baseLevel, mipLevels, mShaderResourceFormat, texture, &srv);
    if (error.isError())
    {
        return error;
    }

    mSrvCache[key] = srv;
    *outSRV = srv;

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11::generateSwizzles(GLenum swizzleRed, GLenum swizzleGreen, GLenum swizzleBlue, GLenum swizzleAlpha)
{
    SwizzleCacheValue swizzleTarget(swizzleRed, swizzleGreen, swizzleBlue, swizzleAlpha);
    for (int level = 0; level < getLevelCount(); level++)
    {
        // Check if the swizzle for this level is out of date
        if (mSwizzleCache[level] != swizzleTarget)
        {
            // Need to re-render the swizzle for this level
            ID3D11ShaderResourceView *sourceSRV = NULL;
            gl::Error error = getSRVLevel(level, &sourceSRV);
            if (error.isError())
            {
                return error;
            }

            ID3D11RenderTargetView *destRTV = NULL;
            error = getSwizzleRenderTarget(level, &destRTV);
            if (error.isError())
            {
                return error;
            }

            gl::Extents size(getLevelWidth(level), getLevelHeight(level), getLevelDepth(level));

            Blit11 *blitter = mRenderer->getBlitter();

            error = blitter->swizzleTexture(sourceSRV, destRTV, size, swizzleRed, swizzleGreen, swizzleBlue, swizzleAlpha);
            if (error.isError())
            {
                return error;
            }

            mSwizzleCache[level] = swizzleTarget;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureStorage11::invalidateSwizzleCacheLevel(int mipLevel)
{
    if (mipLevel >= 0 && static_cast<unsigned int>(mipLevel) < ArraySize(mSwizzleCache))
    {
        // The default constructor of SwizzleCacheValue has GL_NONE for all channels which is not a
        // valid swizzle combination
        mSwizzleCache[mipLevel] = SwizzleCacheValue();
    }
}

void TextureStorage11::invalidateSwizzleCache()
{
    for (unsigned int mipLevel = 0; mipLevel < ArraySize(mSwizzleCache); mipLevel++)
    {
        invalidateSwizzleCacheLevel(mipLevel);
    }
}

gl::Error TextureStorage11::updateSubresourceLevel(ID3D11Resource *srcTexture, unsigned int sourceSubresource,
                                                   const gl::ImageIndex &index, const gl::Box &copyArea)
{
    ASSERT(srcTexture);

    GLint level = index.mipIndex;

    invalidateSwizzleCacheLevel(level);

    gl::Extents texSize(getLevelWidth(level), getLevelHeight(level), getLevelDepth(level));

    bool fullCopy = copyArea.x == 0 &&
                    copyArea.y == 0 &&
                    copyArea.z == 0 &&
                    copyArea.width  == texSize.width &&
                    copyArea.height == texSize.height &&
                    copyArea.depth  == texSize.depth;

    ID3D11Resource *dstTexture = NULL;
    gl::Error error(GL_NO_ERROR);

    // If the zero-LOD workaround is active and we want to update a level greater than zero, then we should
    // update the mipmapped texture, even if mapmaps are currently disabled.
    if (index.mipIndex > 0 && mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        error = getMippedResource(&dstTexture);
    }
    else
    {
        error = getResource(&dstTexture);
    }

    if (error.isError())
    {
        return error;
    }

    unsigned int dstSubresource = getSubresourceIndex(index);

    ASSERT(dstTexture);

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mTextureFormat);
    if (!fullCopy && (dxgiFormatInfo.depthBits > 0 || dxgiFormatInfo.stencilBits > 0))
    {
        // CopySubresourceRegion cannot copy partial depth stencils, use the blitter instead
        Blit11 *blitter = mRenderer->getBlitter();

        return blitter->copyDepthStencil(srcTexture, sourceSubresource, copyArea, texSize,
                                         dstTexture, dstSubresource, copyArea, texSize,
                                         NULL);
    }
    else
    {
        D3D11_BOX srcBox;
        srcBox.left = copyArea.x;
        srcBox.top = copyArea.y;
        srcBox.right = copyArea.x + roundUp(static_cast<UINT>(copyArea.width), dxgiFormatInfo.blockWidth);
        srcBox.bottom = copyArea.y + roundUp(static_cast<UINT>(copyArea.height), dxgiFormatInfo.blockHeight);
        srcBox.front = copyArea.z;
        srcBox.back = copyArea.z + copyArea.depth;

        ID3D11DeviceContext *context = mRenderer->getDeviceContext();

        context->CopySubresourceRegion(dstTexture, dstSubresource, copyArea.x, copyArea.y, copyArea.z,
                                       srcTexture, sourceSubresource, fullCopy ? NULL : &srcBox);
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error TextureStorage11::copySubresourceLevel(ID3D11Resource* dstTexture, unsigned int dstSubresource,
                                                 const gl::ImageIndex &index, const gl::Box &region)
{
    ASSERT(dstTexture);

    ID3D11Resource *srcTexture = NULL;
    gl::Error error(GL_NO_ERROR);

    // If the zero-LOD workaround is active and we want to update a level greater than zero, then we should
    // update the mipmapped texture, even if mapmaps are currently disabled.
    if (index.mipIndex > 0 && mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        error = getMippedResource(&srcTexture);
    }
    else
    {
        error = getResource(&srcTexture);
    }

    if (error.isError())
    {
        return error;
    }

    ASSERT(srcTexture);

    unsigned int srcSubresource = getSubresourceIndex(index);

    ID3D11DeviceContext *context = mRenderer->getDeviceContext();

    // D3D11 can't perform partial CopySubresourceRegion on depth/stencil textures, so pSrcBox should be NULL.
    D3D11_BOX srcBox;
    D3D11_BOX *pSrcBox = NULL;
    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        // However, D3D10Level9 doesn't always perform CopySubresourceRegion correctly unless the source box
        // is specified. This is okay, since we don't perform CopySubresourceRegion on depth/stencil
        // textures on 9_3.
        ASSERT(d3d11::GetDXGIFormatInfo(mTextureFormat).depthBits == 0);
        ASSERT(d3d11::GetDXGIFormatInfo(mTextureFormat).stencilBits == 0);
        srcBox.left = region.x;
        srcBox.right = region.x + region.width;
        srcBox.top = region.y;
        srcBox.bottom = region.y + region.height;
        srcBox.front = region.z;
        srcBox.back = region.z + region.depth;
        pSrcBox = &srcBox;
    }

    context->CopySubresourceRegion(dstTexture, dstSubresource, region.x, region.y, region.z,
                                   srcTexture, srcSubresource, pSrcBox);

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11::generateMipmap(const gl::ImageIndex &sourceIndex, const gl::ImageIndex &destIndex)
{
    ASSERT(sourceIndex.layerIndex == destIndex.layerIndex);

    invalidateSwizzleCacheLevel(destIndex.mipIndex);

    RenderTargetD3D *source = NULL;
    gl::Error error = getRenderTarget(sourceIndex, &source);
    if (error.isError())
    {
        return error;
    }

    RenderTargetD3D *dest = NULL;
    error = getRenderTarget(destIndex, &dest);
    if (error.isError())
    {
        return error;
    }

    ID3D11ShaderResourceView *sourceSRV = GetAs<RenderTarget11>(source)->getShaderResourceView();
    ID3D11RenderTargetView *destRTV = GetAs<RenderTarget11>(dest)->getRenderTargetView();

    gl::Box sourceArea(0, 0, 0, source->getWidth(), source->getHeight(), source->getDepth());
    gl::Extents sourceSize(source->getWidth(), source->getHeight(), source->getDepth());

    gl::Box destArea(0, 0, 0, dest->getWidth(), dest->getHeight(), dest->getDepth());
    gl::Extents destSize(dest->getWidth(), dest->getHeight(), dest->getDepth());

    Blit11 *blitter = mRenderer->getBlitter();
    return blitter->copyTexture(sourceSRV, sourceArea, sourceSize, destRTV, destArea, destSize, NULL,
                                gl::GetInternalFormatInfo(source->getInternalFormat()).format, GL_LINEAR);
}

void TextureStorage11::verifySwizzleExists(GLenum swizzleRed, GLenum swizzleGreen, GLenum swizzleBlue, GLenum swizzleAlpha)
{
    SwizzleCacheValue swizzleTarget(swizzleRed, swizzleGreen, swizzleBlue, swizzleAlpha);
    for (unsigned int level = 0; level < mMipLevels; level++)
    {
        ASSERT(mSwizzleCache[level] == swizzleTarget);
    }
}

void TextureStorage11::clearSRVCache()
{
    invalidateSwizzleCache();

    auto iter = mSrvCache.begin();
    while (iter != mSrvCache.end())
    {
        if (!iter->first.swizzle)
        {
            SafeRelease(iter->second);
            iter = mSrvCache.erase(iter);
        }
        else
        {
            iter++;
        }
    }

    for (size_t level = 0; level < ArraySize(mLevelSRVs); level++)
    {
        SafeRelease(mLevelSRVs[level]);
    }
}

gl::Error TextureStorage11::copyToStorage(TextureStorage *destStorage)
{
    ASSERT(destStorage);

    ID3D11Resource *sourceResouce = NULL;
    gl::Error error = getResource(&sourceResouce);
    if (error.isError())
    {
        return error;
    }

    TextureStorage11 *dest11 = GetAs<TextureStorage11>(destStorage);
    ID3D11Resource *destResource = NULL;
    error = dest11->getResource(&destResource);
    if (error.isError())
    {
        return error;
    }

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
    immediateContext->CopyResource(destResource, sourceResouce);

    dest11->invalidateSwizzleCache();

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11::setData(const gl::ImageIndex &index, ImageD3D *image, const gl::Box *destBox, GLenum type,
                                    const gl::PixelUnpackState &unpack, const uint8_t *pixelData)
{
    ASSERT(!image->isDirty());

    ID3D11Resource *resource = NULL;
    gl::Error error = getResource(&resource);
    if (error.isError())
    {
        return error;
    }
    ASSERT(resource);

    UINT destSubresource = getSubresourceIndex(index);

    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(image->getInternalFormat());

    gl::Box levelBox(0, 0, 0, getLevelWidth(index.mipIndex), getLevelHeight(index.mipIndex), getLevelDepth(index.mipIndex));
    bool fullUpdate = (destBox == NULL || *destBox == levelBox);
    ASSERT(internalFormatInfo.depthBits == 0 || fullUpdate);

    // TODO(jmadill): Handle compressed formats
    // Compressed formats have different load syntax, so we'll have to handle them with slightly
    // different logic. Will implemnent this in a follow-up patch, and ensure we do not use SetData
    // with compressed formats in the calling logic.
    ASSERT(!internalFormatInfo.compressed);

    int width = destBox ? destBox->width : static_cast<int>(image->getWidth());
    int height = destBox ? destBox->height : static_cast<int>(image->getHeight());
    int depth = destBox ? destBox->depth : static_cast<int>(image->getDepth());
    UINT srcRowPitch = internalFormatInfo.computeRowPitch(type, width, unpack.alignment, unpack.rowLength);
    UINT srcDepthPitch = internalFormatInfo.computeDepthPitch(type, width, height, unpack.alignment, unpack.rowLength);

    const d3d11::TextureFormat &d3d11Format = d3d11::GetTextureFormatInfo(image->getInternalFormat(), mRenderer->getRenderer11DeviceCaps());
    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(d3d11Format.texFormat);

    size_t outputPixelSize = dxgiFormatInfo.pixelBytes;

    UINT bufferRowPitch   = static_cast<unsigned int>(outputPixelSize) * width;
    UINT bufferDepthPitch = bufferRowPitch * height;

    size_t neededSize = bufferDepthPitch * depth;
    MemoryBuffer *conversionBuffer = NULL;
    error = mRenderer->getScratchMemoryBuffer(neededSize, &conversionBuffer);
    if (error.isError())
    {
        return error;
    }

    // TODO: fast path
    LoadImageFunction loadFunction = d3d11Format.loadFunctions.at(type);
    loadFunction(width, height, depth,
                 pixelData, srcRowPitch, srcDepthPitch,
                 conversionBuffer->data(), bufferRowPitch, bufferDepthPitch);

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

    if (!fullUpdate)
    {
        ASSERT(destBox);

        D3D11_BOX destD3DBox;
        destD3DBox.left = destBox->x;
        destD3DBox.right = destBox->x + destBox->width;
        destD3DBox.top = destBox->y;
        destD3DBox.bottom = destBox->y + destBox->height;
        destD3DBox.front = destBox->z;
        destD3DBox.back = destBox->z + destBox->depth;

        immediateContext->UpdateSubresource(resource, destSubresource,
                                            &destD3DBox, conversionBuffer->data(),
                                            bufferRowPitch, bufferDepthPitch);
    }
    else
    {
        immediateContext->UpdateSubresource(resource, destSubresource,
                                            NULL, conversionBuffer->data(),
                                            bufferRowPitch, bufferDepthPitch);
    }

    return gl::Error(GL_NO_ERROR);
}

TextureStorage11_2D::TextureStorage11_2D(Renderer11 *renderer, SwapChain11 *swapchain)
    : TextureStorage11(renderer, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0),
      mTexture(swapchain->getOffscreenTexture()),
      mLevelZeroTexture(NULL),
      mLevelZeroRenderTarget(NULL),
      mUseLevelZeroTexture(false),
      mSwizzleTexture(NULL)
{
    mTexture->AddRef();

    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i] = NULL;
        mRenderTarget[i] = NULL;
        mSwizzleRenderTargets[i] = NULL;
    }

    D3D11_TEXTURE2D_DESC texDesc;
    mTexture->GetDesc(&texDesc);
    mMipLevels = texDesc.MipLevels;
    mTextureFormat = texDesc.Format;
    mTextureWidth = texDesc.Width;
    mTextureHeight = texDesc.Height;
    mTextureDepth = 1;

    mInternalFormat = swapchain->GetRenderTargetInternalFormat();

    ID3D11ShaderResourceView *srv = swapchain->getRenderTargetShaderResource();
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srv->GetDesc(&srvDesc);
    mShaderResourceFormat = srvDesc.Format;

    ID3D11RenderTargetView* offscreenRTV = swapchain->getRenderTarget();
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    offscreenRTV->GetDesc(&rtvDesc);
    mRenderTargetFormat = rtvDesc.Format;

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mTextureFormat);
    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(dxgiFormatInfo.internalFormat, mRenderer->getRenderer11DeviceCaps());
    mSwizzleTextureFormat = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat = formatInfo.swizzleRTVFormat;

    mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
}

TextureStorage11_2D::TextureStorage11_2D(Renderer11 *renderer, GLenum internalformat, bool renderTarget, GLsizei width, GLsizei height, int levels, bool hintLevelZeroOnly)
    : TextureStorage11(renderer,
                       GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget),
                       GetTextureMiscFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget, levels)),
      mTexture(NULL),
      mLevelZeroTexture(NULL),
      mLevelZeroRenderTarget(NULL),
      mUseLevelZeroTexture(false),
      mSwizzleTexture(NULL)
{
    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i] = NULL;
        mRenderTarget[i] = NULL;
        mSwizzleRenderTargets[i] = NULL;
    }

    mInternalFormat = internalformat;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalformat, renderer->getRenderer11DeviceCaps());
    mTextureFormat = formatInfo.texFormat;
    mShaderResourceFormat = formatInfo.srvFormat;
    mDepthStencilFormat = formatInfo.dsvFormat;
    mRenderTargetFormat = formatInfo.rtvFormat;
    mSwizzleTextureFormat = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat = formatInfo.swizzleRTVFormat;

    d3d11::MakeValidSize(false, mTextureFormat, &width, &height, &mTopLevel);
    mMipLevels = mTopLevel + levels;
    mTextureWidth = width;
    mTextureHeight = height;
    mTextureDepth = 1;

    if (hintLevelZeroOnly && levels > 1)
    {
        //The LevelZeroOnly hint should only be true if the zero max LOD workaround is active.
        ASSERT(mRenderer->getWorkarounds().zeroMaxLodWorkaround);
        mUseLevelZeroTexture = true;
    }
}

TextureStorage11_2D::~TextureStorage11_2D()
{
    for (unsigned i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        if (mAssociatedImages[i] != NULL)
        {
            bool imageAssociationCorrect = mAssociatedImages[i]->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // We must let the Images recover their data before we delete it from the TextureStorage.
                gl::Error error = mAssociatedImages[i]->recoverFromAssociatedStorage();
                if (error.isError())
                {
                    // TODO: Find a way to report this back to the context
                }
            }
        }
    }

    SafeRelease(mTexture);
    SafeRelease(mSwizzleTexture);

    SafeRelease(mLevelZeroTexture);
    SafeDelete(mLevelZeroRenderTarget);

    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        SafeDelete(mRenderTarget[i]);
        SafeRelease(mSwizzleRenderTargets[i]);
    }
}

gl::Error TextureStorage11_2D::copyToStorage(TextureStorage *destStorage)
{
    ASSERT(destStorage);

    TextureStorage11_2D *dest11 = GetAs<TextureStorage11_2D>(destStorage);

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

        // If either mTexture or mLevelZeroTexture exist, then we need to copy them into the corresponding textures in destStorage.
        if (mTexture)
        {
            gl::Error error = dest11->useLevelZeroWorkaroundTexture(false);
            if (error.isError())
            {
                return error;
            }

            ID3D11Resource *destResource = NULL;
            error = dest11->getResource(&destResource);
            if (error.isError())
            {
                return error;
            }

            immediateContext->CopyResource(destResource, mTexture);
        }

        if (mLevelZeroTexture)
        {
            gl::Error error = dest11->useLevelZeroWorkaroundTexture(true);
            if (error.isError())
            {
                return error;
            }

            ID3D11Resource *destResource = NULL;
            error = dest11->getResource(&destResource);
            if (error.isError())
            {
                return error;
            }

            immediateContext->CopyResource(destResource, mLevelZeroTexture);
        }
    }
    else
    {
        ID3D11Resource *sourceResouce = NULL;
        gl::Error error = getResource(&sourceResouce);
        if (error.isError())
        {
            return error;
        }

        ID3D11Resource *destResource = NULL;
        error = dest11->getResource(&destResource);
        if (error.isError())
        {
            return error;
        }

        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
        immediateContext->CopyResource(destResource, sourceResouce);
    }

    dest11->invalidateSwizzleCache();

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::useLevelZeroWorkaroundTexture(bool useLevelZeroTexture)
{
    if (useLevelZeroTexture && mMipLevels > 1)
    {
        if (!mUseLevelZeroTexture && mTexture)
        {
            gl::Error error = ensureTextureExists(1);
            if (error.isError())
            {
                return error;
            }

            // Pull data back from the mipped texture if necessary.
            ASSERT(mLevelZeroTexture);
            ID3D11DeviceContext *context = mRenderer->getDeviceContext();
            context->CopySubresourceRegion(mLevelZeroTexture, 0, 0, 0, 0, mTexture, 0, NULL);
        }

        mUseLevelZeroTexture = true;
    }
    else
    {
        if (mUseLevelZeroTexture && mLevelZeroTexture)
        {
            gl::Error error = ensureTextureExists(mMipLevels);
            if (error.isError())
            {
                return error;
            }

            // Pull data back from the level zero texture if necessary.
            ASSERT(mTexture);
            ID3D11DeviceContext *context = mRenderer->getDeviceContext();
            context->CopySubresourceRegion(mTexture, 0, 0, 0, 0, mLevelZeroTexture, 0, NULL);
        }

        mUseLevelZeroTexture = false;
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureStorage11_2D::associateImage(Image11* image, const gl::ImageIndex &index)
{
    GLint level = index.mipIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        mAssociatedImages[level] = image;
    }
}

bool TextureStorage11_2D::isAssociatedImageValid(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // This validation check should never return false. It means the Image/TextureStorage association is broken.
        bool retValue = (mAssociatedImages[level] == expectedImage);
        ASSERT(retValue);
        return retValue;
    }

    return false;
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_2D::disassociateImage(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        ASSERT(mAssociatedImages[level] == expectedImage);

        if (mAssociatedImages[level] == expectedImage)
        {
            mAssociatedImages[level] = NULL;
        }
    }
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image recover its data before ending the association.
gl::Error TextureStorage11_2D::releaseAssociatedImage(const gl::ImageIndex &index, Image11* incomingImage)
{
    GLint level = index.mipIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // No need to let the old Image recover its data, if it is also the incoming Image.
        if (mAssociatedImages[level] != NULL && mAssociatedImages[level] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage. This should be true.
            bool imageAssociationCorrect = mAssociatedImages[level]->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // Force the image to recover from storage before its data is overwritten.
                // This will reset mAssociatedImages[level] to NULL too.
                gl::Error error = mAssociatedImages[level]->recoverFromAssociatedStorage();
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::getResource(ID3D11Resource **outResource)
{
    if (mUseLevelZeroTexture && mMipLevels > 1)
    {
        gl::Error error = ensureTextureExists(1);
        if (error.isError())
        {
            return error;
        }

        *outResource = mLevelZeroTexture;
        return gl::Error(GL_NO_ERROR);
    }
    else
    {
        gl::Error error = ensureTextureExists(mMipLevels);
        if (error.isError())
        {
            return error;
        }

        *outResource = mTexture;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error TextureStorage11_2D::getMippedResource(ID3D11Resource **outResource)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    ASSERT(mRenderer->getWorkarounds().zeroMaxLodWorkaround);

    gl::Error error = ensureTextureExists(mMipLevels);
    if (error.isError())
    {
        return error;
    }

    *outResource = mTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::ensureTextureExists(int mipLevels)
{
    // If mMipLevels = 1 then always use mTexture rather than mLevelZeroTexture.
    bool useLevelZeroTexture = mRenderer->getWorkarounds().zeroMaxLodWorkaround ? (mipLevels == 1) && (mMipLevels > 1) : false;
    ID3D11Texture2D **outputTexture = useLevelZeroTexture ? &mLevelZeroTexture : &mTexture;

    // if the width or height is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (*outputTexture == NULL && mTextureWidth > 0 && mTextureHeight > 0)
    {
        ASSERT(mipLevels > 0);

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;      // Compressed texture size constraints?
        desc.Height = mTextureHeight;
        desc.MipLevels = mipLevels;
        desc.ArraySize = 1;
        desc.Format = mTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = getBindFlags();
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = getMiscFlags();

        HRESULT result = device->CreateTexture2D(&desc, NULL, outputTexture);

        // this can happen from windows TDR
        if (d3d11::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 2D texture storage, result: 0x%X.", result);
        }
        else if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 2D texture storage, result: 0x%X.", result);
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());

    int level = index.mipIndex;
    ASSERT(level >= 0 && level < getLevelCount());

    // In GL ES 2.0, the application can only render to level zero of the texture (Section 4.4.3 of the GLES 2.0 spec, page 113 of version 2.0.25).
    // Other parts of TextureStorage11_2D could create RTVs on non-zero levels of the texture (e.g. generateMipmap).
    // On Feature Level 9_3, this is unlikely to be useful. The renderer can't create SRVs on the individual levels of the texture,
    // so methods like generateMipmap can't do anything useful with non-zero-level RTVs.
    // Therefore if level > 0 on 9_3 then there's almost certainly something wrong.
    ASSERT(!(mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3 && level > 0));

    if (!mRenderTarget[level])
    {
        ID3D11Resource *texture = NULL;
        gl::Error error = getResource(&texture);
        if (error.isError())
        {
            return error;
        }

        ID3D11ShaderResourceView *srv = NULL;
        error = getSRVLevel(level, &srv);
        if (error.isError())
        {
            return error;
        }

        if (mUseLevelZeroTexture)
        {
            if (!mLevelZeroRenderTarget)
            {
                ID3D11Device *device = mRenderer->getDevice();

                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format = mRenderTargetFormat;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                rtvDesc.Texture2D.MipSlice = mTopLevel + level;

                ID3D11RenderTargetView *rtv;
                HRESULT result = device->CreateRenderTargetView(mLevelZeroTexture, &rtvDesc, &rtv);

                if (result == E_OUTOFMEMORY)
                {
                    return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
                }
                ASSERT(SUCCEEDED(result));

                mLevelZeroRenderTarget = new TextureRenderTarget11(rtv, mLevelZeroTexture, NULL, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

                // RenderTarget will take ownership of these resources
                SafeRelease(rtv);
            }

            ASSERT(outRT);
            *outRT = mLevelZeroRenderTarget;
            return gl::Error(GL_NO_ERROR);
        }

        if (mRenderTargetFormat != DXGI_FORMAT_UNKNOWN)
        {
            ID3D11Device *device = mRenderer->getDevice();

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = mRenderTargetFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
            rtvDesc.Texture2D.MipSlice = mTopLevel + level;

            ID3D11RenderTargetView *rtv;
            HRESULT result = device->CreateRenderTargetView(texture, &rtvDesc, &rtv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
            }

            mRenderTarget[level] = new TextureRenderTarget11(rtv, texture, srv, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(rtv);
        }
        else if (mDepthStencilFormat != DXGI_FORMAT_UNKNOWN)
        {
            ID3D11Device *device = mRenderer->getDevice();

            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = mDepthStencilFormat;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
            dsvDesc.Texture2D.MipSlice = mTopLevel + level;
            dsvDesc.Flags = 0;

            ID3D11DepthStencilView *dsv;
            HRESULT result = device->CreateDepthStencilView(texture, &dsvDesc, &dsv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                return gl::Error(GL_OUT_OF_MEMORY,"Failed to create internal depth stencil view for texture storage, result: 0x%X.", result);
            }

            mRenderTarget[level] = new TextureRenderTarget11(dsv, texture, srv, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(dsv);
        }
        else
        {
            UNREACHABLE();
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTarget[level];
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::createSRV(int baseLevel, int mipLevels, DXGI_FORMAT format, ID3D11Resource *texture,
                                         ID3D11ShaderResourceView **outSRV) const
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = mTopLevel + baseLevel;
    srvDesc.Texture2D.MipLevels = mipLevels;

    ID3D11Resource *srvTexture = texture;

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        ASSERT(mTopLevel == 0);
        ASSERT(baseLevel == 0);
        // This code also assumes that the incoming texture equals either mLevelZeroTexture or mTexture.

        if (mipLevels == 1 && mMipLevels > 1)
        {
            // We must use a SRV on the level-zero-only texture.
            ASSERT(mLevelZeroTexture != NULL && texture == mLevelZeroTexture);
            srvTexture = mLevelZeroTexture;
        }
        else
        {
            ASSERT(mipLevels == static_cast<int>(mMipLevels));
            ASSERT(mTexture != NULL && texture == mTexture);
            srvTexture = mTexture;
        }
    }

    ID3D11Device *device = mRenderer->getDevice();
    HRESULT result = device->CreateShaderResourceView(srvTexture, &srvDesc, outSRV);

    ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal texture storage SRV, result: 0x%X.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::getSwizzleTexture(ID3D11Resource **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture)
    {
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.MipLevels = mMipLevels;
        desc.ArraySize = 1;
        desc.Format = mSwizzleTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT result = device->CreateTexture2D(&desc, NULL, &mSwizzleTexture);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle texture, result: 0x%X.", result);
        }
    }

    *outTexture = mSwizzleTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2D::getSwizzleRenderTarget(int mipLevel, ID3D11RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel])
    {
        ID3D11Resource *swizzleTexture = NULL;
        gl::Error error = getSwizzleTexture(&swizzleTexture);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = mSwizzleRenderTargetFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mTopLevel + mipLevel;

        HRESULT result = device->CreateRenderTargetView(mSwizzleTexture, &rtvDesc, &mSwizzleRenderTargets[mipLevel]);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle render target view, result: 0x%X.", result);
        }
    }

    *outRTV = mSwizzleRenderTargets[mipLevel];
    return gl::Error(GL_NO_ERROR);
}

TextureStorage11_EGLImage::TextureStorage11_EGLImage(Renderer11 *renderer, EGLImageD3D *eglImage)
    : TextureStorage11(renderer, D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, 0),
      mImage(eglImage),
      mCurrentRenderTarget(0),
      mSwizzleTexture(nullptr),
      mSwizzleRenderTargets(gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS, nullptr)
{
    RenderTargetD3D *renderTargetD3D = nullptr;
    mImage->getRenderTarget(&renderTargetD3D);
    RenderTarget11 *renderTarget11 = GetAs<RenderTarget11>(renderTargetD3D);
    mCurrentRenderTarget           = reinterpret_cast<uintptr_t>(renderTarget11);

    mMipLevels      = 1;
    mTextureFormat  = renderTarget11->getDXGIFormat();
    mTextureWidth   = renderTarget11->getWidth();
    mTextureHeight  = renderTarget11->getHeight();
    mTextureDepth   = 1;
    mInternalFormat = renderTarget11->getInternalFormat();

    ID3D11ShaderResourceView *srv = renderTarget11->getShaderResourceView();
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srv->GetDesc(&srvDesc);
    mShaderResourceFormat = srvDesc.Format;

    ID3D11RenderTargetView *rtv = renderTarget11->getRenderTargetView();
    if (rtv != nullptr)
    {
        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtv->GetDesc(&rtvDesc);
        mRenderTargetFormat = rtvDesc.Format;
    }
    else
    {
        mRenderTargetFormat = DXGI_FORMAT_UNKNOWN;
    }

    ID3D11DepthStencilView *dsv = renderTarget11->getDepthStencilView();
    if (dsv != nullptr)
    {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
        dsv->GetDesc(&dsvDesc);
        mDepthStencilFormat = dsvDesc.Format;
    }
    else
    {
        mDepthStencilFormat = DXGI_FORMAT_UNKNOWN;
    }

    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(mTextureFormat);
    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(
        dxgiFormatInfo.internalFormat, mRenderer->getRenderer11DeviceCaps());
    mSwizzleTextureFormat        = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat   = formatInfo.swizzleRTVFormat;
}

TextureStorage11_EGLImage::~TextureStorage11_EGLImage()
{
    SafeRelease(mSwizzleTexture);
    for (size_t i = 0; i < mSwizzleRenderTargets.size(); i++)
    {
        SafeRelease(mSwizzleRenderTargets[i]);
    }
}

gl::Error TextureStorage11_EGLImage::getResource(ID3D11Resource **outResource)
{
    gl::Error error = checkForUpdatedRenderTarget();
    if (error.isError())
    {
        return error;
    }

    RenderTarget11 *renderTarget11 = nullptr;
    error = getImageRenderTarget(&renderTarget11);
    if (error.isError())
    {
        return error;
    }

    *outResource = renderTarget11->getTexture();
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::getSRV(const gl::TextureState &textureState,
                                            ID3D11ShaderResourceView **outSRV)
{
    gl::Error error = checkForUpdatedRenderTarget();
    if (error.isError())
    {
        return error;
    }

    return TextureStorage11::getSRV(textureState, outSRV);
}

gl::Error TextureStorage11_EGLImage::getMippedResource(ID3D11Resource **)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    // EGL images are unavailable in this configuration.
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureStorage11_EGLImage::getRenderTarget(const gl::ImageIndex &index,
                                                     RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());
    ASSERT(index.mipIndex == 0);
    UNUSED_ASSERTION_VARIABLE(index);

    gl::Error error = checkForUpdatedRenderTarget();
    if (error.isError())
    {
        return error;
    }

    return mImage->getRenderTarget(outRT);
}

gl::Error TextureStorage11_EGLImage::copyToStorage(TextureStorage *destStorage)
{
    ID3D11Resource *sourceResouce = nullptr;
    gl::Error error = getResource(&sourceResouce);
    if (error.isError())
    {
        return error;
    }

    ASSERT(destStorage);
    TextureStorage11_2D *dest11  = GetAs<TextureStorage11_2D>(destStorage);
    ID3D11Resource *destResource = nullptr;
    error = dest11->getResource(&destResource);
    if (error.isError())
    {
        return error;
    }

    ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
    immediateContext->CopyResource(destResource, sourceResouce);

    dest11->invalidateSwizzleCache();

    return gl::Error(GL_NO_ERROR);
}

void TextureStorage11_EGLImage::associateImage(Image11 *, const gl::ImageIndex &)
{
}

void TextureStorage11_EGLImage::disassociateImage(const gl::ImageIndex &, Image11 *)
{
}

bool TextureStorage11_EGLImage::isAssociatedImageValid(const gl::ImageIndex &, Image11 *)
{
    return false;
}

gl::Error TextureStorage11_EGLImage::releaseAssociatedImage(const gl::ImageIndex &, Image11 *)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::useLevelZeroWorkaroundTexture(bool)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureStorage11_EGLImage::getSwizzleTexture(ID3D11Resource **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture)
    {
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width              = mTextureWidth;
        desc.Height             = mTextureHeight;
        desc.MipLevels          = mMipLevels;
        desc.ArraySize          = 1;
        desc.Format             = mSwizzleTextureFormat;
        desc.SampleDesc.Count   = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage              = D3D11_USAGE_DEFAULT;
        desc.BindFlags          = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags     = 0;
        desc.MiscFlags          = 0;

        HRESULT result = device->CreateTexture2D(&desc, NULL, &mSwizzleTexture);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY,
                             "Failed to create internal swizzle texture, result: 0x%X.", result);
        }
    }

    *outTexture = mSwizzleTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::getSwizzleRenderTarget(int mipLevel,
                                                            ID3D11RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel])
    {
        ID3D11Resource *swizzleTexture = NULL;
        gl::Error error = getSwizzleTexture(&swizzleTexture);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format             = mSwizzleRenderTargetFormat;
        rtvDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        rtvDesc.Texture2D.MipSlice = mTopLevel + mipLevel;

        HRESULT result = device->CreateRenderTargetView(mSwizzleTexture, &rtvDesc,
                                                        &mSwizzleRenderTargets[mipLevel]);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY,
                             "Failed to create internal swizzle render target view, result: 0x%X.",
                             result);
        }
    }

    *outRTV = mSwizzleRenderTargets[mipLevel];
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::checkForUpdatedRenderTarget()
{
    RenderTarget11 *renderTarget11 = nullptr;
    gl::Error error = getImageRenderTarget(&renderTarget11);
    if (error.isError())
    {
        return error;
    }

    if (mCurrentRenderTarget != reinterpret_cast<uintptr_t>(renderTarget11))
    {
        clearSRVCache();
        mCurrentRenderTarget = reinterpret_cast<uintptr_t>(renderTarget11);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::createSRV(int baseLevel,
                                               int mipLevels,
                                               DXGI_FORMAT format,
                                               ID3D11Resource *texture,
                                               ID3D11ShaderResourceView **outSRV) const
{
    ASSERT(baseLevel == 0);
    ASSERT(mipLevels == 1);
    ASSERT(outSRV);

    // Create a new SRV only for the swizzle texture.  Otherwise just return the Image's
    // RenderTarget's SRV.
    if (texture == mSwizzleTexture)
    {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format                    = format;
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = mTopLevel + baseLevel;
        srvDesc.Texture2D.MipLevels       = mipLevels;

        ID3D11Device *device = mRenderer->getDevice();
        HRESULT result       = device->CreateShaderResourceView(texture, &srvDesc, outSRV);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY,
                             "Failed to create internal texture storage SRV, result: 0x%X.",
                             result);
        }
    }
    else
    {
        RenderTarget11 *renderTarget = nullptr;
        gl::Error error = getImageRenderTarget(&renderTarget);
        if (error.isError())
        {
            return error;
        }

        ASSERT(texture == renderTarget->getTexture());

        *outSRV = renderTarget->getShaderResourceView();
        (*outSRV)->AddRef();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_EGLImage::getImageRenderTarget(RenderTarget11 **outRT) const
{
    RenderTargetD3D *renderTargetD3D = nullptr;
    gl::Error error = mImage->getRenderTarget(&renderTargetD3D);
    if (error.isError())
    {
        return error;
    }

    *outRT = GetAs<RenderTarget11>(renderTargetD3D);
    return gl::Error(GL_NO_ERROR);
}

TextureStorage11_Cube::TextureStorage11_Cube(Renderer11 *renderer, GLenum internalformat, bool renderTarget, int size, int levels, bool hintLevelZeroOnly)
    : TextureStorage11(renderer,
                       GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget),
                       GetTextureMiscFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget, levels))
{
    mTexture = NULL;
    mSwizzleTexture = NULL;

    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        mSwizzleRenderTargets[level] = NULL;
        for (unsigned int face = 0; face < CUBE_FACE_COUNT; face++)
        {
            mAssociatedImages[face][level] = NULL;
            mRenderTarget[face][level] = NULL;
        }
    }

    mLevelZeroTexture = NULL;
    mUseLevelZeroTexture = false;

    for (unsigned int face = 0; face < CUBE_FACE_COUNT; face++)
    {
        mLevelZeroRenderTarget[face] = NULL;
    }

    mInternalFormat = internalformat;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalformat, renderer->getRenderer11DeviceCaps());
    mTextureFormat = formatInfo.texFormat;
    mShaderResourceFormat = formatInfo.srvFormat;
    mDepthStencilFormat = formatInfo.dsvFormat;
    mRenderTargetFormat = formatInfo.rtvFormat;
    mSwizzleTextureFormat = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat = formatInfo.swizzleRTVFormat;

    // adjust size if needed for compressed textures
    int height = size;
    d3d11::MakeValidSize(false, mTextureFormat, &size, &height, &mTopLevel);

    mMipLevels = mTopLevel + levels;
    mTextureWidth = size;
    mTextureHeight = size;
    mTextureDepth = 1;

    if (hintLevelZeroOnly && levels > 1)
    {
        //The LevelZeroOnly hint should only be true if the zero max LOD workaround is active.
        ASSERT(mRenderer->getWorkarounds().zeroMaxLodWorkaround);
        mUseLevelZeroTexture = true;
    }
}

TextureStorage11_Cube::~TextureStorage11_Cube()
{
    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        for (unsigned int face = 0; face < CUBE_FACE_COUNT; face++)
        {
            if (mAssociatedImages[face][level] != NULL)
            {
                bool imageAssociationCorrect = mAssociatedImages[face][level]->isAssociatedStorageValid(this);
                ASSERT(imageAssociationCorrect);

                if (imageAssociationCorrect)
                {
                    // We must let the Images recover their data before we delete it from the TextureStorage.
                    mAssociatedImages[face][level]->recoverFromAssociatedStorage();
                }
            }
        }
    }

    SafeRelease(mTexture);
    SafeRelease(mSwizzleTexture);
    SafeRelease(mLevelZeroTexture);

    for (unsigned int face = 0; face < CUBE_FACE_COUNT; face++)
    {
        SafeDelete(mLevelZeroRenderTarget[face]);
    }

    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        SafeRelease(mSwizzleRenderTargets[level]);
        for (unsigned int face = 0; face < CUBE_FACE_COUNT; face++)
        {
            SafeDelete(mRenderTarget[face][level]);
        }
    }
}

UINT TextureStorage11_Cube::getSubresourceIndex(const gl::ImageIndex &index) const
{
    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround && mUseLevelZeroTexture && index.mipIndex == 0)
    {
        UINT arraySlice = static_cast<UINT>(index.hasLayer() ? index.layerIndex : 0);
        UINT subresource = D3D11CalcSubresource(0, arraySlice, 1);
        ASSERT(subresource != std::numeric_limits<UINT>::max());
        return subresource;
    }
    else
    {
        UINT mipSlice = static_cast<UINT>(index.mipIndex + mTopLevel);
        UINT arraySlice = static_cast<UINT>(index.hasLayer() ? index.layerIndex : 0);
        UINT subresource = D3D11CalcSubresource(mipSlice, arraySlice, mMipLevels);
        ASSERT(subresource != std::numeric_limits<UINT>::max());
        return subresource;
    }
}

gl::Error TextureStorage11_Cube::copyToStorage(TextureStorage *destStorage)
{
    ASSERT(destStorage);

    TextureStorage11_Cube *dest11 = GetAs<TextureStorage11_Cube>(destStorage);

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();

        // If either mTexture or mLevelZeroTexture exist, then we need to copy them into the corresponding textures in destStorage.
        if (mTexture)
        {
            gl::Error error = dest11->useLevelZeroWorkaroundTexture(false);
            if (error.isError())
            {
                return error;
            }

            ID3D11Resource *destResource = NULL;
            error = dest11->getResource(&destResource);
            if (error.isError())
            {
                return error;
            }

            immediateContext->CopyResource(destResource, mTexture);
        }

        if (mLevelZeroTexture)
        {
            gl::Error error = dest11->useLevelZeroWorkaroundTexture(true);
            if (error.isError())
            {
                return error;
            }

            ID3D11Resource *destResource = NULL;
            error = dest11->getResource(&destResource);
            if (error.isError())
            {
                return error;
            }

            immediateContext->CopyResource(destResource, mLevelZeroTexture);
        }
    }
    else
    {
        ID3D11Resource *sourceResouce = NULL;
        gl::Error error = getResource(&sourceResouce);
        if (error.isError())
        {
            return error;
        }

        ID3D11Resource *destResource = NULL;
        error = dest11->getResource(&destResource);
        if (error.isError())
        {
            return error;
        }

        ID3D11DeviceContext *immediateContext = mRenderer->getDeviceContext();
        immediateContext->CopyResource(destResource, sourceResouce);
    }

    dest11->invalidateSwizzleCache();

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::useLevelZeroWorkaroundTexture(bool useLevelZeroTexture)
{
    if (useLevelZeroTexture && mMipLevels > 1)
    {
        if (!mUseLevelZeroTexture && mTexture)
        {
            gl::Error error = ensureTextureExists(1);
            if (error.isError())
            {
                return error;
            }

            // Pull data back from the mipped texture if necessary.
            ASSERT(mLevelZeroTexture);
            ID3D11DeviceContext *context = mRenderer->getDeviceContext();

            for (int face = 0; face < 6; face++)
            {
                context->CopySubresourceRegion(mLevelZeroTexture, D3D11CalcSubresource(0, face, 1), 0, 0, 0, mTexture, face * mMipLevels, NULL);
            }
        }

        mUseLevelZeroTexture = true;
    }
    else
    {
        if (mUseLevelZeroTexture && mLevelZeroTexture)
        {
            gl::Error error = ensureTextureExists(mMipLevels);
            if (error.isError())
            {
                return error;
            }

            // Pull data back from the level zero texture if necessary.
            ASSERT(mTexture);
            ID3D11DeviceContext *context = mRenderer->getDeviceContext();

            for (int face = 0; face < 6; face++)
            {
                context->CopySubresourceRegion(mTexture, D3D11CalcSubresource(0, face, mMipLevels), 0, 0, 0, mLevelZeroTexture, face, NULL);
            }
        }

        mUseLevelZeroTexture = false;
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureStorage11_Cube::associateImage(Image11* image, const gl::ImageIndex &index)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < CUBE_FACE_COUNT);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        if (0 <= layerTarget && layerTarget < CUBE_FACE_COUNT)
        {
            mAssociatedImages[layerTarget][level] = image;
        }
    }
}

bool TextureStorage11_Cube::isAssociatedImageValid(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        if (0 <= layerTarget && layerTarget < CUBE_FACE_COUNT)
        {
            // This validation check should never return false. It means the Image/TextureStorage association is broken.
            bool retValue = (mAssociatedImages[layerTarget][level] == expectedImage);
            ASSERT(retValue);
            return retValue;
        }
    }

    return false;
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_Cube::disassociateImage(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < CUBE_FACE_COUNT);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        if (0 <= layerTarget && layerTarget < CUBE_FACE_COUNT)
        {
            ASSERT(mAssociatedImages[layerTarget][level] == expectedImage);

            if (mAssociatedImages[layerTarget][level] == expectedImage)
            {
                mAssociatedImages[layerTarget][level] = NULL;
            }
        }
    }
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image recover its data before ending the association.
gl::Error TextureStorage11_Cube::releaseAssociatedImage(const gl::ImageIndex &index, Image11* incomingImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(0 <= layerTarget && layerTarget < CUBE_FACE_COUNT);

    if ((0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS))
    {
        if (0 <= layerTarget && layerTarget < CUBE_FACE_COUNT)
        {
            // No need to let the old Image recover its data, if it is also the incoming Image.
            if (mAssociatedImages[layerTarget][level] != NULL && mAssociatedImages[layerTarget][level] != incomingImage)
            {
                // Ensure that the Image is still associated with this TextureStorage. This should be true.
                bool imageAssociationCorrect = mAssociatedImages[layerTarget][level]->isAssociatedStorageValid(this);
                ASSERT(imageAssociationCorrect);

                if (imageAssociationCorrect)
                {
                    // Force the image to recover from storage before its data is overwritten.
                    // This will reset mAssociatedImages[level] to NULL too.
                    gl::Error error = mAssociatedImages[layerTarget][level]->recoverFromAssociatedStorage();
                    if (error.isError())
                    {
                        return error;
                    }
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::getResource(ID3D11Resource **outResource)
{
    if (mUseLevelZeroTexture && mMipLevels > 1)
    {
        gl::Error error = ensureTextureExists(1);
        if (error.isError())
        {
            return error;
        }

        *outResource = mLevelZeroTexture;
        return gl::Error(GL_NO_ERROR);
    }
    else
    {
        gl::Error error = ensureTextureExists(mMipLevels);
        if (error.isError())
        {
            return error;
        }

        *outResource = mTexture;
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error TextureStorage11_Cube::getMippedResource(ID3D11Resource **outResource)
{
    // This shouldn't be called unless the zero max LOD workaround is active.
    ASSERT(mRenderer->getWorkarounds().zeroMaxLodWorkaround);

    gl::Error error = ensureTextureExists(mMipLevels);
    if (error.isError())
    {
        return error;
    }

    *outResource = mTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::ensureTextureExists(int mipLevels)
{
    // If mMipLevels = 1 then always use mTexture rather than mLevelZeroTexture.
    bool useLevelZeroTexture = mRenderer->getWorkarounds().zeroMaxLodWorkaround ? (mipLevels == 1) && (mMipLevels > 1) : false;
    ID3D11Texture2D **outputTexture = useLevelZeroTexture ? &mLevelZeroTexture : &mTexture;

    // if the size is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (*outputTexture == NULL && mTextureWidth > 0 && mTextureHeight > 0)
    {
        ASSERT(mMipLevels > 0);

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.MipLevels = mipLevels;
        desc.ArraySize = CUBE_FACE_COUNT;
        desc.Format = mTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = getBindFlags();
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE | getMiscFlags();

        HRESULT result = device->CreateTexture2D(&desc, NULL, outputTexture);

        // this can happen from windows TDR
        if (d3d11::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create cube texture storage, result: 0x%X.", result);
        }
        else if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create cube texture storage, result: 0x%X.", result);
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    int faceIndex = index.layerIndex;
    int level = index.mipIndex;

    ASSERT(level >= 0 && level < getLevelCount());
    ASSERT(faceIndex >= 0 && faceIndex < CUBE_FACE_COUNT);

    if (!mRenderTarget[faceIndex][level])
    {
        ID3D11Device *device = mRenderer->getDevice();
        HRESULT result;

        ID3D11Resource *texture = NULL;
        gl::Error error = getResource(&texture);
        if (error.isError())
        {
            return error;
        }

        if (mUseLevelZeroTexture)
        {
            if (!mLevelZeroRenderTarget[faceIndex])
            {
                D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
                rtvDesc.Format = mRenderTargetFormat;
                rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                rtvDesc.Texture2DArray.MipSlice = mTopLevel + level;
                rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;
                rtvDesc.Texture2DArray.ArraySize = 1;

                ID3D11RenderTargetView *rtv;
                result = device->CreateRenderTargetView(mLevelZeroTexture, &rtvDesc, &rtv);

                if (result == E_OUTOFMEMORY)
                {
                    return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
                }
                ASSERT(SUCCEEDED(result));

                mLevelZeroRenderTarget[faceIndex] = new TextureRenderTarget11(rtv, mLevelZeroTexture, NULL, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

                // RenderTarget will take ownership of these resources
                SafeRelease(rtv);
            }

            ASSERT(outRT);
            *outRT = mLevelZeroRenderTarget[faceIndex];
            return gl::Error(GL_NO_ERROR);
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = mShaderResourceFormat;
        srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + level;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = faceIndex;
        srvDesc.Texture2DArray.ArraySize = 1;

        if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        }
        else
        {
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY; // Will be used with Texture2D sampler, not TextureCube
        }

        ID3D11ShaderResourceView *srv;
        result = device->CreateShaderResourceView(texture, &srvDesc, &srv);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal shader resource view for texture storage, result: 0x%X.", result);
        }

        if (mRenderTargetFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = mRenderTargetFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = mTopLevel + level;
            rtvDesc.Texture2DArray.FirstArraySlice = faceIndex;
            rtvDesc.Texture2DArray.ArraySize = 1;

            ID3D11RenderTargetView *rtv;
            result = device->CreateRenderTargetView(texture, &rtvDesc, &rtv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                SafeRelease(srv);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
            }

            mRenderTarget[faceIndex][level] = new TextureRenderTarget11(rtv, texture, srv, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(rtv);
            SafeRelease(srv);
        }
        else if (mDepthStencilFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
            dsvDesc.Format = mDepthStencilFormat;
            dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            dsvDesc.Flags = 0;
            dsvDesc.Texture2DArray.MipSlice = mTopLevel + level;
            dsvDesc.Texture2DArray.FirstArraySlice = faceIndex;
            dsvDesc.Texture2DArray.ArraySize = 1;

            ID3D11DepthStencilView *dsv;
            result = device->CreateDepthStencilView(texture, &dsvDesc, &dsv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                SafeRelease(srv);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal depth stencil view for texture storage, result: 0x%X.", result);
            }

            mRenderTarget[faceIndex][level] = new TextureRenderTarget11(dsv, texture, srv, mInternalFormat, getLevelWidth(level), getLevelHeight(level), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(dsv);
            SafeRelease(srv);
        }
        else
        {
            UNREACHABLE();
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTarget[faceIndex][level];
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::createSRV(int baseLevel, int mipLevels, DXGI_FORMAT format, ID3D11Resource *texture,
                                           ID3D11ShaderResourceView **outSRV) const
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;

    // Unnormalized integer cube maps are not supported by DX11; we emulate them as an array of six 2D textures
    const d3d11::DXGIFormat &dxgiFormatInfo = d3d11::GetDXGIFormatInfo(format);
    if (dxgiFormatInfo.componentType == GL_INT || dxgiFormatInfo.componentType == GL_UNSIGNED_INT)
    {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + baseLevel;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = 0;
        srvDesc.Texture2DArray.ArraySize = CUBE_FACE_COUNT;
    }
    else
    {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = mipLevels;
        srvDesc.TextureCube.MostDetailedMip = mTopLevel + baseLevel;
    }

    ID3D11Resource *srvTexture = texture;

    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        ASSERT(mTopLevel == 0);
        ASSERT(baseLevel == 0);
        // This code also assumes that the incoming texture equals either mLevelZeroTexture or mTexture.

        if (mipLevels == 1 && mMipLevels > 1)
        {
            // We must use a SRV on the level-zero-only texture.
            ASSERT(mLevelZeroTexture != NULL && texture == mLevelZeroTexture);
            srvTexture = mLevelZeroTexture;
        }
        else
        {
            ASSERT(mipLevels == static_cast<int>(mMipLevels));
            ASSERT(mTexture != NULL && texture == mTexture);
            srvTexture = mTexture;
        }
    }

    ID3D11Device *device = mRenderer->getDevice();
    HRESULT result = device->CreateShaderResourceView(srvTexture, &srvDesc, outSRV);

    ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal texture storage SRV, result: 0x%X.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::getSwizzleTexture(ID3D11Resource **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture)
    {
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.MipLevels = mMipLevels;
        desc.ArraySize = CUBE_FACE_COUNT;
        desc.Format = mSwizzleTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

        HRESULT result = device->CreateTexture2D(&desc, NULL, &mSwizzleTexture);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle texture, result: 0x%X.", result);
        }
    }

    *outTexture = mSwizzleTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_Cube::getSwizzleRenderTarget(int mipLevel, ID3D11RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel])
    {
        ID3D11Resource *swizzleTexture = NULL;
        gl::Error error = getSwizzleTexture(&swizzleTexture);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = mSwizzleRenderTargetFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = mTopLevel + mipLevel;
        rtvDesc.Texture2DArray.FirstArraySlice = 0;
        rtvDesc.Texture2DArray.ArraySize = CUBE_FACE_COUNT;

        HRESULT result = device->CreateRenderTargetView(mSwizzleTexture, &rtvDesc, &mSwizzleRenderTargets[mipLevel]);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle render target view, result: 0x%X.", result);
        }
    }

    *outRTV = mSwizzleRenderTargets[mipLevel];
    return gl::Error(GL_NO_ERROR);
}

TextureStorage11_3D::TextureStorage11_3D(Renderer11 *renderer, GLenum internalformat, bool renderTarget,
                                         GLsizei width, GLsizei height, GLsizei depth, int levels)
    : TextureStorage11(renderer,
                       GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget),
                       GetTextureMiscFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget, levels))
{
    mTexture = NULL;
    mSwizzleTexture = NULL;

    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        mAssociatedImages[i] = NULL;
        mLevelRenderTargets[i] = NULL;
        mSwizzleRenderTargets[i] = NULL;
    }

    mInternalFormat = internalformat;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalformat, renderer->getRenderer11DeviceCaps());
    mTextureFormat = formatInfo.texFormat;
    mShaderResourceFormat = formatInfo.srvFormat;
    mDepthStencilFormat = formatInfo.dsvFormat;
    mRenderTargetFormat = formatInfo.rtvFormat;
    mSwizzleTextureFormat = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat = formatInfo.swizzleRTVFormat;

    // adjust size if needed for compressed textures
    d3d11::MakeValidSize(false, mTextureFormat, &width, &height, &mTopLevel);

    mMipLevels = mTopLevel + levels;
    mTextureWidth = width;
    mTextureHeight = height;
    mTextureDepth = depth;
}

TextureStorage11_3D::~TextureStorage11_3D()
{
    for (unsigned i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        if (mAssociatedImages[i] != NULL)
        {
            bool imageAssociationCorrect = mAssociatedImages[i]->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // We must let the Images recover their data before we delete it from the TextureStorage.
                mAssociatedImages[i]->recoverFromAssociatedStorage();
            }
        }
    }

    SafeRelease(mTexture);
    SafeRelease(mSwizzleTexture);

    for (RenderTargetMap::iterator i = mLevelLayerRenderTargets.begin(); i != mLevelLayerRenderTargets.end(); i++)
    {
        SafeDelete(i->second);
    }
    mLevelLayerRenderTargets.clear();

    for (unsigned int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        SafeDelete(mLevelRenderTargets[i]);
        SafeRelease(mSwizzleRenderTargets[i]);
    }
}

void TextureStorage11_3D::associateImage(Image11* image, const gl::ImageIndex &index)
{
    GLint level = index.mipIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        mAssociatedImages[level] = image;
    }
}

bool TextureStorage11_3D::isAssociatedImageValid(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // This validation check should never return false. It means the Image/TextureStorage association is broken.
        bool retValue = (mAssociatedImages[level] == expectedImage);
        ASSERT(retValue);
        return retValue;
    }

    return false;
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_3D::disassociateImage(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;

    ASSERT(0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        ASSERT(mAssociatedImages[level] == expectedImage);

        if (mAssociatedImages[level] == expectedImage)
        {
            mAssociatedImages[level] = NULL;
        }
    }
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image recover its data before ending the association.
gl::Error TextureStorage11_3D::releaseAssociatedImage(const gl::ImageIndex &index, Image11* incomingImage)
{
    GLint level = index.mipIndex;

    ASSERT((0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS));

    if (0 <= level && level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
    {
        // No need to let the old Image recover its data, if it is also the incoming Image.
        if (mAssociatedImages[level] != NULL && mAssociatedImages[level] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage. This should be true.
            bool imageAssociationCorrect = mAssociatedImages[level]->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // Force the image to recover from storage before its data is overwritten.
                // This will reset mAssociatedImages[level] to NULL too.
                gl::Error error = mAssociatedImages[level]->recoverFromAssociatedStorage();
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_3D::getResource(ID3D11Resource **outResource)
{
    // If the width, height or depth are not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (mTexture == NULL && mTextureWidth > 0 && mTextureHeight > 0 && mTextureDepth > 0)
    {
        ASSERT(mMipLevels > 0);

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE3D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.Depth = mTextureDepth;
        desc.MipLevels = mMipLevels;
        desc.Format = mTextureFormat;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = getBindFlags();
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = getMiscFlags();

        HRESULT result = device->CreateTexture3D(&desc, NULL, &mTexture);

        // this can happen from windows TDR
        if (d3d11::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 3D texture storage, result: 0x%X.", result);
        }
        else if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 3D texture storage, result: 0x%X.", result);
        }
    }

    *outResource = mTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_3D::createSRV(int baseLevel, int mipLevels, DXGI_FORMAT format, ID3D11Resource *texture,
                                         ID3D11ShaderResourceView **outSRV) const
{
    ASSERT(outSRV);

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture3D.MostDetailedMip = baseLevel;
    srvDesc.Texture3D.MipLevels = mipLevels;

    ID3D11Device *device = mRenderer->getDevice();
    HRESULT result = device->CreateShaderResourceView(texture, &srvDesc, outSRV);

    ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal texture storage SRV, result: 0x%X.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_3D::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    int mipLevel = index.mipIndex;
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    ASSERT(mRenderTargetFormat != DXGI_FORMAT_UNKNOWN);

    if (!index.hasLayer())
    {
        if (!mLevelRenderTargets[mipLevel])
        {
            ID3D11Resource *texture = NULL;
            gl::Error error = getResource(&texture);
            if (error.isError())
            {
                return error;
            }

            ID3D11ShaderResourceView *srv = NULL;
            error = getSRVLevel(mipLevel, &srv);
            if (error.isError())
            {
                return error;
            }

            ID3D11Device *device = mRenderer->getDevice();

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = mRenderTargetFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = mTopLevel + mipLevel;
            rtvDesc.Texture3D.FirstWSlice = 0;
            rtvDesc.Texture3D.WSize = static_cast<UINT>(-1);

            ID3D11RenderTargetView *rtv;
            HRESULT result = device->CreateRenderTargetView(texture, &rtvDesc, &rtv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                SafeRelease(srv);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
            }

            mLevelRenderTargets[mipLevel] = new TextureRenderTarget11(rtv, texture, srv, mInternalFormat, getLevelWidth(mipLevel), getLevelHeight(mipLevel), getLevelDepth(mipLevel), 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(rtv);
        }

        ASSERT(outRT);
        *outRT = mLevelRenderTargets[mipLevel];
        return gl::Error(GL_NO_ERROR);
    }
    else
    {
        int layer = index.layerIndex;

        LevelLayerKey key(mipLevel, layer);
        if (mLevelLayerRenderTargets.find(key) == mLevelLayerRenderTargets.end())
        {
            ID3D11Device *device = mRenderer->getDevice();
            HRESULT result;

            ID3D11Resource *texture = NULL;
            gl::Error error = getResource(&texture);
            if (error.isError())
            {
                return error;
            }

            // TODO, what kind of SRV is expected here?
            ID3D11ShaderResourceView *srv = NULL;

            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = mRenderTargetFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
            rtvDesc.Texture3D.MipSlice = mTopLevel + mipLevel;
            rtvDesc.Texture3D.FirstWSlice = layer;
            rtvDesc.Texture3D.WSize = 1;

            ID3D11RenderTargetView *rtv;
            result = device->CreateRenderTargetView(texture, &rtvDesc, &rtv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                SafeRelease(srv); return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
            }
            ASSERT(SUCCEEDED(result));

            mLevelLayerRenderTargets[key] = new TextureRenderTarget11(rtv, texture, srv, mInternalFormat, getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(rtv);
        }

        ASSERT(outRT);
        *outRT = mLevelLayerRenderTargets[key];
        return gl::Error(GL_NO_ERROR);
    }
}

gl::Error TextureStorage11_3D::getSwizzleTexture(ID3D11Resource **outTexture)
{
    ASSERT(outTexture);

    if (!mSwizzleTexture)
    {
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE3D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.Depth = mTextureDepth;
        desc.MipLevels = mMipLevels;
        desc.Format = mSwizzleTextureFormat;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT result = device->CreateTexture3D(&desc, NULL, &mSwizzleTexture);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle texture, result: 0x%X.", result);
        }
    }

    *outTexture = mSwizzleTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_3D::getSwizzleRenderTarget(int mipLevel, ID3D11RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel])
    {
        ID3D11Resource *swizzleTexture = NULL;
        gl::Error error = getSwizzleTexture(&swizzleTexture);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = mSwizzleRenderTargetFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE3D;
        rtvDesc.Texture3D.MipSlice = mTopLevel + mipLevel;
        rtvDesc.Texture3D.FirstWSlice = 0;
        rtvDesc.Texture3D.WSize = static_cast<UINT>(-1);

        HRESULT result = device->CreateRenderTargetView(mSwizzleTexture, &rtvDesc, &mSwizzleRenderTargets[mipLevel]);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle render target view, result: 0x%X.", result);
        }
    }

    *outRTV = mSwizzleRenderTargets[mipLevel];
    return gl::Error(GL_NO_ERROR);
}

TextureStorage11_2DArray::TextureStorage11_2DArray(Renderer11 *renderer, GLenum internalformat, bool renderTarget,
                                                   GLsizei width, GLsizei height, GLsizei depth, int levels)
    : TextureStorage11(renderer,
                       GetTextureBindFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget),
                       GetTextureMiscFlags(internalformat, renderer->getRenderer11DeviceCaps(), renderTarget, levels))
{
    mTexture = NULL;
    mSwizzleTexture = NULL;

    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        mSwizzleRenderTargets[level] = NULL;
    }

    mInternalFormat = internalformat;

    const d3d11::TextureFormat &formatInfo = d3d11::GetTextureFormatInfo(internalformat, renderer->getRenderer11DeviceCaps());
    mTextureFormat = formatInfo.texFormat;
    mShaderResourceFormat = formatInfo.srvFormat;
    mDepthStencilFormat = formatInfo.dsvFormat;
    mRenderTargetFormat = formatInfo.rtvFormat;
    mSwizzleTextureFormat = formatInfo.swizzleTexFormat;
    mSwizzleShaderResourceFormat = formatInfo.swizzleSRVFormat;
    mSwizzleRenderTargetFormat = formatInfo.swizzleRTVFormat;

    // adjust size if needed for compressed textures
    d3d11::MakeValidSize(false, mTextureFormat, &width, &height, &mTopLevel);

    mMipLevels = mTopLevel + levels;
    mTextureWidth = width;
    mTextureHeight = height;
    mTextureDepth = depth;
}

TextureStorage11_2DArray::~TextureStorage11_2DArray()
{
    for (ImageMap::iterator i = mAssociatedImages.begin(); i != mAssociatedImages.end(); i++)
    {
        if (i->second)
        {
            bool imageAssociationCorrect = i->second->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // We must let the Images recover their data before we delete it from the TextureStorage.
                i->second->recoverFromAssociatedStorage();
            }
        }
    }
    mAssociatedImages.clear();

    SafeRelease(mTexture);
    SafeRelease(mSwizzleTexture);

    for (unsigned int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        SafeRelease(mSwizzleRenderTargets[level]);
    }

    for (RenderTargetMap::iterator i = mRenderTargets.begin(); i != mRenderTargets.end(); i++)
    {
        SafeDelete(i->second);
    }
    mRenderTargets.clear();
}

void TextureStorage11_2DArray::associateImage(Image11* image, const gl::ImageIndex &index)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    ASSERT(0 <= level && level < getLevelCount());

    if (0 <= level && level < getLevelCount())
    {
        LevelLayerKey key(level, layerTarget);
        mAssociatedImages[key] = image;
    }
}

bool TextureStorage11_2DArray::isAssociatedImageValid(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    LevelLayerKey key(level, layerTarget);

    // This validation check should never return false. It means the Image/TextureStorage association is broken.
    bool retValue = (mAssociatedImages.find(key) != mAssociatedImages.end() && (mAssociatedImages[key] == expectedImage));
    ASSERT(retValue);
    return retValue;
}

// disassociateImage allows an Image to end its association with a Storage.
void TextureStorage11_2DArray::disassociateImage(const gl::ImageIndex &index, Image11* expectedImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    LevelLayerKey key(level, layerTarget);

    bool imageAssociationCorrect = (mAssociatedImages.find(key) != mAssociatedImages.end() && (mAssociatedImages[key] == expectedImage));
    ASSERT(imageAssociationCorrect);

    if (imageAssociationCorrect)
    {
        mAssociatedImages[key] = NULL;
    }
}

// releaseAssociatedImage prepares the Storage for a new Image association. It lets the old Image recover its data before ending the association.
gl::Error TextureStorage11_2DArray::releaseAssociatedImage(const gl::ImageIndex &index, Image11* incomingImage)
{
    GLint level = index.mipIndex;
    GLint layerTarget = index.layerIndex;

    LevelLayerKey key(level, layerTarget);

    if (mAssociatedImages.find(key) != mAssociatedImages.end())
    {
        if (mAssociatedImages[key] != NULL && mAssociatedImages[key] != incomingImage)
        {
            // Ensure that the Image is still associated with this TextureStorage. This should be true.
            bool imageAssociationCorrect = mAssociatedImages[key]->isAssociatedStorageValid(this);
            ASSERT(imageAssociationCorrect);

            if (imageAssociationCorrect)
            {
                // Force the image to recover from storage before its data is overwritten.
                // This will reset mAssociatedImages[level] to NULL too.
                gl::Error error = mAssociatedImages[key]->recoverFromAssociatedStorage();
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2DArray::getResource(ID3D11Resource **outResource)
{
    // if the width, height or depth is not positive this should be treated as an incomplete texture
    // we handle that here by skipping the d3d texture creation
    if (mTexture == NULL && mTextureWidth > 0 && mTextureHeight > 0 && mTextureDepth > 0)
    {
        ASSERT(mMipLevels > 0);

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.MipLevels = mMipLevels;
        desc.ArraySize = mTextureDepth;
        desc.Format = mTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = getBindFlags();
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = getMiscFlags();

        HRESULT result = device->CreateTexture2D(&desc, NULL, &mTexture);

        // this can happen from windows TDR
        if (d3d11::isDeviceLostError(result))
        {
            mRenderer->notifyDeviceLost();
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 2D array texture storage, result: 0x%X.", result);
        }
        else if (FAILED(result))
        {
            ASSERT(result == E_OUTOFMEMORY);
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create 2D array texture storage, result: 0x%X.", result);
        }
    }

    *outResource = mTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2DArray::createSRV(int baseLevel, int mipLevels, DXGI_FORMAT format, ID3D11Resource *texture,
                                              ID3D11ShaderResourceView **outSRV) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
    srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + baseLevel;
    srvDesc.Texture2DArray.MipLevels = mipLevels;
    srvDesc.Texture2DArray.FirstArraySlice = 0;
    srvDesc.Texture2DArray.ArraySize = mTextureDepth;

    ID3D11Device *device = mRenderer->getDevice();
    HRESULT result = device->CreateShaderResourceView(texture, &srvDesc, outSRV);

    ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal texture storage SRV, result: 0x%X.", result);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2DArray::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    ASSERT(index.hasLayer());

    int mipLevel = index.mipIndex;
    int layer = index.layerIndex;

    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());

    LevelLayerKey key(mipLevel, layer);
    if (mRenderTargets.find(key) == mRenderTargets.end())
    {
        ID3D11Device *device = mRenderer->getDevice();
        HRESULT result;

        ID3D11Resource *texture = NULL;
        gl::Error error = getResource(&texture);
        if (error.isError())
        {
            return error;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        srvDesc.Format = mShaderResourceFormat;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = mTopLevel + mipLevel;
        srvDesc.Texture2DArray.MipLevels = 1;
        srvDesc.Texture2DArray.FirstArraySlice = layer;
        srvDesc.Texture2DArray.ArraySize = 1;

        ID3D11ShaderResourceView *srv;
        result = device->CreateShaderResourceView(texture, &srvDesc, &srv);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal shader resource view for texture storage, result: 0x%X.", result);
        }

        if (mRenderTargetFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
            rtvDesc.Format = mRenderTargetFormat;
            rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
            rtvDesc.Texture2DArray.MipSlice = mTopLevel + mipLevel;
            rtvDesc.Texture2DArray.FirstArraySlice = layer;
            rtvDesc.Texture2DArray.ArraySize = 1;

            ID3D11RenderTargetView *rtv;
            result = device->CreateRenderTargetView(texture, &rtvDesc, &rtv);

            ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
            if (FAILED(result))
            {
                SafeRelease(srv);
                return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal render target view for texture storage, result: 0x%X.", result);
            }

            mRenderTargets[key] = new TextureRenderTarget11(rtv, texture, srv, mInternalFormat, getLevelWidth(mipLevel), getLevelHeight(mipLevel), 1, 0);

            // RenderTarget will take ownership of these resources
            SafeRelease(rtv);
            SafeRelease(srv);
        }
        else
        {
            UNREACHABLE();
        }
    }

    ASSERT(outRT);
    *outRT = mRenderTargets[key];
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2DArray::getSwizzleTexture(ID3D11Resource **outTexture)
{
    if (!mSwizzleTexture)
    {
        ID3D11Device *device = mRenderer->getDevice();

        D3D11_TEXTURE2D_DESC desc;
        desc.Width = mTextureWidth;
        desc.Height = mTextureHeight;
        desc.MipLevels = mMipLevels;
        desc.ArraySize = mTextureDepth;
        desc.Format = mSwizzleTextureFormat;
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;

        HRESULT result = device->CreateTexture2D(&desc, NULL, &mSwizzleTexture);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle texture, result: 0x%X.", result);
        }
    }

    *outTexture = mSwizzleTexture;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureStorage11_2DArray::getSwizzleRenderTarget(int mipLevel, ID3D11RenderTargetView **outRTV)
{
    ASSERT(mipLevel >= 0 && mipLevel < getLevelCount());
    ASSERT(outRTV);

    if (!mSwizzleRenderTargets[mipLevel])
    {
        ID3D11Resource *swizzleTexture = NULL;
        gl::Error error = getSwizzleTexture(&swizzleTexture);
        if (error.isError())
        {
            return error;
        }

        ID3D11Device *device = mRenderer->getDevice();

        D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format = mSwizzleRenderTargetFormat;
        rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
        rtvDesc.Texture2DArray.MipSlice = mTopLevel + mipLevel;
        rtvDesc.Texture2DArray.FirstArraySlice = 0;
        rtvDesc.Texture2DArray.ArraySize = mTextureDepth;

        HRESULT result = device->CreateRenderTargetView(mSwizzleTexture, &rtvDesc, &mSwizzleRenderTargets[mipLevel]);

        ASSERT(result == E_OUTOFMEMORY || SUCCEEDED(result));
        if (FAILED(result))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to create internal swizzle render target view, result: 0x%X.", result);
        }
    }

    *outRTV = mSwizzleRenderTargets[mipLevel];
    return gl::Error(GL_NO_ERROR);
}

}
