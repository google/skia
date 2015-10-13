//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureD3D.cpp: Implementations of the Texture interfaces shared betweeen the D3D backends.

#include "libANGLE/renderer/d3d/TextureD3D.h"

#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Config.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Image.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/BufferImpl.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/EGLImageD3D.h"
#include "libANGLE/renderer/d3d/ImageD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/SurfaceD3D.h"
#include "libANGLE/renderer/d3d/TextureStorage.h"

namespace rx
{

namespace
{

gl::Error GetUnpackPointer(const gl::PixelUnpackState &unpack, const uint8_t *pixels,
                           ptrdiff_t layerOffset, const uint8_t **pointerOut)
{
    if (unpack.pixelBuffer.id() != 0)
    {
        // Do a CPU readback here, if we have an unpack buffer bound and the fast GPU path is not supported
        gl::Buffer *pixelBuffer = unpack.pixelBuffer.get();
        ptrdiff_t offset = reinterpret_cast<ptrdiff_t>(pixels);

        // TODO: this is the only place outside of renderer that asks for a buffers raw data.
        // This functionality should be moved into renderer and the getData method of BufferImpl removed.
        BufferD3D *bufferD3D = GetImplAs<BufferD3D>(pixelBuffer);
        ASSERT(bufferD3D);
        const uint8_t *bufferData = NULL;
        gl::Error error = bufferD3D->getData(&bufferData);
        if (error.isError())
        {
            return error;
        }

        *pointerOut = bufferData + offset;
    }
    else
    {
        *pointerOut = pixels;
    }

    // Offset the pointer for 2D array layer (if it's valid)
    if (*pointerOut != nullptr)
    {
        *pointerOut += layerOffset;
    }

    return gl::Error(GL_NO_ERROR);
}

bool IsRenderTargetUsage(GLenum usage)
{
    return (usage == GL_FRAMEBUFFER_ATTACHMENT_ANGLE);
}

}

TextureD3D::TextureD3D(RendererD3D *renderer)
    : mRenderer(renderer),
      mUsage(GL_NONE),
      mDirtyImages(true),
      mImmutable(false),
      mTexStorage(NULL)
{
}

TextureD3D::~TextureD3D()
{
}

gl::Error TextureD3D::getNativeTexture(TextureStorage **outStorage)
{
    // ensure the underlying texture is created
    gl::Error error = initializeStorage(false);
    if (error.isError())
    {
        return error;
    }

    if (mTexStorage)
    {
        error = updateStorage();
        if (error.isError())
        {
            return error;
        }
    }

    ASSERT(outStorage);

    *outStorage = mTexStorage;
    return gl::Error(GL_NO_ERROR);
}

GLint TextureD3D::getBaseLevelWidth() const
{
    const ImageD3D *baseImage = getBaseLevelImage();
    return (baseImage ? baseImage->getWidth() : 0);
}

GLint TextureD3D::getBaseLevelHeight() const
{
    const ImageD3D *baseImage = getBaseLevelImage();
    return (baseImage ? baseImage->getHeight() : 0);
}

GLint TextureD3D::getBaseLevelDepth() const
{
    const ImageD3D *baseImage = getBaseLevelImage();
    return (baseImage ? baseImage->getDepth() : 0);
}

// Note: "base level image" is loosely defined to be any image from the base level,
// where in the base of 2D array textures and cube maps there are several. Don't use
// the base level image for anything except querying texture format and size.
GLenum TextureD3D::getBaseLevelInternalFormat() const
{
    const ImageD3D *baseImage = getBaseLevelImage();
    return (baseImage ? baseImage->getInternalFormat() : GL_NONE);
}

bool TextureD3D::shouldUseSetData(const ImageD3D *image) const
{
    if (!mRenderer->getWorkarounds().setDataFasterThanImageUpload)
    {
        return false;
    }

    gl::InternalFormat internalFormat = gl::GetInternalFormatInfo(image->getInternalFormat());

    // We can only handle full updates for depth-stencil textures, so to avoid complications
    // disable them entirely.
    if (internalFormat.depthBits > 0 || internalFormat.stencilBits > 0)
    {
        return false;
    }

    // TODO(jmadill): Handle compressed internal formats
    return (mTexStorage && !internalFormat.compressed);
}

gl::Error TextureD3D::setImageImpl(const gl::ImageIndex &index,
                                   GLenum type,
                                   const gl::PixelUnpackState &unpack,
                                   const uint8_t *pixels,
                                   ptrdiff_t layerOffset)
{
    if (unpack.skipRows != 0 || unpack.skipPixels != 0 || unpack.imageHeight != 0 || unpack.skipImages != 0)
    {
        // TODO(jmadill): additional unpack parameters
        // return no error here to work around dEQP-GLES3 failing to state reset.
        UNIMPLEMENTED();
        return gl::Error(GL_NO_ERROR);
    }

    ImageD3D *image = getImage(index);
    ASSERT(image);

    // No-op
    if (image->getWidth() == 0 || image->getHeight() == 0 || image->getDepth() == 0)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // We no longer need the "GLenum format" parameter to TexImage to determine what data format "pixels" contains.
    // From our image internal format we know how many channels to expect, and "type" gives the format of pixel's components.
    const uint8_t *pixelData = NULL;
    gl::Error error = GetUnpackPointer(unpack, pixels, layerOffset, &pixelData);
    if (error.isError())
    {
        return error;
    }

    if (pixelData != nullptr)
    {
        if (shouldUseSetData(image))
        {
            error = mTexStorage->setData(index, image, NULL, type, unpack, pixelData);
        }
        else
        {
            gl::Box fullImageArea(0, 0, 0, image->getWidth(), image->getHeight(), image->getDepth());
            error = image->loadData(fullImageArea, unpack, type, pixelData);
        }

        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D::subImage(const gl::ImageIndex &index, const gl::Box &area, GLenum format, GLenum type,
                               const gl::PixelUnpackState &unpack, const uint8_t *pixels, ptrdiff_t layerOffset)
{
    // CPU readback & copy where direct GPU copy is not supported
    const uint8_t *pixelData = NULL;
    gl::Error error = GetUnpackPointer(unpack, pixels, layerOffset, &pixelData);
    if (error.isError())
    {
        return error;
    }

    if (pixelData != NULL)
    {
        ImageD3D *image = getImage(index);
        ASSERT(image);

        if (shouldUseSetData(image))
        {
            return mTexStorage->setData(index, image, &area, type, unpack, pixelData);
        }

        error = image->loadData(area, unpack, type, pixelData);
        if (error.isError())
        {
            return error;
        }

        error = commitRegion(index, area);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D::setCompressedImageImpl(const gl::ImageIndex &index,
                                             const gl::PixelUnpackState &unpack,
                                             const uint8_t *pixels,
                                             ptrdiff_t layerOffset)
{
    if (unpack.skipRows != 0 || unpack.skipPixels != 0 || unpack.imageHeight != 0 || unpack.skipImages != 0)
    {
        UNIMPLEMENTED();
        return gl::Error(GL_INVALID_OPERATION, "unimplemented pixel store state");
    }

    // We no longer need the "GLenum format" parameter to TexImage to determine what data format "pixels" contains.
    // From our image internal format we know how many channels to expect, and "type" gives the format of pixel's components.
    const uint8_t *pixelData = NULL;
    gl::Error error = GetUnpackPointer(unpack, pixels, layerOffset, &pixelData);
    if (error.isError())
    {
        return error;
    }

    if (pixelData != NULL)
    {
        ImageD3D *image = getImage(index);
        ASSERT(image);

        gl::Box fullImageArea(0, 0, 0, image->getWidth(), image->getHeight(), image->getDepth());
        error = image->loadCompressedData(fullImageArea, pixelData);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D::subImageCompressed(const gl::ImageIndex &index, const gl::Box &area, GLenum format,
                                         const gl::PixelUnpackState &unpack, const uint8_t *pixels,
                                         ptrdiff_t layerOffset)
{
    if (unpack.skipRows != 0 || unpack.skipPixels != 0 || unpack.imageHeight != 0 || unpack.skipImages != 0)
    {
        UNIMPLEMENTED();
        return gl::Error(GL_INVALID_OPERATION, "unimplemented pixel store state");
    }

    const uint8_t *pixelData = NULL;
    gl::Error error = GetUnpackPointer(unpack, pixels, layerOffset, &pixelData);
    if (error.isError())
    {
        return error;
    }

    if (pixelData != NULL)
    {
        ImageD3D *image = getImage(index);
        ASSERT(image);

        error = image->loadCompressedData(area, pixelData);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D::isFastUnpackable(const gl::PixelUnpackState &unpack, GLenum sizedInternalFormat)
{
    return unpack.pixelBuffer.id() != 0 && mRenderer->supportsFastCopyBufferToTexture(sizedInternalFormat);
}

gl::Error TextureD3D::fastUnpackPixels(const gl::PixelUnpackState &unpack, const uint8_t *pixels, const gl::Box &destArea,
                                       GLenum sizedInternalFormat, GLenum type, RenderTargetD3D *destRenderTarget)
{
    // No-op
    if (destArea.width <= 0 && destArea.height <= 0 && destArea.depth <= 0)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // In order to perform the fast copy through the shader, we must have the right format, and be able
    // to create a render target.
    ASSERT(mRenderer->supportsFastCopyBufferToTexture(sizedInternalFormat));

    uintptr_t offset = reinterpret_cast<uintptr_t>(pixels);

    gl::Error error = mRenderer->fastCopyBufferToTexture(unpack, static_cast<unsigned int>(offset), destRenderTarget, sizedInternalFormat, type, destArea);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

GLint TextureD3D::creationLevels(GLsizei width, GLsizei height, GLsizei depth) const
{
    if ((gl::isPow2(width) && gl::isPow2(height) && gl::isPow2(depth)) || mRenderer->getRendererExtensions().textureNPOT)
    {
        // Maximum number of levels
        return gl::log2(std::max(std::max(width, height), depth)) + 1;
    }
    else
    {
        // OpenGL ES 2.0 without GL_OES_texture_npot does not permit NPOT mipmaps.
        return 1;
    }
}

int TextureD3D::mipLevels() const
{
    return gl::log2(std::max(std::max(getBaseLevelWidth(), getBaseLevelHeight()), getBaseLevelDepth())) + 1;
}

TextureStorage *TextureD3D::getStorage()
{
    ASSERT(mTexStorage);
    return mTexStorage;
}

ImageD3D *TextureD3D::getBaseLevelImage() const
{
    return getImage(getImageIndex(0, 0));
}

gl::Error TextureD3D::generateMipmaps(const gl::TextureState &textureState)
{
    GLint mipCount = mipLevels();

    if (mipCount == 1)
    {
        return gl::Error(GL_NO_ERROR); // no-op
    }

    if (mTexStorage && mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        // Switch to using the mipmapped texture.
        TextureStorage *textureStorage = NULL;
        gl::Error error = getNativeTexture(&textureStorage);
        if (error.isError())
        {
            return error;
        }

        error = textureStorage->useLevelZeroWorkaroundTexture(false);
        if (error.isError())
        {
            return error;
        }
    }

    // Set up proper mipmap chain in our Image array.
    initMipmapsImages();

    if (mTexStorage && mTexStorage->supportsNativeMipmapFunction())
    {
        gl::Error error = updateStorage();
        if (error.isError())
        {
            return error;
        }

        // Generate the mipmap chain using the ad-hoc DirectX function.
        error = mRenderer->generateMipmapsUsingD3D(mTexStorage, textureState);
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        // Generate the mipmap chain, one level at a time.
        gl::Error error = generateMipmapsUsingImages();
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D::generateMipmapsUsingImages()
{
    GLint mipCount = mipLevels();

    // We know that all layers have the same dimension, for the texture to be complete
    GLint layerCount = static_cast<GLint>(getLayerCount(0));

    // When making mipmaps with the setData workaround enabled, the texture storage has
    // the image data already. For non-render-target storage, we have to pull it out into
    // an image layer.
    if (mRenderer->getWorkarounds().setDataFasterThanImageUpload && mTexStorage)
    {
        if (!mTexStorage->isRenderTarget())
        {
            // Copy from the storage mip 0 to Image mip 0
            for (GLint layer = 0; layer < layerCount; ++layer)
            {
                gl::ImageIndex srcIndex = getImageIndex(0, layer);

                ImageD3D *image = getImage(srcIndex);
                gl::Box area(0, 0, 0, image->getWidth(), image->getHeight(), image->getDepth());
                gl::Offset offset(0, 0, 0);
                gl::Error error = image->copy(offset, area, srcIndex, mTexStorage);
                if (error.isError())
                {
                    return error;
                }
            }
        }
        else
        {
            gl::Error error = updateStorage();
            if (error.isError())
            {
                return error;
            }
        }
    }

    // TODO: Decouple this from zeroMaxLodWorkaround. This is a 9_3 restriction, unrelated to zeroMaxLodWorkaround.
    // The restriction is because Feature Level 9_3 can't create SRVs on individual levels of the texture.
    // As a result, even if the storage is a rendertarget, we can't use the GPU to generate the mipmaps without further work.
    // The D3D9 renderer works around this by copying each level of the texture into its own single-layer GPU texture (in Blit9::boxFilter).
    // Feature Level 9_3 could do something similar, or it could continue to use CPU-side mipmap generation, or something else.
    bool renderableStorage = (mTexStorage && mTexStorage->isRenderTarget() && !(mRenderer->getWorkarounds().zeroMaxLodWorkaround));

    for (GLint layer = 0; layer < layerCount; ++layer)
    {
        for (GLint mip = 1; mip < mipCount; ++mip)
        {
            ASSERT(getLayerCount(mip) == layerCount);

            gl::ImageIndex sourceIndex = getImageIndex(mip - 1, layer);
            gl::ImageIndex destIndex = getImageIndex(mip, layer);

            if (renderableStorage)
            {
                // GPU-side mipmapping
                gl::Error error = mTexStorage->generateMipmap(sourceIndex, destIndex);
                if (error.isError())
                {
                    return error;
                }
            }
            else
            {
                // CPU-side mipmapping
                gl::Error error = mRenderer->generateMipmap(getImage(destIndex), getImage(sourceIndex));
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    if (mTexStorage)
    {
        updateStorage();
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D::isBaseImageZeroSize() const
{
    ImageD3D *baseImage = getBaseLevelImage();

    if (!baseImage || baseImage->getWidth() <= 0)
    {
        return true;
    }

    if (!gl::IsCubeMapTextureTarget(baseImage->getTarget()) && baseImage->getHeight() <= 0)
    {
        return true;
    }

    if (baseImage->getTarget() == GL_TEXTURE_3D && baseImage->getDepth() <= 0)
    {
        return true;
    }

    if (baseImage->getTarget() == GL_TEXTURE_2D_ARRAY && getLayerCount(0) <= 0)
    {
        return true;
    }

    return false;
}

gl::Error TextureD3D::ensureRenderTarget()
{
    gl::Error error = initializeStorage(true);
    if (error.isError())
    {
        return error;
    }

    if (!isBaseImageZeroSize())
    {
        ASSERT(mTexStorage);
        if (!mTexStorage->isRenderTarget())
        {
            TextureStorage *newRenderTargetStorage = NULL;
            error = createCompleteStorage(true, &newRenderTargetStorage);
            if (error.isError())
            {
                return error;
            }

            error = mTexStorage->copyToStorage(newRenderTargetStorage);
            if (error.isError())
            {
                SafeDelete(newRenderTargetStorage);
                return error;
            }

            error = setCompleteTexStorage(newRenderTargetStorage);
            if (error.isError())
            {
                SafeDelete(newRenderTargetStorage);
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D::canCreateRenderTargetForImage(const gl::ImageIndex &index) const
{
    ImageD3D *image = getImage(index);
    bool levelsComplete = (isImageComplete(index) && isImageComplete(getImageIndex(0, 0)));
    return (image->isRenderableFormat() && levelsComplete);
}

gl::Error TextureD3D::commitRegion(const gl::ImageIndex &index, const gl::Box &region)
{
    if (mTexStorage)
    {
        ASSERT(isValidIndex(index));
        ImageD3D *image = getImage(index);
        gl::Error error = image->copyToStorage(mTexStorage, index, region);
        if (error.isError())
        {
            return error;
        }

        image->markClean();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                                FramebufferAttachmentRenderTarget **rtOut)
{
    RenderTargetD3D *rtD3D = nullptr;
    gl::Error error = getRenderTarget(target.textureIndex(), &rtD3D);
    *rtOut = static_cast<FramebufferAttachmentRenderTarget *>(rtD3D);
    return error;
}

TextureD3D_2D::TextureD3D_2D(RendererD3D *renderer)
    : TextureD3D(renderer)
{
    mEGLImageTarget = false;
    for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++i)
    {
        mImageArray[i] = renderer->createImage();
    }
}

TextureD3D_2D::~TextureD3D_2D()
{
    // Delete the Images before the TextureStorage.
    // Images might be relying on the TextureStorage for some of their data.
    // If TextureStorage is deleted before the Images, then their data will be wastefully copied back from the GPU before we delete the Images.
    for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++i)
    {
        delete mImageArray[i];
    }

    SafeDelete(mTexStorage);
}

ImageD3D *TextureD3D_2D::getImage(int level, int layer) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(layer == 0);
    return mImageArray[level];
}

ImageD3D *TextureD3D_2D::getImage(const gl::ImageIndex &index) const
{
    ASSERT(index.mipIndex < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(!index.hasLayer());
    ASSERT(index.type == GL_TEXTURE_2D);
    return mImageArray[index.mipIndex];
}

GLsizei TextureD3D_2D::getLayerCount(int level) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    return 1;
}

GLsizei TextureD3D_2D::getWidth(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getWidth();
    else
        return 0;
}

GLsizei TextureD3D_2D::getHeight(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getHeight();
    else
        return 0;
}

GLenum TextureD3D_2D::getInternalFormat(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getInternalFormat();
    else
        return GL_NONE;
}

bool TextureD3D_2D::isDepth(GLint level) const
{
    return gl::GetInternalFormatInfo(getInternalFormat(level)).depthBits > 0;
}

gl::Error TextureD3D_2D::setImage(GLenum target,
                                  size_t imageLevel,
                                  GLenum internalFormat,
                                  const gl::Extents &size,
                                  GLenum format,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D && size.depth == 1);

    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, type);

    bool fastUnpacked = false;
    GLint level       = static_cast<GLint>(imageLevel);

    redefineImage(level, sizedInternalFormat, size, false);

    gl::ImageIndex index = gl::ImageIndex::Make2D(level);

    // Attempt a fast gpu copy of the pixel data to the surface
    if (isFastUnpackable(unpack, sizedInternalFormat) && isLevelComplete(level))
    {
        // Will try to create RT storage if it does not exist
        RenderTargetD3D *destRenderTarget = NULL;
        gl::Error error = getRenderTarget(index, &destRenderTarget);
        if (error.isError())
        {
            return error;
        }

        gl::Box destArea(0, 0, 0, getWidth(level), getHeight(level), 1);

        error = fastUnpackPixels(unpack, pixels, destArea, sizedInternalFormat, type, destRenderTarget);
        if (error.isError())
        {
            return error;
        }

        // Ensure we don't overwrite our newly initialized data
        mImageArray[level]->markClean();

        fastUnpacked = true;
    }

    if (!fastUnpacked)
    {
        gl::Error error = setImageImpl(index, type, unpack, pixels, 0);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::setSubImage(GLenum target,
                                     size_t imageLevel,
                                     const gl::Box &area,
                                     GLenum format,
                                     GLenum type,
                                     const gl::PixelUnpackState &unpack,
                                     const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D && area.depth == 1 && area.z == 0);

    if (unpack.skipRows != 0 || unpack.skipPixels != 0 || unpack.imageHeight != 0 || unpack.skipImages != 0)
    {
        UNIMPLEMENTED();
        return gl::Error(GL_INVALID_OPERATION, "unimplemented pixel store state");
    }

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::Make2D(level);
    if (isFastUnpackable(unpack, getInternalFormat(level)) && isLevelComplete(level))
    {
        RenderTargetD3D *renderTarget = NULL;
        gl::Error error = getRenderTarget(index, &renderTarget);
        if (error.isError())
        {
            return error;
        }

        ASSERT(!mImageArray[level]->isDirty());

        return fastUnpackPixels(unpack, pixels, area, getInternalFormat(level), type, renderTarget);
    }
    else
    {
        return TextureD3D::subImage(index, area, format, type, unpack, pixels, 0);
    }
}

gl::Error TextureD3D_2D::setCompressedImage(GLenum target,
                                            size_t imageLevel,
                                            GLenum internalFormat,
                                            const gl::Extents &size,
                                            const gl::PixelUnpackState &unpack,
                                            size_t imageSize,
                                            const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D && size.depth == 1);
    GLint level = static_cast<GLint>(imageLevel);

    // compressed formats don't have separate sized internal formats-- we can just use the compressed format directly
    redefineImage(level, internalFormat, size, false);

    return setCompressedImageImpl(gl::ImageIndex::Make2D(level), unpack, pixels, 0);
}

gl::Error TextureD3D_2D::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                               const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D && area.depth == 1 && area.z == 0);

    gl::ImageIndex index = gl::ImageIndex::Make2D(static_cast<GLint>(level));
    gl::Error error = TextureD3D::subImageCompressed(index, area, format, unpack, pixels, 0);
    if (error.isError())
    {
        return error;
    }

    return commitRegion(index, area);
}

gl::Error TextureD3D_2D::copyImage(GLenum target,
                                   size_t imageLevel,
                                   const gl::Rectangle &sourceArea,
                                   GLenum internalFormat,
                                   const gl::Framebuffer *source)
{
    ASSERT(target == GL_TEXTURE_2D);

    GLint level                = static_cast<GLint>(imageLevel);
    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, GL_UNSIGNED_BYTE);
    redefineImage(level, sizedInternalFormat, gl::Extents(sourceArea.width, sourceArea.height, 1),
                  false);

    gl::ImageIndex index = gl::ImageIndex::Make2D(level);
    gl::Offset destOffset(0, 0, 0);

    // If the zero max LOD workaround is active, then we can't sample from individual layers of the framebuffer in shaders,
    // so we should use the non-rendering copy path.
    if (!canCreateRenderTargetForImage(index) || mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        gl::Error error = mImageArray[level]->copy(destOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        mImageArray[level]->markClean();

        if (sourceArea.width != 0 && sourceArea.height != 0 && isValidLevel(level))
        {
            error = mRenderer->copyImage2D(source, sourceArea, internalFormat, destOffset, mTexStorage, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::copySubImage(GLenum target,
                                      size_t imageLevel,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      const gl::Framebuffer *source)
{
    ASSERT(target == GL_TEXTURE_2D && destOffset.z == 0);

    // can only make our texture storage to a render target if level 0 is defined (with a width & height) and
    // the current level we're copying to is defined (with appropriate format, width & height)

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::Make2D(level);

    // If the zero max LOD workaround is active, then we can't sample from individual layers of the framebuffer in shaders,
    // so we should use the non-rendering copy path.
    if (!canCreateRenderTargetForImage(index) || mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        gl::Error error = mImageArray[level]->copy(destOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        if (isValidLevel(level))
        {
            error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }

            error = mRenderer->copyImage2D(source, sourceArea,
                                           gl::GetInternalFormatInfo(getBaseLevelInternalFormat()).format,
                                           destOffset, mTexStorage, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    ASSERT(GL_TEXTURE_2D && size.depth == 1);

    for (size_t level = 0; level < levels; level++)
    {
        gl::Extents levelSize(std::max(1, size.width >> level),
                              std::max(1, size.height >> level),
                              1);
        redefineImage(level, internalFormat, levelSize, true);
    }

    for (size_t level = levels; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        redefineImage(level, GL_NONE, gl::Extents(0, 0, 1), true);
    }

    // TODO(geofflang): Verify storage creation had no errors
    bool renderTarget = IsRenderTargetUsage(mUsage);
    TextureStorage *storage = mRenderer->createTextureStorage2D(
        internalFormat, renderTarget, size.width, size.height, static_cast<int>(levels), false);

    gl::Error error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    error = updateStorage();

    if (error.isError())
    {
        return error;
    }

    mImmutable = true;

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_2D::bindTexImage(egl::Surface *surface)
{
    GLenum internalformat = surface->getConfig()->renderTargetFormat;

    gl::Extents size(surface->getWidth(), surface->getHeight(), 1);
    redefineImage(0, internalformat, size, true);

    if (mTexStorage)
    {
        SafeDelete(mTexStorage);
    }

    SurfaceD3D *surfaceD3D = GetImplAs<SurfaceD3D>(surface);
    ASSERT(surfaceD3D);

    mTexStorage = mRenderer->createTextureStorage2D(surfaceD3D->getSwapChain());
    mEGLImageTarget = false;

    mDirtyImages = true;
}

void TextureD3D_2D::releaseTexImage()
{
    if (mTexStorage)
    {
        SafeDelete(mTexStorage);
    }

    for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
    {
        redefineImage(i, GL_NONE, gl::Extents(0, 0, 1), true);
    }
}

gl::Error TextureD3D_2D::setEGLImageTarget(GLenum target, egl::Image *image)
{
    EGLImageD3D *eglImaged3d = GetImplAs<EGLImageD3D>(image);

    // Set the properties of the base mip level from the EGL image
    GLenum internalformat = image->getInternalFormat();
    gl::Extents size(static_cast<int>(image->getWidth()), static_cast<int>(image->getHeight()), 1);
    redefineImage(0, internalformat, size, true);

    // Clear all other images.
    for (size_t level = 1; level < ArraySize(mImageArray); level++)
    {
        redefineImage(level, GL_NONE, gl::Extents(0, 0, 1), true);
    }

    SafeDelete(mTexStorage);
    mImageArray[0]->markClean();

    mTexStorage     = mRenderer->createTextureStorageEGLImage(eglImaged3d);
    mEGLImageTarget = true;

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_2D::initMipmapsImages()
{
    // Purge array levels 1 through q and reset them to represent the generated mipmap levels.
    int levelCount = mipLevels();
    for (int level = 1; level < levelCount; level++)
    {
        gl::Extents levelSize(std::max(getBaseLevelWidth() >> level, 1),
                              std::max(getBaseLevelHeight() >> level, 1),
                              1);

        redefineImage(level, getBaseLevelInternalFormat(), levelSize, false);
    }
}

gl::Error TextureD3D_2D::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    ASSERT(!index.hasLayer());

    // ensure the underlying texture is created
    gl::Error error = ensureRenderTarget();
    if (error.isError())
    {
        return error;
    }

    error = updateStorageLevel(index.mipIndex);
    if (error.isError())
    {
        return error;
    }

    return mTexStorage->getRenderTarget(index, outRT);
}

bool TextureD3D_2D::isValidLevel(int level) const
{
    return (mTexStorage ? (level >= 0 && level < mTexStorage->getLevelCount()) : false);
}

bool TextureD3D_2D::isLevelComplete(int level) const
{
    if (isImmutable())
    {
        return true;
    }

    const ImageD3D *baseImage = getBaseLevelImage();

    GLsizei width = baseImage->getWidth();
    GLsizei height = baseImage->getHeight();

    if (width <= 0 || height <= 0)
    {
        return false;
    }

    // The base image level is complete if the width and height are positive
    if (level == 0)
    {
        return true;
    }

    ASSERT(level >= 1 && level <= (int)ArraySize(mImageArray) && mImageArray[level] != NULL);
    ImageD3D *image = mImageArray[level];

    if (image->getInternalFormat() != baseImage->getInternalFormat())
    {
        return false;
    }

    if (image->getWidth() != std::max(1, width >> level))
    {
        return false;
    }

    if (image->getHeight() != std::max(1, height >> level))
    {
        return false;
    }

    return true;
}

bool TextureD3D_2D::isImageComplete(const gl::ImageIndex &index) const
{
    return isLevelComplete(index.mipIndex);
}

// Constructs a native texture resource from the texture images
gl::Error TextureD3D_2D::initializeStorage(bool renderTarget)
{
    // Only initialize the first time this texture is used as a render target or shader resource
    if (mTexStorage)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // do not attempt to create storage for nonexistant data
    if (!isLevelComplete(0))
    {
        return gl::Error(GL_NO_ERROR);
    }

    bool createRenderTarget = (renderTarget || IsRenderTargetUsage(mUsage));

    TextureStorage *storage = NULL;
    gl::Error error = createCompleteStorage(createRenderTarget, &storage);
    if (error.isError())
    {
        return error;
    }

    error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    ASSERT(mTexStorage);

    // flush image data to the storage
    error = updateStorage();
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::createCompleteStorage(bool renderTarget, TextureStorage **outTexStorage) const
{
    GLsizei width = getBaseLevelWidth();
    GLsizei height = getBaseLevelHeight();
    GLenum internalFormat = getBaseLevelInternalFormat();

    ASSERT(width > 0 && height > 0);

    // use existing storage level count, when previously specified by TexStorage*D
    GLint levels = (mTexStorage ? mTexStorage->getLevelCount() : creationLevels(width, height, 1));

    bool hintLevelZeroOnly = false;
    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        // If any of the CPU images (levels >= 1) are dirty, then the textureStorage2D should use the mipped texture to begin with.
        // Otherwise, it should use the level-zero-only texture.
        hintLevelZeroOnly = true;
        for (int level = 1; level < levels && hintLevelZeroOnly; level++)
        {
            hintLevelZeroOnly = !(mImageArray[level]->isDirty() && isLevelComplete(level));
        }
    }

    // TODO(geofflang): Determine if the texture creation succeeded
    *outTexStorage = mRenderer->createTextureStorage2D(internalFormat, renderTarget, width, height, levels, hintLevelZeroOnly);

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::setCompleteTexStorage(TextureStorage *newCompleteTexStorage)
{
    if (newCompleteTexStorage && newCompleteTexStorage->isManaged())
    {
        for (int level = 0; level < newCompleteTexStorage->getLevelCount(); level++)
        {
            gl::Error error = mImageArray[level]->setManagedSurface2D(newCompleteTexStorage, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    SafeDelete(mTexStorage);
    mTexStorage = newCompleteTexStorage;

    mDirtyImages = true;

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::updateStorage()
{
    ASSERT(mTexStorage != NULL);
    GLint storageLevels = mTexStorage->getLevelCount();
    for (int level = 0; level < storageLevels; level++)
    {
        if (mImageArray[level]->isDirty() && isLevelComplete(level))
        {
            gl::Error error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2D::updateStorageLevel(int level)
{
    ASSERT(level <= (int)ArraySize(mImageArray) && mImageArray[level] != NULL);
    ASSERT(isLevelComplete(level));

    if (mImageArray[level]->isDirty())
    {
        gl::ImageIndex index = gl::ImageIndex::Make2D(level);
        gl::Box region(0, 0, 0, getWidth(level), getHeight(level), 1);
        gl::Error error = commitRegion(index, region);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_2D::redefineImage(size_t level,
                                  GLenum internalformat,
                                  const gl::Extents &size,
                                  bool forceRelease)
{
    ASSERT(size.depth == 1);

    // If there currently is a corresponding storage texture image, it has these parameters
    const int storageWidth = std::max(1, getBaseLevelWidth() >> level);
    const int storageHeight = std::max(1, getBaseLevelHeight() >> level);
    const GLenum storageFormat = getBaseLevelInternalFormat();

    mImageArray[level]->redefine(GL_TEXTURE_2D, internalformat, size, forceRelease);

    if (mTexStorage)
    {
        const size_t storageLevels = mTexStorage->getLevelCount();

        // If the storage was from an EGL image, copy it back into local images to preserve it
        // while orphaning
        if (level != 0 && mEGLImageTarget)
        {
            gl::Offset offset(0, 0, 0);
            gl::Rectangle sourceArea(0, 0, mImageArray[0]->getWidth(), mImageArray[0]->getHeight());

            RenderTargetD3D *storageRendertarget = nullptr;
            gl::Error error =
                mTexStorage->getRenderTarget(gl::ImageIndex::Make2D(0), &storageRendertarget);
            if (!error.isError())
            {
                mImageArray[0]->copy(offset, sourceArea, storageRendertarget);
            }
        }

        if ((level >= storageLevels && storageLevels != 0) ||
            size.width != storageWidth ||
            size.height != storageHeight ||
            internalformat != storageFormat)   // Discard mismatched storage
        {
            for (size_t i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
            {
                mImageArray[i]->markDirty();
            }

            SafeDelete(mTexStorage);
            mDirtyImages = true;
        }
    }

    // Can't be an EGL image target after being redefined
    mEGLImageTarget = false;
}

gl::ImageIndexIterator TextureD3D_2D::imageIterator() const
{
    return gl::ImageIndexIterator::Make2D(0, mTexStorage->getLevelCount());
}

gl::ImageIndex TextureD3D_2D::getImageIndex(GLint mip, GLint /*layer*/) const
{
    // "layer" does not apply to 2D Textures.
    return gl::ImageIndex::Make2D(mip);
}

bool TextureD3D_2D::isValidIndex(const gl::ImageIndex &index) const
{
    return (mTexStorage && index.type == GL_TEXTURE_2D &&
            index.mipIndex >= 0 && index.mipIndex < mTexStorage->getLevelCount());
}

TextureD3D_Cube::TextureD3D_Cube(RendererD3D *renderer)
    : TextureD3D(renderer)
{
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++j)
        {
            mImageArray[i][j] = renderer->createImage();
        }
    }
}

TextureD3D_Cube::~TextureD3D_Cube()
{
    // Delete the Images before the TextureStorage.
    // Images might be relying on the TextureStorage for some of their data.
    // If TextureStorage is deleted before the Images, then their data will be wastefully copied back from the GPU before we delete the Images.
    for (int i = 0; i < 6; i++)
    {
        for (int j = 0; j < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++j)
        {
            SafeDelete(mImageArray[i][j]);
        }
    }

    SafeDelete(mTexStorage);
}

ImageD3D *TextureD3D_Cube::getImage(int level, int layer) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(layer >= 0 && layer < 6);
    return mImageArray[layer][level];
}

ImageD3D *TextureD3D_Cube::getImage(const gl::ImageIndex &index) const
{
    ASSERT(index.mipIndex < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(index.layerIndex >= 0 && index.layerIndex < 6);
    return mImageArray[index.layerIndex][index.mipIndex];
}

GLsizei TextureD3D_Cube::getLayerCount(int level) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    return 6;
}

GLenum TextureD3D_Cube::getInternalFormat(GLint level, GLint layer) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[layer][level]->getInternalFormat();
    else
        return GL_NONE;
}

bool TextureD3D_Cube::isDepth(GLint level, GLint layer) const
{
    return gl::GetInternalFormatInfo(getInternalFormat(level, layer)).depthBits > 0;
}

gl::Error TextureD3D_Cube::setEGLImageTarget(GLenum target, egl::Image *image)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureD3D_Cube::setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                                    const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    ASSERT(size.depth == 1);

    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, type);
    gl::ImageIndex index       = gl::ImageIndex::MakeCube(target, static_cast<GLint>(level));

    redefineImage(index.layerIndex, static_cast<GLint>(level), sizedInternalFormat, size);

    return setImageImpl(index, type, unpack, pixels, 0);
}

gl::Error TextureD3D_Cube::setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                                       const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    ASSERT(area.depth == 1 && area.z == 0);

    gl::ImageIndex index = gl::ImageIndex::MakeCube(target, static_cast<GLint>(level));
    return TextureD3D::subImage(index, area, format, type, unpack, pixels, 0);
}

gl::Error TextureD3D_Cube::setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                              const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels)
{
    ASSERT(size.depth == 1);

    // compressed formats don't have separate sized internal formats-- we can just use the compressed format directly
    size_t faceIndex = gl::CubeMapTextureTargetToLayerIndex(target);

    redefineImage(static_cast<int>(faceIndex), static_cast<GLint>(level), internalFormat, size);

    gl::ImageIndex index = gl::ImageIndex::MakeCube(target, static_cast<GLint>(level));
    return setCompressedImageImpl(index, unpack, pixels, 0);
}

gl::Error TextureD3D_Cube::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                                 const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels)
{
    ASSERT(area.depth == 1 && area.z == 0);

    gl::ImageIndex index = gl::ImageIndex::MakeCube(target, static_cast<GLint>(level));

    gl::Error error = TextureD3D::subImageCompressed(index, area, format, unpack, pixels, 0);
    if (error.isError())
    {
        return error;
    }

    return commitRegion(index, area);
}

gl::Error TextureD3D_Cube::copyImage(GLenum target,
                                     size_t imageLevel,
                                     const gl::Rectangle &sourceArea,
                                     GLenum internalFormat,
                                     const gl::Framebuffer *source)
{
    int faceIndex              = static_cast<int>(gl::CubeMapTextureTargetToLayerIndex(target));
    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, GL_UNSIGNED_BYTE);

    GLint level = static_cast<GLint>(imageLevel);

    gl::Extents size(sourceArea.width, sourceArea.height, 1);
    redefineImage(static_cast<int>(faceIndex), level, sizedInternalFormat, size);

    gl::ImageIndex index = gl::ImageIndex::MakeCube(target, level);
    gl::Offset destOffset(0, 0, 0);

    // If the zero max LOD workaround is active, then we can't sample from individual layers of the framebuffer in shaders,
    // so we should use the non-rendering copy path.
    if (!canCreateRenderTargetForImage(index) || mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        gl::Error error = mImageArray[faceIndex][level]->copy(destOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        mImageArray[faceIndex][level]->markClean();

        ASSERT(size.width == size.height);

        if (size.width > 0 && isValidFaceLevel(faceIndex, level))
        {
            error = mRenderer->copyImageCube(source, sourceArea, internalFormat, destOffset, mTexStorage, target, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_Cube::copySubImage(GLenum target,
                                        size_t imageLevel,
                                        const gl::Offset &destOffset,
                                        const gl::Rectangle &sourceArea,
                                        const gl::Framebuffer *source)
{
    int faceIndex = static_cast<int>(gl::CubeMapTextureTargetToLayerIndex(target));

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::MakeCube(target, level);

    // If the zero max LOD workaround is active, then we can't sample from individual layers of the framebuffer in shaders,
    // so we should use the non-rendering copy path.
    if (!canCreateRenderTargetForImage(index) || mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        gl::Error error = mImageArray[faceIndex][level]->copy(destOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        if (isValidFaceLevel(faceIndex, level))
        {
            error = updateStorageFaceLevel(faceIndex, level);
            if (error.isError())
            {
                return error;
            }

            error = mRenderer->copyImageCube(source, sourceArea, gl::GetInternalFormatInfo(getBaseLevelInternalFormat()).format,
                                             destOffset, mTexStorage, target, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_Cube::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    ASSERT(size.width == size.height);
    ASSERT(size.depth == 1);

    for (size_t level = 0; level < levels; level++)
    {
        GLsizei mipSize = std::max(1, size.width >> level);
        for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
            mImageArray[faceIndex][level]->redefine(GL_TEXTURE_CUBE_MAP, internalFormat, gl::Extents(mipSize, mipSize, 1), true);
        }
    }

    for (size_t level = levels; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
            mImageArray[faceIndex][level]->redefine(GL_TEXTURE_CUBE_MAP, GL_NONE, gl::Extents(0, 0, 0), true);
        }
    }

    // TODO(geofflang): Verify storage creation had no errors
    bool renderTarget = IsRenderTargetUsage(mUsage);

    TextureStorage *storage = mRenderer->createTextureStorageCube(
        internalFormat, renderTarget, size.width, static_cast<int>(levels), false);

    gl::Error error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    error = updateStorage();

    if (error.isError())
    {
        return error;
    }

    mImmutable = true;

    return gl::Error(GL_NO_ERROR);
}

// Tests for cube texture completeness. [OpenGL ES 2.0.24] section 3.7.10 page 81.
bool TextureD3D_Cube::isCubeComplete() const
{
    int    baseWidth  = getBaseLevelWidth();
    int    baseHeight = getBaseLevelHeight();
    GLenum baseFormat = getBaseLevelInternalFormat();

    if (baseWidth <= 0 || baseWidth != baseHeight)
    {
        return false;
    }

    for (int faceIndex = 1; faceIndex < 6; faceIndex++)
    {
        const ImageD3D &faceBaseImage = *mImageArray[faceIndex][0];

        if (faceBaseImage.getWidth()          != baseWidth  ||
            faceBaseImage.getHeight()         != baseHeight ||
            faceBaseImage.getInternalFormat() != baseFormat )
        {
            return false;
        }
    }

    return true;
}

void TextureD3D_Cube::bindTexImage(egl::Surface *surface)
{
    UNREACHABLE();
}

void TextureD3D_Cube::releaseTexImage()
{
    UNREACHABLE();
}


void TextureD3D_Cube::initMipmapsImages()
{
    // Purge array levels 1 through q and reset them to represent the generated mipmap levels.
    int levelCount = mipLevels();
    for (int faceIndex = 0; faceIndex < 6; faceIndex++)
    {
        for (int level = 1; level < levelCount; level++)
        {
            int faceLevelSize = (std::max(mImageArray[faceIndex][0]->getWidth() >> level, 1));
            redefineImage(faceIndex, level, mImageArray[faceIndex][0]->getInternalFormat(),
                          gl::Extents(faceLevelSize, faceLevelSize, 1));
        }
    }
}

gl::Error TextureD3D_Cube::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    ASSERT(gl::IsCubeMapTextureTarget(index.type));

    // ensure the underlying texture is created
    gl::Error error = ensureRenderTarget();
    if (error.isError())
    {
        return error;
    }

    error = updateStorageFaceLevel(index.layerIndex, index.mipIndex);
    if (error.isError())
    {
        return error;
    }

    return mTexStorage->getRenderTarget(index, outRT);
}

gl::Error TextureD3D_Cube::initializeStorage(bool renderTarget)
{
    // Only initialize the first time this texture is used as a render target or shader resource
    if (mTexStorage)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // do not attempt to create storage for nonexistant data
    if (!isFaceLevelComplete(0, 0))
    {
        return gl::Error(GL_NO_ERROR);
    }

    bool createRenderTarget = (renderTarget || IsRenderTargetUsage(mUsage));

    TextureStorage *storage = NULL;
    gl::Error error = createCompleteStorage(createRenderTarget, &storage);
    if (error.isError())
    {
        return error;
    }

    error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    ASSERT(mTexStorage);

    // flush image data to the storage
    error = updateStorage();
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_Cube::createCompleteStorage(bool renderTarget, TextureStorage **outTexStorage) const
{
    GLsizei size = getBaseLevelWidth();

    ASSERT(size > 0);

    // use existing storage level count, when previously specified by TexStorage*D
    GLint levels = (mTexStorage ? mTexStorage->getLevelCount() : creationLevels(size, size, 1));

    bool hintLevelZeroOnly = false;
    if (mRenderer->getWorkarounds().zeroMaxLodWorkaround)
    {
        // If any of the CPU images (levels >= 1) are dirty, then the textureStorage should use the mipped texture to begin with.
        // Otherwise, it should use the level-zero-only texture.
        hintLevelZeroOnly = true;
        for (int faceIndex = 0; faceIndex < 6 && hintLevelZeroOnly; faceIndex++)
        {
            for (int level = 1; level < levels && hintLevelZeroOnly; level++)
            {
                hintLevelZeroOnly = !(mImageArray[faceIndex][level]->isDirty() && isFaceLevelComplete(faceIndex, level));
            }
        }
    }

    // TODO (geofflang): detect if storage creation succeeded
    *outTexStorage = mRenderer->createTextureStorageCube(getBaseLevelInternalFormat(), renderTarget, size, levels, hintLevelZeroOnly);

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_Cube::setCompleteTexStorage(TextureStorage *newCompleteTexStorage)
{
    if (newCompleteTexStorage && newCompleteTexStorage->isManaged())
    {
        for (int faceIndex = 0; faceIndex < 6; faceIndex++)
        {
            for (int level = 0; level < newCompleteTexStorage->getLevelCount(); level++)
            {
                gl::Error error = mImageArray[faceIndex][level]->setManagedSurfaceCube(newCompleteTexStorage, faceIndex, level);
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    SafeDelete(mTexStorage);
    mTexStorage = newCompleteTexStorage;

    mDirtyImages = true;
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_Cube::updateStorage()
{
    ASSERT(mTexStorage != NULL);
    GLint storageLevels = mTexStorage->getLevelCount();
    for (int face = 0; face < 6; face++)
    {
        for (int level = 0; level < storageLevels; level++)
        {
            if (mImageArray[face][level]->isDirty() && isFaceLevelComplete(face, level))
            {
                gl::Error error = updateStorageFaceLevel(face, level);
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D_Cube::isValidFaceLevel(int faceIndex, int level) const
{
    return (mTexStorage ? (level >= 0 && level < mTexStorage->getLevelCount()) : 0);
}

bool TextureD3D_Cube::isFaceLevelComplete(int faceIndex, int level) const
{
    ASSERT(level >= 0 && faceIndex < 6 && level < (int)ArraySize(mImageArray[faceIndex]) && mImageArray[faceIndex][level] != NULL);

    if (isImmutable())
    {
        return true;
    }

    int baseSize = getBaseLevelWidth();

    if (baseSize <= 0)
    {
        return false;
    }

    // "isCubeComplete" checks for base level completeness and we must call that
    // to determine if any face at level 0 is complete. We omit that check here
    // to avoid re-checking cube-completeness for every face at level 0.
    if (level == 0)
    {
        return true;
    }

    // Check that non-zero levels are consistent with the base level.
    const ImageD3D *faceLevelImage = mImageArray[faceIndex][level];

    if (faceLevelImage->getInternalFormat() != getBaseLevelInternalFormat())
    {
        return false;
    }

    if (faceLevelImage->getWidth() != std::max(1, baseSize >> level))
    {
        return false;
    }

    return true;
}

bool TextureD3D_Cube::isImageComplete(const gl::ImageIndex &index) const
{
    return isFaceLevelComplete(index.layerIndex, index.mipIndex);
}

gl::Error TextureD3D_Cube::updateStorageFaceLevel(int faceIndex, int level)
{
    ASSERT(level >= 0 && faceIndex < 6 && level < (int)ArraySize(mImageArray[faceIndex]) && mImageArray[faceIndex][level] != NULL);
    ImageD3D *image = mImageArray[faceIndex][level];

    if (image->isDirty())
    {
        GLenum faceTarget = gl::LayerIndexToCubeMapTextureTarget(faceIndex);
        gl::ImageIndex index = gl::ImageIndex::MakeCube(faceTarget, level);
        gl::Box region(0, 0, 0, image->getWidth(), image->getHeight(), 1);
        gl::Error error = commitRegion(index, region);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_Cube::redefineImage(int faceIndex, GLint level, GLenum internalformat, const gl::Extents &size)
{
    // If there currently is a corresponding storage texture image, it has these parameters
    const int storageWidth = std::max(1, getBaseLevelWidth() >> level);
    const int storageHeight = std::max(1, getBaseLevelHeight() >> level);
    const GLenum storageFormat = getBaseLevelInternalFormat();

    mImageArray[faceIndex][level]->redefine(GL_TEXTURE_CUBE_MAP, internalformat, size, false);

    if (mTexStorage)
    {
        const int storageLevels = mTexStorage->getLevelCount();

        if ((level >= storageLevels && storageLevels != 0) ||
            size.width != storageWidth ||
            size.height != storageHeight ||
            internalformat != storageFormat)   // Discard mismatched storage
        {
            for (int dirtyLevel = 0; dirtyLevel < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; dirtyLevel++)
            {
                for (int dirtyFace = 0; dirtyFace < 6; dirtyFace++)
                {
                    mImageArray[dirtyFace][dirtyLevel]->markDirty();
                }
            }

            SafeDelete(mTexStorage);

            mDirtyImages = true;
        }
    }
}

gl::ImageIndexIterator TextureD3D_Cube::imageIterator() const
{
    return gl::ImageIndexIterator::MakeCube(0, mTexStorage->getLevelCount());
}

gl::ImageIndex TextureD3D_Cube::getImageIndex(GLint mip, GLint layer) const
{
    // The "layer" of the image index corresponds to the cube face
    return gl::ImageIndex::MakeCube(gl::LayerIndexToCubeMapTextureTarget(layer), mip);
}

bool TextureD3D_Cube::isValidIndex(const gl::ImageIndex &index) const
{
    return (mTexStorage && gl::IsCubeMapTextureTarget(index.type) &&
            index.mipIndex >= 0 && index.mipIndex < mTexStorage->getLevelCount());
}

TextureD3D_3D::TextureD3D_3D(RendererD3D *renderer)
    : TextureD3D(renderer)
{
    for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++i)
    {
        mImageArray[i] = renderer->createImage();
    }
}

TextureD3D_3D::~TextureD3D_3D()
{
    // Delete the Images before the TextureStorage.
    // Images might be relying on the TextureStorage for some of their data.
    // If TextureStorage is deleted before the Images, then their data will be wastefully copied back from the GPU before we delete the Images.
    for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++i)
    {
        delete mImageArray[i];
    }

    SafeDelete(mTexStorage);
}

ImageD3D *TextureD3D_3D::getImage(int level, int layer) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(layer == 0);
    return mImageArray[level];
}

ImageD3D *TextureD3D_3D::getImage(const gl::ImageIndex &index) const
{
    ASSERT(index.mipIndex < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT(!index.hasLayer());
    ASSERT(index.type == GL_TEXTURE_3D);
    return mImageArray[index.mipIndex];
}

GLsizei TextureD3D_3D::getLayerCount(int level) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    return 1;
}

GLsizei TextureD3D_3D::getWidth(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getWidth();
    else
        return 0;
}

GLsizei TextureD3D_3D::getHeight(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getHeight();
    else
        return 0;
}

GLsizei TextureD3D_3D::getDepth(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getDepth();
    else
        return 0;
}

GLenum TextureD3D_3D::getInternalFormat(GLint level) const
{
    if (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS)
        return mImageArray[level]->getInternalFormat();
    else
        return GL_NONE;
}

bool TextureD3D_3D::isDepth(GLint level) const
{
    return gl::GetInternalFormatInfo(getInternalFormat(level)).depthBits > 0;
}

gl::Error TextureD3D_3D::setEGLImageTarget(GLenum target, egl::Image *image)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureD3D_3D::setImage(GLenum target,
                                  size_t imageLevel,
                                  GLenum internalFormat,
                                  const gl::Extents &size,
                                  GLenum format,
                                  GLenum type,
                                  const gl::PixelUnpackState &unpack,
                                  const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_3D);
    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, type);

    GLint level = static_cast<GLint>(imageLevel);
    redefineImage(level, sizedInternalFormat, size);

    bool fastUnpacked = false;

    gl::ImageIndex index = gl::ImageIndex::Make3D(level);

    // Attempt a fast gpu copy of the pixel data to the surface if the app bound an unpack buffer
    if (isFastUnpackable(unpack, sizedInternalFormat) && !size.empty())
    {
        // Will try to create RT storage if it does not exist
        RenderTargetD3D *destRenderTarget = NULL;
        gl::Error error = getRenderTarget(index, &destRenderTarget);
        if (error.isError())
        {
            return error;
        }

        gl::Box destArea(0, 0, 0, getWidth(level), getHeight(level), getDepth(level));

        error = fastUnpackPixels(unpack, pixels, destArea, sizedInternalFormat, type, destRenderTarget);
        if (error.isError())
        {
            return error;
        }

        // Ensure we don't overwrite our newly initialized data
        mImageArray[level]->markClean();

        fastUnpacked = true;
    }

    if (!fastUnpacked)
    {
        gl::Error error = setImageImpl(index, type, unpack, pixels, 0);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_3D::setSubImage(GLenum target,
                                     size_t imageLevel,
                                     const gl::Box &area,
                                     GLenum format,
                                     GLenum type,
                                     const gl::PixelUnpackState &unpack,
                                     const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_3D);

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::Make3D(level);

    // Attempt a fast gpu copy of the pixel data to the surface if the app bound an unpack buffer
    if (isFastUnpackable(unpack, getInternalFormat(level)))
    {
        RenderTargetD3D *destRenderTarget = NULL;
        gl::Error error = getRenderTarget(index, &destRenderTarget);
        if (error.isError())
        {
            return error;
        }

        ASSERT(!mImageArray[level]->isDirty());

        return fastUnpackPixels(unpack, pixels, area, getInternalFormat(level), type, destRenderTarget);
    }
    else
    {
        return TextureD3D::subImage(index, area, format, type, unpack, pixels, 0);
    }
}

gl::Error TextureD3D_3D::setCompressedImage(GLenum target,
                                            size_t imageLevel,
                                            GLenum internalFormat,
                                            const gl::Extents &size,
                                            const gl::PixelUnpackState &unpack,
                                            size_t imageSize,
                                            const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_3D);

    GLint level = static_cast<GLint>(imageLevel);
    // compressed formats don't have separate sized internal formats-- we can just use the compressed format directly
    redefineImage(level, internalFormat, size);

    gl::ImageIndex index = gl::ImageIndex::Make3D(level);
    return setCompressedImageImpl(index, unpack, pixels, 0);
}

gl::Error TextureD3D_3D::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                               const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_3D);

    gl::ImageIndex index = gl::ImageIndex::Make3D(static_cast<GLint>(level));
    gl::Error error = TextureD3D::subImageCompressed(index, area, format, unpack, pixels, 0);
    if (error.isError())
    {
        return error;
    }

    return commitRegion(index, area);
}

gl::Error TextureD3D_3D::copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                                   const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION, "Copying 3D textures is unimplemented.");
}

gl::Error TextureD3D_3D::copySubImage(GLenum target,
                                      size_t imageLevel,
                                      const gl::Offset &destOffset,
                                      const gl::Rectangle &sourceArea,
                                      const gl::Framebuffer *source)
{
    ASSERT(target == GL_TEXTURE_3D);

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::Make3D(level);

    if (canCreateRenderTargetForImage(index))
    {
        gl::Error error = mImageArray[level]->copy(destOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        if (isValidLevel(level))
        {
            error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }

            error = mRenderer->copyImage3D(source, sourceArea,
                                           gl::GetInternalFormatInfo(getBaseLevelInternalFormat()).format,
                                           destOffset, mTexStorage, level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_3D::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    ASSERT(target == GL_TEXTURE_3D);

    for (size_t level = 0; level < levels; level++)
    {
        gl::Extents levelSize(std::max(1, size.width >> level),
                              std::max(1, size.height >> level),
                              std::max(1, size.depth >> level));
        mImageArray[level]->redefine(GL_TEXTURE_3D, internalFormat, levelSize, true);
    }

    for (size_t level = levels; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        mImageArray[level]->redefine(GL_TEXTURE_3D, GL_NONE, gl::Extents(0, 0, 0), true);
    }

    // TODO(geofflang): Verify storage creation had no errors
    bool renderTarget = IsRenderTargetUsage(mUsage);
    TextureStorage *storage =
        mRenderer->createTextureStorage3D(internalFormat, renderTarget, size.width, size.height,
                                          size.depth, static_cast<int>(levels));

    gl::Error error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    error = updateStorage();

    if (error.isError())
    {
        return error;
    }

    mImmutable = true;

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_3D::bindTexImage(egl::Surface *surface)
{
    UNREACHABLE();
}

void TextureD3D_3D::releaseTexImage()
{
    UNREACHABLE();
}


void TextureD3D_3D::initMipmapsImages()
{
    // Purge array levels 1 through q and reset them to represent the generated mipmap levels.
    int levelCount = mipLevels();
    for (int level = 1; level < levelCount; level++)
    {
        gl::Extents levelSize(std::max(getBaseLevelWidth() >> level, 1),
                              std::max(getBaseLevelHeight() >> level, 1),
                              std::max(getBaseLevelDepth() >> level, 1));
        redefineImage(level, getBaseLevelInternalFormat(), levelSize);
    }
}

gl::Error TextureD3D_3D::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    // ensure the underlying texture is created
    gl::Error error = ensureRenderTarget();
    if (error.isError())
    {
        return error;
    }

    if (index.hasLayer())
    {
        error = updateStorage();
        if (error.isError())
        {
            return error;
        }
    }
    else
    {
        error = updateStorageLevel(index.mipIndex);
        if (error.isError())
        {
            return error;
        }
    }

    return mTexStorage->getRenderTarget(index, outRT);
}

gl::Error TextureD3D_3D::initializeStorage(bool renderTarget)
{
    // Only initialize the first time this texture is used as a render target or shader resource
    if (mTexStorage)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // do not attempt to create storage for nonexistant data
    if (!isLevelComplete(0))
    {
        return gl::Error(GL_NO_ERROR);
    }

    bool createRenderTarget = (renderTarget || IsRenderTargetUsage(mUsage));

    TextureStorage *storage = NULL;
    gl::Error error = createCompleteStorage(createRenderTarget, &storage);
    if (error.isError())
    {
        return error;
    }

    error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    ASSERT(mTexStorage);

    // flush image data to the storage
    error = updateStorage();
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_3D::createCompleteStorage(bool renderTarget, TextureStorage **outStorage) const
{
    GLsizei width = getBaseLevelWidth();
    GLsizei height = getBaseLevelHeight();
    GLsizei depth = getBaseLevelDepth();
    GLenum internalFormat = getBaseLevelInternalFormat();

    ASSERT(width > 0 && height > 0 && depth > 0);

    // use existing storage level count, when previously specified by TexStorage*D
    GLint levels = (mTexStorage ? mTexStorage->getLevelCount() : creationLevels(width, height, depth));

    // TODO: Verify creation of the storage succeeded
    *outStorage = mRenderer->createTextureStorage3D(internalFormat, renderTarget, width, height, depth, levels);

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_3D::setCompleteTexStorage(TextureStorage *newCompleteTexStorage)
{
    SafeDelete(mTexStorage);
    mTexStorage = newCompleteTexStorage;
    mDirtyImages = true;

    // We do not support managed 3D storage, as that is D3D9/ES2-only
    ASSERT(!mTexStorage->isManaged());

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_3D::updateStorage()
{
    ASSERT(mTexStorage != NULL);
    GLint storageLevels = mTexStorage->getLevelCount();
    for (int level = 0; level < storageLevels; level++)
    {
        if (mImageArray[level]->isDirty() && isLevelComplete(level))
        {
            gl::Error error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D_3D::isValidLevel(int level) const
{
    return (mTexStorage ? (level >= 0 && level < mTexStorage->getLevelCount()) : 0);
}

bool TextureD3D_3D::isLevelComplete(int level) const
{
    ASSERT(level >= 0 && level < (int)ArraySize(mImageArray) && mImageArray[level] != NULL);

    if (isImmutable())
    {
        return true;
    }

    GLsizei width = getBaseLevelWidth();
    GLsizei height = getBaseLevelHeight();
    GLsizei depth = getBaseLevelDepth();

    if (width <= 0 || height <= 0 || depth <= 0)
    {
        return false;
    }

    if (level == 0)
    {
        return true;
    }

    ImageD3D *levelImage = mImageArray[level];

    if (levelImage->getInternalFormat() != getBaseLevelInternalFormat())
    {
        return false;
    }

    if (levelImage->getWidth() != std::max(1, width >> level))
    {
        return false;
    }

    if (levelImage->getHeight() != std::max(1, height >> level))
    {
        return false;
    }

    if (levelImage->getDepth() != std::max(1, depth >> level))
    {
        return false;
    }

    return true;
}

bool TextureD3D_3D::isImageComplete(const gl::ImageIndex &index) const
{
    return isLevelComplete(index.mipIndex);
}

gl::Error TextureD3D_3D::updateStorageLevel(int level)
{
    ASSERT(level >= 0 && level < (int)ArraySize(mImageArray) && mImageArray[level] != NULL);
    ASSERT(isLevelComplete(level));

    if (mImageArray[level]->isDirty())
    {
        gl::ImageIndex index = gl::ImageIndex::Make3D(level);
        gl::Box region(0, 0, 0, getWidth(level), getHeight(level), getDepth(level));
        gl::Error error = commitRegion(index, region);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_3D::redefineImage(GLint level, GLenum internalformat, const gl::Extents &size)
{
    // If there currently is a corresponding storage texture image, it has these parameters
    const int storageWidth = std::max(1, getBaseLevelWidth() >> level);
    const int storageHeight = std::max(1, getBaseLevelHeight() >> level);
    const int storageDepth = std::max(1, getBaseLevelDepth() >> level);
    const GLenum storageFormat = getBaseLevelInternalFormat();

    mImageArray[level]->redefine(GL_TEXTURE_3D, internalformat, size, false);

    if (mTexStorage)
    {
        const int storageLevels = mTexStorage->getLevelCount();

        if ((level >= storageLevels && storageLevels != 0) ||
            size.width != storageWidth ||
            size.height != storageHeight ||
            size.depth != storageDepth ||
            internalformat != storageFormat)   // Discard mismatched storage
        {
            for (int i = 0; i < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; i++)
            {
                mImageArray[i]->markDirty();
            }

            SafeDelete(mTexStorage);
            mDirtyImages = true;
        }
    }
}

gl::ImageIndexIterator TextureD3D_3D::imageIterator() const
{
    return gl::ImageIndexIterator::Make3D(0, mTexStorage->getLevelCount(),
                                          gl::ImageIndex::ENTIRE_LEVEL, gl::ImageIndex::ENTIRE_LEVEL);
}

gl::ImageIndex TextureD3D_3D::getImageIndex(GLint mip, GLint /*layer*/) const
{
    // The "layer" here does not apply to 3D images. We use one Image per mip.
    return gl::ImageIndex::Make3D(mip);
}

bool TextureD3D_3D::isValidIndex(const gl::ImageIndex &index) const
{
    return (mTexStorage && index.type == GL_TEXTURE_3D &&
            index.mipIndex >= 0 && index.mipIndex < mTexStorage->getLevelCount());
}

TextureD3D_2DArray::TextureD3D_2DArray(RendererD3D *renderer)
    : TextureD3D(renderer)
{
    for (int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++level)
    {
        mLayerCounts[level] = 0;
        mImageArray[level] = NULL;
    }
}

TextureD3D_2DArray::~TextureD3D_2DArray()
{
    // Delete the Images before the TextureStorage.
    // Images might be relying on the TextureStorage for some of their data.
    // If TextureStorage is deleted before the Images, then their data will be wastefully copied back from the GPU before we delete the Images.
    deleteImages();
    SafeDelete(mTexStorage);
}

ImageD3D *TextureD3D_2DArray::getImage(int level, int layer) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT((layer == 0 && mLayerCounts[level] == 0) ||
           layer < mLayerCounts[level]);
    return (mImageArray[level] ? mImageArray[level][layer] : NULL);
}

ImageD3D *TextureD3D_2DArray::getImage(const gl::ImageIndex &index) const
{
    ASSERT(index.mipIndex < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    ASSERT((index.layerIndex == 0 && mLayerCounts[index.mipIndex] == 0) ||
           index.layerIndex < mLayerCounts[index.mipIndex]);
    ASSERT(index.type == GL_TEXTURE_2D_ARRAY);
    return (mImageArray[index.mipIndex] ? mImageArray[index.mipIndex][index.layerIndex] : NULL);
}

GLsizei TextureD3D_2DArray::getLayerCount(int level) const
{
    ASSERT(level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS);
    return mLayerCounts[level];
}

GLsizei TextureD3D_2DArray::getWidth(GLint level) const
{
    return (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS && mLayerCounts[level] > 0) ? mImageArray[level][0]->getWidth() : 0;
}

GLsizei TextureD3D_2DArray::getHeight(GLint level) const
{
    return (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS && mLayerCounts[level] > 0) ? mImageArray[level][0]->getHeight() : 0;
}

GLenum TextureD3D_2DArray::getInternalFormat(GLint level) const
{
    return (level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS && mLayerCounts[level] > 0) ? mImageArray[level][0]->getInternalFormat() : GL_NONE;
}

bool TextureD3D_2DArray::isDepth(GLint level) const
{
    return gl::GetInternalFormatInfo(getInternalFormat(level)).depthBits > 0;
}

gl::Error TextureD3D_2DArray::setEGLImageTarget(GLenum target, egl::Image *image)
{
    UNREACHABLE();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureD3D_2DArray::setImage(GLenum target,
                                       size_t imageLevel,
                                       GLenum internalFormat,
                                       const gl::Extents &size,
                                       GLenum format,
                                       GLenum type,
                                       const gl::PixelUnpackState &unpack,
                                       const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);

    GLenum sizedInternalFormat = gl::GetSizedInternalFormat(internalFormat, type);

    GLint level = static_cast<GLint>(imageLevel);
    redefineImage(level, sizedInternalFormat, size);

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(sizedInternalFormat);
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(type, size.width, size.height, unpack.alignment, unpack.rowLength);

    for (int i = 0; i < size.depth; i++)
    {
        const ptrdiff_t layerOffset = (inputDepthPitch * i);
        gl::ImageIndex index = gl::ImageIndex::Make2DArray(level, i);
        gl::Error error = setImageImpl(index, type, unpack, pixels, layerOffset);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::setSubImage(GLenum target,
                                          size_t imageLevel,
                                          const gl::Box &area,
                                          GLenum format,
                                          GLenum type,
                                          const gl::PixelUnpackState &unpack,
                                          const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);
    GLint level                          = static_cast<GLint>(imageLevel);
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(getInternalFormat(level));
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(type, area.width, area.height, unpack.alignment, unpack.rowLength);

    for (int i = 0; i < area.depth; i++)
    {
        int layer = area.z + i;
        const ptrdiff_t layerOffset = (inputDepthPitch * i);

        gl::Box layerArea(area.x, area.y, 0, area.width, area.height, 1);

        gl::ImageIndex index = gl::ImageIndex::Make2DArray(level, layer);
        gl::Error error = TextureD3D::subImage(index, layerArea, format, type, unpack, pixels, layerOffset);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::setCompressedImage(GLenum target,
                                                 size_t imageLevel,
                                                 GLenum internalFormat,
                                                 const gl::Extents &size,
                                                 const gl::PixelUnpackState &unpack,
                                                 size_t imageSize,
                                                 const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);

    GLint level = static_cast<GLint>(imageLevel);
    // compressed formats don't have separate sized internal formats-- we can just use the compressed format directly
    redefineImage(level, internalFormat, size);

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(GL_UNSIGNED_BYTE, size.width, size.height, 1, 0);

    for (int i = 0; i < size.depth; i++)
    {
        const ptrdiff_t layerOffset = (inputDepthPitch * i);

        gl::ImageIndex index = gl::ImageIndex::Make2DArray(level, i);
        gl::Error error = setCompressedImageImpl(index, unpack, pixels, layerOffset);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                                    const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(format);
    GLsizei inputDepthPitch = formatInfo.computeDepthPitch(GL_UNSIGNED_BYTE, area.width, area.height, 1, 0);

    for (int i = 0; i < area.depth; i++)
    {
        int layer = area.z + i;
        const ptrdiff_t layerOffset = (inputDepthPitch * i);

        gl::Box layerArea(area.x, area.y, 0, area.width, area.height, 1);

        gl::ImageIndex index = gl::ImageIndex::Make2DArray(static_cast<GLint>(level), layer);
        gl::Error error = TextureD3D::subImageCompressed(index, layerArea, format, unpack, pixels, layerOffset);
        if (error.isError())
        {
            return error;
        }

        error = commitRegion(index, layerArea);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                                        const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION, "Copying 2D array textures is unimplemented.");
}

gl::Error TextureD3D_2DArray::copySubImage(GLenum target,
                                           size_t imageLevel,
                                           const gl::Offset &destOffset,
                                           const gl::Rectangle &sourceArea,
                                           const gl::Framebuffer *source)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);

    GLint level          = static_cast<GLint>(imageLevel);
    gl::ImageIndex index = gl::ImageIndex::Make2DArray(level, destOffset.z);

    if (canCreateRenderTargetForImage(index))
    {
        gl::Offset destLayerOffset(destOffset.x, destOffset.y, 0);
        gl::Error error = mImageArray[level][destOffset.z]->copy(destLayerOffset, sourceArea, source);
        if (error.isError())
        {
            return error;
        }

        mDirtyImages = true;
    }
    else
    {
        gl::Error error = ensureRenderTarget();
        if (error.isError())
        {
            return error;
        }

        if (isValidLevel(level))
        {
            error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }

            error = mRenderer->copyImage2DArray(source, sourceArea, gl::GetInternalFormatInfo(getInternalFormat(0)).format,
                                                destOffset, mTexStorage, level);
            if (error.isError())
            {
                return error;
            }
        }
    }
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    ASSERT(target == GL_TEXTURE_2D_ARRAY);

    deleteImages();

    for (size_t level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; level++)
    {
        gl::Extents levelLayerSize(std::max(1, size.width >> level),
                                   std::max(1, size.height >> level),
                                   1);

        mLayerCounts[level] = (level < levels ? size.depth : 0);

        if (mLayerCounts[level] > 0)
        {
            // Create new images for this level
            mImageArray[level] = new ImageD3D*[mLayerCounts[level]];

            for (int layer = 0; layer < mLayerCounts[level]; layer++)
            {
                mImageArray[level][layer] = mRenderer->createImage();
                mImageArray[level][layer]->redefine(GL_TEXTURE_2D_ARRAY, internalFormat, levelLayerSize, true);
            }
        }
    }

    // TODO(geofflang): Verify storage creation had no errors
    bool renderTarget = IsRenderTargetUsage(mUsage);
    TextureStorage *storage =
        mRenderer->createTextureStorage2DArray(internalFormat, renderTarget, size.width,
                                               size.height, size.depth, static_cast<int>(levels));

    gl::Error error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    error = updateStorage();

    if (error.isError())
    {
        return error;
    }

    mImmutable = true;

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_2DArray::bindTexImage(egl::Surface *surface)
{
    UNREACHABLE();
}

void TextureD3D_2DArray::releaseTexImage()
{
    UNREACHABLE();
}


void TextureD3D_2DArray::initMipmapsImages()
{
    int baseWidth = getBaseLevelWidth();
    int baseHeight = getBaseLevelHeight();
    int baseDepth = getLayerCount(0);
    GLenum baseFormat = getBaseLevelInternalFormat();

    // Purge array levels 1 through q and reset them to represent the generated mipmap levels.
    int levelCount = mipLevels();
    for (int level = 1; level < levelCount; level++)
    {
        gl::Extents levelLayerSize(std::max(baseWidth >> level, 1),
                                   std::max(baseHeight >> level, 1),
                                   baseDepth);
        redefineImage(level, baseFormat, levelLayerSize);
    }
}

gl::Error TextureD3D_2DArray::getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT)
{
    // ensure the underlying texture is created
    gl::Error error = ensureRenderTarget();
    if (error.isError())
    {
        return error;
    }

    error = updateStorageLevel(index.mipIndex);
    if (error.isError())
    {
        return error;
    }

    return mTexStorage->getRenderTarget(index, outRT);
}

gl::Error TextureD3D_2DArray::initializeStorage(bool renderTarget)
{
    // Only initialize the first time this texture is used as a render target or shader resource
    if (mTexStorage)
    {
        return gl::Error(GL_NO_ERROR);
    }

    // do not attempt to create storage for nonexistant data
    if (!isLevelComplete(0))
    {
        return gl::Error(GL_NO_ERROR);
    }

    bool createRenderTarget = (renderTarget || IsRenderTargetUsage(mUsage));

    TextureStorage *storage = NULL;
    gl::Error error = createCompleteStorage(createRenderTarget, &storage);
    if (error.isError())
    {
        return error;
    }

    error = setCompleteTexStorage(storage);
    if (error.isError())
    {
        SafeDelete(storage);
        return error;
    }

    ASSERT(mTexStorage);

    // flush image data to the storage
    error = updateStorage();
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::createCompleteStorage(bool renderTarget, TextureStorage **outStorage) const
{
    GLsizei width = getBaseLevelWidth();
    GLsizei height = getBaseLevelHeight();
    GLsizei depth = getLayerCount(0);
    GLenum internalFormat = getBaseLevelInternalFormat();

    ASSERT(width > 0 && height > 0 && depth > 0);

    // use existing storage level count, when previously specified by TexStorage*D
    GLint levels = (mTexStorage ? mTexStorage->getLevelCount() : creationLevels(width, height, 1));

    // TODO(geofflang): Verify storage creation succeeds
    *outStorage = mRenderer->createTextureStorage2DArray(internalFormat, renderTarget, width, height, depth, levels);

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::setCompleteTexStorage(TextureStorage *newCompleteTexStorage)
{
    SafeDelete(mTexStorage);
    mTexStorage = newCompleteTexStorage;
    mDirtyImages = true;

    // We do not support managed 2D array storage, as managed storage is ES2/D3D9 only
    ASSERT(!mTexStorage->isManaged());

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureD3D_2DArray::updateStorage()
{
    ASSERT(mTexStorage != NULL);
    GLint storageLevels = mTexStorage->getLevelCount();
    for (int level = 0; level < storageLevels; level++)
    {
        if (isLevelComplete(level))
        {
            gl::Error error = updateStorageLevel(level);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

bool TextureD3D_2DArray::isValidLevel(int level) const
{
    return (mTexStorage ? (level >= 0 && level < mTexStorage->getLevelCount()) : 0);
}

bool TextureD3D_2DArray::isLevelComplete(int level) const
{
    ASSERT(level >= 0 && level < (int)ArraySize(mImageArray));

    if (isImmutable())
    {
        return true;
    }

    GLsizei width = getBaseLevelWidth();
    GLsizei height = getBaseLevelHeight();
    GLsizei layers = getLayerCount(0);

    if (width <= 0 || height <= 0 || layers <= 0)
    {
        return false;
    }

    if (level == 0)
    {
        return true;
    }

    if (getInternalFormat(level) != getInternalFormat(0))
    {
        return false;
    }

    if (getWidth(level) != std::max(1, width >> level))
    {
        return false;
    }

    if (getHeight(level) != std::max(1, height >> level))
    {
        return false;
    }

    if (getLayerCount(level) != layers)
    {
        return false;
    }

    return true;
}

bool TextureD3D_2DArray::isImageComplete(const gl::ImageIndex &index) const
{
    return isLevelComplete(index.mipIndex);
}

gl::Error TextureD3D_2DArray::updateStorageLevel(int level)
{
    ASSERT(level >= 0 && level < (int)ArraySize(mLayerCounts));
    ASSERT(isLevelComplete(level));

    for (int layer = 0; layer < mLayerCounts[level]; layer++)
    {
        ASSERT(mImageArray[level] != NULL && mImageArray[level][layer] != NULL);
        if (mImageArray[level][layer]->isDirty())
        {
            gl::ImageIndex index = gl::ImageIndex::Make2DArray(level, layer);
            gl::Box region(0, 0, 0, getWidth(level), getHeight(level), 1);
            gl::Error error = commitRegion(index, region);
            if (error.isError())
            {
                return error;
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

void TextureD3D_2DArray::deleteImages()
{
    for (int level = 0; level < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; ++level)
    {
        for (int layer = 0; layer < mLayerCounts[level]; ++layer)
        {
            delete mImageArray[level][layer];
        }
        delete[] mImageArray[level];
        mImageArray[level] = NULL;
        mLayerCounts[level] = 0;
    }
}

void TextureD3D_2DArray::redefineImage(GLint level, GLenum internalformat, const gl::Extents &size)
{
    // If there currently is a corresponding storage texture image, it has these parameters
    const int storageWidth = std::max(1, getBaseLevelWidth() >> level);
    const int storageHeight = std::max(1, getBaseLevelHeight() >> level);
    const int storageDepth = getLayerCount(0);
    const GLenum storageFormat = getBaseLevelInternalFormat();

    // Only reallocate the layers if the size doesn't match
    if (size.depth != mLayerCounts[level])
    {
        for (int layer = 0; layer < mLayerCounts[level]; layer++)
        {
            SafeDelete(mImageArray[level][layer]);
        }
        SafeDeleteArray(mImageArray[level]);
        mLayerCounts[level] = size.depth;

        if (size.depth > 0)
        {
            mImageArray[level] = new ImageD3D*[size.depth];
            for (int layer = 0; layer < mLayerCounts[level]; layer++)
            {
                mImageArray[level][layer] = mRenderer->createImage();
            }
        }
    }

    if (size.depth > 0)
    {
        for (int layer = 0; layer < mLayerCounts[level]; layer++)
        {
            mImageArray[level][layer]->redefine(GL_TEXTURE_2D_ARRAY, internalformat,
                                                gl::Extents(size.width, size.height, 1), false);
        }
    }

    if (mTexStorage)
    {
        const int storageLevels = mTexStorage->getLevelCount();

        if ((level >= storageLevels && storageLevels != 0) ||
            size.width != storageWidth ||
            size.height != storageHeight ||
            size.depth != storageDepth ||
            internalformat != storageFormat)   // Discard mismatched storage
        {
            for (int dirtyLevel = 0; dirtyLevel < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS; dirtyLevel++)
            {
                for (int dirtyLayer = 0; dirtyLayer < mLayerCounts[dirtyLevel]; dirtyLayer++)
                {
                    mImageArray[dirtyLevel][dirtyLayer]->markDirty();
                }
            }

            delete mTexStorage;
            mTexStorage = NULL;
            mDirtyImages = true;
        }
    }
}

gl::ImageIndexIterator TextureD3D_2DArray::imageIterator() const
{
    return gl::ImageIndexIterator::Make2DArray(0, mTexStorage->getLevelCount(), mLayerCounts);
}

gl::ImageIndex TextureD3D_2DArray::getImageIndex(GLint mip, GLint layer) const
{
    return gl::ImageIndex::Make2DArray(mip, layer);
}

bool TextureD3D_2DArray::isValidIndex(const gl::ImageIndex &index) const
{
    // Check for having a storage and the right type of index
    if (!mTexStorage || index.type != GL_TEXTURE_2D_ARRAY)
    {
        return false;
    }

    // Check the mip index
    if (index.mipIndex < 0 || index.mipIndex >= mTexStorage->getLevelCount())
    {
        return false;
    }

    // Check the layer index
    return (!index.hasLayer() || (index.layerIndex >= 0 && index.layerIndex < mLayerCounts[index.mipIndex]));
}

}
