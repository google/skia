//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES2.cpp: Validation functions for OpenGL ES 2.0 entry point parameters

#include "libANGLE/validationES2.h"
#include "libANGLE/validationES.h"
#include "libANGLE/Context.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/FramebufferAttachment.h"

#include "common/mathutil.h"
#include "common/utilities.h"

namespace gl
{

bool ValidateES2TexImageParameters(Context *context, GLenum target, GLint level, GLenum internalformat, bool isCompressed, bool isSubImage,
                                   GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                                   GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
    if (!ValidTexture2DDestinationTarget(context, target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (!ValidImageSize(context, target, level, width, height, 1))
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (level < 0 || xoffset < 0 ||
        std::numeric_limits<GLsizei>::max() - xoffset < width ||
        std::numeric_limits<GLsizei>::max() - yoffset < height)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (!isSubImage && !isCompressed && internalformat != format)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::Caps &caps = context->getCaps();

    if (target == GL_TEXTURE_2D)
    {
        if (static_cast<GLuint>(width) > (caps.max2DTextureSize >> level) ||
            static_cast<GLuint>(height) > (caps.max2DTextureSize >> level))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }
    else if (IsCubeMapTextureTarget(target))
    {
        if (!isSubImage && width != height)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }

        if (static_cast<GLuint>(width) > (caps.maxCubeMapTextureSize >> level) ||
            static_cast<GLuint>(height) > (caps.maxCubeMapTextureSize >> level))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }
    else
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    gl::Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
    if (!texture)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (isSubImage)
    {
        if (format != GL_NONE)
        {
            if (gl::GetSizedInternalFormat(format, type) != texture->getInternalFormat(target, level))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
        }

        if (static_cast<size_t>(xoffset + width) > texture->getWidth(target, level) ||
            static_cast<size_t>(yoffset + height) > texture->getHeight(target, level))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }
    else
    {
        if (texture->getImmutableFormat())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    // Verify zero border
    if (border != 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    GLenum actualInternalFormat = isSubImage ? texture->getInternalFormat(target, level) : internalformat;
    const InternalFormat &actualFormatInfo = GetInternalFormatInfo(actualInternalFormat);

    if (isCompressed != actualFormatInfo.compressed)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (isCompressed)
    {
        if (!ValidCompressedImageSize(context, actualInternalFormat, width, height))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }

        switch (actualInternalFormat)
        {
          case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            if (!context->getExtensions().textureCompressionDXT1)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
            if (!context->getExtensions().textureCompressionDXT1)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            if (!context->getExtensions().textureCompressionDXT5)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
    }
    else
    {
        // validate <type> by itself (used as secondary key below)
        switch (type)
        {
          case GL_UNSIGNED_BYTE:
          case GL_UNSIGNED_SHORT_5_6_5:
          case GL_UNSIGNED_SHORT_4_4_4_4:
          case GL_UNSIGNED_SHORT_5_5_5_1:
          case GL_UNSIGNED_SHORT:
          case GL_UNSIGNED_INT:
          case GL_UNSIGNED_INT_24_8_OES:
          case GL_HALF_FLOAT_OES:
          case GL_FLOAT:
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

        // validate <format> + <type> combinations
        // - invalid <format> -> sets INVALID_ENUM
        // - invalid <format>+<type> combination -> sets INVALID_OPERATION
        switch (format)
        {
          case GL_ALPHA:
          case GL_LUMINANCE:
          case GL_LUMINANCE_ALPHA:
            switch (type)
            {
              case GL_UNSIGNED_BYTE:
              case GL_FLOAT:
              case GL_HALF_FLOAT_OES:
                break;
              default:
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
            }
            break;
          case GL_RED:
          case GL_RG:
              if (!context->getExtensions().textureRG)
              {
                  context->recordError(Error(GL_INVALID_ENUM));
                  return false;
              }
              switch (type)
              {
                case GL_UNSIGNED_BYTE:
                case GL_FLOAT:
                case GL_HALF_FLOAT_OES:
                  break;
                default:
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RGB:
            switch (type)
            {
              case GL_UNSIGNED_BYTE:
              case GL_UNSIGNED_SHORT_5_6_5:
              case GL_FLOAT:
              case GL_HALF_FLOAT_OES:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_RGBA:
            switch (type)
            {
              case GL_UNSIGNED_BYTE:
              case GL_UNSIGNED_SHORT_4_4_4_4:
              case GL_UNSIGNED_SHORT_5_5_5_1:
              case GL_FLOAT:
              case GL_HALF_FLOAT_OES:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_BGRA_EXT:
            switch (type)
            {
              case GL_UNSIGNED_BYTE:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_SRGB_EXT:
          case GL_SRGB_ALPHA_EXT:
            if (!context->getExtensions().sRGB)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            switch (type)
            {
              case GL_UNSIGNED_BYTE:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:  // error cases for compressed textures are handled below
          case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
          case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            break;
          case GL_DEPTH_COMPONENT:
            switch (type)
            {
              case GL_UNSIGNED_SHORT:
              case GL_UNSIGNED_INT:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_DEPTH_STENCIL_OES:
            switch (type)
            {
              case GL_UNSIGNED_INT_24_8_OES:
                break;
              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

        switch (format)
        {
          case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            if (context->getExtensions().textureCompressionDXT1)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
            if (context->getExtensions().textureCompressionDXT3)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            if (context->getExtensions().textureCompressionDXT5)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_DEPTH_COMPONENT:
          case GL_DEPTH_STENCIL_OES:
            if (!context->getExtensions().depthTextures)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return false;
            }
            if (target != GL_TEXTURE_2D)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            // OES_depth_texture supports loading depth data and multiple levels,
            // but ANGLE_depth_texture does not
            if (pixels != NULL || level != 0)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          default:
            break;
        }

        if (type == GL_FLOAT)
        {
            if (!context->getExtensions().textureFloat)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
        }
        else if (type == GL_HALF_FLOAT_OES)
        {
            if (!context->getExtensions().textureHalfFloat)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
        }
    }

    return true;
}



bool ValidateES2CopyTexImageParameters(Context* context, GLenum target, GLint level, GLenum internalformat, bool isSubImage,
                                       GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height,
                                       GLint border)
{
    GLenum textureInternalFormat = GL_NONE;

    if (!ValidateCopyTexImageParametersBase(context, target, level, internalformat, isSubImage,
                                            xoffset, yoffset, 0, x, y, width, height, border, &textureInternalFormat))
    {
        return false;
    }

    gl::Framebuffer *framebuffer = context->getState().getReadFramebuffer();
    GLenum colorbufferFormat = framebuffer->getReadColorbuffer()->getInternalFormat();
    const auto &internalFormatInfo = gl::GetInternalFormatInfo(textureInternalFormat);
    GLenum textureFormat = internalFormatInfo.format;

    // [OpenGL ES 2.0.24] table 3.9
    if (isSubImage)
    {
        switch (textureFormat)
        {
          case GL_ALPHA:
            if (colorbufferFormat != GL_ALPHA8_EXT &&
                colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_RGBA8_OES)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_LUMINANCE:
              if (colorbufferFormat != GL_R8_EXT &&
                  colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_RGBA8_OES)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RED_EXT:
              if (colorbufferFormat != GL_R8_EXT &&
                  colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_RGBA8_OES &&
                  colorbufferFormat != GL_R32F &&
                  colorbufferFormat != GL_RG32F &&
                  colorbufferFormat != GL_RGB32F &&
                  colorbufferFormat != GL_RGBA32F)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RG_EXT:
              if (colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_RGBA8_OES &&
                  colorbufferFormat != GL_RG32F &&
                  colorbufferFormat != GL_RGB32F &&
                  colorbufferFormat != GL_RGBA32F)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RGB:
            if (colorbufferFormat != GL_RGB565 &&
                colorbufferFormat != GL_RGB8_OES &&
                colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_RGBA8_OES &&
                colorbufferFormat != GL_RGB32F &&
                colorbufferFormat != GL_RGBA32F)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_LUMINANCE_ALPHA:
          case GL_RGBA:
            if (colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_RGBA8_OES &&
                colorbufferFormat != GL_RGBA32F)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
          case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
          case GL_DEPTH_COMPONENT:
          case GL_DEPTH_STENCIL_OES:
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
          default:
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }

        if (internalFormatInfo.type == GL_FLOAT &&
            !context->getExtensions().textureFloat)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }
    else
    {
        switch (internalformat)
        {
          case GL_ALPHA:
            if (colorbufferFormat != GL_ALPHA8_EXT &&
                colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_BGRA8_EXT &&
                colorbufferFormat != GL_RGBA8_OES &&
                colorbufferFormat != GL_BGR5_A1_ANGLEX)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_LUMINANCE:
              if (colorbufferFormat != GL_R8_EXT &&
                  colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_BGRA8_EXT &&
                  colorbufferFormat != GL_RGBA8_OES &&
                  colorbufferFormat != GL_BGR5_A1_ANGLEX)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RED_EXT:
              if (colorbufferFormat != GL_R8_EXT &&
                  colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_BGRA8_EXT &&
                  colorbufferFormat != GL_RGBA8_OES &&
                  colorbufferFormat != GL_BGR5_A1_ANGLEX)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RG_EXT:
              if (colorbufferFormat != GL_RG8_EXT &&
                  colorbufferFormat != GL_RGB565 &&
                  colorbufferFormat != GL_RGB8_OES &&
                  colorbufferFormat != GL_RGBA4 &&
                  colorbufferFormat != GL_RGB5_A1 &&
                  colorbufferFormat != GL_BGRA8_EXT &&
                  colorbufferFormat != GL_RGBA8_OES &&
                  colorbufferFormat != GL_BGR5_A1_ANGLEX)
              {
                  context->recordError(Error(GL_INVALID_OPERATION));
                  return false;
              }
              break;
          case GL_RGB:
            if (colorbufferFormat != GL_RGB565 &&
                colorbufferFormat != GL_RGB8_OES &&
                colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_BGRA8_EXT &&
                colorbufferFormat != GL_RGBA8_OES &&
                colorbufferFormat != GL_BGR5_A1_ANGLEX)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_LUMINANCE_ALPHA:
          case GL_RGBA:
            if (colorbufferFormat != GL_RGBA4 &&
                colorbufferFormat != GL_RGB5_A1 &&
                colorbufferFormat != GL_BGRA8_EXT &&
                colorbufferFormat != GL_RGBA8_OES &&
                colorbufferFormat != GL_BGR5_A1_ANGLEX)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            break;
          case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
          case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            if (context->getExtensions().textureCompressionDXT1)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
            if (context->getExtensions().textureCompressionDXT3)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            if (context->getExtensions().textureCompressionDXT5)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
            break;
          case GL_DEPTH_COMPONENT:
          case GL_DEPTH_COMPONENT16:
          case GL_DEPTH_COMPONENT32_OES:
          case GL_DEPTH_STENCIL_OES:
          case GL_DEPTH24_STENCIL8_OES:
            if (context->getExtensions().depthTextures)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
            else
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return false;
            }
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
    }

    // If width or height is zero, it is a no-op.  Return false without setting an error.
    return (width > 0 && height > 0);
}

bool ValidateES2TexStorageParameters(Context *context, GLenum target, GLsizei levels, GLenum internalformat,
                                     GLsizei width, GLsizei height)
{
    if (target != GL_TEXTURE_2D && target != GL_TEXTURE_CUBE_MAP)
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (width < 1 || height < 1 || levels < 1)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (target == GL_TEXTURE_CUBE_MAP && width != height)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (levels != 1 && levels != gl::log2(std::max(width, height)) + 1)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalformat);
    if (formatInfo.format == GL_NONE || formatInfo.type == GL_NONE)
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    const gl::Caps &caps = context->getCaps();

    switch (target)
    {
      case GL_TEXTURE_2D:
        if (static_cast<GLuint>(width) > caps.max2DTextureSize ||
            static_cast<GLuint>(height) > caps.max2DTextureSize)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
        break;
      case GL_TEXTURE_CUBE_MAP:
        if (static_cast<GLuint>(width) > caps.maxCubeMapTextureSize ||
            static_cast<GLuint>(height) > caps.maxCubeMapTextureSize)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (levels != 1 && !context->getExtensions().textureNPOT)
    {
        if (!gl::isPow2(width) || !gl::isPow2(height))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    switch (internalformat)
    {
      case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
      case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        if (!context->getExtensions().textureCompressionDXT1)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
        if (!context->getExtensions().textureCompressionDXT3)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
        if (!context->getExtensions().textureCompressionDXT5)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_RGBA32F_EXT:
      case GL_RGB32F_EXT:
      case GL_ALPHA32F_EXT:
      case GL_LUMINANCE32F_EXT:
      case GL_LUMINANCE_ALPHA32F_EXT:
        if (!context->getExtensions().textureFloat)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_RGBA16F_EXT:
      case GL_RGB16F_EXT:
      case GL_ALPHA16F_EXT:
      case GL_LUMINANCE16F_EXT:
      case GL_LUMINANCE_ALPHA16F_EXT:
        if (!context->getExtensions().textureHalfFloat)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_R8_EXT:
      case GL_RG8_EXT:
      case GL_R16F_EXT:
      case GL_RG16F_EXT:
      case GL_R32F_EXT:
      case GL_RG32F_EXT:
        if (!context->getExtensions().textureRG)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      case GL_DEPTH_COMPONENT16:
      case GL_DEPTH_COMPONENT32_OES:
      case GL_DEPTH24_STENCIL8_OES:
        if (!context->getExtensions().depthTextures)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        if (target != GL_TEXTURE_2D)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
        // ANGLE_depth_texture only supports 1-level textures
        if (levels != 1)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
        break;
      default:
        break;
    }

    gl::Texture *texture = context->getTargetTexture(target);
    if (!texture || texture->id() == 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (texture->getImmutableFormat())
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

// check for combinations of format and type that are valid for ReadPixels
bool ValidES2ReadFormatType(Context *context, GLenum format, GLenum type)
{
    switch (format)
    {
      case GL_RGBA:
        switch (type)
        {
          case GL_UNSIGNED_BYTE:
            break;
          default:
            return false;
        }
        break;
      case GL_BGRA_EXT:
        switch (type)
        {
          case GL_UNSIGNED_BYTE:
          case GL_UNSIGNED_SHORT_4_4_4_4_REV_EXT:
          case GL_UNSIGNED_SHORT_1_5_5_5_REV_EXT:
            break;
          default:
            return false;
        }
        break;
      case GL_RG_EXT:
      case GL_RED_EXT:
        if (!context->getExtensions().textureRG)
        {
            return false;
        }
        switch (type)
        {
          case GL_UNSIGNED_BYTE:
            break;
          default:
            return false;
        }
        break;

      default:
        return false;
    }
    return true;
}

bool ValidateDiscardFramebufferEXT(Context *context, GLenum target, GLsizei numAttachments,
                                   const GLenum *attachments)
{
    bool defaultFramebuffer = false;

    switch (target)
    {
      case GL_FRAMEBUFFER:
        defaultFramebuffer = (context->getState().getTargetFramebuffer(GL_FRAMEBUFFER)->id() == 0);
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM, "Invalid framebuffer target"));
        return false;
    }

    return ValidateDiscardFramebufferBase(context, target, numAttachments, attachments, defaultFramebuffer);
}

bool ValidateDrawBuffers(Context *context, GLsizei n, const GLenum *bufs)
{
    // INVALID_VALUE is generated if n is negative or greater than value of MAX_DRAW_BUFFERS
    if (n < 0 || static_cast<GLuint>(n) > context->getCaps().maxDrawBuffers)
    {
        context->recordError(
            Error(GL_INVALID_VALUE, "n must be non-negative and no greater than MAX_DRAW_BUFFERS"));
        return false;
    }

    ASSERT(context->getState().getDrawFramebuffer());
    GLuint frameBufferId      = context->getState().getDrawFramebuffer()->id();
    GLuint maxColorAttachment = GL_COLOR_ATTACHMENT0_EXT + context->getCaps().maxColorAttachments;

    // This should come first before the check for the default frame buffer
    // because when we switch to ES3.1+, invalid enums will return INVALID_ENUM
    // rather than INVALID_OPERATION
    for (int colorAttachment = 0; colorAttachment < n; colorAttachment++)
    {
        const GLenum attachment = GL_COLOR_ATTACHMENT0_EXT + colorAttachment;

        if (bufs[colorAttachment] != GL_NONE && bufs[colorAttachment] != GL_BACK &&
            (bufs[colorAttachment] < GL_COLOR_ATTACHMENT0_EXT ||
             bufs[colorAttachment] >= maxColorAttachment))
        {
            // Value in bufs is not NONE, BACK, or GL_COLOR_ATTACHMENTi
            // In the 3.0 specs, the error should return GL_INVALID_OPERATION.
            // When we move to 3.1 specs, we should change the error to be GL_INVALID_ENUM
            context->recordError(Error(GL_INVALID_OPERATION, "Invalid buffer value"));
            return false;
        }
        else if (bufs[colorAttachment] != GL_NONE && bufs[colorAttachment] != attachment &&
                 frameBufferId != 0)
        {
            // INVALID_OPERATION-GL is bound to buffer and ith argument
            // is not COLOR_ATTACHMENTi or NONE
            context->recordError(
                Error(GL_INVALID_OPERATION, "Ith value does not match COLOR_ATTACHMENTi or NONE"));
            return false;
        }
    }

    // INVALID_OPERATION is generated if GL is bound to the default framebuffer
    // and n is not 1 or bufs is bound to value other than BACK and NONE
    if (frameBufferId == 0)
    {
        if (n != 1)
        {
            context->recordError(Error(GL_INVALID_OPERATION,
                                       "n must be 1 when GL is bound to the default framebuffer"));
            return false;
        }

        if (bufs[0] != GL_NONE && bufs[0] != GL_BACK)
        {
            context->recordError(Error(
                GL_INVALID_OPERATION,
                "Only NONE or BACK are valid values when drawing to the default framebuffer"));
            return false;
        }
    }

    return true;
}
}
