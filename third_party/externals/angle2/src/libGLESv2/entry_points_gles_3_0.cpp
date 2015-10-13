//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_0.cpp : Implements the GLES 3.0 entry points.

#include "libGLESv2/entry_points_gles_3_0.h"
#include "libGLESv2/entry_points_gles_2_0_ext.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Error.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Query.h"
#include "libANGLE/VertexArray.h"

#include "libANGLE/validationES.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/queryconversions.h"

#include "common/debug.h"

namespace gl
{

void GL_APIENTRY ReadBuffer(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateReadBuffer(context, mode))
        {
            return;
        }

        Framebuffer *readFBO = context->getState().getReadFramebuffer();
        readFBO->setReadBuffer(mode);
    }
}

void GL_APIENTRY DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    EVENT("(GLenum mode = 0x%X, GLuint start = %u, GLuint end = %u, GLsizei count = %d, GLenum type = 0x%X, "
          "const GLvoid* indices = 0x%0.8p)", mode, start, end, count, type, indices);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        IndexRange indexRange;
        if (!ValidateDrawElements(context, mode, count, type, indices, 0, &indexRange))
        {
            return;
        }
        if (indexRange.end > end || indexRange.start < start)
        {
            // GL spec says that behavior in this case is undefined - generating an error is fine.
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // As long as index validation is done, it doesn't matter whether the context receives a drawElements or
        // a drawRangeElements call - the GL back-end is free to choose to call drawRangeElements based on the
        // validated index range. If index validation is removed, adding drawRangeElements to the context interface
        // should be reconsidered.
        Error error =
            context->drawRangeElements(mode, start, end, count, type, indices, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint internalformat = %d, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d, GLint border = %d, GLenum format = 0x%X, "
          "GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, format, type, pixels);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, 0, width, height, depth, border, format, type, pixels))
        {
            return;
        }

        Extents size(width, height, depth);
        Texture *texture = context->getTargetTexture(target);
        Error error = texture->setImage(context, target, level, internalformat, size, format, type,
                                        reinterpret_cast<const uint8_t *>(pixels));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint zoffset = %d, GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, zoffset, width, height, depth, 0,
                                           format, type, pixels))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0 || depth == 0)
        {
            return;
        }

        Box area(xoffset, yoffset, zoffset, width, height, depth);
        Texture *texture = context->getTargetTexture(target);
        Error error = texture->setSubImage(context, target, level, area, format, type,
                                           reinterpret_cast<const uint8_t *>(pixels));
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint zoffset = %d, GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, level, xoffset, yoffset, zoffset, x, y, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3CopyTexImageParameters(context, target, level, GL_NONE, true, xoffset, yoffset, zoffset,
                                               x, y, width, height, 0))
        {
            return;
        }

        Offset destOffset(xoffset, yoffset, zoffset);
        Rectangle sourceArea(x, y, width, height);

        const Framebuffer *framebuffer = context->getState().getReadFramebuffer();
        Texture *texture = context->getTargetTexture(target);
        Error error = texture->copySubImage(target, level, destOffset, sourceArea, framebuffer);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d, GLint border = %d, GLsizei imageSize = %d, "
          "const GLvoid* data = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, imageSize, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!gl::ValidateCompressedTexImage3D(context, target, level, internalformat, width, height,
                                              depth, border, imageSize, data))
        {
            return;
        }

        Extents size(width, height, depth);
        Texture *texture = context->getTargetTexture(target);
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

void GL_APIENTRY CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
        "GLint zoffset = %d, GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, "
        "GLenum format = 0x%X, GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
        target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(format);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!data)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, GL_NONE, true, true,
                                           0, 0, 0, width, height, depth, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0)
        {
            return;
        }

        Box area(xoffset, yoffset, zoffset, width, height, depth);
        Texture *texture = context->getTargetTexture(target);
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

void GL_APIENTRY GenQueries(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (GLsizei i = 0; i < n; i++)
        {
            ids[i] = context->createQuery();
        }
    }
}

void GL_APIENTRY DeleteQueries(GLsizei n, const GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (GLsizei i = 0; i < n; i++)
        {
            context->deleteQuery(ids[i]);
        }
    }
}

GLboolean GL_APIENTRY IsQuery(GLuint id)
{
    EVENT("(GLuint id = %u)", id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return (context->getQuery(id, false, GL_NONE) != NULL) ? GL_TRUE : GL_FALSE;
    }

    return GL_FALSE;
}

