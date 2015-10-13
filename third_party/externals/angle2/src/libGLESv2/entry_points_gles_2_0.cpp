//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_2_0.cpp : Implements the GLES 2.0 entry points.

#include "libGLESv2/entry_points_gles_2_0.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Context.h"
#include "libANGLE/Error.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Program.h"
#include "libANGLE/Texture.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/FramebufferAttachment.h"

#include "libANGLE/validationES.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/queryconversions.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "common/version.h"

namespace gl
{

void GL_APIENTRY ActiveTexture(GLenum texture)
{
    EVENT("(GLenum texture = 0x%X)", texture);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (texture < GL_TEXTURE0 || texture > GL_TEXTURE0 + context->getCaps().maxCombinedTextureImageUnits - 1)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setActiveSampler(texture - GL_TEXTURE0);
    }
}

void GL_APIENTRY AttachShader(GLuint program, GLuint shader)
{
    EVENT("(GLuint program = %d, GLuint shader = %d)", program, shader);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }

        if (!programObject->attachShader(shaderObject))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
    }
}

void GL_APIENTRY BindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, const GLchar* name = 0x%0.8p)", program, index, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (strncmp(name, "gl_", 3) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        programObject->bindAttributeLocation(index, name);
    }
}

void GL_APIENTRY BindBuffer(GLenum target, GLuint buffer)
{
    EVENT("(GLenum target = 0x%X, GLuint buffer = %d)", target, buffer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_ARRAY_BUFFER:
            context->bindArrayBuffer(buffer);
            return;
          case GL_ELEMENT_ARRAY_BUFFER:
            context->bindElementArrayBuffer(buffer);
            return;
          case GL_COPY_READ_BUFFER:
            context->bindCopyReadBuffer(buffer);
            return;
          case GL_COPY_WRITE_BUFFER:
            context->bindCopyWriteBuffer(buffer);
            return;
          case GL_PIXEL_PACK_BUFFER:
            context->bindPixelPackBuffer(buffer);
            return;
          case GL_PIXEL_UNPACK_BUFFER:
            context->bindPixelUnpackBuffer(buffer);
            return;
          case GL_UNIFORM_BUFFER:
            context->bindGenericUniformBuffer(buffer);
            return;
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            context->bindGenericTransformFeedbackBuffer(buffer);
            return;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY BindFramebuffer(GLenum target, GLuint framebuffer)
{
    EVENT("(GLenum target = 0x%X, GLuint framebuffer = %d)", target, framebuffer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (target == GL_READ_FRAMEBUFFER_ANGLE || target == GL_FRAMEBUFFER)
        {
            context->bindReadFramebuffer(framebuffer);
        }

        if (target == GL_DRAW_FRAMEBUFFER_ANGLE || target == GL_FRAMEBUFFER)
        {
            context->bindDrawFramebuffer(framebuffer);
        }
    }
}

void GL_APIENTRY BindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    EVENT("(GLenum target = 0x%X, GLuint renderbuffer = %d)", target, renderbuffer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->bindRenderbuffer(renderbuffer);
    }
}

void GL_APIENTRY BindTexture(GLenum target, GLuint texture)
{
    EVENT("(GLenum target = 0x%X, GLuint texture = %d)", target, texture);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Texture *textureObject = context->getTexture(texture);

        if (textureObject && textureObject->getTarget() != target && texture != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
          case GL_TEXTURE_CUBE_MAP:
            break;

          case GL_TEXTURE_3D:
          case GL_TEXTURE_2D_ARRAY:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->bindTexture(target, texture);
    }
}

void GL_APIENTRY BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    EVENT("(GLclampf red = %f, GLclampf green = %f, GLclampf blue = %f, GLclampf alpha = %f)",
          red, green, blue, alpha);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setBlendColor(clamp01(red), clamp01(green), clamp01(blue), clamp01(alpha));
    }
}

void GL_APIENTRY BlendEquation(GLenum mode)
{
    BlendEquationSeparate(mode, mode);
}

void GL_APIENTRY BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    EVENT("(GLenum modeRGB = 0x%X, GLenum modeAlpha = 0x%X)", modeRGB, modeAlpha);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (modeRGB)
        {
          case GL_FUNC_ADD:
          case GL_FUNC_SUBTRACT:
          case GL_FUNC_REVERSE_SUBTRACT:
          case GL_MIN:
          case GL_MAX:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (modeAlpha)
        {
          case GL_FUNC_ADD:
          case GL_FUNC_SUBTRACT:
          case GL_FUNC_REVERSE_SUBTRACT:
          case GL_MIN:
          case GL_MAX:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setBlendEquation(modeRGB, modeAlpha);
    }
}

void GL_APIENTRY BlendFunc(GLenum sfactor, GLenum dfactor)
{
    BlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
}

void GL_APIENTRY BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    EVENT("(GLenum srcRGB = 0x%X, GLenum dstRGB = 0x%X, GLenum srcAlpha = 0x%X, GLenum dstAlpha = 0x%X)",
          srcRGB, dstRGB, srcAlpha, dstAlpha);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (srcRGB)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
          case GL_SRC_ALPHA_SATURATE:
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        switch (dstRGB)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

          case GL_SRC_ALPHA_SATURATE:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (srcAlpha)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
          case GL_SRC_ALPHA_SATURATE:
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        switch (dstAlpha)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

          case GL_SRC_ALPHA_SATURATE:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (context->getLimitations().noSimultaneousConstantColorAndAlphaBlendFunc)
        {
            bool constantColorUsed =
                (srcRGB == GL_CONSTANT_COLOR || srcRGB == GL_ONE_MINUS_CONSTANT_COLOR ||
                 dstRGB == GL_CONSTANT_COLOR || dstRGB == GL_ONE_MINUS_CONSTANT_COLOR);

            bool constantAlphaUsed =
                (srcRGB == GL_CONSTANT_ALPHA || srcRGB == GL_ONE_MINUS_CONSTANT_ALPHA ||
                 dstRGB == GL_CONSTANT_ALPHA || dstRGB == GL_ONE_MINUS_CONSTANT_ALPHA);

            if (constantColorUsed && constantAlphaUsed)
            {
                ERR(
                    "Simultaneous use of GL_CONSTANT_ALPHA/GL_ONE_MINUS_CONSTANT_ALPHA and "
                    "GL_CONSTANT_COLOR/GL_ONE_MINUS_CONSTANT_COLOR not supported by this "
                    "implementation.");
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }

        context->getState().setBlendFactors(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }
}

