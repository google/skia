//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureD3D.h: Implementations of the Texture interfaces shared betweeen the D3D backends.

#ifndef LIBANGLE_RENDERER_D3D_TEXTURED3D_H_
#define LIBANGLE_RENDERER_D3D_TEXTURED3D_H_

#include "libANGLE/renderer/TextureImpl.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/Constants.h"

namespace gl
{
class Framebuffer;
}

namespace rx
{
class EGLImageD3D;
class ImageD3D;
class RendererD3D;
class RenderTargetD3D;
class TextureStorage;

class TextureD3D : public TextureImpl
{
  public:
    TextureD3D(RendererD3D *renderer);
    virtual ~TextureD3D();

    gl::Error getNativeTexture(TextureStorage **outStorage);

    virtual void setUsage(GLenum usage) { mUsage = usage; }
    bool hasDirtyImages() const { return mDirtyImages; }
    void resetDirty() { mDirtyImages = false; }

    virtual ImageD3D *getImage(const gl::ImageIndex &index) const = 0;
    virtual GLsizei getLayerCount(int level) const = 0;

    GLint getBaseLevelWidth() const;
    GLint getBaseLevelHeight() const;
    GLint getBaseLevelDepth() const;
    GLenum getBaseLevelInternalFormat() const;

    bool isImmutable() const { return mImmutable; }

    virtual gl::Error getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT) = 0;

    // Returns an iterator over all "Images" for this particular Texture.
    virtual gl::ImageIndexIterator imageIterator() const = 0;

    // Returns an ImageIndex for a particular "Image". 3D Textures do not have images for
    // slices of their depth texures, so 3D textures ignore the layer parameter.
    virtual gl::ImageIndex getImageIndex(GLint mip, GLint layer) const = 0;
    virtual bool isValidIndex(const gl::ImageIndex &index) const = 0;

    gl::Error generateMipmaps(const gl::TextureState &textureState) override;
    TextureStorage *getStorage();
    ImageD3D *getBaseLevelImage() const;

    gl::Error getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                        FramebufferAttachmentRenderTarget **rtOut) override;

  protected:
    gl::Error setImageImpl(const gl::ImageIndex &index,
                           GLenum type,
                           const gl::PixelUnpackState &unpack,
                           const uint8_t *pixels,
                           ptrdiff_t layerOffset);
    gl::Error subImage(const gl::ImageIndex &index, const gl::Box &area, GLenum format, GLenum type,
                       const gl::PixelUnpackState &unpack, const uint8_t *pixels, ptrdiff_t layerOffset);
    gl::Error setCompressedImageImpl(const gl::ImageIndex &index,
                                     const gl::PixelUnpackState &unpack,
                                     const uint8_t *pixels,
                                     ptrdiff_t layerOffset);
    gl::Error subImageCompressed(const gl::ImageIndex &index, const gl::Box &area, GLenum format,
                                 const gl::PixelUnpackState &unpack, const uint8_t *pixels, ptrdiff_t layerOffset);
    bool isFastUnpackable(const gl::PixelUnpackState &unpack, GLenum sizedInternalFormat);
    gl::Error fastUnpackPixels(const gl::PixelUnpackState &unpack, const uint8_t *pixels, const gl::Box &destArea,
                               GLenum sizedInternalFormat, GLenum type, RenderTargetD3D *destRenderTarget);

    GLint creationLevels(GLsizei width, GLsizei height, GLsizei depth) const;
    int mipLevels() const;
    virtual void initMipmapsImages() = 0;
    bool isBaseImageZeroSize() const;
    virtual bool isImageComplete(const gl::ImageIndex &index) const = 0;

    bool canCreateRenderTargetForImage(const gl::ImageIndex &index) const;
    virtual gl::Error ensureRenderTarget();

    virtual gl::Error createCompleteStorage(bool renderTarget, TextureStorage **outTexStorage) const = 0;
    virtual gl::Error setCompleteTexStorage(TextureStorage *newCompleteTexStorage) = 0;
    gl::Error commitRegion(const gl::ImageIndex &index, const gl::Box &region);

    RendererD3D *mRenderer;

    GLenum mUsage;

    bool mDirtyImages;

    bool mImmutable;
    TextureStorage *mTexStorage;

  private:
    virtual gl::Error initializeStorage(bool renderTarget) = 0;

    virtual gl::Error updateStorage() = 0;

    bool shouldUseSetData(const ImageD3D *image) const;

    gl::Error generateMipmapsUsingImages();
};