void GL_APIENTRY BeginQuery(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint id = %u)", target, id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateBeginQuery(context, target, id))
        {
            return;
        }

        Error error = context->beginQuery(target, id);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY EndQuery(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateEndQuery(context, target))
        {
            return;
        }

        Error error = context->endQuery(target);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY GetQueryiv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidQueryType(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_CURRENT_QUERY:
            params[0] = static_cast<GLint>(context->getState().getActiveQueryId(target));
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
    EVENT("(GLuint id = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", id, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Query *queryObject = context->getQuery(id, false, GL_NONE);

        if (!queryObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (context->getState().getActiveQueryId(queryObject->getType()) == id)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch(pname)
        {
          case GL_QUERY_RESULT_EXT:
            {
                Error error = queryObject->getResult(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_QUERY_RESULT_AVAILABLE_EXT:
            {
                Error error = queryObject->isResultAvailable(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

GLboolean GL_APIENTRY UnmapBuffer(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return UnmapBufferOES(target);
    }

    return GL_FALSE;
}

void GL_APIENTRY GetBufferPointerv(GLenum target, GLenum pname, GLvoid** params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLvoid** params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GetBufferPointervOES(target, pname, params);
    }
}

void GL_APIENTRY DrawBuffers(GLsizei n, const GLenum* bufs)
{
    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        DrawBuffersEXT(n, bufs);
    }
}

void GL_APIENTRY UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2x3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2x3fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3x2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3x2fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2x4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2x4fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4x2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4x2fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3x4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3x4fv(location, count, transpose, value);
    }
}

void GL_APIENTRY UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4x3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4x3fv(location, count, transpose, value);
    }
}

void GL_APIENTRY BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    EVENT("(GLint srcX0 = %d, GLint srcY0 = %d, GLint srcX1 = %d, GLint srcY1 = %d, GLint dstX0 = %d, "
          "GLint dstY0 = %d, GLint dstX1 = %d, GLint dstY1 = %d, GLbitfield mask = 0x%X, GLenum filter = 0x%X)",
          srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateBlitFramebufferParameters(context, srcX0, srcY0, srcX1, srcY1,
                                               dstX0, dstY0, dstX1, dstY1, mask, filter,
                                               false))
        {
            return;
        }

        Framebuffer *readFramebuffer = context->getState().getReadFramebuffer();
        ASSERT(readFramebuffer);

        Framebuffer *drawFramebuffer = context->getState().getDrawFramebuffer();
        ASSERT(drawFramebuffer);

        Rectangle srcArea(srcX0, srcY0, srcX1 - srcX0, srcY1 - srcY0);
        Rectangle dstArea(dstX0, dstY0, dstX1 - dstX0, dstY1 - dstY0);

        Error error = drawFramebuffer->blit(context->getState(), srcArea, dstArea, mask, filter, readFramebuffer);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei samples = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
        target, samples, internalformat, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3RenderbufferStorageParameters(context, target, samples, internalformat, width, height))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        renderbuffer->setStorageMultisample(samples, internalformat, width, height);
    }
}

void GL_APIENTRY FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLuint texture = %u, GLint level = %d, GLint layer = %d)",
        target, attachment, texture, level, layer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateFramebufferTextureLayer(context, target, attachment, texture,
                                             level, layer))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (texture != 0)
        {
            Texture *textureObject = context->getTexture(texture);

            ImageIndex index = ImageIndex::MakeInvalid();

            if (textureObject->getTarget() == GL_TEXTURE_3D)
            {
                index = ImageIndex::Make3D(level, layer);
            }
            else
            {
                ASSERT(textureObject->getTarget() == GL_TEXTURE_2D_ARRAY);
                index = ImageIndex::Make2DArray(level, layer);
            }

            framebuffer->setAttachment(GL_TEXTURE, attachment, index, textureObject);
        }
        else
        {
            framebuffer->resetAttachment(attachment);
        }
    }
}

GLvoid *GL_APIENTRY MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d, GLbitfield access = 0x%X)",
          target, offset, length, access);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        return MapBufferRangeEXT(target, offset, length, access);
    }

    return NULL;
}

void GL_APIENTRY FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d)", target, offset, length);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        FlushMappedBufferRangeEXT(target, offset, length);
    }
}

void GL_APIENTRY BindVertexArray(GLuint array)
{
    EVENT("(GLuint array = %u)", array);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        VertexArray *vao = context->getVertexArray(array);

        if (!vao)
        {
            // The default VAO should always exist
            ASSERT(array != 0);
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->bindVertexArray(array);
    }
}

void GL_APIENTRY DeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    EVENT("(GLsizei n = %d, const GLuint* arrays = 0x%0.8p)", n, arrays);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
        {
            if (arrays[arrayIndex] != 0)
            {
                context->deleteVertexArray(arrays[arrayIndex]);
            }
        }
    }
}