void GL_APIENTRY BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    EVENT("(GLenum target = 0x%X, GLsizeiptr size = %d, const GLvoid* data = 0x%0.8p, GLenum usage = %d)",
          target, size, data, usage);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (size < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (usage)
        {
          case GL_STREAM_DRAW:
          case GL_STATIC_DRAW:
          case GL_DYNAMIC_DRAW:
            break;

          case GL_STREAM_READ:
          case GL_STREAM_COPY:
          case GL_STATIC_READ:
          case GL_STATIC_COPY:
          case GL_DYNAMIC_READ:
          case GL_DYNAMIC_COPY:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = buffer->bufferData(data, size, usage);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr size = %d, const GLvoid* data = 0x%0.8p)",
          target, offset, size, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (size < 0 || offset < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Check for possible overflow of size + offset
        if (!rx::IsUnsignedAdditionSafe<size_t>(size, offset))
        {
            context->recordError(Error(GL_OUT_OF_MEMORY));
            return;
        }

        if (size + offset > buffer->getSize())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (data == NULL)
        {
            return;
        }

        Error error = buffer->bufferSubData(data, size, offset);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

GLenum GL_APIENTRY CheckFramebufferStatus(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        return framebuffer->checkStatus(context->getData());
    }

    return 0;
}

void GL_APIENTRY Clear(GLbitfield mask)
{
    EVENT("(GLbitfield mask = 0x%X)", mask);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        if (framebufferObject->checkStatus(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
        {
            context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
            return;
        }

        if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Error error = framebufferObject->clear(context, mask);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    EVENT("(GLclampf red = %f, GLclampf green = %f, GLclampf blue = %f, GLclampf alpha = %f)",
          red, green, blue, alpha);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setColorClearValue(red, green, blue, alpha);
    }
}

void GL_APIENTRY ClearDepthf(GLclampf depth)
{
    EVENT("(GLclampf depth = %f)", depth);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setDepthClearValue(depth);
    }
}

void GL_APIENTRY ClearStencil(GLint s)
{
    EVENT("(GLint s = %d)", s);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setStencilClearValue(s);
    }
}

void GL_APIENTRY ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    EVENT("(GLboolean red = %d, GLboolean green = %u, GLboolean blue = %u, GLboolean alpha = %u)",
          red, green, blue, alpha);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setColorMask(red == GL_TRUE, green == GL_TRUE, blue == GL_TRUE, alpha == GL_TRUE);
    }
}

void GL_APIENTRY CompileShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }
        shaderObject->compile(context->getCompiler());
    }
}

void GL_APIENTRY CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                      GLint border, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLint border = %d, GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
          target, level, internalformat, width, height, border, imageSize, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, internalformat, true, false,
                                           0, 0, width, height, border, GL_NONE, GL_NONE, data))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, internalformat, true, false,
                                           0, 0, 0, width, height, 1, border, GL_NONE, GL_NONE, data))
        {
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(internalformat);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Extents size(width, height, 1);
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error =
            texture->setCompressedImage(context, target, level, internalformat, size, imageSize,
                                        reinterpret_cast<const uint8_t *>(data));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                                         GLenum format, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLsizei width = %d, GLsizei height = %d, GLenum format = 0x%X, "
          "GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
          target, level, xoffset, yoffset, width, height, format, imageSize, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, GL_NONE, true, true,
                                           xoffset, yoffset, width, height, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, GL_NONE, true, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(format);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Box area(xoffset, yoffset, 0, width, height, 1);
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error =
            texture->setCompressedSubImage(context, target, level, area, format, imageSize,
                                           reinterpret_cast<const uint8_t *>(data));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, "
          "GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, GLint border = %d)",
          target, level, internalformat, x, y, width, height, border);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2CopyTexImageParameters(context, target, level, internalformat, false,
                                               0, 0, x, y, width, height, border))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3CopyTexImageParameters(context, target, level, internalformat, false,
                                               0, 0, 0, x, y, width, height, border))
        {
            return;
        }

        Rectangle sourceArea(x, y, width, height);

        const Framebuffer *framebuffer = context->getState().getReadFramebuffer();
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error = texture->copyImage(target, level, sourceArea, internalformat, framebuffer);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, level, xoffset, yoffset, x, y, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2CopyTexImageParameters(context, target, level, GL_NONE, true,
                                               xoffset, yoffset, x, y, width, height, 0))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3CopyTexImageParameters(context, target, level, GL_NONE, true,
                                               xoffset, yoffset, 0, x, y, width, height, 0))
        {
            return;
        }

        Offset destOffset(xoffset, yoffset, 0);
        Rectangle sourceArea(x, y, width, height);

        const Framebuffer *framebuffer = context->getState().getReadFramebuffer();
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error = texture->copySubImage(target, level, destOffset, sourceArea, framebuffer);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

GLuint GL_APIENTRY CreateProgram(void)
{
    EVENT("()");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        return context->createProgram();
    }

    return 0;
}

GLuint GL_APIENTRY CreateShader(GLenum type)
{
    EVENT("(GLenum type = 0x%X)", type);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (type)
        {
          case GL_FRAGMENT_SHADER:
          case GL_VERTEX_SHADER:
            return context->createShader(type);

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }
    }

    return 0;
}

void GL_APIENTRY CullFace(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (mode)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setCullMode(mode);
    }
}

void GL_APIENTRY DeleteBuffers(GLsizei n, const GLuint* buffers)
{
    EVENT("(GLsizei n = %d, const GLuint* buffers = 0x%0.8p)", n, buffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteBuffer(buffers[i]);
        }
    }
}

void GL_APIENTRY DeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    EVENT("(GLsizei n = %d, const GLuint* framebuffers = 0x%0.8p)", n, framebuffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            if (framebuffers[i] != 0)
            {
                context->deleteFramebuffer(framebuffers[i]);
            }
        }
    }
}

void GL_APIENTRY DeleteProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (program == 0)
        {
            return;
        }

        if (!context->getProgram(program))
        {
            if(context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        context->deleteProgram(program);
    }
}

void GL_APIENTRY DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    EVENT("(GLsizei n = %d, const GLuint* renderbuffers = 0x%0.8p)", n, renderbuffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteRenderbuffer(renderbuffers[i]);
        }
    }
}

void GL_APIENTRY DeleteShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (shader == 0)
        {
            return;
        }

        if (!context->getShader(shader))
        {
            if(context->getProgram(shader))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        context->deleteShader(shader);
    }
}

void GL_APIENTRY DeleteTextures(GLsizei n, const GLuint* textures)
{
    EVENT("(GLsizei n = %d, const GLuint* textures = 0x%0.8p)", n, textures);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            if (textures[i] != 0)
            {
                context->deleteTexture(textures[i]);
            }
        }
    }
}

void GL_APIENTRY DepthFunc(GLenum func)
{
    EVENT("(GLenum func = 0x%X)", func);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (func)
        {
          case GL_NEVER:
          case GL_ALWAYS:
          case GL_LESS:
          case GL_LEQUAL:
          case GL_EQUAL:
          case GL_GREATER:
          case GL_GEQUAL:
          case GL_NOTEQUAL:
            context->getState().setDepthFunc(func);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY DepthMask(GLboolean flag)
{
    EVENT("(GLboolean flag = %u)", flag);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setDepthMask(flag != GL_FALSE);
    }
}

void GL_APIENTRY DepthRangef(GLclampf zNear, GLclampf zFar)
{
    EVENT("(GLclampf zNear = %f, GLclampf zFar = %f)", zNear, zFar);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setDepthRange(zNear, zFar);
    }
}

void GL_APIENTRY DetachShader(GLuint program, GLuint shader)
{
    EVENT("(GLuint program = %d, GLuint shader = %d)", program, shader);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }

        if (!programObject->detachShader(shaderObject))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
    }
}

void GL_APIENTRY Disable(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setEnableFeature(cap, false);
    }
}

void GL_APIENTRY DisableVertexAttribArray(GLuint index)
{
    EVENT("(GLuint index = %d)", index);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setEnableVertexAttribArray(index, false);
    }
}

