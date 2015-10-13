//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#include "libANGLE/renderer/gl/StateManagerGL.h"

#include "common/BitSetIterator.h"
#include "libANGLE/Data.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/SamplerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

namespace rx
{
StateManagerGL::StateManagerGL(const FunctionsGL *functions, const gl::Caps &rendererCaps)
    : mFunctions(functions),
      mProgram(0),
      mVAO(0),
      mVertexAttribCurrentValues(rendererCaps.maxVertexAttributes),
      mBuffers(),
      mTextureUnitIndex(0),
      mTextures(),
      mSamplers(rendererCaps.maxCombinedTextureImageUnits, 0),
      mUnpackAlignment(4),
      mUnpackRowLength(0),
      mUnpackSkipRows(0),
      mUnpackSkipPixels(0),
      mUnpackImageHeight(0),
      mUnpackSkipImages(0),
      mPackAlignment(4),
      mPackRowLength(0),
      mPackSkipRows(0),
      mPackSkipPixels(0),
      mFramebuffers(angle::FramebufferBindingSingletonMax, 0),
      mRenderbuffer(0),
      mScissorTestEnabled(false),
      mScissor(0, 0, 0, 0),
      mViewport(0, 0, 0, 0),
      mNear(0.0f),
      mFar(1.0f),
      mBlendEnabled(false),
      mBlendColor(0, 0, 0, 0),
      mSourceBlendRGB(GL_ONE),
      mDestBlendRGB(GL_ZERO),
      mSourceBlendAlpha(GL_ONE),
      mDestBlendAlpha(GL_ZERO),
      mBlendEquationRGB(GL_FUNC_ADD),
      mBlendEquationAlpha(GL_FUNC_ADD),
      mColorMaskRed(true),
      mColorMaskGreen(true),
      mColorMaskBlue(true),
      mColorMaskAlpha(true),
      mSampleAlphaToCoverageEnabled(false),
      mSampleCoverageEnabled(false),
      mSampleCoverageValue(1.0f),
      mSampleCoverageInvert(false),
      mDepthTestEnabled(false),
      mDepthFunc(GL_LESS),
      mDepthMask(true),
      mStencilTestEnabled(false),
      mStencilFrontFunc(GL_ALWAYS),
      mStencilFrontValueMask(static_cast<GLuint>(-1)),
      mStencilFrontStencilFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthFailOp(GL_KEEP),
      mStencilFrontStencilPassDepthPassOp(GL_KEEP),
      mStencilFrontWritemask(static_cast<GLuint>(-1)),
      mStencilBackFunc(GL_ALWAYS),
      mStencilBackValueMask(static_cast<GLuint>(-1)),
      mStencilBackStencilFailOp(GL_KEEP),
      mStencilBackStencilPassDepthFailOp(GL_KEEP),
      mStencilBackStencilPassDepthPassOp(GL_KEEP),
      mStencilBackWritemask(static_cast<GLuint>(-1)),
      mCullFaceEnabled(false),
      mCullFace(GL_BACK),
      mFrontFace(GL_CCW),
      mPolygonOffsetFillEnabled(false),
      mPolygonOffsetFactor(0.0f),
      mPolygonOffsetUnits(0.0f),
      mRasterizerDiscardEnabled(false),
      mLineWidth(1.0f),
      mPrimitiveRestartEnabled(false),
      mClearColor(0.0f, 0.0f, 0.0f, 0.0f),
      mClearDepth(1.0f),
      mClearStencil(0),
      mFramebufferSRGBEnabled(false),
      mTextureCubemapSeamlessEnabled(false),
      mLocalDirtyBits()
{
    ASSERT(mFunctions);

    mTextures[GL_TEXTURE_2D].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_CUBE_MAP].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_2D_ARRAY].resize(rendererCaps.maxCombinedTextureImageUnits);
    mTextures[GL_TEXTURE_3D].resize(rendererCaps.maxCombinedTextureImageUnits);

    // Initialize point sprite state for desktop GL
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        mFunctions->enable(GL_PROGRAM_POINT_SIZE);

        // GL_POINT_SPRITE was deprecated in the core profile. Point rasterization is always
        // performed
        // as though POINT_SPRITE were enabled.
        if ((mFunctions->profile & GL_CONTEXT_CORE_PROFILE_BIT) == 0)
        {
            mFunctions->enable(GL_POINT_SPRITE);
        }
    }
}

void StateManagerGL::deleteProgram(GLuint program)
{
    if (program != 0)
    {
        if (mProgram == program)
        {
            useProgram(0);
        }

        mFunctions->deleteProgram(program);
    }
}

void StateManagerGL::deleteVertexArray(GLuint vao)
{
    if (vao != 0)
    {
        if (mVAO == vao)
        {
            bindVertexArray(0, 0);
        }

        mFunctions->deleteVertexArrays(1, &vao);
    }
}