class TextureD3D_2D : public TextureD3D
{
  public:
    TextureD3D_2D(RendererD3D *renderer);
    virtual ~TextureD3D_2D();

    virtual ImageD3D *getImage(int level, int layer) const;
    virtual ImageD3D *getImage(const gl::ImageIndex &index) const;
    virtual GLsizei getLayerCount(int level) const;

    GLsizei getWidth(GLint level) const;
    GLsizei getHeight(GLint level) const;
    GLenum getInternalFormat(GLint level) const;
    bool isDepth(GLint level) const;

    gl::Error setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                       const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;
    gl::Error setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                          const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;

    gl::Error setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                 const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;
    gl::Error setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                    const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;

    gl::Error copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                        const gl::Framebuffer *source) override;
    gl::Error copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea,
                           const gl::Framebuffer *source) override;

    gl::Error setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size) override;

    virtual void bindTexImage(egl::Surface *surface);
    virtual void releaseTexImage();

    gl::Error setEGLImageTarget(GLenum target, egl::Image *image) override;

    virtual gl::Error getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT);

    virtual gl::ImageIndexIterator imageIterator() const;
    virtual gl::ImageIndex getImageIndex(GLint mip, GLint layer) const;
    virtual bool isValidIndex(const gl::ImageIndex &index) const;

  private:
    virtual gl::Error initializeStorage(bool renderTarget);
    virtual gl::Error createCompleteStorage(bool renderTarget, TextureStorage **outTexStorage) const;
    virtual gl::Error setCompleteTexStorage(TextureStorage *newCompleteTexStorage);

    virtual gl::Error updateStorage();
    virtual void initMipmapsImages();

    bool isValidLevel(int level) const;
    bool isLevelComplete(int level) const;
    virtual bool isImageComplete(const gl::ImageIndex &index) const;

    gl::Error updateStorageLevel(int level);

    void redefineImage(size_t level,
                       GLenum internalformat,
                       const gl::Extents &size,
                       bool forceRelease);

    bool mEGLImageTarget;
    ImageD3D *mImageArray[gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
};

class TextureD3D_Cube : public TextureD3D
{
  public:
    TextureD3D_Cube(RendererD3D *renderer);
    virtual ~TextureD3D_Cube();

    virtual ImageD3D *getImage(int level, int layer) const;
    virtual ImageD3D *getImage(const gl::ImageIndex &index) const;
    virtual GLsizei getLayerCount(int level) const;

    virtual bool hasDirtyImages() const { return mDirtyImages; }
    virtual void resetDirty() { mDirtyImages = false; }
    virtual void setUsage(GLenum usage) { mUsage = usage; }

    GLenum getInternalFormat(GLint level, GLint layer) const;
    bool isDepth(GLint level, GLint layer) const;

    gl::Error setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                       const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;
    gl::Error setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                          const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;

    gl::Error setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                 const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;
    gl::Error setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                    const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;

    gl::Error copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                        const gl::Framebuffer *source) override;
    gl::Error copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea,
                           const gl::Framebuffer *source) override;

    gl::Error setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size) override;

    virtual void bindTexImage(egl::Surface *surface);
    virtual void releaseTexImage();

    gl::Error setEGLImageTarget(GLenum target, egl::Image *image) override;

    virtual gl::Error getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT);

    virtual gl::ImageIndexIterator imageIterator() const;
    virtual gl::ImageIndex getImageIndex(GLint mip, GLint layer) const;
    virtual bool isValidIndex(const gl::ImageIndex &index) const;

  private:
    virtual gl::Error initializeStorage(bool renderTarget);
    virtual gl::Error createCompleteStorage(bool renderTarget, TextureStorage **outTexStorage) const;
    virtual gl::Error setCompleteTexStorage(TextureStorage *newCompleteTexStorage);

    virtual gl::Error updateStorage();
    virtual void initMipmapsImages();

    bool isValidFaceLevel(int faceIndex, int level) const;
    bool isFaceLevelComplete(int faceIndex, int level) const;
    bool isCubeComplete() const;
    virtual bool isImageComplete(const gl::ImageIndex &index) const;
    gl::Error updateStorageFaceLevel(int faceIndex, int level);

    void redefineImage(int faceIndex, GLint level, GLenum internalformat, const gl::Extents &size);

    ImageD3D *mImageArray[6][gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
};

class TextureD3D_3D : public TextureD3D
{
  public:
    TextureD3D_3D(RendererD3D *renderer);
    virtual ~TextureD3D_3D();