void GL_APIENTRY DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d)", mode, first, count);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateDrawArrays(context, mode, first, count, 0))
        {
            return;
        }

        Error error = context->drawArrays(mode, first, count);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p)",
          mode, count, type, indices);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        IndexRange indexRange;
        if (!ValidateDrawElements(context, mode, count, type, indices, 0, &indexRange))
        {
            return;
        }

        Error error = context->drawElements(mode, count, type, indices, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY Enable(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (context->getLimitations().noSampleAlphaToCoverageSupport)
        {
            if (cap == GL_SAMPLE_ALPHA_TO_COVERAGE)
            {
                const char *errorMessage = "Current renderer doesn't support alpha-to-coverage";
                context->recordError(Error(GL_INVALID_OPERATION, errorMessage));

                // We also output an error message to the debugger window if tracing is active, so that developers can see the error message.
                ERR("%s", errorMessage);

                return;
            }
        }

        context->getState().setEnableFeature(cap, true);
    }
}

void GL_APIENTRY EnableVertexAttribArray(GLuint index)
{
    EVENT("(GLuint index = %d)", index);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setEnableVertexAttribArray(index, true);
    }
}

void GL_APIENTRY Finish(void)
{
    EVENT("()");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Error error = context->finish();
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY Flush(void)
{
    EVENT("()");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Error error = context->flush();
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum renderbuffertarget = 0x%X, "
          "GLuint renderbuffer = %d)", target, attachment, renderbuffertarget, renderbuffer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target) || (renderbuffertarget != GL_RENDERBUFFER && renderbuffer != 0))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidateFramebufferRenderbufferParameters(context, target, attachment, renderbuffertarget, renderbuffer))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (renderbuffer != 0)
        {
            Renderbuffer *renderbufferObject = context->getRenderbuffer(renderbuffer);
            framebuffer->setAttachment(GL_RENDERBUFFER, attachment, gl::ImageIndex::MakeInvalid(), renderbufferObject);
        }
        else
        {
            framebuffer->resetAttachment(attachment);
        }
    }
}

void GL_APIENTRY FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum textarget = 0x%X, "
          "GLuint texture = %d, GLint level = %d)", target, attachment, textarget, texture, level);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateFramebufferTexture2D(context, target, attachment, textarget, texture, level))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (texture != 0)
        {
            Texture *textureObj = context->getTexture(texture);

            ImageIndex index = ImageIndex::MakeInvalid();

            if (textarget == GL_TEXTURE_2D)
            {
                index = ImageIndex::Make2D(level);
            }
            else
            {
                ASSERT(IsCubeMapTextureTarget(textarget));
                index = ImageIndex::MakeCube(textarget, level);
            }

            framebuffer->setAttachment(GL_TEXTURE, attachment, index, textureObj);
        }
        else
        {
            framebuffer->resetAttachment(attachment);
        }
    }
}

void GL_APIENTRY FrontFace(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (mode)
        {
          case GL_CW:
          case GL_CCW:
            context->getState().setFrontFace(mode);
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GenBuffers(GLsizei n, GLuint* buffers)
{
    EVENT("(GLsizei n = %d, GLuint* buffers = 0x%0.8p)", n, buffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            buffers[i] = context->createBuffer();
        }
    }
}

void GL_APIENTRY GenerateMipmap(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (texture == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GLenum baseTarget = (target == GL_TEXTURE_CUBE_MAP) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X : target;
        GLenum internalFormat = texture->getInternalFormat(baseTarget, 0);
        const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
        const InternalFormat &formatInfo = GetInternalFormatInfo(internalFormat);

        // GenerateMipmap should not generate an INVALID_OPERATION for textures created with
        // unsized formats or that are color renderable and filterable.  Since we do not track if
        // the texture was created with sized or unsized format (only sized formats are stored),
        // it is not possible to make sure the the LUMA formats can generate mipmaps (they should
        // be able to) because they aren't color renderable.  Simply do a special case for LUMA
        // textures since they're the only texture format that can be created with unsized formats
        // that is not color renderable.  New unsized formats are unlikely to be added, since ES2
        // was the last version to use add them.
        bool isLUMA = internalFormat == GL_LUMINANCE8_EXT ||
                      internalFormat == GL_LUMINANCE8_ALPHA8_EXT ||
                      internalFormat == GL_ALPHA8_EXT;

        if (formatInfo.depthBits > 0 || formatInfo.stencilBits > 0 || !formatCaps.filterable ||
            (!formatCaps.renderable && !isLUMA) || formatInfo.compressed)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // GL_EXT_sRGB does not support mipmap generation on sRGB textures
        if (context->getClientVersion() == 2 && formatInfo.colorEncoding == GL_SRGB)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Non-power of 2 ES2 check
        if (!context->getExtensions().textureNPOT &&
            (!isPow2(static_cast<int>(texture->getWidth(baseTarget, 0))) ||
             !isPow2(static_cast<int>(texture->getHeight(baseTarget, 0)))))
        {
            ASSERT(context->getClientVersion() <= 2 && (target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP));
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Cube completeness check
        if (target == GL_TEXTURE_CUBE_MAP && !texture->isCubeComplete())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = texture->generateMipmaps();
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY GenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    EVENT("(GLsizei n = %d, GLuint* framebuffers = 0x%0.8p)", n, framebuffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            framebuffers[i] = context->createFramebuffer();
        }
    }
}

void GL_APIENTRY GenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    EVENT("(GLsizei n = %d, GLuint* renderbuffers = 0x%0.8p)", n, renderbuffers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            renderbuffers[i] = context->createRenderbuffer();
        }
    }
}

void GL_APIENTRY GenTextures(GLsizei n, GLuint* textures)
{
    EVENT("(GLsizei n = %d, GLuint* textures = 0x%0.8p)", n, textures);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            textures[i] = context->createTexture();
        }
    }
}

void GL_APIENTRY GetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, GLsizei bufsize = %d, GLsizei *length = 0x%0.8p, "
          "GLint *size = 0x%0.8p, GLenum *type = %0.8p, GLchar *name = %0.8p)",
          program, index, bufsize, length, size, type, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (index >= (GLuint)programObject->getActiveAttributeCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveAttribute(index, bufsize, length, size, type, name);
    }
}

void GL_APIENTRY GetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, GLsizei bufsize = %d, "
          "GLsizei* length = 0x%0.8p, GLint* size = 0x%0.8p, GLenum* type = 0x%0.8p, GLchar* name = 0x%0.8p)",
          program, index, bufsize, length, size, type, name);


    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (index >= (GLuint)programObject->getActiveUniformCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveUniform(index, bufsize, length, size, type, name);
    }
}

void GL_APIENTRY GetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    EVENT("(GLuint program = %d, GLsizei maxcount = %d, GLsizei* count = 0x%0.8p, GLuint* shaders = 0x%0.8p)",
          program, maxcount, count, shaders);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (maxcount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        return programObject->getAttachedShaders(maxcount, count, shaders);
    }
}

GLint GL_APIENTRY GetAttribLocation(GLuint program, const GLchar* name)
{
    EVENT("(GLuint program = %d, const GLchar* name = %s)", program, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return -1;
        }

        if (!programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getAttributeLocation(name);
    }

    return -1;
}