void StateManagerGL::deleteTexture(GLuint texture)
{
    if (texture != 0)
    {
        for (const auto &textureTypeIter : mTextures)
        {
            const std::vector<GLuint> &textureVector = textureTypeIter.second;
            for (size_t textureUnitIndex = 0; textureUnitIndex < textureVector.size(); textureUnitIndex++)
            {
                if (textureVector[textureUnitIndex] == texture)
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureTypeIter.first, 0);
                }
            }
        }

        mFunctions->deleteTextures(1, &texture);
    }
}

void StateManagerGL::deleteSampler(GLuint sampler)
{
    if (sampler != 0)
    {
        for (size_t unit = 0; unit < mSamplers.size(); unit++)
        {
            if (mSamplers[unit] == sampler)
            {
                bindSampler(unit, 0);
            }
        }

        mFunctions->deleteSamplers(1, &sampler);
    }
}

void StateManagerGL::deleteBuffer(GLuint buffer)
{
    if (buffer != 0)
    {
        for (const auto &bufferTypeIter : mBuffers)
        {
            if (bufferTypeIter.second == buffer)
            {
                bindBuffer(bufferTypeIter.first, 0);
            }
        }

        mFunctions->deleteBuffers(1, &buffer);
    }
}

void StateManagerGL::deleteFramebuffer(GLuint fbo)
{
    if (fbo != 0)
    {
        for (size_t binding = 0; binding < mFramebuffers.size(); ++binding)
        {
            if (mFramebuffers[binding] == fbo)
            {
                GLenum enumValue = angle::FramebufferBindingToEnum(
                    static_cast<angle::FramebufferBinding>(binding));
                bindFramebuffer(enumValue, 0);
            }
            mFunctions->deleteFramebuffers(1, &fbo);
        }
    }
}

void StateManagerGL::deleteRenderbuffer(GLuint rbo)
{
    if (rbo != 0)
    {
        if (mRenderbuffer == rbo)
        {
            bindRenderbuffer(GL_RENDERBUFFER, 0);
        }

        mFunctions->deleteRenderbuffers(1, &rbo);
    }
}

void StateManagerGL::useProgram(GLuint program)
{
    if (mProgram != program)
    {
        mProgram = program;
        mFunctions->useProgram(mProgram);
    }
}

void StateManagerGL::bindVertexArray(GLuint vao, GLuint elementArrayBuffer)
{
    if (mVAO != vao)
    {
        mVAO = vao;
        mBuffers[GL_ELEMENT_ARRAY_BUFFER] = elementArrayBuffer;
        mFunctions->bindVertexArray(vao);
    }
}

void StateManagerGL::bindBuffer(GLenum type, GLuint buffer)
{
    if (mBuffers[type] != buffer)
    {
        mBuffers[type] = buffer;
        mFunctions->bindBuffer(type, buffer);
    }
}

void StateManagerGL::activeTexture(size_t unit)
{
    if (mTextureUnitIndex != unit)
    {
        mTextureUnitIndex = unit;
        mFunctions->activeTexture(GL_TEXTURE0 + static_cast<GLenum>(mTextureUnitIndex));
    }
}

void StateManagerGL::bindTexture(GLenum type, GLuint texture)
{
    if (mTextures[type][mTextureUnitIndex] != texture)
    {
        mTextures[type][mTextureUnitIndex] = texture;
        mFunctions->bindTexture(type, texture);
    }
}

void StateManagerGL::bindSampler(size_t unit, GLuint sampler)
{
    if (mSamplers[unit] != sampler)
    {
        mSamplers[unit] = sampler;
        mFunctions->bindSampler(static_cast<GLuint>(unit), sampler);
    }
}

void StateManagerGL::setPixelUnpackState(const gl::PixelUnpackState &unpack)
{
    GLuint unpackBufferID          = 0;
    const gl::Buffer *unpackBuffer = unpack.pixelBuffer.get();
    if (unpackBuffer != nullptr)
    {
        unpackBufferID = GetImplAs<BufferGL>(unpackBuffer)->getBufferID();
    }
    setPixelUnpackState(unpack.alignment, unpack.rowLength, unpack.skipRows, unpack.skipPixels,
                        unpack.imageHeight, unpack.skipImages, unpackBufferID);
}

void StateManagerGL::setPixelUnpackState(GLint alignment,
                                         GLint rowLength,
                                         GLint skipRows,
                                         GLint skipPixels,
                                         GLint imageHeight,
                                         GLint skipImages,
                                         GLuint unpackBuffer)
{
    if (mUnpackAlignment != alignment)
    {
        mUnpackAlignment = alignment;
        mFunctions->pixelStorei(GL_UNPACK_ALIGNMENT, mUnpackAlignment);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ALIGNMENT);
    }

    if (mUnpackRowLength != rowLength)
    {
        mUnpackRowLength = rowLength;
        mFunctions->pixelStorei(GL_UNPACK_ROW_LENGTH, mUnpackRowLength);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH);
    }

    if (mUnpackSkipRows != skipRows)
    {
        mUnpackSkipRows = skipRows;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_ROWS, mUnpackSkipRows);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackSkipPixels != skipPixels)
    {
        mUnpackSkipPixels = skipPixels;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_PIXELS, mUnpackSkipPixels);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackImageHeight != imageHeight)
    {
        mUnpackImageHeight = imageHeight;
        mFunctions->pixelStorei(GL_UNPACK_IMAGE_HEIGHT, mUnpackImageHeight);

        // TODO: set dirty bit once one exists
    }

    if (mUnpackSkipImages != skipImages)
    {
        mUnpackSkipImages = skipImages;
        mFunctions->pixelStorei(GL_UNPACK_SKIP_IMAGES, mUnpackSkipImages);

        // TODO: set dirty bit once one exists
    }

    bindBuffer(GL_PIXEL_UNPACK_BUFFER, unpackBuffer);
}

