//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageD3D.h: Defines the rx::ImageD3D class, an abstract base class for the
// renderer-specific classes which will define the interface to the underlying
// surfaces or resources.

#ifndef LIBANGLE_RENDERER_D3D_IMAGED3D_H_
#define LIBANGLE_RENDERER_D3D_IMAGED3D_H_

#include "common/debug.h"

#include "libANGLE/Error.h"

namespace gl
{
class Framebuffer;
struct ImageIndex;
struct Box;
struct Extents;
struct Offset;
struct Rectangle;
struct PixelUnpackState;
}

namespace rx
{
class TextureStorage;
class RendererD3D;
class RenderTargetD3D;

class ImageD3D : angle::NonCopyable
{
  public:
    ImageD3D();
    virtual ~ImageD3D() {};

    GLsizei getWidth() const { return mWidth; }
    GLsizei getHeight() const { return mHeight; }
    GLsizei getDepth() const { return mDepth; }
    GLenum getInternalFormat() const { return mInternalFormat; }
    GLenum getTarget() const { return mTarget; }
    bool isRenderableFormat() const { return mRenderable; }

    void markDirty() { mDirty = true; }
    void markClean() { mDirty = false; }
    virtual bool isDirty() const = 0;

    virtual bool redefine(GLenum target, GLenum internalformat, const gl::Extents &size, bool forceRelease) = 0;

    virtual gl::Error loadData(const gl::Box &area, const gl::PixelUnpackState &unpack, GLenum type, const void *input) = 0;
    virtual gl::Error loadCompressedData(const gl::Box &area, const void *input) = 0;

    virtual gl::Error setManagedSurface2D(TextureStorage *storage, int level) { return gl::Error(GL_NO_ERROR); };
    virtual gl::Error setManagedSurfaceCube(TextureStorage *storage, int face, int level) { return gl::Error(GL_NO_ERROR); };
    virtual gl::Error setManagedSurface3D(TextureStorage *storage, int level) { return gl::Error(GL_NO_ERROR); };
    virtual gl::Error setManagedSurface2DArray(TextureStorage *storage, int layer, int level) { return gl::Error(GL_NO_ERROR); };
    virtual gl::Error copyToStorage(TextureStorage *storage, const gl::ImageIndex &index, const gl::Box &region) = 0;

    virtual gl::Error copy(const gl::Offset &destOffset, const gl::Box &sourceArea,
                           const gl::ImageIndex &sourceIndex, TextureStorage *source) = 0;

    gl::Error copy(const gl::Offset &destOffset, const gl::Rectangle &sourceArea, const gl::Framebuffer *source);
    virtual gl::Error copy(const gl::Offset &destOffset,
                           const gl::Rectangle &sourceArea,
                           RenderTargetD3D *source) = 0;

  protected:
    GLsizei mWidth;
    GLsizei mHeight;
    GLsizei mDepth;
    GLenum mInternalFormat;
    bool mRenderable;
    GLenum mTarget;

    bool mDirty;
};

}

#endif // LIBANGLE_RENDERER_D3D_IMAGED3D_H_