void GL_APIENTRY GetBooleanv(GLenum pname, GLboolean* params)
{
    EVENT("(GLenum pname = 0x%X, GLboolean* params = 0x%0.8p)",  pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_BOOL)
        {
            context->getBooleanv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GL_APIENTRY GetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidBufferParameter(context, pname))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            // A null buffer means that "0" is bound to the requested buffer target
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (pname)
        {
          case GL_BUFFER_USAGE:
            *params = static_cast<GLint>(buffer->getUsage());
            break;
          case GL_BUFFER_SIZE:
            *params = clampCast<GLint>(buffer->getSize());
            break;
          case GL_BUFFER_ACCESS_FLAGS:
            *params = buffer->getAccessFlags();
            break;
          case GL_BUFFER_ACCESS_OES:
            *params = buffer->getAccess();
            break;
          case GL_BUFFER_MAPPED:
            static_assert(GL_BUFFER_MAPPED == GL_BUFFER_MAPPED_OES, "GL enums should be equal.");
            *params = static_cast<GLint>(buffer->isMapped());
            break;
          case GL_BUFFER_MAP_OFFSET:
            *params = clampCast<GLint>(buffer->getMapOffset());
            break;
          case GL_BUFFER_MAP_LENGTH:
            *params = clampCast<GLint>(buffer->getMapLength());
            break;
          default: UNREACHABLE(); break;
        }
    }
}

GLenum GL_APIENTRY GetError(void)
{
    EVENT("()");

    Context *context = GetGlobalContext();

    if (context)
    {
        return context->getError();
    }

    return GL_NO_ERROR;
}