void StateManagerGL::setPixelPackState(const gl::PixelPackState &pack)
{
    GLuint packBufferID          = 0;
    const gl::Buffer *packBuffer = pack.pixelBuffer.get();
    if (packBuffer != nullptr)
    {
        packBufferID = GetImplAs<BufferGL>(packBuffer)->getBufferID();
    }
    setPixelPackState(pack.alignment, pack.rowLength, pack.skipRows, pack.skipPixels, packBufferID);
}

void StateManagerGL::setPixelPackState(GLint alignment,
                                       GLint rowLength,
                                       GLint skipRows,
                                       GLint skipPixels,
                                       GLuint packBuffer)
{
    if (mPackAlignment != alignment)
    {
        mPackAlignment = alignment;
        mFunctions->pixelStorei(GL_PACK_ALIGNMENT, mPackAlignment);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_PACK_ALIGNMENT);
    }

    if (mPackRowLength != rowLength)
    {
        mPackRowLength = rowLength;
        mFunctions->pixelStorei(GL_PACK_ROW_LENGTH, mPackRowLength);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH);
    }

    if (mPackSkipRows != skipRows)
    {
        mPackSkipRows = skipRows;
        mFunctions->pixelStorei(GL_PACK_SKIP_ROWS, mPackSkipRows);

        // TODO: set dirty bit once one exists
    }

    if (mPackSkipPixels != skipPixels)
    {
        mPackSkipPixels = skipPixels;
        mFunctions->pixelStorei(GL_PACK_SKIP_PIXELS, mPackSkipPixels);

        // TODO: set dirty bit once one exists
    }

    bindBuffer(GL_PIXEL_PACK_BUFFER, packBuffer);
}

void StateManagerGL::bindFramebuffer(GLenum type, GLuint framebuffer)
{
    if (type == GL_FRAMEBUFFER)
    {
        if (mFramebuffers[angle::FramebufferBindingRead] != framebuffer ||
            mFramebuffers[angle::FramebufferBindingDraw] != framebuffer)
        {
            mFramebuffers[angle::FramebufferBindingRead] = framebuffer;
            mFramebuffers[angle::FramebufferBindingDraw] = framebuffer;
            mFunctions->bindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        }
    }
    else
    {
        angle::FramebufferBinding binding = angle::EnumToFramebufferBinding(type);

        if (mFramebuffers[binding] != framebuffer)
        {
            mFramebuffers[binding] = framebuffer;
            mFunctions->bindFramebuffer(type, framebuffer);
        }
    }
}

void StateManagerGL::bindRenderbuffer(GLenum type, GLuint renderbuffer)
{
    ASSERT(type == GL_RENDERBUFFER);
    if (mRenderbuffer != renderbuffer)
    {
        mRenderbuffer = renderbuffer;
        mFunctions->bindRenderbuffer(type, mRenderbuffer);
    }
}

gl::Error StateManagerGL::setDrawArraysState(const gl::Data &data,
                                             GLint first,
                                             GLsizei count,
                                             GLsizei instanceCount)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();

    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);

    gl::Error error = vaoGL->syncDrawArraysState(program->getActiveAttribLocationsMask(), first,
                                                 count, instanceCount);
    if (error.isError())
    {
        return error;
    }

    bindVertexArray(vaoGL->getVertexArrayID(), vaoGL->getAppliedElementArrayBufferID());

    return setGenericDrawState(data);
}

gl::Error StateManagerGL::setDrawElementsState(const gl::Data &data,
                                               GLsizei count,
                                               GLenum type,
                                               const GLvoid *indices,
                                               GLsizei instanceCount,
                                               const GLvoid **outIndices)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();

    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);

    gl::Error error =
        vaoGL->syncDrawElementsState(program->getActiveAttribLocationsMask(), count, type, indices,
                                     instanceCount, state.isPrimitiveRestartEnabled(), outIndices);
    if (error.isError())
    {
        return error;
    }

    bindVertexArray(vaoGL->getVertexArrayID(), vaoGL->getAppliedElementArrayBufferID());

    return setGenericDrawState(data);
}

