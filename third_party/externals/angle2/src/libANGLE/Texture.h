//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Texture.h: Defines the gl::Texture class [OpenGL ES 2.0.24] section 3.7 page 63.

#ifndef LIBANGLE_TEXTURE_H_
#define LIBANGLE_TEXTURE_H_

#include <vector>
#include <map>

#include "angle_gl.h"
#include "common/debug.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Error.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Image.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/TextureImpl.h"

namespace egl
{
class Surface;
}

namespace gl
{
class Context;
class Framebuffer;
struct Data;

bool IsMipmapFiltered(const gl::SamplerState &samplerState);

class Texture final : public egl::ImageSibling, public gl::FramebufferAttachmentObject
{
  public:
    Texture(rx::TextureImpl *impl, GLuint id, GLenum target);
    ~Texture() override;

    GLenum getTarget() const;

    void setSwizzleRed(GLenum swizzleRed);
    GLenum getSwizzleRed() const;

    void setSwizzleGreen(GLenum swizzleGreen);
    GLenum getSwizzleGreen() const;

    void setSwizzleBlue(GLenum swizzleBlue);
    GLenum getSwizzleBlue() const;

    void setSwizzleAlpha(GLenum swizzleAlpha);
    GLenum getSwizzleAlpha() const;

    void setMinFilter(GLenum minFilter);
    GLenum getMinFilter() const;

    void setMagFilter(GLenum magFilter);
    GLenum getMagFilter() const;

    void setWrapS(GLenum wrapS);
    GLenum getWrapS() const;

    void setWrapT(GLenum wrapT);
    GLenum getWrapT() const;

    void setWrapR(GLenum wrapR);
    GLenum getWrapR() const;

    void setMaxAnisotropy(float maxAnisotropy);
    float getMaxAnisotropy() const;

    void setMinLod(GLfloat minLod);
    GLfloat getMinLod() const;

    void setMaxLod(GLfloat maxLod);
    GLfloat getMaxLod() const;

    void setCompareMode(GLenum compareMode);
    GLenum getCompareMode() const;

    void setCompareFunc(GLenum compareFunc);
    GLenum getCompareFunc() const;

    const SamplerState &getSamplerState() const;

    void setBaseLevel(GLuint baseLevel);
    GLuint getBaseLevel() const;

    void setMaxLevel(GLuint maxLevel);
    GLuint getMaxLevel() const;

    bool getImmutableFormat() const;

    GLuint getImmutableLevels() const;

    void setUsage(GLenum usage);
    GLenum getUsage() const;

    const TextureState &getTextureState() const;

    size_t getWidth(GLenum target, size_t level) const;
    size_t getHeight(GLenum target, size_t level) const;
    size_t getDepth(GLenum target, size_t level) const;
    GLenum getInternalFormat(GLenum target, size_t level) const;

    bool isSamplerComplete(const SamplerState &samplerState, const Data &data) const;
    bool isMipmapComplete() const;
    bool isCubeComplete() const;
    size_t getMipCompleteLevels() const;

    Error setImage(Context *context,
                   GLenum target,
                   size_t level,
                   GLenum internalFormat,
                   const Extents &size,
                   GLenum format,
                   GLenum type,
                   const uint8_t *pixels);
    Error setSubImage(Context *context,
                      GLenum target,
                      size_t level,
                      const Box &area,
                      GLenum format,
                      GLenum type,
                      const uint8_t *pixels);

    Error setCompressedImage(Context *context,
                             GLenum target,
                             size_t level,
                             GLenum internalFormat,
                             const Extents &size,
                             size_t imageSize,
                             const uint8_t *pixels);
    Error setCompressedSubImage(Context *context,
                                GLenum target,
                                size_t level,
                                const Box &area,
                                GLenum format,
                                size_t imageSize,
                                const uint8_t *pixels);

    Error copyImage(GLenum target,
                    size_t level,
                    const Rectangle &sourceArea,
                    GLenum internalFormat,
                    const Framebuffer *source);
    Error copySubImage(GLenum target,
                       size_t level,
                       const Offset &destOffset,
                       const Rectangle &sourceArea,
                       const Framebuffer *source);

    Error setStorage(GLenum target, size_t levels, GLenum internalFormat, const Extents &size);

    Error setEGLImageTarget(GLenum target, egl::Image *imageTarget);

    Error generateMipmaps();

    egl::Surface *getBoundSurface() const;

    rx::TextureImpl *getImplementation() { return mTexture; }
    const rx::TextureImpl *getImplementation() const { return mTexture; }

    // FramebufferAttachmentObject implementation
    GLsizei getAttachmentWidth(const FramebufferAttachment::Target &target) const override;
    GLsizei getAttachmentHeight(const FramebufferAttachment::Target &target) const override;
    GLenum getAttachmentInternalFormat(const FramebufferAttachment::Target &target) const override;
    GLsizei getAttachmentSamples(const FramebufferAttachment::Target &target) const override;

    void onAttach() override;
    void onDetach() override;
    GLuint getId() const override;

  private:
    rx::FramebufferAttachmentObjectImpl *getAttachmentImpl() const override { return mTexture; }

    // ANGLE-only method, used internally
    friend class egl::Surface;
    void bindTexImageFromSurface(egl::Surface *surface);
    void releaseTexImageFromSurface();

    rx::TextureImpl *mTexture;

    TextureState mTextureState;

    GLenum mTarget;

    struct ImageDesc
    {
        Extents size;
        GLenum internalFormat;

        ImageDesc();
        ImageDesc(const Extents &size, GLenum internalFormat);
    };

    GLenum getBaseImageTarget() const;

    bool computeSamplerCompleteness(const SamplerState &samplerState, const Data &data) const;
    bool computeMipmapCompleteness() const;
    bool computeLevelCompleteness(GLenum target, size_t level) const;

    const ImageDesc &getImageDesc(GLenum target, size_t level) const;
    void setImageDesc(GLenum target, size_t level, const ImageDesc &desc);
    void setImageDescChain(size_t levels, Extents baseSize, GLenum sizedInternalFormat);
    void clearImageDesc(GLenum target, size_t level);
    void clearImageDescs();
    void releaseTexImageInternal();

    std::vector<ImageDesc> mImageDescs;

    struct SamplerCompletenessCache
    {
        SamplerCompletenessCache();

        bool cacheValid;

        // All values that affect sampler completeness that are not stored within
        // the texture itself
        SamplerState samplerState;
        bool filterable;
        GLint clientVersion;
        bool supportsNPOT;

        // Result of the sampler completeness with the above parameters
        bool samplerComplete;
    };
    mutable SamplerCompletenessCache mCompletenessCache;

    egl::Surface *mBoundSurface;
};

}

#endif   // LIBANGLE_TEXTURE_H_