void GL_APIENTRY GenVertexArrays(GLsizei n, GLuint* arrays)
{
    EVENT("(GLsizei n = %d, GLuint* arrays = 0x%0.8p)", n, arrays);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
        {
            arrays[arrayIndex] = context->createVertexArray();
        }
    }
}

GLboolean GL_APIENTRY IsVertexArray(GLuint array)
{
    EVENT("(GLuint array = %u)", array);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        if (array == 0)
        {
            return GL_FALSE;
        }

        VertexArray *vao = context->getVertexArray(array);

        return (vao != NULL ? GL_TRUE : GL_FALSE);
    }

    return GL_FALSE;
}

void GL_APIENTRY GetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLint* data = 0x%0.8p)",
          target, index, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER_START:
          case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
          case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER_START:
          case GL_UNIFORM_BUFFER_SIZE:
          case GL_UNIFORM_BUFFER_BINDING:
            if (index >= caps.maxCombinedUniformBlocks)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!(context->getIndexedIntegerv(target, index, data)))
        {
            GLenum nativeType;
            unsigned int numParams = 0;
            if (!context->getIndexedQueryParameterInfo(target, &nativeType, &numParams))
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }

            if (numParams == 0)
            {
                return; // it is known that pname is valid, but there are no parameters to return
            }

            if (nativeType == GL_INT_64_ANGLEX)
            {
                GLint64 minIntValue = static_cast<GLint64>(std::numeric_limits<int>::min());
                GLint64 maxIntValue = static_cast<GLint64>(std::numeric_limits<int>::max());
                GLint64 *int64Params = new GLint64[numParams];

                context->getIndexedInteger64v(target, index, int64Params);

                for (unsigned int i = 0; i < numParams; ++i)
                {
                    GLint64 clampedValue = std::max(std::min(int64Params[i], maxIntValue), minIntValue);
                    data[i] = static_cast<GLint>(clampedValue);
                }

                delete [] int64Params;
            }
            else
            {
                UNREACHABLE();
            }
        }
    }
}

void GL_APIENTRY BeginTransformFeedback(GLenum primitiveMode)
{
    EVENT("(GLenum primitiveMode = 0x%X)", primitiveMode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (primitiveMode)
        {
          case GL_TRIANGLES:
          case GL_LINES:
          case GL_POINTS:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        if (transformFeedback->isActive())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (transformFeedback->isPaused())
        {
            transformFeedback->resume();
        }
        else
        {
            transformFeedback->begin(primitiveMode);
        }
    }
}

void GL_APIENTRY EndTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        if (!transformFeedback->isActive())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->end();
    }
}

void GL_APIENTRY BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLuint buffer = %u, GLintptr offset = %d, GLsizeiptr size = %d)",
          target, index, buffer, offset, size);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (buffer != 0 && size <= 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            {
                // size and offset must be a multiple of 4
                if (buffer != 0 && ((offset % 4) != 0 || (size % 4) != 0))
                {
                    context->recordError(Error(GL_INVALID_VALUE));
                    return;
                }

                // Cannot bind a transform feedback buffer if the current transform feedback is active (3.0.4 pg 91 section 2.15.2)
                TransformFeedback *curTransformFeedback = context->getState().getCurrentTransformFeedback();
                if (curTransformFeedback && curTransformFeedback->isActive())
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                context->bindIndexedTransformFeedbackBuffer(buffer, index, offset, size);
                context->bindGenericTransformFeedbackBuffer(buffer);
                break;
            }

          case GL_UNIFORM_BUFFER:

            // it is an error to bind an offset not a multiple of the alignment
            if (buffer != 0 && (offset % caps.uniformBufferOffsetAlignment) != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            context->bindIndexedUniformBuffer(buffer, index, offset, size);
            context->bindGenericUniformBuffer(buffer);
            break;

          default:
            UNREACHABLE();
        }
    }
}

void GL_APIENTRY BindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLuint buffer = %u)",
          target, index, buffer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            {
                // Cannot bind a transform feedback buffer if the current transform feedback is active (3.0.4 pg 91 section 2.15.2)
                TransformFeedback *curTransformFeedback = context->getState().getCurrentTransformFeedback();
                if (curTransformFeedback && curTransformFeedback->isActive())
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                context->bindIndexedTransformFeedbackBuffer(buffer, index, 0, 0);
                context->bindGenericTransformFeedbackBuffer(buffer);
                break;
            }
          case GL_UNIFORM_BUFFER:
            context->bindIndexedUniformBuffer(buffer, index, 0, 0);
            context->bindGenericUniformBuffer(buffer);
            break;

          default:
            UNREACHABLE();
        }
    }
}