gl::Error StateManagerGL::setGenericDrawState(const gl::Data &data)
{
    const gl::State &state = *data.state;

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);
    useProgram(programGL->getProgramID());

    const std::vector<SamplerBindingGL> &appliedSamplerUniforms = programGL->getAppliedSamplerUniforms();
    for (const SamplerBindingGL &samplerUniform : appliedSamplerUniforms)
    {
        GLenum textureType = samplerUniform.textureType;
        for (GLuint textureUnitIndex : samplerUniform.boundTextureUnits)
        {
            const gl::Texture *texture = state.getSamplerTexture(textureUnitIndex, textureType);
            if (texture != nullptr)
            {
                const TextureGL *textureGL = GetImplAs<TextureGL>(texture);

                if (mTextures[textureType][textureUnitIndex] != textureGL->getTextureID())
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureType, textureGL->getTextureID());
                }

                textureGL->syncState(textureUnitIndex, texture->getTextureState());
            }
            else
            {
                if (mTextures[textureType][textureUnitIndex] != 0)
                {
                    activeTexture(textureUnitIndex);
                    bindTexture(textureType, 0);
                }
            }

            const gl::Sampler *sampler = state.getSampler(textureUnitIndex);
            if (sampler != nullptr)
            {
                const SamplerGL *samplerGL = GetImplAs<SamplerGL>(sampler);
                samplerGL->syncState(sampler->getSamplerState());
                bindSampler(textureUnitIndex, samplerGL->getSamplerID());
            }
            else
            {
                bindSampler(textureUnitIndex, 0);
            }
        }
    }

    const gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
    const FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferGL->getFramebufferID());
    framebufferGL->syncDrawState();

    // Seamless cubemaps are required for ES3 and higher contexts.
    setTextureCubemapSeamlessEnabled(data.clientVersion >= 3);

    return gl::Error(GL_NO_ERROR);
}

void StateManagerGL::setAttributeCurrentData(size_t index,
                                             const gl::VertexAttribCurrentValueData &data)
{
    if (mVertexAttribCurrentValues[index] != data)
    {
        mVertexAttribCurrentValues[index] = data;
        switch (mVertexAttribCurrentValues[index].Type)
        {
            case GL_FLOAT:
                mFunctions->vertexAttrib4fv(static_cast<GLuint>(index),
                                            mVertexAttribCurrentValues[index].FloatValues);
                break;
            case GL_INT:
                mFunctions->vertexAttrib4iv(static_cast<GLuint>(index),
                                            mVertexAttribCurrentValues[index].IntValues);
                break;
            case GL_UNSIGNED_INT:
                mFunctions->vertexAttrib4uiv(static_cast<GLuint>(index),
                                             mVertexAttribCurrentValues[index].UnsignedIntValues);
                break;
          default: UNREACHABLE();
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CURRENT_VALUE_0 + index);
    }
}

void StateManagerGL::setScissorTestEnabled(bool enabled)
{
    if (mScissorTestEnabled != enabled)
    {
        mScissorTestEnabled = enabled;
        if (mScissorTestEnabled)
        {
            mFunctions->enable(GL_SCISSOR_TEST);
        }
        else
        {
            mFunctions->disable(GL_SCISSOR_TEST);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_SCISSOR_TEST_ENABLED);
    }
}

void StateManagerGL::setScissor(const gl::Rectangle &scissor)
{
    if (scissor != mScissor)
    {
        mScissor = scissor;
        mFunctions->scissor(mScissor.x, mScissor.y, mScissor.width, mScissor.height);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_SCISSOR);
    }
}

void StateManagerGL::setViewport(const gl::Rectangle &viewport)
{
    if (viewport != mViewport)
    {
        mViewport = viewport;
        mFunctions->viewport(mViewport.x, mViewport.y, mViewport.width, mViewport.height);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_VIEWPORT);
    }
}

void StateManagerGL::setDepthRange(float near, float far)
{
    if (mNear != near || mFar != far)
    {
        mNear = near;
        mFar = far;

        // The glDepthRangef function isn't available until OpenGL 4.1.  Prefer it when it is
        // available because OpenGL ES only works in floats.
        if (mFunctions->depthRangef)
        {
            mFunctions->depthRangef(mNear, mFar);
        }
        else
        {
            ASSERT(mFunctions->depthRange);
            mFunctions->depthRange(mNear, mFar);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_RANGE);
    }
}

void StateManagerGL::setBlendEnabled(bool enabled)
{
    if (mBlendEnabled != enabled)
    {
        mBlendEnabled = enabled;
        if (mBlendEnabled)
        {
            mFunctions->enable(GL_BLEND);
        }
        else
        {
            mFunctions->disable(GL_BLEND);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_ENABLED);
    }
}

void StateManagerGL::setBlendColor(const gl::ColorF &blendColor)
{
    if (mBlendColor != blendColor)
    {
        mBlendColor = blendColor;
        mFunctions->blendColor(mBlendColor.red, mBlendColor.green, mBlendColor.blue, mBlendColor.alpha);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_COLOR);
    }
}

