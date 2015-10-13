//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_2_0_ext.cpp : Implements the GLES 2.0 extension entry points.

#include "libGLESv2/entry_points_gles_2_0_ext.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Error.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Query.h"

#include "libANGLE/validationES.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"

#include "common/debug.h"
#include "common/utilities.h"

namespace gl
{

void GL_APIENTRY BeginQueryEXT(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint %d)", target, id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
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

void GL_APIENTRY DeleteFencesNV(GLsizei n, const GLuint* fences)
{
    EVENT("(GLsizei n = %d, const GLuint* fences = 0x%0.8p)", n, fences);

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
            context->deleteFenceNV(fences[i]);
        }
    }
}

void GL_APIENTRY DeleteQueriesEXT(GLsizei n, const GLuint *ids)
{
    EVENT("(GLsizei n = %d, const GLuint *ids = 0x%0.8p)", n, ids);

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
            context->deleteQuery(ids[i]);
        }
    }
}

void GL_APIENTRY DrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d, GLsizei primcount = %d)", mode, first, count, primcount);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateDrawArraysInstancedANGLE(context, mode, first, count, primcount))
        {
            return;
        }

        Error error = context->drawArraysInstanced(mode, first, count, primcount);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY DrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p, GLsizei primcount = %d)",
          mode, count, type, indices, primcount);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        IndexRange indexRange;
        if (!ValidateDrawElementsInstancedANGLE(context, mode, count, type, indices, primcount, &indexRange))
        {
            return;
        }

        Error error =
            context->drawElementsInstanced(mode, count, type, indices, primcount, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY EndQueryEXT(GLenum target)
{
    EVENT("GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
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

void GL_APIENTRY FinishFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (fenceObject->isSet() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        fenceObject->finish();
    }
}

void GL_APIENTRY GenFencesNV(GLsizei n, GLuint* fences)
{
    EVENT("(GLsizei n = %d, GLuint* fences = 0x%0.8p)", n, fences);

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
            fences[i] = context->createFenceNV();
        }
    }
}

void GL_APIENTRY GenQueriesEXT(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = GetValidGlobalContext();
    if (context)
    {
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

void GL_APIENTRY GetFenceivNV(GLuint fence, GLenum pname, GLint *params)
{
    EVENT("(GLuint fence = %d, GLenum pname = 0x%X, GLint *params = 0x%0.8p)", fence, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (fenceObject->isSet() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (pname)
        {
          case GL_FENCE_STATUS_NV:
            {
                // GL_NV_fence spec:
                // Once the status of a fence has been finished (via FinishFenceNV) or tested and the returned status is TRUE (via either TestFenceNV
                // or GetFenceivNV querying the FENCE_STATUS_NV), the status remains TRUE until the next SetFenceNV of the fence.
                GLboolean status = GL_TRUE;
                if (fenceObject->getStatus() != GL_TRUE)
                {
                    Error error = fenceObject->test(&status);
                    if (error.isError())
                    {
                        context->recordError(error);
                        return;
                    }
                }
                *params = status;
                break;
            }

          case GL_FENCE_CONDITION_NV:
            {
                *params = static_cast<GLint>(fenceObject->getCondition());
                break;
            }

          default:
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
        }
    }
}

GLenum GL_APIENTRY GetGraphicsResetStatusEXT(void)
{
    EVENT("()");

    Context *context = GetGlobalContext();

    if (context)
    {
        return context->getResetStatus();
    }

    return GL_NO_ERROR;
}

void GL_APIENTRY GetQueryivEXT(GLenum target, GLenum pname, GLint *params)
{
    EVENT("GLenum target = 0x%X, GLenum pname = 0x%X, GLint *params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidQueryType(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_CURRENT_QUERY_EXT:
            params[0] = context->getState().getActiveQueryId(target);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GL_APIENTRY GetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params)
{
    EVENT("(GLuint id = %d, GLenum pname = 0x%X, GLuint *params = 0x%0.8p)", id, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
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

void GL_APIENTRY GetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
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

        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Only returns extra info if ANGLE_GENERATE_SHADER_DEBUG_INFO is defined
        shaderObject->getTranslatedSourceWithDebugInfo(bufsize, length, source);
    }
}

void GL_APIENTRY GetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLsizei bufSize = %d, GLfloat* params = 0x%0.8p)",
          program, location, bufSize, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateGetnUniformfvEXT(context, program, location, bufSize, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformfv(location, params);
    }
}

void GL_APIENTRY GetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLsizei bufSize = %d, GLint* params = 0x%0.8p)",
          program, location, bufSize, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateGetnUniformivEXT(context, program, location, bufSize, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformiv(location, params);
    }
}