void GL_APIENTRY TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    EVENT("(GLuint program = %u, GLsizei count = %d, const GLchar* const* varyings = 0x%0.8p, GLenum bufferMode = 0x%X)",
          program, count, varyings, bufferMode);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (bufferMode)
        {
          case GL_INTERLEAVED_ATTRIBS:
            break;
          case GL_SEPARATE_ATTRIBS:
            if (static_cast<GLuint>(count) > caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        programObject->setTransformFeedbackVaryings(count, varyings, bufferMode);
    }
}

void GL_APIENTRY GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    EVENT("(GLuint program = %u, GLuint index = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, "
          "GLsizei* size = 0x%0.8p, GLenum* type = 0x%0.8p, GLchar* name = 0x%0.8p)",
          program, index, bufSize, length, size, type, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return;
        }

        if (index >= static_cast<GLuint>(programObject->getTransformFeedbackVaryingCount()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getTransformFeedbackVarying(index, bufSize, length, size, type, name);
    }
}

void GL_APIENTRY VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    EVENT("(GLuint index = %u, GLint size = %d, GLenum type = 0x%X, GLsizei stride = %d, const GLvoid* pointer = 0x%0.8p)",
          index, size, type, stride, pointer);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

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
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_INT_2_10_10_10_REV:
          case GL_UNSIGNED_INT_2_10_10_10_REV:
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
        if (context->getState().getVertexArray()->id() != 0 && context->getState().getArrayBufferId() == 0 && pointer != NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->getState().setVertexAttribState(index, context->getState().getTargetBuffer(GL_ARRAY_BUFFER), size, type, false, true,
                                                 stride, pointer);
    }
}

void GL_APIENTRY GetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
    EVENT("(GLuint index = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          index, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

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
                params[i] = currentValueData.IntValues[i];
            }
        }
        else
        {
            const VertexAttribute &attribState = context->getState().getVertexArray()->getVertexAttribute(index);
            *params = QuerySingleVertexAttributeParameter<GLint>(attribState, pname);
        }
    }
}

void GL_APIENTRY GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
    EVENT("(GLuint index = %u, GLenum pname = 0x%X, GLuint* params = 0x%0.8p)",
          index, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

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
                params[i] = currentValueData.UnsignedIntValues[i];
            }
        }
        else
        {
            const VertexAttribute &attribState = context->getState().getVertexArray()->getVertexAttribute(index);
            *params = QuerySingleVertexAttributeParameter<GLuint>(attribState, pname);
        }
    }
}

void GL_APIENTRY VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    EVENT("(GLuint index = %u, GLint x = %d, GLint y = %d, GLint z = %d, GLint w = %d)",
          index, x, y, z, w);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLint vals[4] = { x, y, z, w };
        context->getState().setVertexAttribi(index, vals);
    }
}

void GL_APIENTRY VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    EVENT("(GLuint index = %u, GLuint x = %u, GLuint y = %u, GLuint z = %u, GLuint w = %u)",
          index, x, y, z, w);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLuint vals[4] = { x, y, z, w };
        context->getState().setVertexAttribu(index, vals);
    }
}

void GL_APIENTRY VertexAttribI4iv(GLuint index, const GLint* v)
{
    EVENT("(GLuint index = %u, const GLint* v = 0x%0.8p)", index, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribi(index, v);
    }
}

void GL_APIENTRY VertexAttribI4uiv(GLuint index, const GLuint* v)
{
    EVENT("(GLuint index = %u, const GLuint* v = 0x%0.8p)", index, v);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribu(index, v);
    }
}

void GL_APIENTRY GetUniformuiv(GLuint program, GLint location, GLuint* params)
{
    EVENT("(GLuint program = %u, GLint location = %d, GLuint* params = 0x%0.8p)",
          program, location, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateGetUniformuiv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformuiv(location, params);
    }
}

GLint GL_APIENTRY GetFragDataLocation(GLuint program, const GLchar *name)
{
    EVENT("(GLuint program = %u, const GLchar *name = 0x%0.8p)",
          program, name);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        if (program == 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return -1;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject || !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getFragDataLocation(name);
    }

    return 0;
}

void GL_APIENTRY Uniform1ui(GLint location, GLuint v0)
{
    Uniform1uiv(location, 1, &v0);
}

void GL_APIENTRY Uniform2ui(GLint location, GLuint v0, GLuint v1)
{
    const GLuint xy[] = { v0, v1 };
    Uniform2uiv(location, 1, xy);
}

void GL_APIENTRY Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    const GLuint xyz[] = { v0, v1, v2 };
    Uniform3uiv(location, 1, xyz);
}