void StateManagerGL::setBlendFuncs(GLenum sourceBlendRGB,
                                   GLenum destBlendRGB,
                                   GLenum sourceBlendAlpha,
                                   GLenum destBlendAlpha)
{
    if (mSourceBlendRGB != sourceBlendRGB || mDestBlendRGB != destBlendRGB ||
        mSourceBlendAlpha != sourceBlendAlpha || mDestBlendAlpha != destBlendAlpha)
    {
        mSourceBlendRGB = sourceBlendRGB;
        mDestBlendRGB = destBlendRGB;
        mSourceBlendAlpha = sourceBlendAlpha;
        mDestBlendAlpha = destBlendAlpha;

        mFunctions->blendFuncSeparate(mSourceBlendRGB, mDestBlendRGB, mSourceBlendAlpha, mDestBlendAlpha);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_FUNCS);
    }
}

void StateManagerGL::setBlendEquations(GLenum blendEquationRGB, GLenum blendEquationAlpha)
{
    if (mBlendEquationRGB != blendEquationRGB || mBlendEquationAlpha != blendEquationAlpha)
    {
        mBlendEquationRGB = blendEquationRGB;
        mBlendEquationAlpha = blendEquationAlpha;

        mFunctions->blendEquationSeparate(mBlendEquationRGB, mBlendEquationAlpha);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_BLEND_EQUATIONS);
    }
}

void StateManagerGL::setColorMask(bool red, bool green, bool blue, bool alpha)
{
    if (mColorMaskRed != red || mColorMaskGreen != green || mColorMaskBlue != blue || mColorMaskAlpha != alpha)
    {
        mColorMaskRed = red;
        mColorMaskGreen = green;
        mColorMaskBlue = blue;
        mColorMaskAlpha = alpha;
        mFunctions->colorMask(mColorMaskRed, mColorMaskGreen, mColorMaskBlue, mColorMaskAlpha);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_COLOR_MASK);
    }
}

void StateManagerGL::setSampleAlphaToCoverageEnabled(bool enabled)
{
    if (mSampleAlphaToCoverageEnabled != enabled)
    {
        mSampleAlphaToCoverageEnabled = enabled;
        if (mSampleAlphaToCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_ALPHA_TO_COVERAGE);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED);
    }
}

void StateManagerGL::setSampleCoverageEnabled(bool enabled)
{
    if (mSampleCoverageEnabled != enabled)
    {
        mSampleCoverageEnabled = enabled;
        if (mSampleCoverageEnabled)
        {
            mFunctions->enable(GL_SAMPLE_COVERAGE);
        }
        else
        {
            mFunctions->disable(GL_SAMPLE_COVERAGE);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED);
    }
}

void StateManagerGL::setSampleCoverage(float value, bool invert)
{
    if (mSampleCoverageValue != value || mSampleCoverageInvert != invert)
    {
        mSampleCoverageValue = value;
        mSampleCoverageInvert = invert;
        mFunctions->sampleCoverage(mSampleCoverageValue, mSampleCoverageInvert);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_SAMPLE_COVERAGE);
    }
}

void StateManagerGL::setDepthTestEnabled(bool enabled)
{
    if (mDepthTestEnabled != enabled)
    {
        mDepthTestEnabled = enabled;
        if (mDepthTestEnabled)
        {
            mFunctions->enable(GL_DEPTH_TEST);
        }
        else
        {
            mFunctions->disable(GL_DEPTH_TEST);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED);
    }
}

void StateManagerGL::setDepthFunc(GLenum depthFunc)
{
    if (mDepthFunc != depthFunc)
    {
        mDepthFunc = depthFunc;
        mFunctions->depthFunc(mDepthFunc);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_FUNC);
    }
}

void StateManagerGL::setDepthMask(bool mask)
{
    if (mDepthMask != mask)
    {
        mDepthMask = mask;
        mFunctions->depthMask(mDepthMask);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_DEPTH_MASK);
    }
}

void StateManagerGL::setStencilTestEnabled(bool enabled)
{
    if (mStencilTestEnabled != enabled)
    {
        mStencilTestEnabled = enabled;
        if (mStencilTestEnabled)
        {
            mFunctions->enable(GL_STENCIL_TEST);
        }
        else
        {
            mFunctions->disable(GL_STENCIL_TEST);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED);
    }
}

void StateManagerGL::setStencilFrontWritemask(GLuint mask)
{
    if (mStencilFrontWritemask != mask)
    {
        mStencilFrontWritemask = mask;
        mFunctions->stencilMaskSeparate(GL_FRONT, mStencilFrontWritemask);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT);
    }
}

void StateManagerGL::setStencilBackWritemask(GLuint mask)
{
    if (mStencilBackWritemask != mask)
    {
        mStencilBackWritemask = mask;
        mFunctions->stencilMaskSeparate(GL_BACK, mStencilBackWritemask);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK);
    }
}