void GL_APIENTRY GetFloatv(GLenum pname, GLfloat* params)
{
    EVENT("(GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_FLOAT)
        {
            context->getFloatv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GL_APIENTRY GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          target, attachment, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        int clientVersion = context->getClientVersion();

        switch (pname)
        {
          case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
          case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            break;

          case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            if (clientVersion < 3 && !context->getExtensions().sRGB)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // Determine if the attachment is a valid enum
        switch (attachment)
        {
          case GL_BACK:
          case GL_FRONT:
          case GL_DEPTH:
          case GL_STENCIL:
          case GL_DEPTH_STENCIL_ATTACHMENT:
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          case GL_DEPTH_ATTACHMENT:
          case GL_STENCIL_ATTACHMENT:
            break;

          default:
            if (attachment < GL_COLOR_ATTACHMENT0_EXT ||
                (attachment - GL_COLOR_ATTACHMENT0_EXT) >= context->getCaps().maxColorAttachments)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;
        }

        const Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->id() == 0)
        {
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }

            switch (attachment)
            {
              case GL_BACK:
              case GL_DEPTH:
              case GL_STENCIL:
                break;

              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }
        else
        {
            if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
            {
                // Valid attachment query
            }
            else
            {
                switch (attachment)
                {
                  case GL_DEPTH_ATTACHMENT:
                  case GL_STENCIL_ATTACHMENT:
                    break;

                  case GL_DEPTH_STENCIL_ATTACHMENT:
                    if (framebuffer->hasValidDepthStencil())
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return;
                    }
                    break;

                  default:
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
            }
        }

        const FramebufferAttachment *attachmentObject = framebuffer->getAttachment(attachment);
        if (attachmentObject)
        {
            ASSERT(attachmentObject->type() == GL_RENDERBUFFER ||
                   attachmentObject->type() == GL_TEXTURE ||
                   attachmentObject->type() == GL_FRAMEBUFFER_DEFAULT);

            switch (pname)
            {
              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = attachmentObject->type();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (attachmentObject->type() != GL_RENDERBUFFER && attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->id();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->mipLevel();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->cubeMapFace();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
                *params = attachmentObject->getRedSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
                *params = attachmentObject->getGreenSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
                *params = attachmentObject->getBlueSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
                *params = attachmentObject->getAlphaSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
                *params = attachmentObject->getDepthSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
                *params = attachmentObject->getStencilSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
                if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
                *params = attachmentObject->getComponentType();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
                *params = attachmentObject->getColorEncoding();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->layer();
                break;

              default:
                UNREACHABLE();
                break;
            }
        }
        else
        {
            // ES 2.0.25 spec pg 127 states that if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
            // is NONE, then querying any other pname will generate INVALID_ENUM.

            // ES 3.0.2 spec pg 235 states that if the attachment type is none,
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero and be an
            // INVALID_OPERATION for all other pnames

            switch (pname)
            {
              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = GL_NONE;
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (clientVersion < 3)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = 0;
                break;

              default:
                if (clientVersion < 3)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                else
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
            }
        }
    }
}

void GL_APIENTRY GetIntegerv(GLenum pname, GLint* params)
{
    EVENT("(GLenum pname = 0x%X, GLint* params = 0x%0.8p)", pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;

        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_INT)
        {
            context->getIntegerv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GL_APIENTRY GetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %d, GLenum pname = %d, GLint* params = 0x%0.8p)", program, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (context->getClientVersion() < 3)
        {
            switch (pname)
            {
              case GL_ACTIVE_UNIFORM_BLOCKS:
              case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
              case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
              case GL_TRANSFORM_FEEDBACK_VARYINGS:
              case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
        }

        switch (pname)
        {
          case GL_DELETE_STATUS:
            *params = programObject->isFlaggedForDeletion();
            return;
          case GL_LINK_STATUS:
            *params = programObject->isLinked();
            return;
          case GL_VALIDATE_STATUS:
            *params = programObject->isValidated();
            return;
          case GL_INFO_LOG_LENGTH:
            *params = programObject->getInfoLogLength();
            return;
          case GL_ATTACHED_SHADERS:
            *params = programObject->getAttachedShadersCount();
            return;
          case GL_ACTIVE_ATTRIBUTES:
            *params = programObject->getActiveAttributeCount();
            return;
          case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
            *params = programObject->getActiveAttributeMaxLength();
            return;
          case GL_ACTIVE_UNIFORMS:
            *params = programObject->getActiveUniformCount();
            return;
          case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            *params = programObject->getActiveUniformMaxLength();
            return;
          case GL_PROGRAM_BINARY_LENGTH_OES:
            *params = programObject->getBinaryLength();
            return;
          case GL_ACTIVE_UNIFORM_BLOCKS:
            *params = programObject->getActiveUniformBlockCount();
            return;
          case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
            *params = programObject->getActiveUniformBlockMaxLength();
            break;
          case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
            *params = programObject->getTransformFeedbackBufferMode();
            break;
          case GL_TRANSFORM_FEEDBACK_VARYINGS:
            *params = programObject->getTransformFeedbackVaryingCount();
            break;
          case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
            *params = programObject->getTransformFeedbackVaryingMaxLength();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    EVENT("(GLuint program = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* infolog = 0x%0.8p)",
          program, bufsize, length, infolog);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        programObject->getInfoLog(bufsize, length, infolog);
    }
}

void GL_APIENTRY GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (context->getState().getRenderbufferId() == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Renderbuffer *renderbuffer = context->getRenderbuffer(context->getState().getRenderbufferId());

        switch (pname)
        {
          case GL_RENDERBUFFER_WIDTH:           *params = renderbuffer->getWidth();          break;
          case GL_RENDERBUFFER_HEIGHT:          *params = renderbuffer->getHeight();         break;
          case GL_RENDERBUFFER_INTERNAL_FORMAT: *params = renderbuffer->getInternalFormat(); break;
          case GL_RENDERBUFFER_RED_SIZE:        *params = renderbuffer->getRedSize();        break;
          case GL_RENDERBUFFER_GREEN_SIZE:      *params = renderbuffer->getGreenSize();      break;
          case GL_RENDERBUFFER_BLUE_SIZE:       *params = renderbuffer->getBlueSize();       break;
          case GL_RENDERBUFFER_ALPHA_SIZE:      *params = renderbuffer->getAlphaSize();      break;
          case GL_RENDERBUFFER_DEPTH_SIZE:      *params = renderbuffer->getDepthSize();      break;
          case GL_RENDERBUFFER_STENCIL_SIZE:    *params = renderbuffer->getStencilSize();    break;

          case GL_RENDERBUFFER_SAMPLES_ANGLE:
            if (!context->getExtensions().framebufferMultisample)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = renderbuffer->getSamples();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    EVENT("(GLuint shader = %d, GLenum pname = %d, GLint* params = 0x%0.8p)", shader, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }

        switch (pname)
        {
          case GL_SHADER_TYPE:
            *params = shaderObject->getType();
            return;
          case GL_DELETE_STATUS:
            *params = shaderObject->isFlaggedForDeletion();
            return;
          case GL_COMPILE_STATUS:
            *params = shaderObject->isCompiled() ? GL_TRUE : GL_FALSE;
            return;
          case GL_INFO_LOG_LENGTH:
            *params = shaderObject->getInfoLogLength();
            return;
          case GL_SHADER_SOURCE_LENGTH:
            *params = shaderObject->getSourceLength();
            return;
          case GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE:
            *params = shaderObject->getTranslatedSourceLength();
            return;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    EVENT("(GLuint shader = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* infolog = 0x%0.8p)",
          shader, bufsize, length, infolog);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }

        shaderObject->getInfoLog(bufsize, length, infolog);
    }
}

void GL_APIENTRY GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    EVENT("(GLenum shadertype = 0x%X, GLenum precisiontype = 0x%X, GLint* range = 0x%0.8p, GLint* precision = 0x%0.8p)",
          shadertype, precisiontype, range, precision);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (shadertype)
        {
          case GL_VERTEX_SHADER:
            switch (precisiontype)
            {
              case GL_LOW_FLOAT:
                context->getCaps().vertexLowpFloat.get(range, precision);
                break;
              case GL_MEDIUM_FLOAT:
                context->getCaps().vertexMediumpFloat.get(range, precision);
                break;
              case GL_HIGH_FLOAT:
                context->getCaps().vertexHighpFloat.get(range, precision);
                break;

              case GL_LOW_INT:
                context->getCaps().vertexLowpInt.get(range, precision);
                break;
              case GL_MEDIUM_INT:
                context->getCaps().vertexMediumpInt.get(range, precision);
                break;
              case GL_HIGH_INT:
                context->getCaps().vertexHighpInt.get(range, precision);
                break;

              default:
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;
          case GL_FRAGMENT_SHADER:
            switch (precisiontype)
            {
              case GL_LOW_FLOAT:
                context->getCaps().fragmentLowpFloat.get(range, precision);
                break;
              case GL_MEDIUM_FLOAT:
                context->getCaps().fragmentMediumpFloat.get(range, precision);
                break;
              case GL_HIGH_FLOAT:
                context->getCaps().fragmentHighpFloat.get(range, precision);
                break;

              case GL_LOW_INT:
                context->getCaps().fragmentLowpInt.get(range, precision);
                break;
              case GL_MEDIUM_INT:
                context->getCaps().fragmentMediumpInt.get(range, precision);
                break;
              case GL_HIGH_INT:
                context->getCaps().fragmentHighpInt.get(range, precision);
                break;

              default:
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

    }
}

void GL_APIENTRY GetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    EVENT("(GLuint shader = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* source = 0x%0.8p)",
          shader, bufsize, length, source);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }

        shaderObject->getSource(bufsize, length, source);
    }
}

const GLubyte *GL_APIENTRY GetString(GLenum name)
{
    EVENT("(GLenum name = 0x%X)", name);

    Context *context = GetValidGlobalContext();

    if (context)
    {
        switch (name)
        {
            case GL_VENDOR:
                return reinterpret_cast<const GLubyte *>("Google Inc.");

            case GL_RENDERER:
                return reinterpret_cast<const GLubyte *>(context->getRendererString().c_str());

            case GL_VERSION:
                if (context->getClientVersion() == 2)
                {
                    return reinterpret_cast<const GLubyte *>(
                        "OpenGL ES 2.0 (ANGLE " ANGLE_VERSION_STRING ")");
                }
                else
                {
                    return reinterpret_cast<const GLubyte *>(
                        "OpenGL ES 3.0 (ANGLE " ANGLE_VERSION_STRING ")");
                }

            case GL_SHADING_LANGUAGE_VERSION:
                if (context->getClientVersion() == 2)
                {
                    return reinterpret_cast<const GLubyte *>(
                        "OpenGL ES GLSL ES 1.00 (ANGLE " ANGLE_VERSION_STRING ")");
                }
                else
                {
                    return reinterpret_cast<const GLubyte *>(
                        "OpenGL ES GLSL ES 3.00 (ANGLE " ANGLE_VERSION_STRING ")");
                }

            case GL_EXTENSIONS:
                return reinterpret_cast<const GLubyte *>(context->getExtensionString().c_str());

            default:
            context->recordError(Error(GL_INVALID_ENUM));
            return nullptr;
        }
    }

    return nullptr;
}

void GL_APIENTRY GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM, "Invalid texture target"));
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_MAG_FILTER:
              *params = (GLfloat)texture->getMagFilter();
            break;
          case GL_TEXTURE_MIN_FILTER:
              *params = (GLfloat)texture->getMinFilter();
            break;
          case GL_TEXTURE_WRAP_S:
              *params = (GLfloat)texture->getWrapS();
            break;
          case GL_TEXTURE_WRAP_T:
              *params = (GLfloat)texture->getWrapT();
            break;
          case GL_TEXTURE_WRAP_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getWrapR();
            break;
          case GL_TEXTURE_IMMUTABLE_FORMAT:
            // Exposed to ES2.0 through EXT_texture_storage, no client version validation.
            *params = (GLfloat)(texture->getImmutableFormat() ? GL_TRUE : GL_FALSE);
            break;
          case GL_TEXTURE_IMMUTABLE_LEVELS:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getImmutableLevels();
            break;
          case GL_TEXTURE_USAGE_ANGLE:
            *params = (GLfloat)texture->getUsage();
            break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!context->getExtensions().textureFilterAnisotropic)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getMaxAnisotropy();
            break;
          case GL_TEXTURE_SWIZZLE_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSwizzleRed();
            break;
          case GL_TEXTURE_SWIZZLE_G:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSwizzleGreen();
            break;
          case GL_TEXTURE_SWIZZLE_B:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSwizzleBlue();
            break;
          case GL_TEXTURE_SWIZZLE_A:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSwizzleAlpha();
            break;
          case GL_TEXTURE_BASE_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getBaseLevel();
            break;
          case GL_TEXTURE_MAX_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getMaxLevel();
            break;
          case GL_TEXTURE_MIN_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().minLod;
            break;
          case GL_TEXTURE_MAX_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().maxLod;
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM, "Invalid texture target"));
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_MAG_FILTER:
            *params = texture->getSamplerState().magFilter;
            break;
          case GL_TEXTURE_MIN_FILTER:
            *params = texture->getSamplerState().minFilter;
            break;
          case GL_TEXTURE_WRAP_S:
            *params = texture->getSamplerState().wrapS;
            break;
          case GL_TEXTURE_WRAP_T:
            *params = texture->getSamplerState().wrapT;
            break;
          case GL_TEXTURE_WRAP_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().wrapR;
            break;
          case GL_TEXTURE_IMMUTABLE_FORMAT:
            // Exposed to ES2.0 through EXT_texture_storage, no client version validation.
            *params = texture->getImmutableFormat() ? GL_TRUE : GL_FALSE;
            break;
          case GL_TEXTURE_IMMUTABLE_LEVELS:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = static_cast<GLint>(texture->getImmutableLevels());
            break;
          case GL_TEXTURE_USAGE_ANGLE:
            *params = texture->getUsage();
            break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!context->getExtensions().textureFilterAnisotropic)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getMaxAnisotropy();
            break;
          case GL_TEXTURE_SWIZZLE_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSwizzleRed();
            break;
          case GL_TEXTURE_SWIZZLE_G:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSwizzleGreen();
            break;
          case GL_TEXTURE_SWIZZLE_B:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSwizzleBlue();
            break;
          case GL_TEXTURE_SWIZZLE_A:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSwizzleAlpha();
            break;
          case GL_TEXTURE_BASE_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getBaseLevel();
            break;
          case GL_TEXTURE_MAX_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getMaxLevel();
            break;
          case GL_TEXTURE_MIN_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getMinLod();
            break;
          case GL_TEXTURE_MAX_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getMaxLod();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLfloat* params = 0x%0.8p)", program, location, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateGetUniformfv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformfv(location, params);
    }
}