void GL_APIENTRY Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    const GLuint xyzw[] = { v0, v1, v2, v3 };
    Uniform4uiv(location, 1, xyzw);
}

void GL_APIENTRY Uniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1uiv(location, count, value);
    }
}

void GL_APIENTRY Uniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2uiv(location, count, value);
    }
}

void GL_APIENTRY Uniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value)",
          location, count, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3uiv(location, count, value);
    }
}

void GL_APIENTRY Uniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4uiv(location, count, value);
    }
}

void GL_APIENTRY ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLint* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_STENCIL:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        Error error = framebufferObject->clearBufferiv(context, buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLuint* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        Error error = framebufferObject->clearBufferuiv(context, buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLfloat* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_DEPTH:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        Error error = framebufferObject->clearBufferfv(context, buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, GLfloat depth, GLint stencil = %d)",
          buffer, drawbuffer, depth, stencil);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_DEPTH_STENCIL:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        // If a buffer is not present, the clear has no effect
        if (framebufferObject->getDepthbuffer() == nullptr && framebufferObject->getStencilbuffer() == nullptr)
        {
            return;
        }

        Error error = framebufferObject->clearBufferfi(context, buffer, drawbuffer, depth, stencil);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

const GLubyte *GL_APIENTRY GetStringi(GLenum name, GLuint index)
{
    EVENT("(GLenum name = 0x%X, GLuint index = %u)", name, index);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if (name != GL_EXTENSIONS)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (index >= context->getExtensionStringCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        return reinterpret_cast<const GLubyte*>(context->getExtensionString(index).c_str());
    }

    return NULL;
}

void GL_APIENTRY CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    EVENT("(GLenum readTarget = 0x%X, GLenum writeTarget = 0x%X, GLintptr readOffset = %d, GLintptr writeOffset = %d, GLsizeiptr size = %d)",
          readTarget, writeTarget, readOffset, writeOffset, size);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidBufferTarget(context, readTarget) || !ValidBufferTarget(context, writeTarget))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *readBuffer = context->getState().getTargetBuffer(readTarget);
        Buffer *writeBuffer = context->getState().getTargetBuffer(writeTarget);

        if (!readBuffer || !writeBuffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Verify that readBuffer and writeBuffer are not currently mapped
        if (readBuffer->isMapped() || writeBuffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (readOffset < 0 || writeOffset < 0 || size < 0 ||
            static_cast<unsigned int>(readOffset + size) > readBuffer->getSize() ||
            static_cast<unsigned int>(writeOffset + size) > writeBuffer->getSize())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (readBuffer == writeBuffer && std::abs(readOffset - writeOffset) < size)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // if size is zero, the copy is a successful no-op
        if (size > 0)
        {
            Error error = writeBuffer->copyBufferSubData(readBuffer, readOffset, writeOffset, size);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void GL_APIENTRY GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    EVENT("(GLuint program = %u, GLsizei uniformCount = %d, const GLchar* const* uniformNames = 0x%0.8p, GLuint* uniformIndices = 0x%0.8p)",
          program, uniformCount, uniformNames, uniformIndices);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformCount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (!programObject->isLinked())
        {
            for (int uniformId = 0; uniformId < uniformCount; uniformId++)
            {
                uniformIndices[uniformId] = GL_INVALID_INDEX;
            }
        }
        else
        {
            for (int uniformId = 0; uniformId < uniformCount; uniformId++)
            {
                uniformIndices[uniformId] = programObject->getUniformIndex(uniformNames[uniformId]);
            }
        }
    }
}

void GL_APIENTRY GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %u, GLsizei uniformCount = %d, const GLuint* uniformIndices = 0x%0.8p, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          program, uniformCount, uniformIndices, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformCount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        switch (pname)
        {
          case GL_UNIFORM_TYPE:
          case GL_UNIFORM_SIZE:
          case GL_UNIFORM_NAME_LENGTH:
          case GL_UNIFORM_BLOCK_INDEX:
          case GL_UNIFORM_OFFSET:
          case GL_UNIFORM_ARRAY_STRIDE:
          case GL_UNIFORM_MATRIX_STRIDE:
          case GL_UNIFORM_IS_ROW_MAJOR:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (uniformCount > programObject->getActiveUniformCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            const GLuint index = uniformIndices[uniformId];

            if (index >= static_cast<GLuint>(programObject->getActiveUniformCount()))
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            const GLuint index = uniformIndices[uniformId];
            params[uniformId] = programObject->getActiveUniformi(index, pname);
        }
    }
}