void StateManagerGL::setStencilFrontFuncs(GLenum func, GLint ref, GLuint mask)
{
    if (mStencilFrontFunc != func || mStencilFrontRef != ref || mStencilFrontValueMask != mask)
    {
        mStencilFrontFunc = func;
        mStencilFrontRef = ref;
        mStencilFrontValueMask = mask;
        mFunctions->stencilFuncSeparate(GL_FRONT, mStencilFrontFunc, mStencilFrontRef, mStencilFrontValueMask);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT);
    }
}

void StateManagerGL::setStencilBackFuncs(GLenum func, GLint ref, GLuint mask)
{
    if (mStencilBackFunc != func || mStencilBackRef != ref || mStencilBackValueMask != mask)
    {
        mStencilBackFunc = func;
        mStencilBackRef = ref;
        mStencilBackValueMask = mask;
        mFunctions->stencilFuncSeparate(GL_BACK, mStencilBackFunc, mStencilBackRef, mStencilBackValueMask);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK);
    }
}

void StateManagerGL::setStencilFrontOps(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    if (mStencilFrontStencilFailOp != sfail || mStencilFrontStencilPassDepthFailOp != dpfail || mStencilFrontStencilPassDepthPassOp != dppass)
    {
        mStencilFrontStencilFailOp = sfail;
        mStencilFrontStencilPassDepthFailOp = dpfail;
        mStencilFrontStencilPassDepthPassOp = dppass;
        mFunctions->stencilOpSeparate(GL_FRONT, mStencilFrontStencilFailOp, mStencilFrontStencilPassDepthFailOp, mStencilFrontStencilPassDepthPassOp);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_OPS_FRONT);
    }
}

void StateManagerGL::setStencilBackOps(GLenum sfail, GLenum dpfail, GLenum dppass)
{
    if (mStencilBackStencilFailOp != sfail || mStencilBackStencilPassDepthFailOp != dpfail || mStencilBackStencilPassDepthPassOp != dppass)
    {
        mStencilBackStencilFailOp = sfail;
        mStencilBackStencilPassDepthFailOp = dpfail;
        mStencilBackStencilPassDepthPassOp = dppass;
        mFunctions->stencilOpSeparate(GL_BACK, mStencilBackStencilFailOp, mStencilBackStencilPassDepthFailOp, mStencilBackStencilPassDepthPassOp);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_STENCIL_OPS_BACK);
    }
}

void StateManagerGL::setCullFaceEnabled(bool enabled)
{
    if (mCullFaceEnabled != enabled)
    {
        mCullFaceEnabled = enabled;
        if (mCullFaceEnabled)
        {
            mFunctions->enable(GL_CULL_FACE);
        }
        else
        {
            mFunctions->disable(GL_CULL_FACE);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CULL_FACE_ENABLED);
    }
}

void StateManagerGL::setCullFace(GLenum cullFace)
{
    if (mCullFace != cullFace)
    {
        mCullFace = cullFace;
        mFunctions->cullFace(mCullFace);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CULL_FACE);
    }
}

void StateManagerGL::setFrontFace(GLenum frontFace)
{
    if (mFrontFace != frontFace)
    {
        mFrontFace = frontFace;
        mFunctions->frontFace(mFrontFace);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_FRONT_FACE);
    }
}

void StateManagerGL::setPolygonOffsetFillEnabled(bool enabled)
{
    if (mPolygonOffsetFillEnabled != enabled)
    {
        mPolygonOffsetFillEnabled = enabled;
        if (mPolygonOffsetFillEnabled)
        {
            mFunctions->enable(GL_POLYGON_OFFSET_FILL);
        }
        else
        {
            mFunctions->disable(GL_POLYGON_OFFSET_FILL);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED);
    }
}

void StateManagerGL::setPolygonOffset(float factor, float units)
{
    if (mPolygonOffsetFactor != factor || mPolygonOffsetUnits != units)
    {
        mPolygonOffsetFactor = factor;
        mPolygonOffsetUnits = units;
        mFunctions->polygonOffset(mPolygonOffsetFactor, mPolygonOffsetUnits);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_POLYGON_OFFSET);
    }
}

void StateManagerGL::setRasterizerDiscardEnabled(bool enabled)
{
    if (mRasterizerDiscardEnabled != enabled)
    {
        mRasterizerDiscardEnabled = enabled;
        if (mRasterizerDiscardEnabled)
        {
            mFunctions->enable(GL_RASTERIZER_DISCARD);
        }
        else
        {
            mFunctions->disable(GL_RASTERIZER_DISCARD);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED);
    }
}

void StateManagerGL::setLineWidth(float width)
{
    if (mLineWidth != width)
    {
        mLineWidth = width;
        mFunctions->lineWidth(mLineWidth);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_LINE_WIDTH);
    }
}

void StateManagerGL::setPrimitiveRestartEnabled(bool enabled)
{
    if (mPrimitiveRestartEnabled != enabled)
    {
        mPrimitiveRestartEnabled = enabled;

        if (mPrimitiveRestartEnabled)
        {
            mFunctions->enable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }
        else
        {
            mFunctions->disable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED);
    }
}

