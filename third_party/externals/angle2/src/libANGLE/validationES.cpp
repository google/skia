//
// Copyright (c) 2013-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES.h: Validation functions for generic OpenGL ES entry point parameters

#include "libANGLE/validationES.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/Image.h"
#include "libANGLE/Query.h"
#include "libANGLE/Program.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexArray.h"

#include "common/mathutil.h"
#include "common/utilities.h"

namespace gl
{
namespace
{
bool ValidateDrawAttribs(gl::Context *context, GLint primcount, GLint maxVertex)
{
    const gl::State &state     = context->getState();
    const gl::Program *program = state.getProgram();

    const VertexArray *vao     = state.getVertexArray();
    const auto &vertexAttribs  = vao->getVertexAttributes();
    size_t maxEnabledAttrib = vao->getMaxEnabledAttribute();
    for (size_t attributeIndex = 0; attributeIndex < maxEnabledAttrib; ++attributeIndex)
    {
        const VertexAttribute &attrib = vertexAttribs[attributeIndex];
        if (program->isAttribLocationActive(attributeIndex) && attrib.enabled)
        {
            gl::Buffer *buffer = attrib.buffer.get();

            if (buffer)
            {
                GLint64 attribStride     = static_cast<GLint64>(ComputeVertexAttributeStride(attrib));
                GLint64 maxVertexElement = 0;

                if (attrib.divisor > 0)
                {
                    maxVertexElement =
                        static_cast<GLint64>(primcount) / static_cast<GLint64>(attrib.divisor);
                }
                else
                {
                    maxVertexElement = static_cast<GLint64>(maxVertex);
                }

                // If we're drawing zero vertices, we have enough data.
                if (maxVertexElement > 0)
                {
                    // Note: Last vertex element does not take the full stride!
                    GLint64 attribSize =
                        static_cast<GLint64>(ComputeVertexAttributeTypeSize(attrib));
                    GLint64 attribDataSize = (maxVertexElement - 1) * attribStride + attribSize;

                    // [OpenGL ES 3.0.2] section 2.9.4 page 40:
                    // We can return INVALID_OPERATION if our vertex attribute does not have
                    // enough backing data.
                    if (attribDataSize > buffer->getSize())
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }
                }
            }
            else if (attrib.pointer == NULL)
            {
                // This is an application error that would normally result in a crash,
                // but we catch it and return an error
                context->recordError(Error(
                    GL_INVALID_OPERATION, "An enabled vertex array has no buffer and no pointer."));
                return false;
            }
        }
    }

    return true;
}

}  // anonymous namespace

bool ValidCap(const Context *context, GLenum cap)
{
    switch (cap)
    {
      case GL_CULL_FACE:
      case GL_POLYGON_OFFSET_FILL:
      case GL_SAMPLE_ALPHA_TO_COVERAGE:
      case GL_SAMPLE_COVERAGE:
      case GL_SCISSOR_TEST:
      case GL_STENCIL_TEST:
      case GL_DEPTH_TEST:
      case GL_BLEND:
      case GL_DITHER:
        return true;
      case GL_PRIMITIVE_RESTART_FIXED_INDEX:
      case GL_RASTERIZER_DISCARD:
        return (context->getClientVersion() >= 3);
      default:
        return false;
    }
}

bool ValidTextureTarget(const Context *context, GLenum target)
{
    switch (target)
    {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP:
        return true;

      case GL_TEXTURE_3D:
      case GL_TEXTURE_2D_ARRAY:
        return (context->getClientVersion() >= 3);

      default:
        return false;
    }
}

// This function differs from ValidTextureTarget in that the target must be
// usable as the destination of a 2D operation-- so a cube face is valid, but
// GL_TEXTURE_CUBE_MAP is not.
// Note: duplicate of IsInternalTextureTarget
bool ValidTexture2DDestinationTarget(const Context *context, GLenum target)
{
    switch (target)
    {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        return true;
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_3D:
        return (context->getClientVersion() >= 3);
      default:
        return false;
    }
}

bool ValidFramebufferTarget(GLenum target)
{
    static_assert(GL_DRAW_FRAMEBUFFER_ANGLE == GL_DRAW_FRAMEBUFFER && GL_READ_FRAMEBUFFER_ANGLE == GL_READ_FRAMEBUFFER,
                  "ANGLE framebuffer enums must equal the ES3 framebuffer enums.");

    switch (target)
    {
      case GL_FRAMEBUFFER:      return true;
      case GL_READ_FRAMEBUFFER: return true;
      case GL_DRAW_FRAMEBUFFER: return true;
      default:                  return false;
    }
}

bool ValidBufferTarget(const Context *context, GLenum target)
{
    switch (target)
    {
      case GL_ARRAY_BUFFER:
      case GL_ELEMENT_ARRAY_BUFFER:
        return true;

      case GL_PIXEL_PACK_BUFFER:
      case GL_PIXEL_UNPACK_BUFFER:
          return (context->getExtensions().pixelBufferObject || context->getClientVersion() >= 3);

      case GL_COPY_READ_BUFFER:
      case GL_COPY_WRITE_BUFFER:
      case GL_TRANSFORM_FEEDBACK_BUFFER:
      case GL_UNIFORM_BUFFER:
        return (context->getClientVersion() >= 3);

      default:
        return false;
    }
}

bool ValidBufferParameter(const Context *context, GLenum pname)
{
    const Extensions &extensions = context->getExtensions();

    switch (pname)
    {
      case GL_BUFFER_USAGE:
      case GL_BUFFER_SIZE:
        return true;

      case GL_BUFFER_ACCESS_OES:
        return extensions.mapBuffer;

      case GL_BUFFER_MAPPED:
        static_assert(GL_BUFFER_MAPPED == GL_BUFFER_MAPPED_OES, "GL enums should be equal.");
        return (context->getClientVersion() >= 3) || extensions.mapBuffer || extensions.mapBufferRange;

      // GL_BUFFER_MAP_POINTER is a special case, and may only be
      // queried with GetBufferPointerv
      case GL_BUFFER_ACCESS_FLAGS:
      case GL_BUFFER_MAP_OFFSET:
      case GL_BUFFER_MAP_LENGTH:
        return (context->getClientVersion() >= 3) || extensions.mapBufferRange;

      default:
        return false;
    }
}