GLboolean GL_APIENTRY IsFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            return GL_FALSE;
        }

        // GL_NV_fence spec:
        // A name returned by GenFencesNV, but not yet set via SetFenceNV, is not the name of an existing fence.
        return fenceObject->isSet();
    }

    return GL_FALSE;
}

GLboolean GL_APIENTRY IsQueryEXT(GLuint id)
{
    EVENT("(GLuint id = %d)", id);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        return (context->getQuery(id, false, GL_NONE) != NULL) ? GL_TRUE : GL_FALSE;
    }

    return GL_FALSE;
}

void GL_APIENTRY ReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height,
                                GLenum format, GLenum type, GLsizei bufSize,
                                GLvoid *data)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, GLsizei bufSize = 0x%d, GLvoid *data = 0x%0.8p)",
          x, y, width, height, format, type, bufSize, data);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (width < 0 || height < 0 || bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateReadPixelsParameters(context, x, y, width, height,
                                              format, type, &bufSize, data))
        {
            return;
        }

        Framebuffer *framebufferObject = context->getState().getReadFramebuffer();
        ASSERT(framebufferObject);

        Rectangle area(x, y, width, height);
        Error error = framebufferObject->readPixels(context, area, format, type, data);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY RenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei samples = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
        target, samples, internalformat, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateRenderbufferStorageParametersANGLE(context, target, samples, internalformat,
            width, height))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        Error error = renderbuffer->setStorageMultisample(samples, internalformat, width, height);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY SetFenceNV(GLuint fence, GLenum condition)
{
    EVENT("(GLuint fence = %d, GLenum condition = 0x%X)", fence, condition);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (condition != GL_ALL_COMPLETED_NV)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = fenceObject->set(condition);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

GLboolean GL_APIENTRY TestFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_TRUE;
        }

        if (fenceObject->isSet() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_TRUE;
        }

        GLboolean result;
        Error error = fenceObject->test(&result);
        if (error.isError())
        {
            context->recordError(error);
            return GL_TRUE;
        }

        return result;
    }

    return GL_TRUE;
}

void GL_APIENTRY TexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
           target, levels, internalformat, width, height);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().textureStorage)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (context->getClientVersion() < 3 &&
            !ValidateES2TexStorageParameters(context, target, levels, internalformat, width, height))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, 1))
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

void GL_APIENTRY VertexAttribDivisorANGLE(GLuint index, GLuint divisor)
{
    EVENT("(GLuint index = %d, GLuint divisor = %d)", index, divisor);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (context->getLimitations().attributeZeroRequiresZeroDivisorInEXT)
        {
            if (index == 0 && divisor != 0)
            {
                const char *errorMessage = "The current context doesn't support setting a non-zero divisor on the attribute with index zero. "
                                           "Please reorder the attributes in your vertex shader so that attribute zero can have a zero divisor.";
                context->recordError(Error(GL_INVALID_OPERATION, errorMessage));

                // We also output an error message to the debugger window if tracing is active, so that developers can see the error message.
                ERR("%s", errorMessage);

                return;
            }
        }

        context->setVertexAttribDivisor(index, divisor);
    }
}

void GL_APIENTRY BlitFramebufferANGLE(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                          GLbitfield mask, GLenum filter)
{
    EVENT("(GLint srcX0 = %d, GLint srcY0 = %d, GLint srcX1 = %d, GLint srcY1 = %d, "
          "GLint dstX0 = %d, GLint dstY0 = %d, GLint dstX1 = %d, GLint dstY1 = %d, "
          "GLbitfield mask = 0x%X, GLenum filter = 0x%X)",
          srcX0, srcY0, srcX1, srcX1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateBlitFramebufferParameters(context, srcX0, srcY0, srcX1, srcY1,
                                               dstX0, dstY0, dstX1, dstY1, mask, filter,
                                               true))
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

void GL_APIENTRY DiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments)
{
    EVENT("(GLenum target = 0x%X, GLsizei numAttachments = %d, attachments = 0x%0.8p)", target, numAttachments, attachments);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().discardFramebuffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION, "Extension not enabled"));
            return;
        }

        if (!ValidateDiscardFramebufferEXT(context, target, numAttachments, attachments))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        // The specification isn't clear what should be done when the framebuffer isn't complete.
        // We leave it up to the framebuffer implementation to decide what to do.
        Error error = framebuffer->discard(numAttachments, attachments);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY TexImage3DOES(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
                   GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, "
          "GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, GLint border = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%x, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, format, type, pixels);

    UNIMPLEMENTED();   // FIXME
}