GLuint GL_APIENTRY GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
    EVENT("(GLuint program = %u, const GLchar* uniformBlockName = 0x%0.8p)", program, uniformBlockName);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_INVALID_INDEX;
        }

        Program *programObject = GetValidProgram(context, program);
        if (!programObject)
        {
            return GL_INVALID_INDEX;
        }

        return programObject->getUniformBlockIndex(uniformBlockName);
    }

    return 0;
}

void GL_APIENTRY GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          program, uniformBlockIndex, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_UNIFORM_BLOCK_BINDING:
            *params = static_cast<GLint>(programObject->getUniformBlockBinding(uniformBlockIndex));
            break;

          case GL_UNIFORM_BLOCK_DATA_SIZE:
          case GL_UNIFORM_BLOCK_NAME_LENGTH:
          case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
          case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
          case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
          case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
            programObject->getActiveUniformBlockiv(uniformBlockIndex, pname, params);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLchar* uniformBlockName = 0x%0.8p)",
          program, uniformBlockIndex, bufSize, length, uniformBlockName);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveUniformBlockName(uniformBlockIndex, bufSize, length, uniformBlockName);
    }
}

void GL_APIENTRY UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLuint uniformBlockBinding = %u)",
          program, uniformBlockIndex, uniformBlockBinding);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformBlockBinding >= context->getCaps().maxUniformBufferBindings)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = GetValidProgram(context, program);

        if (!programObject)
        {
            return;
        }

        // if never linked, there won't be any uniform blocks
        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->bindUniformBlock(uniformBlockIndex, uniformBlockBinding);
    }
}

void GL_APIENTRY DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d, GLsizei instanceCount = %d)",
          mode, first, count, instanceCount);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateDrawArraysInstanced(context, mode, first, count, instanceCount))
        {
            return;
        }

        Error error = context->drawArraysInstanced(mode, first, count, instanceCount);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p, GLsizei instanceCount = %d)",
          mode, count, type, indices, instanceCount);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        IndexRange indexRange;
        if (!ValidateDrawElementsInstanced(context, mode, count, type, indices, instanceCount, &indexRange))
        {
            return;
        }

        Error error =
            context->drawElementsInstanced(mode, count, type, indices, instanceCount, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

GLsync GL_APIENTRY FenceSync_(GLenum condition, GLbitfield flags)
{
    EVENT("(GLenum condition = 0x%X, GLbitfield flags = 0x%X)", condition, flags);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return 0;
        }

        if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }

        if (flags != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return 0;
        }

        GLsync fenceSync = context->createFenceSync();

        FenceSync *fenceSyncObject = context->getFenceSync(fenceSync);
        Error error = fenceSyncObject->set(condition, flags);
        if (error.isError())
        {
            context->deleteFenceSync(fenceSync);
            context->recordError(error);
            return NULL;
        }

        return fenceSync;
    }

    return NULL;
}

GLboolean GL_APIENTRY IsSync(GLsync sync)
{
    EVENT("(GLsync sync = 0x%0.8p)", sync);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return (context->getFenceSync(sync) != NULL);
    }

    return GL_FALSE;
}

void GL_APIENTRY DeleteSync(GLsync sync)
{
    EVENT("(GLsync sync = 0x%0.8p)", sync);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (sync != static_cast<GLsync>(0) && !context->getFenceSync(sync))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->deleteFenceSync(sync);
    }
}

GLenum GL_APIENTRY ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    EVENT("(GLsync sync = 0x%0.8p, GLbitfield flags = 0x%X, GLuint64 timeout = %llu)",
          sync, flags, timeout);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_WAIT_FAILED;
        }

        if ((flags & ~(GL_SYNC_FLUSH_COMMANDS_BIT)) != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return GL_WAIT_FAILED;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return GL_WAIT_FAILED;
        }

        GLenum result = GL_WAIT_FAILED;
        Error error = fenceSync->clientWait(flags, timeout, &result);
        if (error.isError())
        {
            context->recordError(error);
            return GL_WAIT_FAILED;
        }

        return result;
    }

    return GL_FALSE;
}

void GL_APIENTRY WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    EVENT("(GLsync sync = 0x%0.8p, GLbitfield flags = 0x%X, GLuint64 timeout = %llu)",
          sync, flags, timeout);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (flags != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (timeout != GL_TIMEOUT_IGNORED)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Error error = fenceSync->serverWait(flags, timeout);
        if (error.isError())
        {
            context->recordError(error);
        }
    }
}

