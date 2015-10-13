//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.cpp: Implements the class methods for RendererGL.

#include "libANGLE/renderer/gl/RendererGL.h"

#include <EGL/eglext.h>

#include "common/debug.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/Data.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/BlitGL.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/CompilerGL.h"
#include "libANGLE/renderer/gl/FenceNVGL.h"
#include "libANGLE/renderer/gl/FenceSyncGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/SamplerGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/TransformFeedbackGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

#ifndef NDEBUG
static void INTERNAL_GL_APIENTRY LogGLDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                                                   const GLchar *message, const void *userParam)
{
    std::string sourceText;
    switch (source)
    {
      case GL_DEBUG_SOURCE_API:             sourceText = "OpenGL";          break;
      case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sourceText = "Windows";         break;
      case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceText = "Shader Compiler"; break;
      case GL_DEBUG_SOURCE_THIRD_PARTY:     sourceText = "Third Party";     break;
      case GL_DEBUG_SOURCE_APPLICATION:     sourceText = "Application";     break;
      case GL_DEBUG_SOURCE_OTHER:           sourceText = "Other";           break;
      default:                              sourceText = "UNKNOWN";         break;
    }

    std::string typeText;
    switch (type)
    {
      case GL_DEBUG_TYPE_ERROR:               typeText = "Error";               break;
      case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeText = "Deprecated behavior"; break;
      case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeText = "Undefined behavior";  break;
      case GL_DEBUG_TYPE_PORTABILITY:         typeText = "Portability";         break;
      case GL_DEBUG_TYPE_PERFORMANCE:         typeText = "Performance";         break;
      case GL_DEBUG_TYPE_OTHER:               typeText = "Other";               break;
      case GL_DEBUG_TYPE_MARKER:              typeText = "Marker";              break;
      default:                                typeText = "UNKNOWN";             break;
    }

    std::string severityText;
    switch (severity)
    {
      case GL_DEBUG_SEVERITY_HIGH:         severityText = "High";         break;
      case GL_DEBUG_SEVERITY_MEDIUM:       severityText = "Medium";       break;
      case GL_DEBUG_SEVERITY_LOW:          severityText = "Low";          break;
      case GL_DEBUG_SEVERITY_NOTIFICATION: severityText = "Notification"; break;
      default:                             severityText = "UNKNOWN";      break;
    }

    ERR("\n\tSource: %s\n\tType: %s\n\tID: %d\n\tSeverity: %s\n\tMessage: %s", sourceText.c_str(), typeText.c_str(), id,
        severityText.c_str(), message);
}
#endif

namespace rx
{

RendererGL::RendererGL(const FunctionsGL *functions, const egl::AttributeMap &attribMap)
    : Renderer(),
      mMaxSupportedESVersion(0, 0),
      mFunctions(functions),
      mStateManager(nullptr),
      mBlitter(nullptr),
      mSkipDrawCalls(false)
{
    ASSERT(mFunctions);
    mStateManager = new StateManagerGL(mFunctions, getRendererCaps());
    nativegl_gl::GenerateWorkarounds(mFunctions, &mWorkarounds);
    mBlitter = new BlitGL(functions, mWorkarounds, mStateManager);

#ifndef NDEBUG
    if (mFunctions->debugMessageControl && mFunctions->debugMessageCallback)
    {
        mFunctions->enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_FALSE);
        mFunctions->debugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
        mFunctions->debugMessageCallback(&LogGLDebugMessage, nullptr);
    }
#endif

    EGLint deviceType = attribMap.get(EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_NONE);
    if (deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE)
    {
        mSkipDrawCalls = true;
    }
}

RendererGL::~RendererGL()
{
    SafeDelete(mStateManager);
    SafeDelete(mBlitter);
}