void GL_APIENTRY GetUniformiv(GLuint program, GLint location, GLint* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLint* params = 0x%0.8p)", program, location, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateGetUniformiv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformiv(location, params);
    }
}

GLint GL_APIENTRY GetUniformLocation(GLuint program, const GLchar* name)
{
    EVENT("(GLuint program = %d, const GLchar* name = 0x%0.8p)", program, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (strstr(name, "gl_") == name)
        {
            return -1;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return -1;
        }

        if (!programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getUniformLocation(name);
    }

    return -1;
}

void GL_APIENTRY GetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", index, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                params[i] = currentValueData.FloatValues[i];
            }
        }
        else
        {
            const VertexAttribute &attribState = context->getState().getVertexArray()->getVertexAttribute(index);
            *params = QuerySingleVertexAttributeParameter<GLfloat>(attribState, pname);
        }
    }
}

void GL_APIENTRY GetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", index, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                float currentValue = currentValueData.FloatValues[i];
                params[i] = iround<GLint>(currentValue);
            }
        }
        else
        {
            const VertexAttribute &attribState = context->getState().getVertexArray()->getVertexAttribute(index);
            *params = QuerySingleVertexAttributeParameter<GLint>(attribState, pname);
        }
    }
}

void GL_APIENTRY GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLvoid** pointer = 0x%0.8p)", index, pname, pointer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        *pointer = const_cast<GLvoid*>(context->getState().getVertexAttribPointer(index));
    }
}

void GL_APIENTRY Hint(GLenum target, GLenum mode)
{
    EVENT("(GLenum target = 0x%X, GLenum mode = 0x%X)", target, mode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (mode)
        {
          case GL_FASTEST:
          case GL_NICEST:
          case GL_DONT_CARE:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_GENERATE_MIPMAP_HINT:
            context->getState().setGenerateMipmapHint(mode);
            break;

          case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:
            context->getState().setFragmentShaderDerivativeHint(mode);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

GLboolean GL_APIENTRY IsBuffer(GLuint buffer)
{
    EVENT("(GLuint buffer = %d)", buffer);

    Context *context = GetValidGlobalContext();
    if (context && buffer)
    {
        Buffer *bufferObject = context->getBuffer(buffer);

        if (bufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsEnabled(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return GL_FALSE;
        }

        return context->getState().getEnableFeature(cap);
    }

    return false;
}

GLboolean GL_APIENTRY IsFramebuffer(GLuint framebuffer)
{
    EVENT("(GLuint framebuffer = %d)", framebuffer);

    Context *context = GetValidGlobalContext();
    if (context && framebuffer)
    {
        Framebuffer *framebufferObject = context->getFramebuffer(framebuffer);

        if (framebufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = GetValidGlobalContext();
    if (context && program)
    {
        Program *programObject = context->getProgram(program);

        if (programObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsRenderbuffer(GLuint renderbuffer)
{
    EVENT("(GLuint renderbuffer = %d)", renderbuffer);

    Context *context = GetValidGlobalContext();
    if (context && renderbuffer)
    {
        Renderbuffer *renderbufferObject = context->getRenderbuffer(renderbuffer);

        if (renderbufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = GetValidGlobalContext();
    if (context && shader)
    {
        Shader *shaderObject = context->getShader(shader);

        if (shaderObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsTexture(GLuint texture)
{
    EVENT("(GLuint texture = %d)", texture);

    Context *context = GetValidGlobalContext();
    if (context && texture)
    {
        Texture *textureObject = context->getTexture(texture);

        if (textureObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

void GL_APIENTRY LineWidth(GLfloat width)
{
    EVENT("(GLfloat width = %f)", width);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (width <= 0.0f)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setLineWidth(width);
    }
}

void GL_APIENTRY LinkProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        Error error = programObject->link(context->getData());
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY PixelStorei(GLenum pname, GLint param)
{
    EVENT("(GLenum pname = 0x%X, GLint param = %d)", pname, param);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            switch (pname)
            {
              case GL_UNPACK_IMAGE_HEIGHT:
              case GL_UNPACK_SKIP_IMAGES:
                  context->recordError(Error(GL_INVALID_ENUM));
                  return;

              case GL_UNPACK_ROW_LENGTH:
              case GL_UNPACK_SKIP_ROWS:
              case GL_UNPACK_SKIP_PIXELS:
                  if (!context->getExtensions().unpackSubimage)
                  {
                      context->recordError(Error(GL_INVALID_ENUM));
                      return;
                  }
                  break;

              case GL_PACK_ROW_LENGTH:
              case GL_PACK_SKIP_ROWS:
              case GL_PACK_SKIP_PIXELS:
                  if (!context->getExtensions().packSubimage)
                  {
                      context->recordError(Error(GL_INVALID_ENUM));
                      return;
                  }
                  break;
            }
        }

        if (param < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE, "Cannot use negative values in PixelStorei"));
            return;
        }

        State &state = context->getState();

        switch (pname)
        {
          case GL_UNPACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            state.setUnpackAlignment(param);
            break;

          case GL_PACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            state.setPackAlignment(param);
            break;

          case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
            state.setPackReverseRowOrder(param != 0);
            break;

          case GL_UNPACK_ROW_LENGTH:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().unpackSubimage);
            state.setUnpackRowLength(param);
            break;

          case GL_UNPACK_IMAGE_HEIGHT:
            ASSERT(context->getClientVersion() >= 3);
            state.getUnpackState().imageHeight = param;
            break;

          case GL_UNPACK_SKIP_IMAGES:
            ASSERT(context->getClientVersion() >= 3);
            state.getUnpackState().skipImages = param;
            break;

          case GL_UNPACK_SKIP_ROWS:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().unpackSubimage);
            state.getUnpackState().skipRows = param;
            break;

          case GL_UNPACK_SKIP_PIXELS:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().unpackSubimage);
            state.getUnpackState().skipPixels = param;
            break;

          case GL_PACK_ROW_LENGTH:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().packSubimage);
            state.getPackState().rowLength = param;
            break;

          case GL_PACK_SKIP_ROWS:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().packSubimage);
            state.getPackState().skipRows = param;
            break;

          case GL_PACK_SKIP_PIXELS:
              ASSERT((context->getClientVersion() >= 3) || context->getExtensions().packSubimage);
            state.getPackState().skipPixels = param;
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY PolygonOffset(GLfloat factor, GLfloat units)
{
    EVENT("(GLfloat factor = %f, GLfloat units = %f)", factor, units);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        context->getState().setPolygonOffsetParams(factor, units);
    }
}

void GL_APIENTRY ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                            GLenum format, GLenum type, GLvoid* pixels)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, GLvoid* pixels = 0x%0.8p)",
          x, y, width, height, format, type,  pixels);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateReadPixelsParameters(context, x, y, width, height,
                                              format, type, NULL, pixels))
        {
            return;
        }

        Framebuffer *framebufferObject = context->getState().getReadFramebuffer();
        ASSERT(framebufferObject);

        Rectangle area(x, y, width, height);
        Error error = framebufferObject->readPixels(context, area, format, type, pixels);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ReleaseShaderCompiler(void)
{
    EVENT("()");

    Context *context = GetValidGlobalContext();

    if (context)
    {
        Compiler *compiler = context->getCompiler();
        Error error = compiler->release();
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
          target, internalformat, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateRenderbufferStorageParametersANGLE(context, target, 0, internalformat,
                                                        width, height))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        Error error = renderbuffer->setStorage(internalformat, width, height);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY SampleCoverage(GLclampf value, GLboolean invert)
{
    EVENT("(GLclampf value = %f, GLboolean invert = %u)", value, invert);

    Context* context = GetValidGlobalContext();

    if (context)
    {
        context->getState().setSampleCoverageParams(clamp01(value), invert == GL_TRUE);
    }
}

void GL_APIENTRY Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)", x, y, width, height);

    Context* context = GetValidGlobalContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setScissorParams(x, y, width, height);
    }
}

void GL_APIENTRY ShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    EVENT("(GLsizei n = %d, const GLuint* shaders = 0x%0.8p, GLenum binaryformat = 0x%X, "
          "const GLvoid* binary = 0x%0.8p, GLsizei length = %d)",
          n, shaders, binaryformat, binary, length);

    Context* context = GetValidGlobalContext();
    if (context)
    {
        const std::vector<GLenum> &shaderBinaryFormats = context->getCaps().shaderBinaryFormats;
        if (std::find(shaderBinaryFormats.begin(), shaderBinaryFormats.end(), binaryformat) == shaderBinaryFormats.end())
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // No binary shader formats are supported.
        UNIMPLEMENTED();
    }
}

void GL_APIENTRY ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    EVENT("(GLuint shader = %d, GLsizei count = %d, const GLchar** string = 0x%0.8p, const GLint* length = 0x%0.8p)",
          shader, count, string, length);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = GetValidShader(context, shader);
        if (!shaderObject)
        {
            return;
        }
        shaderObject->setSource(count, string, length);
    }
}