void GL_APIENTRY GetInteger64v(GLenum pname, GLint64* params)
{
    EVENT("(GLenum pname = 0x%X, GLint64* params = 0x%0.8p)",
          pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_INT_64_ANGLEX)
        {
            context->getInteger64v(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GL_APIENTRY GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    EVENT("(GLsync sync = 0x%0.8p, GLenum pname = 0x%X, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLint* values = 0x%0.8p)",
          sync, pname, bufSize, length, values);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_OBJECT_TYPE:     values[0] = static_cast<GLint>(GL_SYNC_FENCE);              break;
          case GL_SYNC_CONDITION:  values[0] = static_cast<GLint>(fenceSync->getCondition());  break;
          case GL_SYNC_FLAGS:      values[0] = static_cast<GLint>(fenceSync->getFlags());      break;

          case GL_SYNC_STATUS:
            {
                Error error = fenceSync->getStatus(values);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
                break;
            }

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLint64* data = 0x%0.8p)",
          target, index, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER_START:
          case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
          case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER_START:
          case GL_UNIFORM_BUFFER_SIZE:
          case GL_UNIFORM_BUFFER_BINDING:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!(context->getIndexedInteger64v(target, index, data)))
        {
            GLenum nativeType;
            unsigned int numParams = 0;
            if (!context->getIndexedQueryParameterInfo(target, &nativeType, &numParams))
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }

            if (numParams == 0)
                return; // it is known that pname is valid, but there are no parameters to return

            if (nativeType == GL_INT)
            {
                GLint *intParams = new GLint[numParams];

                context->getIndexedIntegerv(target, index, intParams);

                for (unsigned int i = 0; i < numParams; ++i)
                {
                    data[i] = static_cast<GLint64>(intParams[i]);
                }

                delete [] intParams;
            }
            else
            {
                UNREACHABLE();
            }
        }
    }
}

void GL_APIENTRY GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint64* params = 0x%0.8p)",
          target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

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
            *params = static_cast<GLint64>(buffer->getUsage());
            break;
          case GL_BUFFER_SIZE:
            *params = buffer->getSize();
            break;
          case GL_BUFFER_ACCESS_FLAGS:
            *params = static_cast<GLint64>(buffer->getAccessFlags());
            break;
          case GL_BUFFER_MAPPED:
            *params = static_cast<GLint64>(buffer->isMapped());
            break;
          case GL_BUFFER_MAP_OFFSET:
            *params = buffer->getMapOffset();
            break;
          case GL_BUFFER_MAP_LENGTH:
            *params = buffer->getMapLength();
            break;
          default: UNREACHABLE(); break;
        }
    }
}

void GL_APIENTRY GenSamplers(GLsizei count, GLuint* samplers)
{
    EVENT("(GLsizei count = %d, GLuint* samplers = 0x%0.8p)", count, samplers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < count; i++)
        {
            samplers[i] = context->createSampler();
        }
    }
}

void GL_APIENTRY DeleteSamplers(GLsizei count, const GLuint* samplers)
{
    EVENT("(GLsizei count = %d, const GLuint* samplers = 0x%0.8p)", count, samplers);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < count; i++)
        {
            context->deleteSampler(samplers[i]);
        }
    }
}

GLboolean GL_APIENTRY IsSampler(GLuint sampler)
{
    EVENT("(GLuint sampler = %u)", sampler);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return context->isSampler(sampler);
    }

    return GL_FALSE;
}

void GL_APIENTRY BindSampler(GLuint unit, GLuint sampler)
{
    EVENT("(GLuint unit = %u, GLuint sampler = %u)", unit, sampler);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (sampler != 0 && !context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (unit >= context->getCaps().maxCombinedTextureImageUnits)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->bindSampler(unit, sampler);
    }
}

void GL_APIENTRY SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLint param = %d)", sampler, pname, param);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!ValidateTexParamParameters(context, pname, param))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->samplerParameteri(sampler, pname, param);
    }
}

void GL_APIENTRY SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
    SamplerParameteri(sampler, pname, *param);
}

void GL_APIENTRY SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLfloat param = %g)", sampler, pname, param);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!ValidateTexParamParameters(context, pname, static_cast<GLint>(param)))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->samplerParameterf(sampler, pname, param);
    }
}

void GL_APIENTRY SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
    SamplerParameterf(sampler, pname, *param);
}

void GL_APIENTRY GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", sampler, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        *params = context->getSamplerParameteri(sampler, pname);
    }
}

void GL_APIENTRY GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
    EVENT("(GLuint sample = %ur, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", sampler, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        *params = context->getSamplerParameterf(sampler, pname);
    }
}

void GL_APIENTRY VertexAttribDivisor(GLuint index, GLuint divisor)
{
    EVENT("(GLuint index = %u, GLuint divisor = %u)", index, divisor);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->setVertexAttribDivisor(index, divisor);
    }
}

