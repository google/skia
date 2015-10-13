//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererD3D.cpp: Implementation of the base D3D Renderer.

#include "libANGLE/renderer/d3d/RendererD3D.h"

#include "common/MemoryBuffer.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/Display.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/State.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/BufferD3D.h"
#include "libANGLE/renderer/d3d/CompilerD3D.h"
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#include "libANGLE/renderer/d3d/IndexDataManager.h"
#include "libANGLE/renderer/d3d/ProgramD3D.h"
#include "libANGLE/renderer/d3d/SamplerD3D.h"

namespace rx
{

namespace
{
// If we request a scratch buffer requesting a smaller size this many times,
// release and recreate the scratch buffer. This ensures we don't have a
// degenerate case where we are stuck hogging memory.
const int ScratchMemoryBufferLifetime = 1000;

}  // anonymous namespace

const uintptr_t RendererD3D::DirtyPointer = std::numeric_limits<uintptr_t>::max();

RendererD3D::RendererD3D(egl::Display *display)
    : mDisplay(display),
      mDeviceLost(false),
      mAnnotator(nullptr),
      mScratchMemoryBufferResetCounter(0),
      mWorkaroundsInitialized(false)
{
}

RendererD3D::~RendererD3D()
{
    cleanup();
}

void RendererD3D::cleanup()
{
    mScratchMemoryBuffer.resize(0);
    for (auto &incompleteTexture : mIncompleteTextures)
    {
        incompleteTexture.second.set(NULL);
    }
    mIncompleteTextures.clear();

    if (mAnnotator != nullptr)
    {
        gl::UninitializeDebugAnnotations();
        SafeDelete(mAnnotator);
    }
}

CompilerImpl *RendererD3D::createCompiler()
{
    return new CompilerD3D(getRendererClass());
}

SamplerImpl *RendererD3D::createSampler()
{
    return new SamplerD3D();
}

gl::Error RendererD3D::drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count)
{
    return genericDrawArrays(data, mode, first, count, 0);
}

gl::Error RendererD3D::drawArraysInstanced(const gl::Data &data,
                                           GLenum mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei instanceCount)
{
    return genericDrawArrays(data, mode, first, count, instanceCount);
}

gl::Error RendererD3D::drawElements(const gl::Data &data,
                                    GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    const gl::IndexRange &indexRange)
{
    return genericDrawElements(data, mode, count, type, indices, 0, indexRange);
}

gl::Error RendererD3D::drawElementsInstanced(const gl::Data &data,
                                             GLenum mode,
                                             GLsizei count,
                                             GLenum type,
                                             const GLvoid *indices,
                                             GLsizei instances,
                                             const gl::IndexRange &indexRange)
{
    return genericDrawElements(data, mode, count, type, indices, instances, indexRange);
}

gl::Error RendererD3D::drawRangeElements(const gl::Data &data,
                                         GLenum mode,
                                         GLuint start,
                                         GLuint end,
                                         GLsizei count,
                                         GLenum type,
                                         const GLvoid *indices,
                                         const gl::IndexRange &indexRange)
{
    return genericDrawElements(data, mode, count, type, indices, 0, indexRange);
}