void GL_APIENTRY StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    StencilFuncSeparate(GL_FRONT_AND_BACK, func, ref, mask);
}

void GL_APIENTRY StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    EVENT("(GLenum face = 0x%X, GLenum func = 0x%X, GLint ref = %d, GLuint mask = %d)", face, func, ref, mask);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (func)
        {
          case GL_NEVER:
          case GL_ALWAYS:
          case GL_LESS:
          case GL_LEQUAL:
          case GL_EQUAL:
          case GL_GEQUAL:
          case GL_GREATER:
          case GL_NOTEQUAL:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilParams(func, ref, mask);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackParams(func, ref, mask);
        }
    }
}

void GL_APIENTRY StencilMask(GLuint mask)
{
    StencilMaskSeparate(GL_FRONT_AND_BACK, mask);
}

void GL_APIENTRY StencilMaskSeparate(GLenum face, GLuint mask)
{
    EVENT("(GLenum face = 0x%X, GLuint mask = %d)", face, mask);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilWritemask(mask);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackWritemask(mask);
        }
    }
}

void GL_APIENTRY StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    StencilOpSeparate(GL_FRONT_AND_BACK, fail, zfail, zpass);
}

void GL_APIENTRY StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    EVENT("(GLenum face = 0x%X, GLenum fail = 0x%X, GLenum zfail = 0x%X, GLenum zpas = 0x%Xs)",
          face, fail, zfail, zpass);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (fail)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (zfail)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (zpass)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilOperations(fail, zfail, zpass);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackOperations(fail, zfail, zpass);
        }
    }
}

void GL_APIENTRY TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                            GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint internalformat = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLint border = %d, GLenum format = 0x%X, GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, border, format, type, pixels);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, width, height, border, format, type, pixels))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, 0, width, height, 1, border, format, type, pixels))
        {
            return;
        }

        Extents size(width, height, 1);
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error = texture->setImage(context, target, level, internalformat, size, format, type,
                                        reinterpret_cast<const uint8_t *>(pixels));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint param = %f)", target, pname, param);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM, "Invalid texture target"));
            return;
        }

        if (!ValidateTexParamParameters(context, pname, static_cast<GLint>(param)))
        {
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // clang-format off
        switch (pname)
        {
          case GL_TEXTURE_WRAP_S:               texture->setWrapS(uiround<GLenum>(param));        break;
          case GL_TEXTURE_WRAP_T:               texture->setWrapT(uiround<GLenum>(param));        break;
          case GL_TEXTURE_WRAP_R:               texture->setWrapR(uiround<GLenum>(param));        break;
          case GL_TEXTURE_MIN_FILTER:           texture->setMinFilter(uiround<GLenum>(param));    break;
          case GL_TEXTURE_MAG_FILTER:           texture->setMagFilter(uiround<GLenum>(param));    break;
          case GL_TEXTURE_USAGE_ANGLE:          texture->setUsage(uiround<GLenum>(param));        break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:   texture->setMaxAnisotropy(std::min(param, context->getExtensions().maxTextureAnisotropy)); break;
          case GL_TEXTURE_COMPARE_MODE:         texture->setCompareMode(uiround<GLenum>(param));  break;
          case GL_TEXTURE_COMPARE_FUNC:         texture->setCompareFunc(uiround<GLenum>(param));  break;
          case GL_TEXTURE_SWIZZLE_R:            texture->setSwizzleRed(uiround<GLenum>(param));   break;
          case GL_TEXTURE_SWIZZLE_G:            texture->setSwizzleGreen(uiround<GLenum>(param)); break;
          case GL_TEXTURE_SWIZZLE_B:            texture->setSwizzleBlue(uiround<GLenum>(param));  break;
          case GL_TEXTURE_SWIZZLE_A:            texture->setSwizzleAlpha(uiround<GLenum>(param)); break;
          case GL_TEXTURE_BASE_LEVEL:           texture->setBaseLevel(uiround<GLuint>(param));    break;
          case GL_TEXTURE_MAX_LEVEL:            texture->setMaxLevel(uiround<GLuint>(param));     break;
          case GL_TEXTURE_MIN_LOD:              texture->setMinLod(param);                        break;
          case GL_TEXTURE_MAX_LOD:              texture->setMaxLod(param);                        break;
          default: UNREACHABLE(); break;
        }
        // clang-format on
    }
}

void GL_APIENTRY TexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    TexParameterf(target, pname, (GLfloat)*params);
}