gl::Error RendererGL::flush()
{
    mFunctions->flush();
    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::finish()
{
    mFunctions->finish();
    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count)
{
    gl::Error error = mStateManager->setDrawArraysState(data, first, count, 0);
    if (error.isError())
    {
        return error;
    }

    if (!mSkipDrawCalls)
    {
        mFunctions->drawArrays(mode, first, count);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawArraysInstanced(const gl::Data &data,
                                          GLenum mode,
                                          GLint first,
                                          GLsizei count,
                                          GLsizei instanceCount)
{
    gl::Error error = mStateManager->setDrawArraysState(data, first, count, instanceCount);
    if (error.isError())
    {
        return error;
    }

    if (!mSkipDrawCalls)
    {
        mFunctions->drawArraysInstanced(mode, first, count, instanceCount);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawElements(const gl::Data &data,
                                   GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   const gl::IndexRange &indexRange)
{
    const GLvoid *drawIndexPointer = nullptr;
    gl::Error error =
        mStateManager->setDrawElementsState(data, count, type, indices, 0, &drawIndexPointer);
    if (error.isError())
    {
        return error;
    }

    if (!mSkipDrawCalls)
    {
        mFunctions->drawElements(mode, count, type, drawIndexPointer);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawElementsInstanced(const gl::Data &data,
                                            GLenum mode,
                                            GLsizei count,
                                            GLenum type,
                                            const GLvoid *indices,
                                            GLsizei instances,
                                            const gl::IndexRange &indexRange)
{
    const GLvoid *drawIndexPointer = nullptr;
    gl::Error error = mStateManager->setDrawElementsState(data, count, type, indices, instances,
                                                          &drawIndexPointer);
    if (error.isError())
    {
        return error;
    }

    if (!mSkipDrawCalls)
    {
        mFunctions->drawElementsInstanced(mode, count, type, drawIndexPointer, instances);
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawRangeElements(const gl::Data &data,
                                        GLenum mode,
                                        GLuint start,
                                        GLuint end,
                                        GLsizei count,
                                        GLenum type,
                                        const GLvoid *indices,
                                        const gl::IndexRange &indexRange)
{
    const GLvoid *drawIndexPointer = nullptr;
    gl::Error error =
        mStateManager->setDrawElementsState(data, count, type, indices, 0, &drawIndexPointer);
    if (error.isError())
    {
        return error;
    }

    if (!mSkipDrawCalls)
    {
        mFunctions->drawRangeElements(mode, start, end, count, type, drawIndexPointer);
    }

    return gl::Error(GL_NO_ERROR);
}

CompilerImpl *RendererGL::createCompiler()
{
    return new CompilerGL(mFunctions);
}

ShaderImpl *RendererGL::createShader(const gl::Shader::Data &data)
{
    return new ShaderGL(data, mFunctions);
}

ProgramImpl *RendererGL::createProgram(const gl::Program::Data &data)
{
    return new ProgramGL(data, mFunctions, mStateManager);
}

FramebufferImpl *RendererGL::createFramebuffer(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(data, mFunctions, mStateManager, mWorkarounds, false);
}

TextureImpl *RendererGL::createTexture(GLenum target)
{
    return new TextureGL(target, mFunctions, mWorkarounds, mStateManager, mBlitter);
}

RenderbufferImpl *RendererGL::createRenderbuffer()
{
    return new RenderbufferGL(mFunctions, mWorkarounds, mStateManager, getRendererTextureCaps());
}

BufferImpl *RendererGL::createBuffer()
{
    return new BufferGL(mFunctions, mStateManager);
}

VertexArrayImpl *RendererGL::createVertexArray(const gl::VertexArray::Data &data)
{
    return new VertexArrayGL(data, mFunctions, mStateManager);
}

QueryImpl *RendererGL::createQuery(GLenum type)
{
    return new QueryGL(type);
}

FenceNVImpl *RendererGL::createFenceNV()
{
    return new FenceNVGL(mFunctions);
}

FenceSyncImpl *RendererGL::createFenceSync()
{
    return new FenceSyncGL(mFunctions);
}

TransformFeedbackImpl *RendererGL::createTransformFeedback()
{
    return new TransformFeedbackGL();
}

SamplerImpl *RendererGL::createSampler()
{
    return new SamplerGL(mFunctions, mStateManager);
}

void RendererGL::insertEventMarker(GLsizei, const char *)
{
    UNREACHABLE();
}

void RendererGL::pushGroupMarker(GLsizei, const char *)
{
    UNREACHABLE();
}

void RendererGL::popGroupMarker()
{
    UNREACHABLE();
}

void RendererGL::notifyDeviceLost()
{
    UNIMPLEMENTED();
}

bool RendererGL::isDeviceLost() const
{
    UNIMPLEMENTED();
    return bool();
}

bool RendererGL::testDeviceLost()
{
    UNIMPLEMENTED();
    return bool();
}

bool RendererGL::testDeviceResettable()
{
    UNIMPLEMENTED();
    return bool();
}

VendorID RendererGL::getVendorId() const
{
    UNIMPLEMENTED();
    return VendorID();
}

std::string RendererGL::getVendorString() const
{
    return std::string(reinterpret_cast<const char*>(mFunctions->getString(GL_VENDOR)));
}

std::string RendererGL::getRendererDescription() const
{
    std::string nativeVendorString(reinterpret_cast<const char*>(mFunctions->getString(GL_VENDOR)));
    std::string nativeRendererString(reinterpret_cast<const char*>(mFunctions->getString(GL_RENDERER)));

    std::ostringstream rendererString;
    rendererString << nativeVendorString << " " << nativeRendererString << " OpenGL";
    if (mFunctions->standard == STANDARD_GL_ES)
    {
        rendererString << " ES";
    }
    rendererString << " " << mFunctions->version.major << "." << mFunctions->version.minor;
    if (mFunctions->standard == STANDARD_GL_DESKTOP)
    {
        // Some drivers (NVIDIA) use a profile mask of 0 when in compatibility profile.
        if ((mFunctions->profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) != 0 ||
            (mFunctions->isAtLeastGL(gl::Version(3, 2)) && mFunctions->profile == 0))
        {
            rendererString << " compatibility";
        }
        else if ((mFunctions->profile & GL_CONTEXT_CORE_PROFILE_BIT) != 0)
        {
            rendererString << " core";
        }
    }

    return rendererString.str();
}

const gl::Version &RendererGL::getMaxSupportedESVersion() const
{
    // Force generation of caps
    getRendererCaps();

    return mMaxSupportedESVersion;
}

void RendererGL::generateCaps(gl::Caps *outCaps, gl::TextureCapsMap* outTextureCaps,
                              gl::Extensions *outExtensions,
                              gl::Limitations * /* outLimitations */) const
{
    nativegl_gl::GenerateCaps(mFunctions, outCaps, outTextureCaps, outExtensions, &mMaxSupportedESVersion);
}

void RendererGL::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    mStateManager->syncState(state, dirtyBits);
}
}