void StateManagerGL::setClearDepth(float clearDepth)
{
    if (mClearDepth != clearDepth)
    {
        mClearDepth = clearDepth;

        // The glClearDepthf function isn't available until OpenGL 4.1.  Prefer it when it is
        // available because OpenGL ES only works in floats.
        if (mFunctions->clearDepthf)
        {
            mFunctions->clearDepthf(mClearDepth);
        }
        else
        {
            ASSERT(mFunctions->clearDepth);
            mFunctions->clearDepth(mClearDepth);
        }

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_DEPTH);
    }
}

void StateManagerGL::setClearColor(const gl::ColorF &clearColor)
{
    if (mClearColor != clearColor)
    {
        mClearColor = clearColor;
        mFunctions->clearColor(mClearColor.red, mClearColor.green, mClearColor.blue, mClearColor.alpha);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_COLOR);
    }
}

void StateManagerGL::setClearStencil(GLint clearStencil)
{
    if (mClearStencil != clearStencil)
    {
        mClearStencil = clearStencil;
        mFunctions->clearStencil(mClearStencil);

        mLocalDirtyBits.set(gl::State::DIRTY_BIT_CLEAR_STENCIL);
    }
}

void StateManagerGL::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    // TODO(jmadill): Investigate only syncing vertex state for active attributes
    for (unsigned int dirtyBit : angle::IterateBitSet(dirtyBits | mLocalDirtyBits))
    {
        switch (dirtyBit)
        {
            case gl::State::DIRTY_BIT_SCISSOR_TEST_ENABLED:
                setScissorTestEnabled(state.isScissorTestEnabled());
                break;
            case gl::State::DIRTY_BIT_SCISSOR:
                setScissor(state.getScissor());
                break;
            case gl::State::DIRTY_BIT_VIEWPORT:
                setViewport(state.getViewport());
                break;
            case gl::State::DIRTY_BIT_DEPTH_RANGE:
                setDepthRange(state.getNearPlane(), state.getFarPlane());
                break;
            case gl::State::DIRTY_BIT_BLEND_ENABLED:
                setBlendEnabled(state.isBlendEnabled());
                break;
            case gl::State::DIRTY_BIT_BLEND_COLOR:
                setBlendColor(state.getBlendColor());
                break;
            case gl::State::DIRTY_BIT_BLEND_FUNCS:
            {
                const auto &blendState = state.getBlendState();
                setBlendFuncs(blendState.sourceBlendRGB, blendState.destBlendRGB,
                              blendState.sourceBlendAlpha, blendState.destBlendAlpha);
                break;
            }
            case gl::State::DIRTY_BIT_BLEND_EQUATIONS:
            {
                const auto &blendState = state.getBlendState();
                setBlendEquations(blendState.blendEquationRGB, blendState.blendEquationAlpha);
                break;
            }
            case gl::State::DIRTY_BIT_COLOR_MASK:
            {
                const auto &blendState = state.getBlendState();
                setColorMask(blendState.colorMaskRed, blendState.colorMaskGreen,
                             blendState.colorMaskBlue, blendState.colorMaskAlpha);
                break;
            }
            case gl::State::DIRTY_BIT_SAMPLE_ALPHA_TO_COVERAGE_ENABLED:
                setSampleAlphaToCoverageEnabled(state.isSampleAlphaToCoverageEnabled());
                break;
            case gl::State::DIRTY_BIT_SAMPLE_COVERAGE_ENABLED:
                setSampleCoverageEnabled(state.isSampleCoverageEnabled());
                break;
            case gl::State::DIRTY_BIT_SAMPLE_COVERAGE:
                setSampleCoverage(state.getSampleCoverageValue(), state.getSampleCoverageInvert());
                break;
            case gl::State::DIRTY_BIT_DEPTH_TEST_ENABLED:
                setDepthTestEnabled(state.isDepthTestEnabled());
                break;
            case gl::State::DIRTY_BIT_DEPTH_FUNC:
                setDepthFunc(state.getDepthStencilState().depthFunc);
                break;
            case gl::State::DIRTY_BIT_DEPTH_MASK:
                setDepthMask(state.getDepthStencilState().depthMask);
                break;
            case gl::State::DIRTY_BIT_STENCIL_TEST_ENABLED:
                setStencilTestEnabled(state.isStencilTestEnabled());
                break;
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontFuncs(depthStencilState.stencilFunc, state.getStencilRef(),
                                     depthStencilState.stencilMask);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_FUNCS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackFuncs(depthStencilState.stencilBackFunc, state.getStencilBackRef(),
                                    depthStencilState.stencilBackMask);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_OPS_FRONT:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilFrontOps(depthStencilState.stencilFail,
                                   depthStencilState.stencilPassDepthFail,
                                   depthStencilState.stencilPassDepthPass);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_OPS_BACK:
            {
                const auto &depthStencilState = state.getDepthStencilState();
                setStencilBackOps(depthStencilState.stencilBackFail,
                                  depthStencilState.stencilBackPassDepthFail,
                                  depthStencilState.stencilBackPassDepthPass);
                break;
            }
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_FRONT:
                setStencilFrontWritemask(state.getDepthStencilState().stencilWritemask);
                break;
            case gl::State::DIRTY_BIT_STENCIL_WRITEMASK_BACK:
                setStencilBackWritemask(state.getDepthStencilState().stencilBackWritemask);
                break;
            case gl::State::DIRTY_BIT_CULL_FACE_ENABLED:
                setCullFaceEnabled(state.isCullFaceEnabled());
                break;
            case gl::State::DIRTY_BIT_CULL_FACE:
                setCullFace(state.getRasterizerState().cullMode);
                break;
            case gl::State::DIRTY_BIT_FRONT_FACE:
                setFrontFace(state.getRasterizerState().frontFace);
                break;
            case gl::State::DIRTY_BIT_POLYGON_OFFSET_FILL_ENABLED:
                setPolygonOffsetFillEnabled(state.isPolygonOffsetFillEnabled());
                break;
            case gl::State::DIRTY_BIT_POLYGON_OFFSET:
            {
                const auto &rasterizerState = state.getRasterizerState();
                setPolygonOffset(rasterizerState.polygonOffsetFactor,
                                 rasterizerState.polygonOffsetUnits);
                break;
            }
            case gl::State::DIRTY_BIT_RASTERIZER_DISCARD_ENABLED:
                setRasterizerDiscardEnabled(state.isRasterizerDiscardEnabled());
                break;
            case gl::State::DIRTY_BIT_LINE_WIDTH:
                setLineWidth(state.getLineWidth());
                break;
            case gl::State::DIRTY_BIT_PRIMITIVE_RESTART_ENABLED:
                setPrimitiveRestartEnabled(state.isPrimitiveRestartEnabled());
                break;
            case gl::State::DIRTY_BIT_CLEAR_COLOR:
                setClearColor(state.getColorClearValue());
                break;
            case gl::State::DIRTY_BIT_CLEAR_DEPTH:
                setClearDepth(state.getDepthClearValue());
                break;
            case gl::State::DIRTY_BIT_CLEAR_STENCIL:
                setClearStencil(state.getStencilClearValue());
                break;
            case gl::State::DIRTY_BIT_UNPACK_ALIGNMENT:
                // TODO(jmadill): split this
                setPixelUnpackState(state.getUnpackState());
                break;
            case gl::State::DIRTY_BIT_UNPACK_ROW_LENGTH:
                // TODO(jmadill): split this
                setPixelUnpackState(state.getUnpackState());
                break;
            case gl::State::DIRTY_BIT_PACK_ALIGNMENT:
                // TODO(jmadill): split this
                setPixelPackState(state.getPackState());
                break;
            case gl::State::DIRTY_BIT_PACK_REVERSE_ROW_ORDER:
                // TODO(jmadill): split this
                setPixelPackState(state.getPackState());
                break;
            case gl::State::DIRTY_BIT_DITHER_ENABLED:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_GENERATE_MIPMAP_HINT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_SHADER_DERIVATIVE_HINT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_READ_FRAMEBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_READ_FRAMEBUFFER_OBJECT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_DRAW_FRAMEBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_DRAW_FRAMEBUFFER_OBJECT:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_RENDERBUFFER_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_VERTEX_ARRAY_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_VERTEX_ARRAY_OBJECT:
                state.getVertexArray()->syncImplState();
                break;
            case gl::State::DIRTY_BIT_PROGRAM_BINDING:
                // TODO(jmadill): implement this
                break;
            case gl::State::DIRTY_BIT_PROGRAM_OBJECT:
                // TODO(jmadill): implement this
                break;
            default:
            {
                ASSERT(dirtyBit >= gl::State::DIRTY_BIT_CURRENT_VALUE_0 &&
                       dirtyBit < gl::State::DIRTY_BIT_CURRENT_VALUE_MAX);
                size_t attribIndex =
                    static_cast<size_t>(dirtyBit) - gl::State::DIRTY_BIT_CURRENT_VALUE_0;
                setAttributeCurrentData(attribIndex, state.getVertexAttribCurrentValue(
                                                         static_cast<unsigned int>(attribIndex)));
                break;
            }
        }

        mLocalDirtyBits.reset();
    }
}

void StateManagerGL::setFramebufferSRGBEnabled(bool enabled)
{
    if (mFramebufferSRGBEnabled != enabled)
    {
        mFramebufferSRGBEnabled = enabled;
        if (mFramebufferSRGBEnabled)
        {
            mFunctions->enable(GL_FRAMEBUFFER_SRGB);
        }
        else
        {
            mFunctions->disable(GL_FRAMEBUFFER_SRGB);
        }
    }
}

void StateManagerGL::setTextureCubemapSeamlessEnabled(bool enabled)
{
    if (mTextureCubemapSeamlessEnabled != enabled)
    {
        mTextureCubemapSeamlessEnabled = enabled;
        if (mTextureCubemapSeamlessEnabled)
        {
            mFunctions->enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
        else
        {
            mFunctions->disable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        }
    }
}

}
