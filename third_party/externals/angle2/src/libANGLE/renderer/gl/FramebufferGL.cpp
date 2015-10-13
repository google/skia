//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FramebufferGL.cpp: Implements the class methods for FramebufferGL.

#include "libANGLE/renderer/gl/FramebufferGL.h"

#include "common/debug.h"
#include "libANGLE/Data.h"
#include "libANGLE/State.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/WorkaroundsGL.h"

namespace rx
{

FramebufferGL::FramebufferGL(const gl::Framebuffer::Data &data,
                             const FunctionsGL *functions,
                             StateManagerGL *stateManager,
                             const WorkaroundsGL &workarounds,
                             bool isDefault)
    : FramebufferImpl(data),
      mFunctions(functions),
      mStateManager(stateManager),
      mWorkarounds(workarounds),
      mFramebufferID(0),
      mIsDefault(isDefault)
{
    if (!mIsDefault)
    {
        mFunctions->genFramebuffers(1, &mFramebufferID);
    }
}

FramebufferGL::FramebufferGL(GLuint id,
                             const gl::Framebuffer::Data &data,
                             const FunctionsGL *functions,
                             const WorkaroundsGL &workarounds,
                             StateManagerGL *stateManager)
    : FramebufferImpl(data),
      mFunctions(functions),
      mStateManager(stateManager),
      mWorkarounds(workarounds),
      mFramebufferID(id),
      mIsDefault(true)
{
}

FramebufferGL::~FramebufferGL()
{
    mStateManager->deleteFramebuffer(mFramebufferID);
    mFramebufferID = 0;
}

static void BindFramebufferAttachment(const FunctionsGL *functions, GLenum attachmentPoint,
                                      const gl::FramebufferAttachment *attachment)
{
    if (attachment)
    {
        if (attachment->type() == GL_TEXTURE)
        {
            const gl::Texture *texture = attachment->getTexture();
            const TextureGL *textureGL = GetImplAs<TextureGL>(texture);

            if (texture->getTarget() == GL_TEXTURE_2D)
            {
                functions->framebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D,
                                                textureGL->getTextureID(), attachment->mipLevel());
            }
            else if (texture->getTarget() == GL_TEXTURE_CUBE_MAP)
            {
                functions->framebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, attachment->cubeMapFace(),
                                                textureGL->getTextureID(), attachment->mipLevel());
            }
            else if (texture->getTarget() == GL_TEXTURE_2D_ARRAY || texture->getTarget() == GL_TEXTURE_3D)
            {
                functions->framebufferTextureLayer(GL_FRAMEBUFFER, attachmentPoint, textureGL->getTextureID(),
                                                   attachment->mipLevel(), attachment->layer());
            }
            else
            {
                UNREACHABLE();
            }
        }
        else if (attachment->type() == GL_RENDERBUFFER)
        {
            const gl::Renderbuffer *renderbuffer = attachment->getRenderbuffer();
            const RenderbufferGL *renderbufferGL = GetImplAs<RenderbufferGL>(renderbuffer);

            functions->framebufferRenderbuffer(GL_FRAMEBUFFER, attachmentPoint, GL_RENDERBUFFER,
                                               renderbufferGL->getRenderbufferID());
        }
        else
        {
            UNREACHABLE();
        }
    }
    else
    {
        // Unbind this attachment
        functions->framebufferTexture2D(GL_FRAMEBUFFER, attachmentPoint, GL_TEXTURE_2D, 0, 0);
    }
}

void FramebufferGL::onUpdateColorAttachment(size_t index)
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        BindFramebufferAttachment(mFunctions, GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(index),
                                  mData.getColorAttachment(static_cast<unsigned int>(index)));
    }
}

void FramebufferGL::onUpdateDepthAttachment()
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        BindFramebufferAttachment(mFunctions,
                                  GL_DEPTH_ATTACHMENT,
                                  mData.getDepthAttachment());
    }
}

void FramebufferGL::onUpdateStencilAttachment()
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        BindFramebufferAttachment(mFunctions,
                                  GL_STENCIL_ATTACHMENT,
                                  mData.getStencilAttachment());
    }
}

void FramebufferGL::onUpdateDepthStencilAttachment()
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        BindFramebufferAttachment(mFunctions,
                                  GL_DEPTH_STENCIL_ATTACHMENT,
                                  mData.getDepthStencilAttachment());
    }
}

void FramebufferGL::setDrawBuffers(size_t count, const GLenum *buffers)
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        mFunctions->drawBuffers(static_cast<GLsizei>(count), buffers);
    }
}

void FramebufferGL::setReadBuffer(GLenum buffer)
{
    if (!mIsDefault)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        mFunctions->readBuffer(buffer);
    }
}