gl::Error RendererD3D::genericDrawElements(const gl::Data &data,
                                           GLenum mode,
                                           GLsizei count,
                                           GLenum type,
                                           const GLvoid *indices,
                                           GLsizei instances,
                                           const gl::IndexRange &indexRange)
{
    if (data.state->isPrimitiveRestartEnabled())
    {
        UNIMPLEMENTED();
        return gl::Error(GL_INVALID_OPERATION, "Primitive restart not implemented");
    }

    gl::Program *program = data.state->getProgram();
    ASSERT(program != nullptr);
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(program);
    bool usesPointSize     = programD3D->usesPointSize();

    programD3D->updateSamplerMapping();

    gl::Error error = generateSwizzles(data);
    if (error.isError())
    {
        return error;
    }

    if (!applyPrimitiveType(mode, count, usesPointSize))
    {
        return gl::Error(GL_NO_ERROR);
    }

    error = applyRenderTarget(data, mode, false);
    if (error.isError())
    {
        return error;
    }

    error = applyState(data, mode);
    if (error.isError())
    {
        return error;
    }

    gl::VertexArray *vao = data.state->getVertexArray();
    TranslatedIndexData indexInfo;
    indexInfo.indexRange = indexRange;

    SourceIndexData sourceIndexInfo;

    error = applyIndexBuffer(indices, vao->getElementArrayBuffer().get(), count, mode, type, &indexInfo, &sourceIndexInfo);
    if (error.isError())
    {
        return error;
    }

    applyTransformFeedbackBuffers(*data.state);
    // Transform feedback is not allowed for DrawElements, this error should have been caught at the API validation
    // layer.
    ASSERT(!data.state->isTransformFeedbackActiveUnpaused());

    size_t vertexCount = indexInfo.indexRange.vertexCount();
    error = applyVertexBuffer(*data.state, mode, static_cast<GLsizei>(indexInfo.indexRange.start),
                              static_cast<GLsizei>(vertexCount), instances, &sourceIndexInfo);
    if (error.isError())
    {
        return error;
    }

    error = applyShaders(data);
    if (error.isError())
    {
        return error;
    }

    error = applyTextures(data);
    if (error.isError())
    {
        return error;
    }

    error = programD3D->applyUniformBuffers(data);
    if (error.isError())
    {
        return error;
    }

    if (!skipDraw(data, mode))
    {
        error = drawElementsImpl(mode, count, type, indices, vao->getElementArrayBuffer().get(),
                                 indexInfo, instances, usesPointSize);
        if (error.isError())
        {
            return error;
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererD3D::genericDrawArrays(const gl::Data &data,
                                         GLenum mode,
                                         GLint first,
                                         GLsizei count,
                                         GLsizei instances)
{
    gl::Program *program = data.state->getProgram();
    ASSERT(program != nullptr);
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(program);
    bool usesPointSize     = programD3D->usesPointSize();

    programD3D->updateSamplerMapping();

    gl::Error error = generateSwizzles(data);
    if (error.isError())
    {
        return error;
    }

    if (!applyPrimitiveType(mode, count, usesPointSize))
    {
        return gl::Error(GL_NO_ERROR);
    }

    error = applyRenderTarget(data, mode, false);
    if (error.isError())
    {
        return error;
    }

    error = applyState(data, mode);
    if (error.isError())
    {
        return error;
    }

    applyTransformFeedbackBuffers(*data.state);

    error = applyVertexBuffer(*data.state, mode, first, count, instances, nullptr);
    if (error.isError())
    {
        return error;
    }

    error = applyShaders(data);
    if (error.isError())
    {
        return error;
    }

    error = applyTextures(data);
    if (error.isError())
    {
        return error;
    }

    error = programD3D->applyUniformBuffers(data);
    if (error.isError())
    {
        return error;
    }

    if (!skipDraw(data, mode))
    {
        error = drawArraysImpl(data, mode, count, instances, usesPointSize);
        if (error.isError())
        {
            return error;
        }

        if (data.state->isTransformFeedbackActiveUnpaused())
        {
            markTransformFeedbackUsage(data);
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererD3D::generateSwizzles(const gl::Data &data, gl::SamplerType type)
{
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(data.state->getProgram());

    unsigned int samplerRange = static_cast<unsigned int>(programD3D->getUsedSamplerRange(type));

    for (unsigned int i = 0; i < samplerRange; i++)
    {
        GLenum textureType = programD3D->getSamplerTextureType(type, i);
        GLint textureUnit = programD3D->getSamplerMapping(type, i, *data.caps);
        if (textureUnit != -1)
        {
            gl::Texture *texture = data.state->getSamplerTexture(textureUnit, textureType);
            ASSERT(texture);
            if (texture->getTextureState().swizzleRequired())
            {
                gl::Error error = generateSwizzle(texture);
                if (error.isError())
                {
                    return error;
                }
            }
        }
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererD3D::generateSwizzles(const gl::Data &data)
{
    gl::Error error = generateSwizzles(data, gl::SAMPLER_VERTEX);
    if (error.isError())
    {
        return error;
    }

    error = generateSwizzles(data, gl::SAMPLER_PIXEL);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

// Applies the render target surface, depth stencil surface, viewport rectangle and
// scissor rectangle to the renderer
gl::Error RendererD3D::applyRenderTarget(const gl::Data &data, GLenum drawMode, bool ignoreViewport)
{
    const gl::Framebuffer *framebufferObject = data.state->getDrawFramebuffer();
    ASSERT(framebufferObject && framebufferObject->checkStatus(data) == GL_FRAMEBUFFER_COMPLETE);

    gl::Error error = applyRenderTarget(framebufferObject);
    if (error.isError())
    {
        return error;
    }

    float nearZ = data.state->getNearPlane();
    float farZ = data.state->getFarPlane();
    setViewport(data.state->getViewport(), nearZ, farZ, drawMode,
                data.state->getRasterizerState().frontFace, ignoreViewport);

    setScissorRectangle(data.state->getScissor(), data.state->isScissorTestEnabled());

    return gl::Error(GL_NO_ERROR);
}

// Applies the fixed-function state (culling, depth test, alpha blending, stenciling, etc) to the Direct3D device
gl::Error RendererD3D::applyState(const gl::Data &data, GLenum drawMode)
{
    const gl::Framebuffer *framebufferObject = data.state->getDrawFramebuffer();
    int samples = framebufferObject->getSamples(data);

    gl::RasterizerState rasterizer = data.state->getRasterizerState();
    rasterizer.pointDrawMode = (drawMode == GL_POINTS);
    rasterizer.multiSample = (samples != 0);

    gl::Error error = setRasterizerState(rasterizer);
    if (error.isError())
    {
        return error;
    }

    unsigned int mask = 0;
    if (data.state->isSampleCoverageEnabled())
    {
        GLclampf coverageValue = data.state->getSampleCoverageValue();
        if (coverageValue != 0)
        {
            float threshold = 0.5f;

            for (int i = 0; i < samples; ++i)
            {
                mask <<= 1;

                if ((i + 1) * coverageValue >= threshold)
                {
                    threshold += 1.0f;
                    mask |= 1;
                }
            }
        }

        bool coverageInvert = data.state->getSampleCoverageInvert();
        if (coverageInvert)
        {
            mask = ~mask;
        }
    }
    else
    {
        mask = 0xFFFFFFFF;
    }
    error = setBlendState(framebufferObject, data.state->getBlendState(), data.state->getBlendColor(), mask);
    if (error.isError())
    {
        return error;
    }

    error = setDepthStencilState(data.state->getDepthStencilState(), data.state->getStencilRef(),
                                 data.state->getStencilBackRef(), rasterizer.frontFace == GL_CCW);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

// Applies the shaders and shader constants to the Direct3D device
gl::Error RendererD3D::applyShaders(const gl::Data &data)
{
    gl::Program *program = data.state->getProgram();
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(program);
    programD3D->updateCachedInputLayout(*data.state);

    const gl::Framebuffer *fbo = data.state->getDrawFramebuffer();

    gl::Error error = applyShaders(program, fbo, data.state->getRasterizerState().rasterizerDiscard, data.state->isTransformFeedbackActiveUnpaused());
    if (error.isError())
    {
        return error;
    }

    return programD3D->applyUniforms();
}

// For each Direct3D sampler of either the pixel or vertex stage,
// looks up the corresponding OpenGL texture image unit and texture type,
// and sets the texture and its addressing/filtering state (or NULL when inactive).
gl::Error RendererD3D::applyTextures(const gl::Data &data, gl::SamplerType shaderType,
                                     const FramebufferTextureArray &framebufferTextures, size_t framebufferTextureCount)
{
    ProgramD3D *programD3D = GetImplAs<ProgramD3D>(data.state->getProgram());

    unsigned int samplerRange = programD3D->getUsedSamplerRange(shaderType);
    for (unsigned int samplerIndex = 0; samplerIndex < samplerRange; samplerIndex++)
    {
        GLenum textureType = programD3D->getSamplerTextureType(shaderType, samplerIndex);
        GLint textureUnit = programD3D->getSamplerMapping(shaderType, samplerIndex, *data.caps);
        if (textureUnit != -1)
        {
            gl::Texture *texture = data.state->getSamplerTexture(textureUnit, textureType);
            ASSERT(texture);

            gl::Sampler *samplerObject = data.state->getSampler(textureUnit);

            const gl::SamplerState &samplerState =
                samplerObject ? samplerObject->getSamplerState() : texture->getSamplerState();

            // TODO: std::binary_search may become unavailable using older versions of GCC
            if (texture->isSamplerComplete(samplerState, data) &&
                !std::binary_search(framebufferTextures.begin(),
                                    framebufferTextures.begin() + framebufferTextureCount, texture))
            {
                gl::Error error = setSamplerState(shaderType, samplerIndex, texture, samplerState);
                if (error.isError())
                {
                    return error;
                }

                error = setTexture(shaderType, samplerIndex, texture);
                if (error.isError())
                {
                    return error;
                }
            }
            else
            {
                // Texture is not sampler complete or it is in use by the framebuffer.  Bind the incomplete texture.
                gl::Texture *incompleteTexture = getIncompleteTexture(textureType);
                gl::Error error = setTexture(shaderType, samplerIndex, incompleteTexture);
                if (error.isError())
                {
                    return error;
                }
            }
        }
        else
        {
            // No texture bound to this slot even though it is used by the shader, bind a NULL texture
            gl::Error error = setTexture(shaderType, samplerIndex, NULL);
            if (error.isError())
            {
                return error;
            }
        }
    }

    // Set all the remaining textures to NULL
    size_t samplerCount = (shaderType == gl::SAMPLER_PIXEL) ? data.caps->maxTextureImageUnits
                                                            : data.caps->maxVertexTextureImageUnits;
    clearTextures(shaderType, samplerRange, samplerCount);

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererD3D::applyTextures(const gl::Data &data)
{
    FramebufferTextureArray framebufferTextures;
    size_t framebufferSerialCount = getBoundFramebufferTextures(data, &framebufferTextures);

    gl::Error error = applyTextures(data, gl::SAMPLER_VERTEX, framebufferTextures, framebufferSerialCount);
    if (error.isError())
    {
        return error;
    }

    error = applyTextures(data, gl::SAMPLER_PIXEL, framebufferTextures, framebufferSerialCount);
    if (error.isError())
    {
        return error;
    }

    return gl::Error(GL_NO_ERROR);
}

bool RendererD3D::skipDraw(const gl::Data &data, GLenum drawMode)
{
    const gl::State &state = *data.state;

    if (drawMode == GL_POINTS)
    {
        bool usesPointSize = GetImplAs<ProgramD3D>(state.getProgram())->usesPointSize();

        // ProgramBinary assumes non-point rendering if gl_PointSize isn't written,
        // which affects varying interpolation. Since the value of gl_PointSize is
        // undefined when not written, just skip drawing to avoid unexpected results.
        if (!usesPointSize && !state.isTransformFeedbackActiveUnpaused())
        {
            // This is stictly speaking not an error, but developers should be
            // notified of risking undefined behavior.
            ERR("Point rendering without writing to gl_PointSize.");

            return true;
        }
    }
    else if (gl::IsTriangleMode(drawMode))
    {
        if (state.getRasterizerState().cullFace &&
            state.getRasterizerState().cullMode == GL_FRONT_AND_BACK)
        {
            return true;
        }
    }

    return false;
}

void RendererD3D::markTransformFeedbackUsage(const gl::Data &data)
{
    const gl::TransformFeedback *transformFeedback = data.state->getCurrentTransformFeedback();
    for (size_t i = 0; i < transformFeedback->getIndexedBufferCount(); i++)
    {
        const OffsetBindingPointer<gl::Buffer> &binding = transformFeedback->getIndexedBuffer(i);
        if (binding.get() != nullptr)
        {
            BufferD3D *bufferD3D = GetImplAs<BufferD3D>(binding.get());
            bufferD3D->markTransformFeedbackUsage();
        }
    }
}

size_t RendererD3D::getBoundFramebufferTextures(const gl::Data &data, FramebufferTextureArray *outTextureArray)
{
    size_t textureCount = 0;

    const gl::Framebuffer *drawFramebuffer = data.state->getDrawFramebuffer();
    for (size_t i = 0; i < drawFramebuffer->getNumColorBuffers(); i++)
    {
        const gl::FramebufferAttachment *attachment = drawFramebuffer->getColorbuffer(i);
        if (attachment && attachment->type() == GL_TEXTURE)
        {
            (*outTextureArray)[textureCount++] = attachment->getTexture();
        }
    }

    const gl::FramebufferAttachment *depthStencilAttachment = drawFramebuffer->getDepthOrStencilbuffer();
    if (depthStencilAttachment && depthStencilAttachment->type() == GL_TEXTURE)
    {
        (*outTextureArray)[textureCount++] = depthStencilAttachment->getTexture();
    }

    std::sort(outTextureArray->begin(), outTextureArray->begin() + textureCount);

    return textureCount;
}

gl::Texture *RendererD3D::getIncompleteTexture(GLenum type)
{
    if (mIncompleteTextures.find(type) == mIncompleteTextures.end())
    {
        const GLubyte color[] = { 0, 0, 0, 255 };
        const gl::Extents colorSize(1, 1, 1);
        const gl::PixelUnpackState unpack(1, 0);
        const gl::Box area(0, 0, 0, 1, 1, 1);

        // Skip the API layer to avoid needing to pass the Context and mess with dirty bits.
        gl::Texture *t =
            new gl::Texture(createTexture(type), std::numeric_limits<GLuint>::max(), type);
        t->setStorage(type, 1, GL_RGBA8, colorSize);

        if (type == GL_TEXTURE_CUBE_MAP)
        {
            for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++)
            {
                t->getImplementation()->setSubImage(face, 0, area, GL_RGBA8, GL_UNSIGNED_BYTE,
                                                    unpack, color);
            }
        }
        else
        {
            t->getImplementation()->setSubImage(type, 0, area, GL_RGBA8, GL_UNSIGNED_BYTE, unpack,
                                                color);
        }

        mIncompleteTextures[type].set(t);
    }

    return mIncompleteTextures[type].get();
}

bool RendererD3D::isDeviceLost() const
{
    return mDeviceLost;
}

void RendererD3D::notifyDeviceLost()
{
    mDeviceLost = true;
    mDisplay->notifyDeviceLost();
}

std::string RendererD3D::getVendorString() const
{
    LUID adapterLuid = { 0 };

    if (getLUID(&adapterLuid))
    {
        char adapterLuidString[64];
        sprintf_s(adapterLuidString, sizeof(adapterLuidString), "(adapter LUID: %08x%08x)", adapterLuid.HighPart, adapterLuid.LowPart);
        return std::string(adapterLuidString);
    }

    return std::string("");
}

gl::Error RendererD3D::getScratchMemoryBuffer(size_t requestedSize, MemoryBuffer **bufferOut)
{
    if (mScratchMemoryBuffer.size() == requestedSize)
    {
        mScratchMemoryBufferResetCounter = ScratchMemoryBufferLifetime;
        *bufferOut = &mScratchMemoryBuffer;
        return gl::Error(GL_NO_ERROR);
    }

    if (mScratchMemoryBuffer.size() > requestedSize)
    {
        mScratchMemoryBufferResetCounter--;
    }

    if (mScratchMemoryBufferResetCounter <= 0 || mScratchMemoryBuffer.size() < requestedSize)
    {
        mScratchMemoryBuffer.resize(0);
        if (!mScratchMemoryBuffer.resize(requestedSize))
        {
            return gl::Error(GL_OUT_OF_MEMORY, "Failed to allocate internal buffer.");
        }
        mScratchMemoryBufferResetCounter = ScratchMemoryBufferLifetime;
    }

    ASSERT(mScratchMemoryBuffer.size() >= requestedSize);

    *bufferOut = &mScratchMemoryBuffer;
    return gl::Error(GL_NO_ERROR);
}

void RendererD3D::insertEventMarker(GLsizei length, const char *marker)
{
    std::vector<wchar_t> wcstring (length + 1);
    size_t convertedChars = 0;
    errno_t err = mbstowcs_s(&convertedChars, wcstring.data(), length + 1, marker, _TRUNCATE);
    if (err == 0)
    {
        getAnnotator()->setMarker(wcstring.data());
    }
}

void RendererD3D::pushGroupMarker(GLsizei length, const char *marker)
{
    std::vector<wchar_t> wcstring(length + 1);
    size_t convertedChars = 0;
    errno_t err = mbstowcs_s(&convertedChars, wcstring.data(), length + 1, marker, _TRUNCATE);
    if (err == 0)
    {
        getAnnotator()->beginEvent(wcstring.data());
    }
}

void RendererD3D::popGroupMarker()
{
    getAnnotator()->endEvent();
}

void RendererD3D::initializeDebugAnnotator()
{
    createAnnotator();
    ASSERT(mAnnotator);
    gl::InitializeDebugAnnotations(mAnnotator);
}

gl::DebugAnnotator *RendererD3D::getAnnotator()
{
    ASSERT(mAnnotator);
    return mAnnotator;
}

}