bool ValidMipLevel(const Context *context, GLenum target, GLint level)
{
    size_t maxDimension = 0;
    switch (target)
    {
      case GL_TEXTURE_2D:                  maxDimension = context->getCaps().max2DTextureSize;       break;
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z: maxDimension = context->getCaps().maxCubeMapTextureSize;  break;
      case GL_TEXTURE_3D:                  maxDimension = context->getCaps().max3DTextureSize;       break;
      case GL_TEXTURE_2D_ARRAY:            maxDimension = context->getCaps().max2DTextureSize;       break;
      default: UNREACHABLE();
    }

    return level <= gl::log2(static_cast<int>(maxDimension));
}

bool ValidImageSize(const Context *context, GLenum target, GLint level,
                    GLsizei width, GLsizei height, GLsizei depth)
{
    if (level < 0 || width < 0 || height < 0 || depth < 0)
    {
        return false;
    }

    if (!context->getExtensions().textureNPOT &&
        (level != 0 && (!gl::isPow2(width) || !gl::isPow2(height) || !gl::isPow2(depth))))
    {
        return false;
    }

    if (!ValidMipLevel(context, target, level))
    {
        return false;
    }

    return true;
}

bool CompressedTextureFormatRequiresExactSize(GLenum internalFormat)
{
    // List of compressed format that require that the texture size is smaller than or a multiple of
    // the compressed block size.
    switch (internalFormat)
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE:
            return true;

        default:
            return false;
    }
}

bool ValidCompressedImageSize(const Context *context, GLenum internalFormat, GLsizei width, GLsizei height)
{
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
    if (!formatInfo.compressed)
    {
        return false;
    }

    if (width < 0 || height < 0)
    {
        return false;
    }

    if (CompressedTextureFormatRequiresExactSize(internalFormat))
    {
        if ((static_cast<GLuint>(width) > formatInfo.compressedBlockWidth &&
             width % formatInfo.compressedBlockWidth != 0) ||
            (static_cast<GLuint>(height) > formatInfo.compressedBlockHeight &&
             height % formatInfo.compressedBlockHeight != 0))
        {
            return false;
        }
    }

    return true;
}

bool ValidQueryType(const Context *context, GLenum queryType)
{
    static_assert(GL_ANY_SAMPLES_PASSED == GL_ANY_SAMPLES_PASSED_EXT, "GL extension enums not equal.");
    static_assert(GL_ANY_SAMPLES_PASSED_CONSERVATIVE == GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, "GL extension enums not equal.");

    switch (queryType)
    {
      case GL_ANY_SAMPLES_PASSED:
      case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
        return true;
      case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
        return (context->getClientVersion() >= 3);
      default:
        return false;
    }
}

Program *GetValidProgram(Context *context, GLuint id)
{
    // ES3 spec (section 2.11.1) -- "Commands that accept shader or program object names will generate the
    // error INVALID_VALUE if the provided name is not the name of either a shader or program object and
    // INVALID_OPERATION if the provided name identifies an object that is not the expected type."

    Program *validProgram = context->getProgram(id);

    if (!validProgram)
    {
        if (context->getShader(id))
        {
            context->recordError(
                Error(GL_INVALID_OPERATION, "Expected a program name, but found a shader name"));
        }
        else
        {
            context->recordError(Error(GL_INVALID_VALUE, "Program name is not valid"));
        }
    }

    return validProgram;
}

Shader *GetValidShader(Context *context, GLuint id)
{
    // See ValidProgram for spec details.

    Shader *validShader = context->getShader(id);

    if (!validShader)
    {
        if (context->getProgram(id))
        {
            context->recordError(
                Error(GL_INVALID_OPERATION, "Expected a shader name, but found a program name"));
        }
        else
        {
            context->recordError(Error(GL_INVALID_VALUE, "Shader name is invalid"));
        }
    }

    return validShader;
}