void GL_APIENTRY GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)
{
    EVENT("(GLenum program = 0x%X, bufSize = %d, length = 0x%0.8p, binaryFormat = 0x%0.8p, binary = 0x%0.8p)",
          program, bufSize, length, binaryFormat, binary);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject || !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = programObject->saveBinary(binaryFormat, binary, bufSize, length);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY ProgramBinaryOES(GLuint program, GLenum binaryFormat, const void *binary, GLint length)
{
    EVENT("(GLenum program = 0x%X, binaryFormat = 0x%x, binary = 0x%0.8p, length = %d)",
          program, binaryFormat, binary, length);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        const std::vector<GLenum> &programBinaryFormats = context->getCaps().programBinaryFormats;
        if (std::find(programBinaryFormats.begin(), programBinaryFormats.end(), binaryFormat) == programBinaryFormats.end())
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Program *programObject = context->getProgram(program);
        if (!programObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = programObject->loadBinary(binaryFormat, binary, length);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GL_APIENTRY DrawBuffersEXT(GLsizei n, const GLenum *bufs)
{
    EVENT("(GLenum n = %d, bufs = 0x%0.8p)", n, bufs);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidateDrawBuffers(context, n, bufs))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getDrawFramebuffer();
        ASSERT(framebuffer);

        framebuffer->setDrawBuffers(n, bufs);
    }
}

void GL_APIENTRY GetBufferPointervOES(GLenum target, GLenum pname, void** params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLvoid** params = 0x%0.8p)", target, pname, params);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (pname != GL_BUFFER_MAP_POINTER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer || !buffer->isMapped())
        {
            *params = NULL;
        }
        else
        {
            *params = buffer->getMapPointer();
        }
    }
}

void *GL_APIENTRY MapBufferOES(GLenum target, GLenum access)
{
    EVENT("(GLenum target = 0x%X, GLbitfield access = 0x%X)", target, access);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if (access != GL_WRITE_ONLY_OES)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        Error error = buffer->map(access);
        if (error.isError())
        {
            context->recordError(error);
            return NULL;
        }

        return buffer->getMapPointer();
    }

    return NULL;
}

GLboolean GL_APIENTRY UnmapBufferOES(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return GL_FALSE;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL || !buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        GLboolean result;
        Error error = buffer->unmap(&result);
        if (error.isError())
        {
            context->recordError(error);
            return GL_FALSE;
        }

        return result;
    }

    return GL_FALSE;
}