void GL_APIENTRY TexParameteri(GLenum target, GLenum pname, GLint param)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint param = %d)", target, pname, param);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM, "Invalid Texture target"));
            return;
        }

        if (!ValidateTexParamParameters(context, pname, param))
        {
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // clang-format off
        switch (pname)
        {
          case GL_TEXTURE_WRAP_S:               texture->setWrapS(static_cast<GLenum>(param));        break;
          case GL_TEXTURE_WRAP_T:               texture->setWrapT(static_cast<GLenum>(param));        break;
          case GL_TEXTURE_WRAP_R:               texture->setWrapR(static_cast<GLenum>(param));        break;
          case GL_TEXTURE_MIN_FILTER:           texture->setMinFilter(static_cast<GLenum>(param));    break;
          case GL_TEXTURE_MAG_FILTER:           texture->setMagFilter(static_cast<GLenum>(param));    break;
          case GL_TEXTURE_USAGE_ANGLE:          texture->setUsage(static_cast<GLenum>(param));        break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:   texture->setMaxAnisotropy(std::min(static_cast<GLfloat>(param), context->getExtensions().maxTextureAnisotropy)); break;
          case GL_TEXTURE_COMPARE_MODE:         texture->setCompareMode(static_cast<GLenum>(param));  break;
          case GL_TEXTURE_COMPARE_FUNC:         texture->setCompareFunc(static_cast<GLenum>(param));  break;
          case GL_TEXTURE_SWIZZLE_R:            texture->setSwizzleRed(static_cast<GLenum>(param));   break;
          case GL_TEXTURE_SWIZZLE_G:            texture->setSwizzleGreen(static_cast<GLenum>(param)); break;
          case GL_TEXTURE_SWIZZLE_B:            texture->setSwizzleBlue(static_cast<GLenum>(param));  break;
          case GL_TEXTURE_SWIZZLE_A:            texture->setSwizzleAlpha(static_cast<GLenum>(param)); break;
          case GL_TEXTURE_BASE_LEVEL:           texture->setBaseLevel(static_cast<GLuint>(param));    break;
          case GL_TEXTURE_MAX_LEVEL:            texture->setMaxLevel(static_cast<GLuint>(param));     break;
          case GL_TEXTURE_MIN_LOD:              texture->setMinLod(static_cast<GLfloat>(param));      break;
          case GL_TEXTURE_MAX_LOD:              texture->setMaxLod(static_cast<GLfloat>(param));      break;
          default: UNREACHABLE(); break;
        }
        // clang-format on
    }
}

void GL_APIENTRY TexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    TexParameteri(target, pname, *params);
}

void GL_APIENTRY TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                               GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLsizei width = %d, GLsizei height = %d, GLenum format = 0x%X, GLenum type = 0x%X, "
          "const GLvoid* pixels = 0x%0.8p)",
           target, level, xoffset, yoffset, width, height, format, type, pixels);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, width, height, 0, format, type, pixels))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, format, type, pixels))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0)
        {
            return;
        }

        Box area(xoffset, yoffset, 0, width, height, 1);
        Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
        Error error = texture->setSubImage(context, target, level, area, format, type,
                                           reinterpret_cast<const uint8_t *>(pixels));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY Uniform1f(GLint location, GLfloat x)
{
    Uniform1fv(location, 1, &x);
}

void GL_APIENTRY Uniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1fv(location, count, v);
    }
}

void GL_APIENTRY Uniform1i(GLint location, GLint x)
{
    Uniform1iv(location, 1, &x);
}

void GL_APIENTRY Uniform1iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1iv(location, count, v);
    }
}

void GL_APIENTRY Uniform2f(GLint location, GLfloat x, GLfloat y)
{
    GLfloat xy[2] = {x, y};

    Uniform2fv(location, 1, xy);
}

void GL_APIENTRY Uniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2fv(location, count, v);
    }
}

void GL_APIENTRY Uniform2i(GLint location, GLint x, GLint y)
{
    GLint xy[2] = {x, y};

    Uniform2iv(location, 1, xy);
}

void GL_APIENTRY Uniform2iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2iv(location, count, v);
    }
}

void GL_APIENTRY Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat xyz[3] = {x, y, z};

    Uniform3fv(location, 1, xyz);
}

void GL_APIENTRY Uniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3fv(location, count, v);
    }
}

void GL_APIENTRY Uniform3i(GLint location, GLint x, GLint y, GLint z)
{
    GLint xyz[3] = {x, y, z};

    Uniform3iv(location, 1, xyz);
}

void GL_APIENTRY Uniform3iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3iv(location, count, v);
    }
}

void GL_APIENTRY Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat xyzw[4] = {x, y, z, w};

    Uniform4fv(location, 1, xyzw);
}

void GL_APIENTRY Uniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4fv(location, count, v);
    }
}

void GL_APIENTRY Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    GLint xyzw[4] = {x, y, z, w};

    Uniform4iv(location, 1, xyzw);
}

void GL_APIENTRY Uniform4iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4iv(location, count, v);
    }
}

void GL_APIENTRY UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UseProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject && program != 0)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (program != 0 && !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->useProgram(program);
    }
}

void GL_APIENTRY ValidateProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        programObject->validate(context->getCaps());
    }
}

void GL_APIENTRY VertexAttrib1f(GLuint index, GLfloat x)
{
    EVENT("(GLuint index = %d, GLfloat x = %f)", index, x);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, 0, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib1fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], 0, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f)", index, x, y);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib2fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], values[1], 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f, GLfloat z = %f)", index, x, y, z);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, z, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib3fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], values[1], values[2], 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f, GLfloat z = %f, GLfloat w = %f)", index, x, y, z, w);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, z, w };
        context->getState().setVertexAttribf(index, vals);
    }
}

void GL_APIENTRY VertexAttrib4fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribf(index, values);
    }
}

void GL_APIENTRY VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    EVENT("(GLuint index = %d, GLint size = %d, GLenum type = 0x%X, "
          "GLboolean normalized = %u, GLsizei stride = %d, const GLvoid* ptr = 0x%0.8p)",
          index, size, type, normalized, stride, ptr);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (size < 1 || size > 4)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (type)
        {
          case GL_BYTE:
          case GL_UNSIGNED_BYTE:
          case GL_SHORT:
          case GL_UNSIGNED_SHORT:
          case GL_FIXED:
          case GL_FLOAT:
            break;

          case GL_HALF_FLOAT:
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_INT_2_10_10_10_REV:
          case GL_UNSIGNED_INT_2_10_10_10_REV:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (stride < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if ((type == GL_INT_2_10_10_10_REV || type == GL_UNSIGNED_INT_2_10_10_10_REV) && size != 4)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // [OpenGL ES 3.0.2] Section 2.8 page 24:
        // An INVALID_OPERATION error is generated when a non-zero vertex array object
        // is bound, zero is bound to the ARRAY_BUFFER buffer object binding point,
        // and the pointer argument is not NULL.
        if (context->getState().getVertexArray()->id() != 0 && context->getState().getArrayBufferId() == 0 && ptr != NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->getState().setVertexAttribState(index, context->getState().getTargetBuffer(GL_ARRAY_BUFFER), size, type,
                                                 normalized == GL_TRUE, false, stride, ptr);
    }
}

void GL_APIENTRY Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)", x, y, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setViewportParams(x, y, width, height);
    }
}

}