bool ValidateAttachmentTarget(gl::Context *context, GLenum attachment)
{
    if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
    {
        const unsigned int colorAttachment = (attachment - GL_COLOR_ATTACHMENT0_EXT);

        if (colorAttachment >= context->getCaps().maxColorAttachments)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }
    else
    {
        switch (attachment)
        {
          case GL_DEPTH_ATTACHMENT:
          case GL_STENCIL_ATTACHMENT:
            break;

          case GL_DEPTH_STENCIL_ATTACHMENT:
            if (context->getClientVersion() < 3)
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

    return true;
}

bool ValidateRenderbufferStorageParametersBase(gl::Context *context, GLenum target, GLsizei samples,
                                               GLenum internalformat, GLsizei width, GLsizei height)
{
    switch (target)
    {
      case GL_RENDERBUFFER:
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (width < 0 || height < 0 || samples < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
    if (!formatCaps.renderable)
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    // ANGLE_framebuffer_multisample does not explicitly state that the internal format must be
    // sized but it does state that the format must be in the ES2.0 spec table 4.5 which contains
    // only sized internal formats.
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalformat);
    if (formatInfo.pixelBytes == 0)
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (static_cast<GLuint>(std::max(width, height)) > context->getCaps().maxRenderbufferSize)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    GLuint handle = context->getState().getRenderbufferId();
    if (handle == 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateRenderbufferStorageParametersANGLE(gl::Context *context, GLenum target, GLsizei samples,
                                                GLenum internalformat, GLsizei width, GLsizei height)
{
    ASSERT(samples == 0 || context->getExtensions().framebufferMultisample);

    // ANGLE_framebuffer_multisample states that the value of samples must be less than or equal
    // to MAX_SAMPLES_ANGLE (Context::getCaps().maxSamples) otherwise GL_INVALID_VALUE is
    // generated.
    if (static_cast<GLuint>(samples) > context->getCaps().maxSamples)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    // ANGLE_framebuffer_multisample states GL_OUT_OF_MEMORY is generated on a failure to create
    // the specified storage. This is different than ES 3.0 in which a sample number higher
    // than the maximum sample number supported  by this format generates a GL_INVALID_VALUE.
    // The TextureCaps::getMaxSamples method is only guarenteed to be valid when the context is ES3.
    if (context->getClientVersion() >= 3)
    {
        const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
        if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
        {
            context->recordError(Error(GL_OUT_OF_MEMORY));
            return false;
        }
    }

    return ValidateRenderbufferStorageParametersBase(context, target, samples, internalformat, width, height);
}

bool ValidateFramebufferRenderbufferParameters(gl::Context *context, GLenum target, GLenum attachment,
                                               GLenum renderbuffertarget, GLuint renderbuffer)
{
    if (!ValidFramebufferTarget(target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    gl::Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);

    ASSERT(framebuffer);
    if (framebuffer->id() == 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION, "Cannot change default FBO's attachments"));
        return false;
    }

    if (!ValidateAttachmentTarget(context, attachment))
    {
        return false;
    }

    // [OpenGL ES 2.0.25] Section 4.4.3 page 112
    // [OpenGL ES 3.0.2] Section 4.4.2 page 201
    // 'renderbuffer' must be either zero or the name of an existing renderbuffer object of
    // type 'renderbuffertarget', otherwise an INVALID_OPERATION error is generated.
    if (renderbuffer != 0)
    {
        if (!context->getRenderbuffer(renderbuffer))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    return true;
}

static bool IsPartialBlit(gl::Context *context, const gl::FramebufferAttachment *readBuffer, const gl::FramebufferAttachment *writeBuffer,
                          GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                          GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1)
{
    if (srcX0 != 0 || srcY0 != 0 || dstX0 != 0 || dstY0 != 0 ||
        dstX1 != writeBuffer->getWidth() || dstY1 != writeBuffer->getHeight() ||
        srcX1 != readBuffer->getWidth() || srcY1 != readBuffer->getHeight())
    {
        return true;
    }
    else if (context->getState().isScissorTestEnabled())
    {
        const Rectangle &scissor = context->getState().getScissor();

        return scissor.x > 0 || scissor.y > 0 ||
               scissor.width < writeBuffer->getWidth() ||
               scissor.height < writeBuffer->getHeight();
    }
    else
    {
        return false;
    }
}

bool ValidateBlitFramebufferParameters(gl::Context *context, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1,
                                       GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask,
                                       GLenum filter, bool fromAngleExtension)
{
    switch (filter)
    {
      case GL_NEAREST:
        break;
      case GL_LINEAR:
        if (fromAngleExtension)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)) != 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (mask == 0)
    {
        // ES3.0 spec, section 4.3.2 specifies that a mask of zero is valid and no
        // buffers are copied.
        return false;
    }

    if (fromAngleExtension && (srcX1 - srcX0 != dstX1 - dstX0 || srcY1 - srcY0 != dstY1 - dstY0))
    {
        ERR("Scaling and flipping in BlitFramebufferANGLE not supported by this implementation.");
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    // ES3.0 spec, section 4.3.2 states that linear filtering is only available for the
    // color buffer, leaving only nearest being unfiltered from above
    if ((mask & ~GL_COLOR_BUFFER_BIT) != 0 && filter != GL_NEAREST)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (context->getState().getReadFramebuffer()->id() == context->getState().getDrawFramebuffer()->id())
    {
        if (fromAngleExtension)
        {
            ERR("Blits with the same source and destination framebuffer are not supported by this "
                "implementation.");
        }
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::Framebuffer *readFramebuffer = context->getState().getReadFramebuffer();
    const gl::Framebuffer *drawFramebuffer = context->getState().getDrawFramebuffer();

    if (!readFramebuffer || !drawFramebuffer)
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    if (!readFramebuffer->checkStatus(context->getData()))
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    if (!drawFramebuffer->checkStatus(context->getData()))
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    if (drawFramebuffer->getSamples(context->getData()) != 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    bool sameBounds = srcX0 == dstX0 && srcY0 == dstY0 && srcX1 == dstX1 && srcY1 == dstY1;

    if (mask & GL_COLOR_BUFFER_BIT)
    {
        const gl::FramebufferAttachment *readColorBuffer = readFramebuffer->getReadColorbuffer();
        const gl::FramebufferAttachment *drawColorBuffer = drawFramebuffer->getFirstColorbuffer();

        if (readColorBuffer && drawColorBuffer)
        {
            GLenum readInternalFormat = readColorBuffer->getInternalFormat();
            const InternalFormat &readFormatInfo = GetInternalFormatInfo(readInternalFormat);

            for (size_t i = 0; i < drawFramebuffer->getNumColorBuffers(); i++)
            {
                if (drawFramebuffer->isEnabledColorAttachment(i))
                {
                    GLenum drawInternalFormat = drawFramebuffer->getColorbuffer(i)->getInternalFormat();
                    const InternalFormat &drawFormatInfo = GetInternalFormatInfo(drawInternalFormat);

                    // The GL ES 3.0.2 spec (pg 193) states that:
                    // 1) If the read buffer is fixed point format, the draw buffer must be as well
                    // 2) If the read buffer is an unsigned integer format, the draw buffer must be as well
                    // 3) If the read buffer is a signed integer format, the draw buffer must be as well
                    if ( (readFormatInfo.componentType == GL_UNSIGNED_NORMALIZED || readFormatInfo.componentType == GL_SIGNED_NORMALIZED) &&
                        !(drawFormatInfo.componentType == GL_UNSIGNED_NORMALIZED || drawFormatInfo.componentType == GL_SIGNED_NORMALIZED))
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }

                    if (readFormatInfo.componentType == GL_UNSIGNED_INT && drawFormatInfo.componentType != GL_UNSIGNED_INT)
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }

                    if (readFormatInfo.componentType == GL_INT && drawFormatInfo.componentType != GL_INT)
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }

                    if (readColorBuffer->getSamples() > 0 && (readInternalFormat != drawInternalFormat || !sameBounds))
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }
                }
            }

            if ((readFormatInfo.componentType == GL_INT || readFormatInfo.componentType == GL_UNSIGNED_INT) && filter == GL_LINEAR)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }

            if (fromAngleExtension)
            {
                const FramebufferAttachment *readColorAttachment = readFramebuffer->getReadColorbuffer();
                if (!readColorAttachment ||
                    (!(readColorAttachment->type() == GL_TEXTURE && readColorAttachment->getTextureImageIndex().type == GL_TEXTURE_2D) &&
                    readColorAttachment->type() != GL_RENDERBUFFER &&
                    readColorAttachment->type() != GL_FRAMEBUFFER_DEFAULT))
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }

                for (size_t colorAttachment = 0;
                     colorAttachment < drawFramebuffer->getNumColorBuffers(); ++colorAttachment)
                {
                    if (drawFramebuffer->isEnabledColorAttachment(colorAttachment))
                    {
                        const FramebufferAttachment *attachment = drawFramebuffer->getColorbuffer(colorAttachment);
                        ASSERT(attachment);

                        if (!(attachment->type() == GL_TEXTURE && attachment->getTextureImageIndex().type == GL_TEXTURE_2D) &&
                            attachment->type() != GL_RENDERBUFFER &&
                            attachment->type() != GL_FRAMEBUFFER_DEFAULT)
                        {
                            context->recordError(Error(GL_INVALID_OPERATION));
                            return false;
                        }

                        // Return an error if the destination formats do not match
                        if (attachment->getInternalFormat() != readColorBuffer->getInternalFormat())
                        {
                            context->recordError(Error(GL_INVALID_OPERATION));
                            return false;
                        }
                    }
                }

                int readSamples = readFramebuffer->getSamples(context->getData());

                if (readSamples != 0 && IsPartialBlit(context, readColorBuffer, drawColorBuffer,
                                                      srcX0, srcY0, srcX1, srcY1,
                                                      dstX0, dstY0, dstX1, dstY1))
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }
            }
        }
    }

    GLenum masks[] = {GL_DEPTH_BUFFER_BIT, GL_STENCIL_BUFFER_BIT};
    GLenum attachments[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
    for (size_t i = 0; i < 2; i++)
    {
        if (mask & masks[i])
        {
            const gl::FramebufferAttachment *readBuffer = readFramebuffer->getAttachment(attachments[i]);
            const gl::FramebufferAttachment *drawBuffer = drawFramebuffer->getAttachment(attachments[i]);

            if (readBuffer && drawBuffer)
            {
                if (readBuffer->getInternalFormat() != drawBuffer->getInternalFormat())
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }

                if (readBuffer->getSamples() > 0 && !sameBounds)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }

                if (fromAngleExtension)
                {
                    if (IsPartialBlit(context, readBuffer, drawBuffer,
                                      srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1))
                    {
                        ERR("Only whole-buffer depth and stencil blits are supported by this implementation.");
                        context->recordError(Error(GL_INVALID_OPERATION)); // only whole-buffer copies are permitted
                        return false;
                    }

                    if (readBuffer->getSamples() != 0 || drawBuffer->getSamples() != 0)
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return false;
                    }
                }
            }
        }
    }

    return true;
}