gl::Error FramebufferGL::discard(size_t count, const GLenum *attachments)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error FramebufferGL::invalidate(size_t count, const GLenum *attachments)
{
    // Since this function is just a hint and not available until OpenGL 4.3, only call it if it is available.
    if (mFunctions->invalidateFramebuffer)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        mFunctions->invalidateFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(count), attachments);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::invalidateSub(size_t count, const GLenum *attachments, const gl::Rectangle &area)
{
    // Since this function is just a hint and not available until OpenGL 4.3, only call it if it is available.
    if (mFunctions->invalidateSubFramebuffer)
    {
        mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
        mFunctions->invalidateSubFramebuffer(GL_FRAMEBUFFER, static_cast<GLsizei>(count),
                                             attachments, area.x, area.y, area.width, area.height);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clear(const gl::Data &data, GLbitfield mask)
{
    syncClearState(mask);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctions->clear(mask);

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferfv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLfloat *values)
{
    syncClearBufferState(buffer, drawbuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctions->clearBufferfv(buffer, drawbuffer, values);

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferuiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLuint *values)
{
    syncClearBufferState(buffer, drawbuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctions->clearBufferuiv(buffer, drawbuffer, values);

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferiv(const gl::State &state, GLenum buffer, GLint drawbuffer, const GLint *values)
{
    syncClearBufferState(buffer, drawbuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctions->clearBufferiv(buffer, drawbuffer, values);

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::clearBufferfi(const gl::State &state, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    syncClearBufferState(buffer, drawbuffer);
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    mFunctions->clearBufferfi(buffer, drawbuffer, depth, stencil);

    return gl::Error(GL_NO_ERROR);
}

GLenum FramebufferGL::getImplementationColorReadFormat() const
{
    const gl::FramebufferAttachment *readAttachment = getData().getReadAttachment();
    GLenum internalFormat = readAttachment->getInternalFormat();
    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);
    return internalFormatInfo.format;
}

GLenum FramebufferGL::getImplementationColorReadType() const
{
    const gl::FramebufferAttachment *readAttachment = getData().getReadAttachment();
    GLenum internalFormat = readAttachment->getInternalFormat();
    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);
    return internalFormatInfo.type;
}

gl::Error FramebufferGL::readPixels(const gl::State &state, const gl::Rectangle &area, GLenum format, GLenum type, GLvoid *pixels) const
{
    // TODO: don't sync the pixel pack state here once the dirty bits contain the pixel pack buffer
    // binding
    const gl::PixelPackState &packState = state.getPackState();
    mStateManager->setPixelPackState(packState);

    mStateManager->bindFramebuffer(GL_READ_FRAMEBUFFER, mFramebufferID);
    mFunctions->readPixels(area.x, area.y, area.width, area.height, format, type, pixels);

    return gl::Error(GL_NO_ERROR);
}

gl::Error FramebufferGL::blit(const gl::State &state, const gl::Rectangle &sourceArea, const gl::Rectangle &destArea,
                              GLbitfield mask, GLenum filter, const gl::Framebuffer *sourceFramebuffer)
{
    const FramebufferGL *sourceFramebufferGL = GetImplAs<FramebufferGL>(sourceFramebuffer);

    mStateManager->bindFramebuffer(GL_READ_FRAMEBUFFER, sourceFramebufferGL->getFramebufferID());
    mStateManager->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mFramebufferID);

    mFunctions->blitFramebuffer(sourceArea.x, sourceArea.y, sourceArea.x + sourceArea.width, sourceArea.y + sourceArea.height,
                                destArea.x, destArea.y, destArea.x + destArea.width, destArea.y + destArea.height,
                                mask, filter);

    return gl::Error(GL_NO_ERROR);
}

GLenum FramebufferGL::checkStatus() const
{
    mStateManager->bindFramebuffer(GL_FRAMEBUFFER, mFramebufferID);
    return mFunctions->checkFramebufferStatus(GL_FRAMEBUFFER);
}

GLuint FramebufferGL::getFramebufferID() const
{
    return mFramebufferID;
}

void FramebufferGL::syncDrawState() const
{
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        // Enable SRGB blending for all framebuffers except the default framebuffer on Desktop
        // OpenGL.
        // When SRGB blending is enabled, only SRGB capable formats will use it but the default
        // framebuffer will always use it if it is enabled.
        // TODO(geofflang): Update this when the framebuffer binding dirty changes, when it exists.
        mStateManager->setFramebufferSRGBEnabled(!mIsDefault);
    }
}

void FramebufferGL::syncClearState(GLbitfield mask)
{
    if (mWorkarounds.doesSRGBClearsOnLinearFramebufferAttachments &&
        (mask & GL_COLOR_BUFFER_BIT) != 0 && !mIsDefault)
    {
        bool hasSRBAttachment = false;
        for (const auto &attachment : mData.getColorAttachments())
        {
            if (attachment.isAttached() && attachment.getColorEncoding() == GL_SRGB)
            {
                hasSRBAttachment = true;
                break;
            }
        }

        mStateManager->setFramebufferSRGBEnabled(hasSRBAttachment);
    }
    else
    {
        mStateManager->setFramebufferSRGBEnabled(!mIsDefault);
    }
}

void FramebufferGL::syncClearBufferState(GLenum buffer, GLint drawBuffer)
{
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        if (mWorkarounds.doesSRGBClearsOnLinearFramebufferAttachments && buffer == GL_COLOR &&
            !mIsDefault)
        {
            // If doing a clear on a color buffer, set SRGB blend enabled only if the color buffer
            // is an SRGB format.
            const auto &drawbufferState  = mData.getDrawBufferStates();
            const auto &colorAttachments = mData.getColorAttachments();

            const gl::FramebufferAttachment *attachment = nullptr;
            if (drawbufferState[drawBuffer] >= GL_COLOR_ATTACHMENT0 &&
                drawbufferState[drawBuffer] < GL_COLOR_ATTACHMENT0 + colorAttachments.size())
            {
                size_t attachmentIdx =
                    static_cast<size_t>(drawbufferState[drawBuffer] - GL_COLOR_ATTACHMENT0);
                attachment = &colorAttachments[attachmentIdx];
            }

            if (attachment != nullptr)
            {
                mStateManager->setFramebufferSRGBEnabled(attachment->getColorEncoding() == GL_SRGB);
            }
        }
        else
        {
            mStateManager->setFramebufferSRGBEnabled(!mIsDefault);
        }
    }
}
}