void *GL_APIENTRY MapBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d, GLbitfield access = 0x%X)",
          target, offset, length, access);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (offset < 0 || length < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        // Check for buffer overflow
        size_t offsetSize = static_cast<size_t>(offset);
        size_t lengthSize = static_cast<size_t>(length);

        if (!rx::IsUnsignedAdditionSafe(offsetSize, lengthSize) ||
            offsetSize + lengthSize > static_cast<size_t>(buffer->getSize()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        // Check for invalid bits in the mask
        GLbitfield allAccessBits = GL_MAP_READ_BIT |
                                   GL_MAP_WRITE_BIT |
                                   GL_MAP_INVALIDATE_RANGE_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT |
                                   GL_MAP_FLUSH_EXPLICIT_BIT |
                                   GL_MAP_UNSYNCHRONIZED_BIT;

        if (access & ~(allAccessBits))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        if (length == 0 || buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        // Check for invalid bit combinations
        if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        GLbitfield writeOnlyBits = GL_MAP_INVALIDATE_RANGE_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT |
                                   GL_MAP_UNSYNCHRONIZED_BIT;

        if ((access & GL_MAP_READ_BIT) != 0 && (access & writeOnlyBits) != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if ((access & GL_MAP_WRITE_BIT) == 0 && (access & GL_MAP_FLUSH_EXPLICIT_BIT) != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        Error error = buffer->mapRange(offset, length, access);
        if (error.isError())
        {
            context->recordError(error);
            return NULL;
        }

        return buffer->getMapPointer();
    }

    return NULL;
}

void GL_APIENTRY FlushMappedBufferRangeEXT(GLenum target, GLintptr offset, GLsizeiptr length)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d)", target, offset, length);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (offset < 0 || length < 0)
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

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!buffer->isMapped() || (buffer->getAccessFlags() & GL_MAP_FLUSH_EXPLICIT_BIT) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Check for buffer overflow
        size_t offsetSize = static_cast<size_t>(offset);
        size_t lengthSize = static_cast<size_t>(length);

        if (!rx::IsUnsignedAdditionSafe(offsetSize, lengthSize) ||
            offsetSize + lengthSize > static_cast<size_t>(buffer->getMapLength()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // We do not currently support a non-trivial implementation of FlushMappedBufferRange
    }
}

void GL_APIENTRY InsertEventMarkerEXT(GLsizei length, const char *marker)
{
    // Don't run an EVENT() macro on the EXT_debug_marker entry points.
    // It can interfere with the debug events being set by the caller.

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().debugMarker)
        {
            // The debug marker calls should not set error state
            // However, it seems reasonable to set an error state if the extension is not enabled
            context->recordError(Error(GL_INVALID_OPERATION, "Extension not enabled"));
            return;
        }

        if (!ValidateInsertEventMarkerEXT(context, length, marker))
        {
            return;
        }

        context->insertEventMarker(length, marker);
    }
}

void GL_APIENTRY PushGroupMarkerEXT(GLsizei length, const char *marker)
{
    // Don't run an EVENT() macro on the EXT_debug_marker entry points.
    // It can interfere with the debug events being set by the caller.

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().debugMarker)
        {
            // The debug marker calls should not set error state
            // However, it seems reasonable to set an error state if the extension is not enabled
            context->recordError(Error(GL_INVALID_OPERATION, "Extension not enabled"));
            return;
        }

        if (!ValidatePushGroupMarkerEXT(context, length, marker))
        {
            return;
        }

        if (marker == nullptr)
        {
            // From the EXT_debug_marker spec,
            // "If <marker> is null then an empty string is pushed on the stack."
            context->pushGroupMarker(length, "");
        }
        else
        {
            context->pushGroupMarker(length, marker);
        }
    }
}

void GL_APIENTRY PopGroupMarkerEXT()
{
    // Don't run an EVENT() macro on the EXT_debug_marker entry points.
    // It can interfere with the debug events being set by the caller.

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().debugMarker)
        {
            // The debug marker calls should not set error state
            // However, it seems reasonable to set an error state if the extension is not enabled
            context->recordError(Error(GL_INVALID_OPERATION, "Extension not enabled"));
            return;
        }

        context->popGroupMarker();
    }
}

ANGLE_EXPORT void GL_APIENTRY EGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image)
{
    EVENT("(GLenum target = 0x%X, GLeglImageOES image = 0x%0.8p)", target, image);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        egl::Display *display   = egl::GetGlobalDisplay();
        egl::Image *imageObject = reinterpret_cast<egl::Image *>(image);
        if (!ValidateEGLImageTargetTexture2DOES(context, display, target, imageObject))
        {
            return;
        }

        Texture *texture = context->getTargetTexture(target);
        Error error = texture->setEGLImageTarget(target, imageObject);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

ANGLE_EXPORT void GL_APIENTRY EGLImageTargetRenderbufferStorageOES(GLenum target,
                                                                   GLeglImageOES image)
{
    EVENT("(GLenum target = 0x%X, GLeglImageOES image = 0x%0.8p)", target, image);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        egl::Display *display   = egl::GetGlobalDisplay();
        egl::Image *imageObject = reinterpret_cast<egl::Image *>(image);
        if (!ValidateEGLImageTargetRenderbufferStorageOES(context, display, target, imageObject))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        Error error = renderbuffer->setStorageEGLImageTarget(imageObject);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}
}