void GL_APIENTRY BindTransformFeedback(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint id = %u)", target, id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK:
            {
                // Cannot bind a transform feedback object if the current one is started and not paused (3.0.2 pg 85 section 2.14.1)
                TransformFeedback *curTransformFeedback = context->getState().getCurrentTransformFeedback();
                if (curTransformFeedback && curTransformFeedback->isActive() && !curTransformFeedback->isPaused())
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                // Cannot bind a transform feedback object that does not exist (3.0.2 pg 85 section 2.14.1)
                if (context->getTransformFeedback(id) == NULL)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                context->bindTransformFeedback(id);
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY DeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
    EVENT("(GLsizei n = %d, const GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteTransformFeedback(ids[i]);
        }
    }
}

void GL_APIENTRY GenTransformFeedbacks(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            ids[i] = context->createTransformFeedback();
        }
    }
}

GLboolean GL_APIENTRY IsTransformFeedback(GLuint id)
{
    EVENT("(GLuint id = %u)", id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return ((context->getTransformFeedback(id) != NULL) ? GL_TRUE : GL_FALSE);
    }

    return GL_FALSE;
}

void GL_APIENTRY PauseTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        // Current transform feedback must be active and not paused in order to pause (3.0.2 pg 86)
        if (!transformFeedback->isActive() || transformFeedback->isPaused())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->pause();
    }
}

void GL_APIENTRY ResumeTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        // Current transform feedback must be active and paused in order to resume (3.0.2 pg 86)
        if (!transformFeedback->isActive() || !transformFeedback->isPaused())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->resume();
    }
}

void GL_APIENTRY GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    EVENT("(GLuint program = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLenum* binaryFormat = 0x%0.8p, GLvoid* binary = 0x%0.8p)",
          program, bufSize, length, binaryFormat, binary);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // TODO: Pipe through to the OES extension for now, needs proper validation
        return GetProgramBinaryOES(program, bufSize, length, binaryFormat, binary);
    }
}

void GL_APIENTRY ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    EVENT("(GLuint program = %u, GLenum binaryFormat = 0x%X, const GLvoid* binary = 0x%0.8p, GLsizei length = %d)",
          program, binaryFormat, binary, length);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // TODO: Pipe through to the OES extension for now, needs proper validation
        return ProgramBinaryOES(program, binaryFormat, binary, length);
    }
}

void GL_APIENTRY ProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    EVENT("(GLuint program = %u, GLenum pname = 0x%X, GLint value = %d)",
          program, pname, value);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glProgramParameteri
        UNIMPLEMENTED();
    }
}

void GL_APIENTRY InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    EVENT("(GLenum target = 0x%X, GLsizei numAttachments = %d, const GLenum* attachments = 0x%0.8p)",
          target, numAttachments, attachments);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateInvalidateFramebuffer(context, target, numAttachments, attachments))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->checkStatus(context->getData()) == GL_FRAMEBUFFER_COMPLETE)
        {
            Error error = framebuffer->invalidate(numAttachments, attachments);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void GL_APIENTRY InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei numAttachments = %d, const GLenum* attachments = 0x%0.8p, GLint x = %d, "
          "GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, numAttachments, attachments, x, y, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateInvalidateFramebuffer(context, target, numAttachments, attachments))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->checkStatus(context->getData()) == GL_FRAMEBUFFER_COMPLETE)
        {
            Rectangle area(x, y, width, height);
            Error error = framebuffer->invalidateSub(numAttachments, attachments, area);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void GL_APIENTRY TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
          target, levels, internalformat, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, 1))
        {
            return;
        }

        Extents size(width, height, 1);
        Texture *texture = context->getTargetTexture(target);
        Error error = texture->setStorage(target, levels, internalformat, size);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d)",
          target, levels, internalformat, width, height, depth);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, depth))
        {
            return;
        }

        Extents size(width, height, depth);
        Texture *texture = context->getTargetTexture(target);
        Error error = texture->setStorage(target, levels, internalformat, size);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum internalformat = 0x%X, GLenum pname = 0x%X, GLsizei bufSize = %d, "
          "GLint* params = 0x%0.8p)",
          target, internalformat, pname, bufSize, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
        if (!formatCaps.renderable)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_NUM_SAMPLE_COUNTS:
            if (bufSize != 0)
            {
                *params = static_cast<GLint>(formatCaps.sampleCounts.size());
            }
            break;

          case GL_SAMPLES:
            {
                size_t returnCount = std::min<size_t>(bufSize, formatCaps.sampleCounts.size());
                auto sampleReverseIt = formatCaps.sampleCounts.rbegin();
                for (size_t sampleIndex = 0; sampleIndex < returnCount; ++sampleIndex)
                {
                    params[sampleIndex] = *sampleReverseIt++;;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

}