bool ValidateGetVertexAttribParameters(Context *context, GLenum pname)
{
    switch (pname)
    {
      case GL_VERTEX_ATTRIB_ARRAY_ENABLED:
      case GL_VERTEX_ATTRIB_ARRAY_SIZE:
      case GL_VERTEX_ATTRIB_ARRAY_STRIDE:
      case GL_VERTEX_ATTRIB_ARRAY_TYPE:
      case GL_VERTEX_ATTRIB_ARRAY_NORMALIZED:
      case GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING:
      case GL_CURRENT_VERTEX_ATTRIB:
        return true;

      case GL_VERTEX_ATTRIB_ARRAY_DIVISOR:
        // Don't verify ES3 context because GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE uses
        // the same constant.
        static_assert(GL_VERTEX_ATTRIB_ARRAY_DIVISOR == GL_VERTEX_ATTRIB_ARRAY_DIVISOR_ANGLE,
                      "ANGLE extension enums not equal to GL enums.");
        return true;

      case GL_VERTEX_ATTRIB_ARRAY_INTEGER:
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        return true;

      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }
}

bool ValidateTexParamParameters(gl::Context *context, GLenum pname, GLint param)
{
    switch (pname)
    {
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      default: break;
    }

    switch (pname)
    {
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
        switch (param)
        {
          case GL_REPEAT:
          case GL_CLAMP_TO_EDGE:
          case GL_MIRRORED_REPEAT:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

      case GL_TEXTURE_MIN_FILTER:
        switch (param)
        {
          case GL_NEAREST:
          case GL_LINEAR:
          case GL_NEAREST_MIPMAP_NEAREST:
          case GL_LINEAR_MIPMAP_NEAREST:
          case GL_NEAREST_MIPMAP_LINEAR:
          case GL_LINEAR_MIPMAP_LINEAR:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_MAG_FILTER:
        switch (param)
        {
          case GL_NEAREST:
          case GL_LINEAR:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_USAGE_ANGLE:
        switch (param)
        {
          case GL_NONE:
          case GL_FRAMEBUFFER_ATTACHMENT_ANGLE:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_MAX_ANISOTROPY_EXT:
        if (!context->getExtensions().textureFilterAnisotropic)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

        // we assume the parameter passed to this validation method is truncated, not rounded
        if (param < 1)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
        return true;

      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
        // any value is permissible
        return true;

      case GL_TEXTURE_COMPARE_MODE:
        // Acceptable mode parameters from GLES 3.0.2 spec, table 3.17
        switch (param)
        {
          case GL_NONE:
          case GL_COMPARE_REF_TO_TEXTURE:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_COMPARE_FUNC:
        // Acceptable function parameters from GLES 3.0.2 spec, table 3.17
        switch (param)
        {
          case GL_LEQUAL:
          case GL_GEQUAL:
          case GL_LESS:
          case GL_GREATER:
          case GL_EQUAL:
          case GL_NOTEQUAL:
          case GL_ALWAYS:
          case GL_NEVER:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_SWIZZLE_R:
      case GL_TEXTURE_SWIZZLE_G:
      case GL_TEXTURE_SWIZZLE_B:
      case GL_TEXTURE_SWIZZLE_A:
        switch (param)
        {
          case GL_RED:
          case GL_GREEN:
          case GL_BLUE:
          case GL_ALPHA:
          case GL_ZERO:
          case GL_ONE:
            return true;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;

      case GL_TEXTURE_BASE_LEVEL:
      case GL_TEXTURE_MAX_LEVEL:
        if (param < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
        return true;

      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }
}

bool ValidateSamplerObjectParameter(gl::Context *context, GLenum pname)
{
    switch (pname)
    {
      case GL_TEXTURE_MIN_FILTER:
      case GL_TEXTURE_MAG_FILTER:
      case GL_TEXTURE_WRAP_S:
      case GL_TEXTURE_WRAP_T:
      case GL_TEXTURE_WRAP_R:
      case GL_TEXTURE_MIN_LOD:
      case GL_TEXTURE_MAX_LOD:
      case GL_TEXTURE_COMPARE_MODE:
      case GL_TEXTURE_COMPARE_FUNC:
        return true;

      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }
}

bool ValidateReadPixelsParameters(gl::Context *context, GLint x, GLint y, GLsizei width, GLsizei height,
                                  GLenum format, GLenum type, GLsizei *bufSize, GLvoid *pixels)
{
    gl::Framebuffer *framebuffer = context->getState().getReadFramebuffer();
    ASSERT(framebuffer);

    if (framebuffer->checkStatus(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    if (context->getState().getReadFramebuffer()->id() != 0 &&
        framebuffer->getSamples(context->getData()) != 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const FramebufferAttachment *readBuffer = framebuffer->getReadColorbuffer();
    if (!readBuffer)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    GLenum currentFormat = framebuffer->getImplementationColorReadFormat();
    GLenum currentType = framebuffer->getImplementationColorReadType();
    GLenum currentInternalFormat = readBuffer->getInternalFormat();
    GLuint clientVersion = context->getClientVersion();

    bool validReadFormat = (clientVersion < 3) ? ValidES2ReadFormatType(context, format, type) :
                                                 ValidES3ReadFormatType(context, currentInternalFormat, format, type);

    if (!(currentFormat == format && currentType == type) && !validReadFormat)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    GLenum sizedInternalFormat = GetSizedInternalFormat(format, type);
    const InternalFormat &sizedFormatInfo = GetInternalFormatInfo(sizedInternalFormat);

    GLsizei outputPitch = sizedFormatInfo.computeRowPitch(type, width, context->getState().getPackAlignment(), 0);
    // sized query sanity check
    if (bufSize)
    {
        int requiredSize = outputPitch * height;
        if (requiredSize > *bufSize)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    return true;
}

bool ValidateBeginQuery(gl::Context *context, GLenum target, GLuint id)
{
    if (!ValidQueryType(context, target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (id == 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    // From EXT_occlusion_query_boolean: If BeginQueryEXT is called with an <id>
    // of zero, if the active query object name for <target> is non-zero (for the
    // targets ANY_SAMPLES_PASSED_EXT and ANY_SAMPLES_PASSED_CONSERVATIVE_EXT, if
    // the active query for either target is non-zero), if <id> is the name of an
    // existing query object whose type does not match <target>, or if <id> is the
    // active query object name for any query type, the error INVALID_OPERATION is
    // generated.

    // Ensure no other queries are active
    // NOTE: If other queries than occlusion are supported, we will need to check
    // separately that:
    //    a) The query ID passed is not the current active query for any target/type
    //    b) There are no active queries for the requested target (and in the case
    //       of GL_ANY_SAMPLES_PASSED_EXT and GL_ANY_SAMPLES_PASSED_CONSERVATIVE_EXT,
    //       no query may be active for either if glBeginQuery targets either.
    if (context->getState().isQueryActive())
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    Query *queryObject = context->getQuery(id, true, target);

    // check that name was obtained with glGenQueries
    if (!queryObject)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    // check for type mismatch
    if (queryObject->getType() != target)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateEndQuery(gl::Context *context, GLenum target)
{
    if (!ValidQueryType(context, target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    const Query *queryObject = context->getState().getActiveQuery(target);

    if (queryObject == NULL)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

static bool ValidateUniformCommonBase(gl::Context *context,
                                      GLenum targetUniformType,
                                      GLint location,
                                      GLsizei count,
                                      const LinkedUniform **uniformOut)
{
    if (count < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    gl::Program *program = context->getState().getProgram();
    if (!program)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (location == -1)
    {
        // Silently ignore the uniform command
        return false;
    }

    if (!program->isValidUniformLocation(location))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const LinkedUniform &uniform = program->getUniformByLocation(location);

    // attempting to write an array to a non-array uniform is an INVALID_OPERATION
    if (!uniform.isArray() && count > 1)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    *uniformOut = &uniform;
    return true;
}

bool ValidateUniform(gl::Context *context, GLenum uniformType, GLint location, GLsizei count)
{
    // Check for ES3 uniform entry points
    if (VariableComponentType(uniformType) == GL_UNSIGNED_INT && context->getClientVersion() < 3)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const LinkedUniform *uniform = nullptr;
    if (!ValidateUniformCommonBase(context, uniformType, location, count, &uniform))
    {
        return false;
    }

    GLenum targetBoolType = VariableBoolVectorType(uniformType);
    bool samplerUniformCheck = (IsSamplerType(uniform->type) && uniformType == GL_INT);
    if (!samplerUniformCheck && uniformType != uniform->type && targetBoolType != uniform->type)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateUniformMatrix(gl::Context *context, GLenum matrixType, GLint location, GLsizei count,
                           GLboolean transpose)
{
    // Check for ES3 uniform entry points
    int rows = VariableRowCount(matrixType);
    int cols = VariableColumnCount(matrixType);
    if (rows != cols && context->getClientVersion() < 3)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (transpose != GL_FALSE && context->getClientVersion() < 3)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    const LinkedUniform *uniform = nullptr;
    if (!ValidateUniformCommonBase(context, matrixType, location, count, &uniform))
    {
        return false;
    }

    if (uniform->type != matrixType)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateStateQuery(gl::Context *context, GLenum pname, GLenum *nativeType, unsigned int *numParams)
{
    if (!context->getQueryParameterInfo(pname, nativeType, numParams))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    const Caps &caps = context->getCaps();

    if (pname >= GL_DRAW_BUFFER0 && pname <= GL_DRAW_BUFFER15)
    {
        unsigned int colorAttachment = (pname - GL_DRAW_BUFFER0);

        if (colorAttachment >= caps.maxDrawBuffers)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    switch (pname)
    {
      case GL_TEXTURE_BINDING_2D:
      case GL_TEXTURE_BINDING_CUBE_MAP:
      case GL_TEXTURE_BINDING_3D:
      case GL_TEXTURE_BINDING_2D_ARRAY:
        if (context->getState().getActiveSampler() >= caps.maxCombinedTextureImageUnits)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
        break;

      case GL_IMPLEMENTATION_COLOR_READ_TYPE:
      case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
        {
            Framebuffer *framebuffer = context->getState().getReadFramebuffer();
            ASSERT(framebuffer);
            if (framebuffer->checkStatus(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }

            const FramebufferAttachment *attachment = framebuffer->getReadColorbuffer();
            if (!attachment)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return false;
            }
        }
        break;

      default:
        break;
    }

    // pname is valid, but there are no parameters to return
    if (numParams == 0)
    {
        return false;
    }

    return true;
}

bool ValidateCopyTexImageParametersBase(gl::Context *context, GLenum target, GLint level, GLenum internalformat, bool isSubImage,
                                        GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height,
                                        GLint border, GLenum *textureFormatOut)
{

    if (!ValidTexture2DDestinationTarget(context, target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (level < 0 || xoffset < 0 || yoffset < 0 || zoffset < 0 || width < 0 || height < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (std::numeric_limits<GLsizei>::max() - xoffset < width || std::numeric_limits<GLsizei>::max() - yoffset < height)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (border != 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (!ValidMipLevel(context, target, level))
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    gl::Framebuffer *framebuffer = context->getState().getReadFramebuffer();
    if (framebuffer->checkStatus(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    if (context->getState().getReadFramebuffer()->id() != 0 && framebuffer->getSamples(context->getData()) != 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::Caps &caps = context->getCaps();

    GLuint maxDimension = 0;
    switch (target)
    {
      case GL_TEXTURE_2D:
        maxDimension = caps.max2DTextureSize;
        break;

      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
        maxDimension = caps.maxCubeMapTextureSize;
        break;

      case GL_TEXTURE_2D_ARRAY:
        maxDimension = caps.max2DTextureSize;
        break;

      case GL_TEXTURE_3D:
        maxDimension = caps.max3DTextureSize;
        break;

      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    gl::Texture *texture = context->getTargetTexture(IsCubeMapTextureTarget(target) ? GL_TEXTURE_CUBE_MAP : target);
    if (!texture)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (texture->getImmutableFormat() && !isSubImage)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalformat);

    if (formatInfo.depthBits > 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (formatInfo.compressed && !ValidCompressedImageSize(context, internalformat, width, height))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (isSubImage)
    {
        if (static_cast<size_t>(xoffset + width) > texture->getWidth(target, level) ||
            static_cast<size_t>(yoffset + height) > texture->getHeight(target, level) ||
            static_cast<size_t>(zoffset) >= texture->getDepth(target, level))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }
    else
    {
        if (IsCubeMapTextureTarget(target) && width != height)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }

        if (!formatInfo.textureSupport(context->getClientVersion(), context->getExtensions()))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

        int maxLevelDimension = (maxDimension >> level);
        if (static_cast<int>(width) > maxLevelDimension || static_cast<int>(height) > maxLevelDimension)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }

    *textureFormatOut = texture->getInternalFormat(target, level);
    return true;
}

static bool ValidateDrawBase(Context *context, GLenum mode, GLsizei count, GLsizei primcount)
{
    switch (mode)
    {
      case GL_POINTS:
      case GL_LINES:
      case GL_LINE_LOOP:
      case GL_LINE_STRIP:
      case GL_TRIANGLES:
      case GL_TRIANGLE_STRIP:
      case GL_TRIANGLE_FAN:
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (count < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    const State &state = context->getState();

    // Check for mapped buffers
    if (state.hasMappedBuffer(GL_ARRAY_BUFFER))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (context->getLimitations().noSeparateStencilRefsAndMasks)
    {
        const gl::DepthStencilState &depthStencilState = state.getDepthStencilState();
        if (depthStencilState.stencilWritemask != depthStencilState.stencilBackWritemask ||
            state.getStencilRef() != state.getStencilBackRef() ||
            depthStencilState.stencilMask != depthStencilState.stencilBackMask)
        {
            // Note: these separate values are not supported in WebGL, due to D3D's limitations. See
            // Section 6.10 of the WebGL 1.0 spec
            ERR(
                "This ANGLE implementation does not support separate front/back stencil "
                "writemasks, reference values, or stencil mask values.");
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    const gl::Framebuffer *fbo = state.getDrawFramebuffer();
    if (!fbo || fbo->checkStatus(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
    {
        context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
        return false;
    }

    gl::Program *program = state.getProgram();
    if (!program)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (!program->validateSamplers(NULL, context->getCaps()))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    // Uniform buffer validation
    for (unsigned int uniformBlockIndex = 0; uniformBlockIndex < program->getActiveUniformBlockCount(); uniformBlockIndex++)
    {
        const gl::UniformBlock &uniformBlock = program->getUniformBlockByIndex(uniformBlockIndex);
        GLuint blockBinding = program->getUniformBlockBinding(uniformBlockIndex);
        const gl::Buffer *uniformBuffer = state.getIndexedUniformBuffer(blockBinding);

        if (!uniformBuffer)
        {
            // undefined behaviour
            context->recordError(Error(GL_INVALID_OPERATION, "It is undefined behaviour to have a used but unbound uniform buffer."));
            return false;
        }

        size_t uniformBufferSize = state.getIndexedUniformBufferSize(blockBinding);

        if (uniformBufferSize == 0)
        {
            // Bind the whole buffer.
            uniformBufferSize = static_cast<size_t>(uniformBuffer->getSize());
        }

        if (uniformBufferSize < uniformBlock.dataSize)
        {
            // undefined behaviour
            context->recordError(Error(GL_INVALID_OPERATION, "It is undefined behaviour to use a uniform buffer that is too small."));
            return false;
        }
    }

    // No-op if zero count
    return (count > 0);
}

bool ValidateDrawArrays(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    if (first < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    const State &state = context->getState();
    gl::TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
    if (curTransformFeedback && curTransformFeedback->isActive() && !curTransformFeedback->isPaused() &&
        curTransformFeedback->getPrimitiveMode() != mode)
    {
        // It is an invalid operation to call DrawArrays or DrawArraysInstanced with a draw mode
        // that does not match the current transform feedback object's draw mode (if transform feedback
        // is active), (3.0.2, section 2.14, pg 86)
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (!ValidateDrawBase(context, mode, count, primcount))
    {
        return false;
    }

    if (!ValidateDrawAttribs(context, primcount, count))
    {
        return false;
    }

    return true;
}

bool ValidateDrawArraysInstanced(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    if (primcount < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (!ValidateDrawArrays(context, mode, first, count, primcount))
    {
        return false;
    }

    // No-op if zero primitive count
    return (primcount > 0);
}

static bool ValidateDrawInstancedANGLE(Context *context)
{
    // Verify there is at least one active attribute with a divisor of zero
    const gl::State& state = context->getState();

    gl::Program *program = state.getProgram();

    const VertexArray *vao = state.getVertexArray();
    for (size_t attributeIndex = 0; attributeIndex < MAX_VERTEX_ATTRIBS; attributeIndex++)
    {
        const VertexAttribute &attrib = vao->getVertexAttribute(attributeIndex);
        if (program->isAttribLocationActive(attributeIndex) && attrib.divisor == 0)
        {
            return true;
        }
    }

    context->recordError(Error(GL_INVALID_OPERATION, "ANGLE_instanced_arrays requires that at least one active attribute"
                                                     "has a divisor of zero."));
    return false;
}

bool ValidateDrawArraysInstancedANGLE(Context *context, GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    if (!ValidateDrawInstancedANGLE(context))
    {
        return false;
    }

    return ValidateDrawArraysInstanced(context, mode, first, count, primcount);
}

bool ValidateDrawElements(Context *context,
                          GLenum mode,
                          GLsizei count,
                          GLenum type,
                          const GLvoid *indices,
                          GLsizei primcount,
                          IndexRange *indexRangeOut)
{
    switch (type)
    {
      case GL_UNSIGNED_BYTE:
      case GL_UNSIGNED_SHORT:
        break;
      case GL_UNSIGNED_INT:
        if (!context->getExtensions().elementIndexUint)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }
        break;
      default:
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    const State &state = context->getState();

    gl::TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
    if (curTransformFeedback && curTransformFeedback->isActive() && !curTransformFeedback->isPaused())
    {
        // It is an invalid operation to call DrawElements, DrawRangeElements or DrawElementsInstanced
        // while transform feedback is active, (3.0.2, section 2.14, pg 86)
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    // Check for mapped buffers
    if (state.hasMappedBuffer(GL_ELEMENT_ARRAY_BUFFER))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    const gl::VertexArray *vao = state.getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer().get();
    if (!indices && !elementArrayBuffer)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (elementArrayBuffer)
    {
        const gl::Type &typeInfo = gl::GetTypeInfo(type);

        GLint64 offset = reinterpret_cast<GLint64>(indices);
        GLint64 byteCount = static_cast<GLint64>(typeInfo.bytes) * static_cast<GLint64>(count)+offset;

        // check for integer overflows
        if (static_cast<GLuint>(count) > (std::numeric_limits<GLuint>::max() / typeInfo.bytes) ||
            byteCount > static_cast<GLint64>(std::numeric_limits<GLuint>::max()))
        {
            context->recordError(Error(GL_OUT_OF_MEMORY));
            return false;
        }

        // Check for reading past the end of the bound buffer object
        if (byteCount > elementArrayBuffer->getSize())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }
    else if (!indices)
    {
        // Catch this programming error here
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (!ValidateDrawBase(context, mode, count, primcount))
    {
        return false;
    }

    // Use max index to validate if our vertex buffers are large enough for the pull.
    // TODO: offer fast path, with disabled index validation.
    // TODO: also disable index checking on back-ends that are robust to out-of-range accesses.
    if (elementArrayBuffer)
    {
        uintptr_t offset = reinterpret_cast<uintptr_t>(indices);
        Error error =
            elementArrayBuffer->getIndexRange(type, static_cast<size_t>(offset), count,
                                              state.isPrimitiveRestartEnabled(), indexRangeOut);
        if (error.isError())
        {
            context->recordError(error);
            return false;
        }
    }
    else
    {
        *indexRangeOut = ComputeIndexRange(type, indices, count, state.isPrimitiveRestartEnabled());
    }

    if (!ValidateDrawAttribs(context, primcount, static_cast<GLsizei>(indexRangeOut->end)))
    {
        return false;
    }

    // No op if there are no real indices in the index data (all are primitive restart).
    return (indexRangeOut->vertexIndexCount > 0);
}

bool ValidateDrawElementsInstanced(Context *context,
                                   GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   GLsizei primcount,
                                   IndexRange *indexRangeOut)
{
    if (primcount < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (!ValidateDrawElements(context, mode, count, type, indices, primcount, indexRangeOut))
    {
        return false;
    }

    // No-op zero primitive count
    return (primcount > 0);
}

bool ValidateDrawElementsInstancedANGLE(Context *context,
                                        GLenum mode,
                                        GLsizei count,
                                        GLenum type,
                                        const GLvoid *indices,
                                        GLsizei primcount,
                                        IndexRange *indexRangeOut)
{
    if (!ValidateDrawInstancedANGLE(context))
    {
        return false;
    }

    return ValidateDrawElementsInstanced(context, mode, count, type, indices, primcount, indexRangeOut);
}

bool ValidateFramebufferTextureBase(Context *context, GLenum target, GLenum attachment,
                                    GLuint texture, GLint level)
{
    if (!ValidFramebufferTarget(target))
    {
        context->recordError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (!ValidateAttachmentTarget(context, attachment))
    {
        return false;
    }

    if (texture != 0)
    {
        gl::Texture *tex = context->getTexture(texture);

        if (tex == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }

        if (level < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return false;
        }
    }

    const gl::Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->id() == 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION, "Cannot change default FBO's attachments"));
        return false;
    }

    return true;
}

bool ValidateFramebufferTexture2D(Context *context, GLenum target, GLenum attachment,
                                  GLenum textarget, GLuint texture, GLint level)
{
    // Attachments are required to be bound to level 0 without ES3 or the GL_OES_fbo_render_mipmap extension
    if (context->getClientVersion() < 3 && !context->getExtensions().fboRenderMipmap && level != 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    if (!ValidateFramebufferTextureBase(context, target, attachment, texture, level))
    {
        return false;
    }

    if (texture != 0)
    {
        gl::Texture *tex = context->getTexture(texture);
        ASSERT(tex);

        const gl::Caps &caps = context->getCaps();

        switch (textarget)
        {
          case GL_TEXTURE_2D:
            {
                if (level > gl::log2(caps.max2DTextureSize))
                {
                    context->recordError(Error(GL_INVALID_VALUE));
                    return false;
                }
                if (tex->getTarget() != GL_TEXTURE_2D)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                if (level > gl::log2(caps.maxCubeMapTextureSize))
                {
                    context->recordError(Error(GL_INVALID_VALUE));
                    return false;
                }
                if (tex->getTarget() != GL_TEXTURE_CUBE_MAP)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return false;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return false;
        }

        const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(tex->getInternalFormat(textarget, level));
        if (internalFormatInfo.compressed)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return false;
        }
    }

    return true;
}

bool ValidateGetUniformBase(Context *context, GLuint program, GLint location)
{
    if (program == 0)
    {
        context->recordError(Error(GL_INVALID_VALUE));
        return false;
    }

    gl::Program *programObject = GetValidProgram(context, program);
    if (!programObject)
    {
        return false;
    }

    if (!programObject || !programObject->isLinked())
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    if (!programObject->isValidUniformLocation(location))
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateGetUniformfv(Context *context, GLuint program, GLint location, GLfloat* params)
{
    return ValidateGetUniformBase(context, program, location);
}

bool ValidateGetUniformiv(Context *context, GLuint program, GLint location, GLint* params)
{
    return ValidateGetUniformBase(context, program, location);
}

static bool ValidateSizedGetUniform(Context *context, GLuint program, GLint location, GLsizei bufSize)
{
    if (!ValidateGetUniformBase(context, program, location))
    {
        return false;
    }

    gl::Program *programObject = context->getProgram(program);
    ASSERT(programObject);

    // sized queries -- ensure the provided buffer is large enough
    const LinkedUniform &uniform = programObject->getUniformByLocation(location);
    size_t requiredBytes = VariableExternalSize(uniform.type);
    if (static_cast<size_t>(bufSize) < requiredBytes)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    return true;
}

bool ValidateGetnUniformfvEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
    return ValidateSizedGetUniform(context, program, location, bufSize);
}

bool ValidateGetnUniformivEXT(Context *context, GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
    return ValidateSizedGetUniform(context, program, location, bufSize);
}

bool ValidateDiscardFramebufferBase(Context *context, GLenum target, GLsizei numAttachments,
                                    const GLenum *attachments, bool defaultFramebuffer)
{
    if (numAttachments < 0)
    {
        context->recordError(Error(GL_INVALID_VALUE, "numAttachments must not be less than zero"));
        return false;
    }

    for (GLsizei i = 0; i < numAttachments; ++i)
    {
        if (attachments[i] >= GL_COLOR_ATTACHMENT0 && attachments[i] <= GL_COLOR_ATTACHMENT15)
        {
            if (defaultFramebuffer)
            {
                context->recordError(Error(GL_INVALID_ENUM, "Invalid attachment when the default framebuffer is bound"));
                return false;
            }

            if (attachments[i] >= GL_COLOR_ATTACHMENT0 + context->getCaps().maxColorAttachments)
            {
                context->recordError(Error(GL_INVALID_OPERATION,
                                           "Requested color attachment is greater than the maximum supported color attachments"));
                return false;
            }
        }
        else
        {
            switch (attachments[i])
            {
              case GL_DEPTH_ATTACHMENT:
              case GL_STENCIL_ATTACHMENT:
              case GL_DEPTH_STENCIL_ATTACHMENT:
                if (defaultFramebuffer)
                {
                    context->recordError(Error(GL_INVALID_ENUM, "Invalid attachment when the default framebuffer is bound"));
                    return false;
                }
                break;
              case GL_COLOR:
              case GL_DEPTH:
              case GL_STENCIL:
                if (!defaultFramebuffer)
                {
                    context->recordError(Error(GL_INVALID_ENUM, "Invalid attachment when the default framebuffer is not bound"));
                    return false;
                }
                break;
              default:
                context->recordError(Error(GL_INVALID_ENUM, "Invalid attachment"));
                return false;
            }
        }
    }

    return true;
}

bool ValidateInsertEventMarkerEXT(Context *context, GLsizei length, const char *marker)
{
    // Note that debug marker calls must not set error state

    if (length < 0)
    {
        return false;
    }

    if (marker == nullptr)
    {
        return false;
    }

    return true;
}

bool ValidatePushGroupMarkerEXT(Context *context, GLsizei length, const char *marker)
{
    // Note that debug marker calls must not set error state

    if (length < 0)
    {
        return false;
    }

    if (length > 0 && marker == nullptr)
    {
        return false;
    }

    return true;
}

bool ValidateEGLImageTargetTexture2DOES(Context *context,
                                        egl::Display *display,
                                        GLenum target,
                                        egl::Image *image)
{
    if (!context->getExtensions().eglImage && !context->getExtensions().eglImageExternal)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    switch (target)
    {
        case GL_TEXTURE_2D:
            break;

        default:
            context->recordError(Error(GL_INVALID_ENUM, "invalid texture target."));
            return false;
    }

    if (!display->isValidImage(image))
    {
        context->recordError(Error(GL_INVALID_VALUE, "EGL image is not valid."));
        return false;
    }

    if (image->getSamples() > 0)
    {
        context->recordError(Error(GL_INVALID_OPERATION,
                                   "cannot create a 2D texture from a multisampled EGL image."));
        return false;
    }

    const TextureCaps &textureCaps = context->getTextureCaps().get(image->getInternalFormat());
    if (!textureCaps.texturable)
    {
        context->recordError(Error(GL_INVALID_OPERATION,
                                   "EGL image internal format is not supported as a texture."));
        return false;
    }

    return true;
}

bool ValidateEGLImageTargetRenderbufferStorageOES(Context *context,
                                                  egl::Display *display,
                                                  GLenum target,
                                                  egl::Image *image)
{
    if (!context->getExtensions().eglImage)
    {
        context->recordError(Error(GL_INVALID_OPERATION));
        return false;
    }

    switch (target)
    {
        case GL_RENDERBUFFER:
            break;

        default:
            context->recordError(Error(GL_INVALID_ENUM, "invalid renderbuffer target."));
            return false;
    }

    if (!display->isValidImage(image))
    {
        context->recordError(Error(GL_INVALID_VALUE, "EGL image is not valid."));
        return false;
    }

    const TextureCaps &textureCaps = context->getTextureCaps().get(image->getInternalFormat());
    if (!textureCaps.renderable)
    {
        context->recordError(Error(
            GL_INVALID_OPERATION, "EGL image internal format is not supported as a renderbuffer."));
        return false;
    }

    return true;
}
}