    virtual ImageD3D *getImage(int level, int layer) const;
    virtual ImageD3D *getImage(const gl::ImageIndex &index) const;
    virtual GLsizei getLayerCount(int level) const;

    GLsizei getWidth(GLint level) const;
    GLsizei getHeight(GLint level) const;
    GLsizei getDepth(GLint level) const;
    GLenum getInternalFormat(GLint level) const;
    bool isDepth(GLint level) const;

    gl::Error setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                       const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;
    gl::Error setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                          const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;

    gl::Error setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                 const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;
    gl::Error setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                    const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;

    gl::Error copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                        const gl::Framebuffer *source) override;
    gl::Error copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea,
                           const gl::Framebuffer *source) override;

    gl::Error setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size) override;

    virtual void bindTexImage(egl::Surface *surface);
    virtual void releaseTexImage();

    gl::Error setEGLImageTarget(GLenum target, egl::Image *image) override;

    virtual gl::Error getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT);

    virtual gl::ImageIndexIterator imageIterator() const;
    virtual gl::ImageIndex getImageIndex(GLint mip, GLint layer) const;
    virtual bool isValidIndex(const gl::ImageIndex &index) const;

  private:
    virtual gl::Error initializeStorage(bool renderTarget);
    virtual gl::Error createCompleteStorage(bool renderTarget, TextureStorage **outStorage) const;
    virtual gl::Error setCompleteTexStorage(TextureStorage *newCompleteTexStorage);

    virtual gl::Error updateStorage();
    virtual void initMipmapsImages();

    bool isValidLevel(int level) const;
    bool isLevelComplete(int level) const;
    virtual bool isImageComplete(const gl::ImageIndex &index) const;
    gl::Error updateStorageLevel(int level);

    void redefineImage(GLint level, GLenum internalformat, const gl::Extents &size);

    ImageD3D *mImageArray[gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
};

class TextureD3D_2DArray : public TextureD3D
{
  public:
    TextureD3D_2DArray(RendererD3D *renderer);
    virtual ~TextureD3D_2DArray();

    virtual ImageD3D *getImage(int level, int layer) const;
    virtual ImageD3D *getImage(const gl::ImageIndex &index) const;
    virtual GLsizei getLayerCount(int level) const;

    GLsizei getWidth(GLint level) const;
    GLsizei getHeight(GLint level) const;
    GLenum getInternalFormat(GLint level) const;
    bool isDepth(GLint level) const;

    gl::Error setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                       const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;
    gl::Error setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                          const gl::PixelUnpackState &unpack, const uint8_t *pixels) override;

    gl::Error setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                 const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;
    gl::Error setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                    const gl::PixelUnpackState &unpack, size_t imageSize, const uint8_t *pixels) override;

    gl::Error copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                        const gl::Framebuffer *source) override;
    gl::Error copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea,
                           const gl::Framebuffer *source) override;

    gl::Error setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size) override;

    virtual void bindTexImage(egl::Surface *surface);
    virtual void releaseTexImage();

    gl::Error setEGLImageTarget(GLenum target, egl::Image *image) override;

    virtual gl::Error getRenderTarget(const gl::ImageIndex &index, RenderTargetD3D **outRT);

    virtual gl::ImageIndexIterator imageIterator() const;
    virtual gl::ImageIndex getImageIndex(GLint mip, GLint layer) const;
    virtual bool isValidIndex(const gl::ImageIndex &index) const;

  private:
    virtual gl::Error initializeStorage(bool renderTarget);
    virtual gl::Error createCompleteStorage(bool renderTarget, TextureStorage **outStorage) const;
    virtual gl::Error setCompleteTexStorage(TextureStorage *newCompleteTexStorage);

    virtual gl::Error updateStorage();
    virtual void initMipmapsImages();

    bool isValidLevel(int level) const;
    bool isLevelComplete(int level) const;
    virtual bool isImageComplete(const gl::ImageIndex &index) const;
    gl::Error updateStorageLevel(int level);

    void deleteImages();
    void redefineImage(GLint level, GLenum internalformat, const gl::Extents &size);

    // Storing images as an array of single depth textures since D3D11 treats each array level of a
    // Texture2D object as a separate subresource.  Each layer would have to be looped over
    // to update all the texture layers since they cannot all be updated at once and it makes the most
    // sense for the Image class to not have to worry about layer subresource as well as mip subresources.
    GLsizei mLayerCounts[gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
    ImageD3D **mImageArray[gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
};

}

#endif // LIBANGLE_RENDERER_D3D_TEXTURED3D_H_
