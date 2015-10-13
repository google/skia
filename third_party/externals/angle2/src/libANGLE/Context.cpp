//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Context.cpp: Implements the gl::Context class, managing all GL state and performing
// rendering operations. It is the GLES2 specific implementation of EGLContext.

#include "libANGLE/Context.h"

#include <iterator>
#include <sstream>

#include "common/platform.h"
#include "common/utilities.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Compiler.h"
#include "libANGLE/Display.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/Program.h"
#include "libANGLE/Query.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/ResourceManager.h"
#include "libANGLE/Sampler.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Texture.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/validationES.h"
#include "libANGLE/renderer/Renderer.h"

namespace
{

void MarkTransformFeedbackBufferUsage(gl::TransformFeedback *transformFeedback)
{
    if (transformFeedback->isActive() && !transformFeedback->isPaused())
    {
        for (size_t tfBufferIndex = 0; tfBufferIndex < transformFeedback->getIndexedBufferCount();
             tfBufferIndex++)
        {
            const OffsetBindingPointer<gl::Buffer> &buffer =
                transformFeedback->getIndexedBuffer(tfBufferIndex);
            if (buffer.get() != nullptr)
            {
                buffer->onTransformFeedback();
            }
        }
    }
}
}  // anonymous namespace

namespace gl
{

Context::Context(const egl::Config *config,
                 int clientVersion,
                 const Context *shareContext,
                 rx::Renderer *renderer,
                 bool notifyResets,
                 bool robustAccess)
    : mRenderer(renderer),
      mConfig(config),
      mCurrentSurface(nullptr),
      mData(clientVersion, mState, mCaps, mTextureCaps, mExtensions, nullptr)
{
    ASSERT(robustAccess == false);   // Unimplemented

    initCaps(clientVersion);
    mState.initialize(mCaps, clientVersion);

    mClientVersion = clientVersion;

    mClientType = EGL_OPENGL_ES_API;

    mFenceNVHandleAllocator.setBaseHandle(0);

    if (shareContext != NULL)
    {
        mResourceManager = shareContext->mResourceManager;
        mResourceManager->addRef();
    }
    else
    {
        mResourceManager = new ResourceManager(mRenderer);
    }

    mData.resourceManager = mResourceManager;

    // [OpenGL ES 2.0.24] section 3.7 page 83:
    // In the initial state, TEXTURE_2D and TEXTURE_CUBE_MAP have twodimensional
    // and cube map texture state vectors respectively associated with them.
    // In order that access to these initial textures not be lost, they are treated as texture
    // objects all of whose names are 0.

    Texture *zeroTexture2D = new Texture(mRenderer->createTexture(GL_TEXTURE_2D), 0, GL_TEXTURE_2D);
    mZeroTextures[GL_TEXTURE_2D].set(zeroTexture2D);

    Texture *zeroTextureCube = new Texture(mRenderer->createTexture(GL_TEXTURE_CUBE_MAP), 0, GL_TEXTURE_CUBE_MAP);
    mZeroTextures[GL_TEXTURE_CUBE_MAP].set(zeroTextureCube);

    if (mClientVersion >= 3)
    {
        // TODO: These could also be enabled via extension
        Texture *zeroTexture3D = new Texture(mRenderer->createTexture(GL_TEXTURE_3D), 0, GL_TEXTURE_3D);
        mZeroTextures[GL_TEXTURE_3D].set(zeroTexture3D);

        Texture *zeroTexture2DArray = new Texture(mRenderer->createTexture(GL_TEXTURE_2D_ARRAY), 0, GL_TEXTURE_2D_ARRAY);
        mZeroTextures[GL_TEXTURE_2D_ARRAY].set(zeroTexture2DArray);
    }

    mState.initializeZeroTextures(mZeroTextures);

    bindVertexArray(0);
    bindArrayBuffer(0);
    bindElementArrayBuffer(0);

    bindRenderbuffer(0);

    bindGenericUniformBuffer(0);
    for (unsigned int i = 0; i < mCaps.maxCombinedUniformBlocks; i++)
    {
        bindIndexedUniformBuffer(0, i, 0, -1);
    }

    bindCopyReadBuffer(0);
    bindCopyWriteBuffer(0);
    bindPixelPackBuffer(0);
    bindPixelUnpackBuffer(0);

    // [OpenGL ES 3.0.2] section 2.14.1 pg 85:
    // In the initial state, a default transform feedback object is bound and treated as
    // a transform feedback object with a name of zero. That object is bound any time
    // BindTransformFeedback is called with id of zero
    mTransformFeedbackZero.set(new TransformFeedback(mRenderer->createTransformFeedback(), 0, mCaps));
    bindTransformFeedback(0);

    mHasBeenCurrent = false;
    mContextLost = false;
    mResetStatus = GL_NO_ERROR;
    mResetStrategy = (notifyResets ? GL_LOSE_CONTEXT_ON_RESET_EXT : GL_NO_RESET_NOTIFICATION_EXT);
    mRobustAccess = robustAccess;

    mCompiler = new Compiler(mRenderer, getData());
}

Context::~Context()
{
    mState.reset();

    for (auto framebuffer : mFramebufferMap)
    {
        // Default framebuffer are owned by their respective Surface
        if (framebuffer.second != nullptr && framebuffer.second->id() != 0)
        {
            SafeDelete(framebuffer.second);
        }
    }

    for (auto fence : mFenceNVMap)
    {
        SafeDelete(fence.second);
    }

    for (auto query : mQueryMap)
    {
        query.second->release();
    }

    for (auto vertexArray : mVertexArrayMap)
    {
        SafeDelete(vertexArray.second);
    }

    mTransformFeedbackZero.set(NULL);
    for (auto transformFeedback : mTransformFeedbackMap)
    {
        SafeDelete(transformFeedback.second);
    }

    for (auto &zeroTexture : mZeroTextures)
    {
        zeroTexture.second.set(NULL);
    }
    mZeroTextures.clear();

    if (mCurrentSurface != nullptr)
    {
        releaseSurface();
    }

    if (mResourceManager)
    {
        mResourceManager->release();
    }

    SafeDelete(mCompiler);
}

void Context::makeCurrent(egl::Surface *surface)
{
    ASSERT(surface != nullptr);

    if (!mHasBeenCurrent)
    {
        initRendererString();
        initExtensionStrings();

        mState.setViewportParams(0, 0, surface->getWidth(), surface->getHeight());
        mState.setScissorParams(0, 0, surface->getWidth(), surface->getHeight());

        mHasBeenCurrent = true;
    }

    // TODO(jmadill): Rework this when we support ContextImpl
    mState.setAllDirtyBits();

    if (mCurrentSurface)
    {
        releaseSurface();
    }
    surface->setIsCurrent(true);
    mCurrentSurface = surface;

    // Update default framebuffer, the binding of the previous default
    // framebuffer (or lack of) will have a nullptr.
    {
        Framebuffer *newDefault = surface->getDefaultFramebuffer();
        if (mState.getReadFramebuffer() == nullptr)
        {
            mState.setReadFramebufferBinding(newDefault);
        }
        if (mState.getDrawFramebuffer() == nullptr)
        {
            mState.setDrawFramebufferBinding(newDefault);
        }
        mFramebufferMap[0] = newDefault;
    }
}

void Context::releaseSurface()
{
    ASSERT(mCurrentSurface != nullptr);

    // Remove the default framebuffer
    {
        Framebuffer *currentDefault = mCurrentSurface->getDefaultFramebuffer();
        if (mState.getReadFramebuffer() == currentDefault)
        {
            mState.setReadFramebufferBinding(nullptr);
        }
        if (mState.getDrawFramebuffer() == currentDefault)
        {
            mState.setDrawFramebufferBinding(nullptr);
        }
        mFramebufferMap.erase(0);
    }

    mCurrentSurface->setIsCurrent(false);
    mCurrentSurface = nullptr;
}

// NOTE: this function should not assume that this context is current!
void Context::markContextLost()
{
    if (mResetStrategy == GL_LOSE_CONTEXT_ON_RESET_EXT)
        mResetStatus = GL_UNKNOWN_CONTEXT_RESET_EXT;
    mContextLost = true;
}

bool Context::isContextLost()
{
    return mContextLost;
}

GLuint Context::createBuffer()
{
    return mResourceManager->createBuffer();
}

GLuint Context::createProgram()
{
    return mResourceManager->createProgram();
}

GLuint Context::createShader(GLenum type)
{
    return mResourceManager->createShader(mRenderer->getRendererLimitations(), type);
}

GLuint Context::createTexture()
{
    return mResourceManager->createTexture();
}

GLuint Context::createRenderbuffer()
{
    return mResourceManager->createRenderbuffer();
}

GLsync Context::createFenceSync()
{
    GLuint handle = mResourceManager->createFenceSync();

    return reinterpret_cast<GLsync>(static_cast<uintptr_t>(handle));
}

GLuint Context::createVertexArray()
{
    GLuint handle = mVertexArrayHandleAllocator.allocate();

    // Although the spec states VAO state is not initialized until the object is bound,
    // we create it immediately. The resulting behaviour is transparent to the application,
    // since it's not currently possible to access the state until the object is bound.
    VertexArray *vertexArray = new VertexArray(mRenderer, handle, MAX_VERTEX_ATTRIBS);
    mVertexArrayMap[handle] = vertexArray;
    return handle;
}

GLuint Context::createSampler()
{
    return mResourceManager->createSampler();
}

GLuint Context::createTransformFeedback()
{
    GLuint handle = mTransformFeedbackAllocator.allocate();
    TransformFeedback *transformFeedback = new TransformFeedback(mRenderer->createTransformFeedback(), handle, mCaps);
    transformFeedback->addRef();
    mTransformFeedbackMap[handle] = transformFeedback;
    return handle;
}

// Returns an unused framebuffer name
GLuint Context::createFramebuffer()
{
    GLuint handle = mFramebufferHandleAllocator.allocate();

    mFramebufferMap[handle] = NULL;

    return handle;
}

GLuint Context::createFenceNV()
{
    GLuint handle = mFenceNVHandleAllocator.allocate();

    mFenceNVMap[handle] = new FenceNV(mRenderer->createFenceNV());

    return handle;
}

// Returns an unused query name
GLuint Context::createQuery()
{
    GLuint handle = mQueryHandleAllocator.allocate();

    mQueryMap[handle] = NULL;

    return handle;
}

void Context::deleteBuffer(GLuint buffer)
{
    if (mResourceManager->getBuffer(buffer))
    {
        detachBuffer(buffer);
    }

    mResourceManager->deleteBuffer(buffer);
}

void Context::deleteShader(GLuint shader)
{
    mResourceManager->deleteShader(shader);
}

void Context::deleteProgram(GLuint program)
{
    mResourceManager->deleteProgram(program);
}

void Context::deleteTexture(GLuint texture)
{
    if (mResourceManager->getTexture(texture))
    {
        detachTexture(texture);
    }

    mResourceManager->deleteTexture(texture);
}

void Context::deleteRenderbuffer(GLuint renderbuffer)
{
    if (mResourceManager->getRenderbuffer(renderbuffer))
    {
        detachRenderbuffer(renderbuffer);
    }

    mResourceManager->deleteRenderbuffer(renderbuffer);
}

void Context::deleteFenceSync(GLsync fenceSync)
{
    // The spec specifies the underlying Fence object is not deleted until all current
    // wait commands finish. However, since the name becomes invalid, we cannot query the fence,
    // and since our API is currently designed for being called from a single thread, we can delete
    // the fence immediately.
    mResourceManager->deleteFenceSync(static_cast<GLuint>(reinterpret_cast<uintptr_t>(fenceSync)));
}

void Context::deleteVertexArray(GLuint vertexArray)
{
    auto vertexArrayObject = mVertexArrayMap.find(vertexArray);

    if (vertexArrayObject != mVertexArrayMap.end())
    {
        detachVertexArray(vertexArray);

        mVertexArrayHandleAllocator.release(vertexArrayObject->first);
        delete vertexArrayObject->second;
        mVertexArrayMap.erase(vertexArrayObject);
    }
}

void Context::deleteSampler(GLuint sampler)
{
    if (mResourceManager->getSampler(sampler))
    {
        detachSampler(sampler);
    }

    mResourceManager->deleteSampler(sampler);
}

void Context::deleteTransformFeedback(GLuint transformFeedback)
{
    auto iter = mTransformFeedbackMap.find(transformFeedback);
    if (iter != mTransformFeedbackMap.end())
    {
        detachTransformFeedback(transformFeedback);
        mTransformFeedbackAllocator.release(transformFeedback);
        iter->second->release();
        mTransformFeedbackMap.erase(iter);
    }
}

void Context::deleteFramebuffer(GLuint framebuffer)
{
    FramebufferMap::iterator framebufferObject = mFramebufferMap.find(framebuffer);

    if (framebufferObject != mFramebufferMap.end())
    {
        detachFramebuffer(framebuffer);

        mFramebufferHandleAllocator.release(framebufferObject->first);
        delete framebufferObject->second;
        mFramebufferMap.erase(framebufferObject);
    }
}

void Context::deleteFenceNV(GLuint fence)
{
    FenceNVMap::iterator fenceObject = mFenceNVMap.find(fence);

    if (fenceObject != mFenceNVMap.end())
    {
        mFenceNVHandleAllocator.release(fenceObject->first);
        delete fenceObject->second;
        mFenceNVMap.erase(fenceObject);
    }
}

void Context::deleteQuery(GLuint query)
{
    QueryMap::iterator queryObject = mQueryMap.find(query);
    if (queryObject != mQueryMap.end())
    {
        mQueryHandleAllocator.release(queryObject->first);
        if (queryObject->second)
        {
            queryObject->second->release();
        }
        mQueryMap.erase(queryObject);
    }
}

Buffer *Context::getBuffer(GLuint handle)
{
    return mResourceManager->getBuffer(handle);
}

Shader *Context::getShader(GLuint handle) const
{
    return mResourceManager->getShader(handle);
}

Program *Context::getProgram(GLuint handle) const
{
    return mResourceManager->getProgram(handle);
}

Texture *Context::getTexture(GLuint handle) const
{
    return mResourceManager->getTexture(handle);
}

Renderbuffer *Context::getRenderbuffer(GLuint handle)
{
    return mResourceManager->getRenderbuffer(handle);
}

FenceSync *Context::getFenceSync(GLsync handle) const
{
    return mResourceManager->getFenceSync(static_cast<GLuint>(reinterpret_cast<uintptr_t>(handle)));
}

VertexArray *Context::getVertexArray(GLuint handle) const
{
    auto vertexArray = mVertexArrayMap.find(handle);

    if (vertexArray == mVertexArrayMap.end())
    {
        return NULL;
    }
    else
    {
        return vertexArray->second;
    }
}

Sampler *Context::getSampler(GLuint handle) const
{
    return mResourceManager->getSampler(handle);
}

TransformFeedback *Context::getTransformFeedback(GLuint handle) const
{
    if (handle == 0)
    {
        return mTransformFeedbackZero.get();
    }
    else
    {
        TransformFeedbackMap::const_iterator iter = mTransformFeedbackMap.find(handle);
        return (iter != mTransformFeedbackMap.end()) ? iter->second : NULL;
    }
}

bool Context::isSampler(GLuint samplerName) const
{
    return mResourceManager->isSampler(samplerName);
}

void Context::bindArrayBuffer(unsigned int buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setArrayBufferBinding(getBuffer(buffer));
}

void Context::bindElementArrayBuffer(unsigned int buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.getVertexArray()->setElementArrayBuffer(getBuffer(buffer));
}

void Context::bindTexture(GLenum target, GLuint handle)
{
    Texture *texture = NULL;

    if (handle == 0)
    {
        texture = mZeroTextures[target].get();
    }
    else
    {
        mResourceManager->checkTextureAllocation(handle, target);
        texture = getTexture(handle);
    }

    ASSERT(texture);

    mState.setSamplerTexture(target, texture);
}

void Context::bindReadFramebuffer(GLuint framebuffer)
{
    if (!getFramebuffer(framebuffer))
    {
        mFramebufferMap[framebuffer] = new Framebuffer(mCaps, mRenderer, framebuffer);
    }

    mState.setReadFramebufferBinding(getFramebuffer(framebuffer));
}

void Context::bindDrawFramebuffer(GLuint framebuffer)
{
    if (!getFramebuffer(framebuffer))
    {
        mFramebufferMap[framebuffer] = new Framebuffer(mCaps, mRenderer, framebuffer);
    }

    mState.setDrawFramebufferBinding(getFramebuffer(framebuffer));
}

void Context::bindRenderbuffer(GLuint renderbuffer)
{
    mResourceManager->checkRenderbufferAllocation(renderbuffer);

    mState.setRenderbufferBinding(getRenderbuffer(renderbuffer));
}

void Context::bindVertexArray(GLuint vertexArray)
{
    if (!getVertexArray(vertexArray))
    {
        VertexArray *vertexArrayObject = new VertexArray(mRenderer, vertexArray, MAX_VERTEX_ATTRIBS);
        mVertexArrayMap[vertexArray] = vertexArrayObject;
    }

    mState.setVertexArrayBinding(getVertexArray(vertexArray));
}

void Context::bindSampler(GLuint textureUnit, GLuint sampler)
{
    ASSERT(textureUnit < mCaps.maxCombinedTextureImageUnits);
    mResourceManager->checkSamplerAllocation(sampler);

    mState.setSamplerBinding(textureUnit, getSampler(sampler));
}

void Context::bindGenericUniformBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setGenericUniformBufferBinding(getBuffer(buffer));
}

void Context::bindIndexedUniformBuffer(GLuint buffer, GLuint index, GLintptr offset, GLsizeiptr size)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setIndexedUniformBufferBinding(index, getBuffer(buffer), offset, size);
}

void Context::bindGenericTransformFeedbackBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.getCurrentTransformFeedback()->bindGenericBuffer(getBuffer(buffer));
}

void Context::bindIndexedTransformFeedbackBuffer(GLuint buffer, GLuint index, GLintptr offset, GLsizeiptr size)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.getCurrentTransformFeedback()->bindIndexedBuffer(index, getBuffer(buffer), offset, size);
}

void Context::bindCopyReadBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setCopyReadBufferBinding(getBuffer(buffer));
}

void Context::bindCopyWriteBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setCopyWriteBufferBinding(getBuffer(buffer));
}

void Context::bindPixelPackBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setPixelPackBufferBinding(getBuffer(buffer));
}

void Context::bindPixelUnpackBuffer(GLuint buffer)
{
    mResourceManager->checkBufferAllocation(buffer);

    mState.setPixelUnpackBufferBinding(getBuffer(buffer));
}

void Context::useProgram(GLuint program)
{
    mState.setProgram(getProgram(program));
}

void Context::bindTransformFeedback(GLuint transformFeedback)
{
    mState.setTransformFeedbackBinding(getTransformFeedback(transformFeedback));
}

Error Context::beginQuery(GLenum target, GLuint query)
{
    Query *queryObject = getQuery(query, true, target);
    ASSERT(queryObject);

    // begin query
    Error error = queryObject->begin();
    if (error.isError())
    {
        return error;
    }

    // set query as active for specified target only if begin succeeded
    mState.setActiveQuery(target, queryObject);

    return Error(GL_NO_ERROR);
}

Error Context::endQuery(GLenum target)
{
    Query *queryObject = mState.getActiveQuery(target);
    ASSERT(queryObject);

    gl::Error error = queryObject->end();

    // Always unbind the query, even if there was an error. This may delete the query object.
    mState.setActiveQuery(target, NULL);

    return error;
}

Framebuffer *Context::getFramebuffer(unsigned int handle) const
{
    FramebufferMap::const_iterator framebuffer = mFramebufferMap.find(handle);

    if (framebuffer == mFramebufferMap.end())
    {
        return NULL;
    }
    else
    {
        return framebuffer->second;
    }
}

FenceNV *Context::getFenceNV(unsigned int handle)
{
    FenceNVMap::iterator fence = mFenceNVMap.find(handle);

    if (fence == mFenceNVMap.end())
    {
        return NULL;
    }
    else
    {
        return fence->second;
    }
}

Query *Context::getQuery(unsigned int handle, bool create, GLenum type)
{
    QueryMap::iterator query = mQueryMap.find(handle);

    if (query == mQueryMap.end())
    {
        return NULL;
    }
    else
    {
        if (!query->second && create)
        {
            query->second = new Query(mRenderer->createQuery(type), handle);
            query->second->addRef();
        }
        return query->second;
    }
}

Texture *Context::getTargetTexture(GLenum target) const
{
    ASSERT(ValidTextureTarget(this, target));

    return getSamplerTexture(mState.getActiveSampler(), target);
}

Texture *Context::getSamplerTexture(unsigned int sampler, GLenum type) const
{
    return mState.getSamplerTexture(sampler, type);
}

Compiler *Context::getCompiler() const
{
    return mCompiler;
}

void Context::getBooleanv(GLenum pname, GLboolean *params)
{
    switch (pname)
    {
      case GL_SHADER_COMPILER:           *params = GL_TRUE;                             break;
      case GL_CONTEXT_ROBUST_ACCESS_EXT: *params = mRobustAccess ? GL_TRUE : GL_FALSE;  break;
      default:
        mState.getBooleanv(pname, params);
        break;
    }
}

void Context::getFloatv(GLenum pname, GLfloat *params)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    switch (pname)
    {
      case GL_ALIASED_LINE_WIDTH_RANGE:
        params[0] = mCaps.minAliasedLineWidth;
        params[1] = mCaps.maxAliasedLineWidth;
        break;
      case GL_ALIASED_POINT_SIZE_RANGE:
        params[0] = mCaps.minAliasedPointSize;
        params[1] = mCaps.maxAliasedPointSize;
        break;
      case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        ASSERT(mExtensions.textureFilterAnisotropic);
        *params = mExtensions.maxTextureAnisotropy;
        break;
      case GL_MAX_TEXTURE_LOD_BIAS:
        *params = mCaps.maxLODBias;
        break;
      default:
        mState.getFloatv(pname, params);
        break;
    }
}

void Context::getIntegerv(GLenum pname, GLint *params)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.

    switch (pname)
    {
      case GL_MAX_VERTEX_ATTRIBS:                       *params = mCaps.maxVertexAttributes;                            break;
      case GL_MAX_VERTEX_UNIFORM_VECTORS:               *params = mCaps.maxVertexUniformVectors;                        break;
      case GL_MAX_VERTEX_UNIFORM_COMPONENTS:            *params = mCaps.maxVertexUniformComponents;                     break;
      case GL_MAX_VARYING_VECTORS:                      *params = mCaps.maxVaryingVectors;                              break;
      case GL_MAX_VARYING_COMPONENTS:                   *params = mCaps.maxVertexOutputComponents;                      break;
      case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:         *params = mCaps.maxCombinedTextureImageUnits;                   break;
      case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:           *params = mCaps.maxVertexTextureImageUnits;                     break;
      case GL_MAX_TEXTURE_IMAGE_UNITS:                  *params = mCaps.maxTextureImageUnits;                           break;
      case GL_MAX_FRAGMENT_UNIFORM_VECTORS:             *params = mCaps.maxFragmentUniformVectors;                      break;
      case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:          *params = mCaps.maxFragmentInputComponents;                     break;
      case GL_MAX_RENDERBUFFER_SIZE:                    *params = mCaps.maxRenderbufferSize;                            break;
      case GL_MAX_COLOR_ATTACHMENTS_EXT:                *params = mCaps.maxColorAttachments;                            break;
      case GL_MAX_DRAW_BUFFERS_EXT:                     *params = mCaps.maxDrawBuffers;                                 break;
      //case GL_FRAMEBUFFER_BINDING:                    // now equivalent to GL_DRAW_FRAMEBUFFER_BINDING_ANGLE
      case GL_SUBPIXEL_BITS:                            *params = 4;                                                    break;
      case GL_MAX_TEXTURE_SIZE:                         *params = mCaps.max2DTextureSize;                               break;
      case GL_MAX_CUBE_MAP_TEXTURE_SIZE:                *params = mCaps.maxCubeMapTextureSize;                          break;
      case GL_MAX_3D_TEXTURE_SIZE:                      *params = mCaps.max3DTextureSize;                               break;
      case GL_MAX_ARRAY_TEXTURE_LAYERS:                 *params = mCaps.maxArrayTextureLayers;                          break;
      case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:          *params = mCaps.uniformBufferOffsetAlignment;                   break;
      case GL_MAX_UNIFORM_BUFFER_BINDINGS:              *params = mCaps.maxUniformBufferBindings;                       break;
      case GL_MAX_VERTEX_UNIFORM_BLOCKS:                *params = mCaps.maxVertexUniformBlocks;                         break;
      case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:              *params = mCaps.maxFragmentUniformBlocks;                       break;
      case GL_MAX_COMBINED_UNIFORM_BLOCKS:              *params = mCaps.maxCombinedTextureImageUnits;                   break;
      case GL_MAX_VERTEX_OUTPUT_COMPONENTS:             *params = mCaps.maxVertexOutputComponents;                      break;
      case GL_MAX_FRAGMENT_INPUT_COMPONENTS:            *params = mCaps.maxFragmentInputComponents;                     break;
      case GL_MIN_PROGRAM_TEXEL_OFFSET:                 *params = mCaps.minProgramTexelOffset;                          break;
      case GL_MAX_PROGRAM_TEXEL_OFFSET:                 *params = mCaps.maxProgramTexelOffset;                          break;
      case GL_MAJOR_VERSION:                            *params = mClientVersion;                                       break;
      case GL_MINOR_VERSION:                            *params = 0;                                                    break;
      case GL_MAX_ELEMENTS_INDICES:                     *params = mCaps.maxElementsIndices;                             break;
      case GL_MAX_ELEMENTS_VERTICES:                    *params = mCaps.maxElementsVertices;                            break;
      case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS: *params = mCaps.maxTransformFeedbackInterleavedComponents; break;
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:       *params = mCaps.maxTransformFeedbackSeparateAttributes;    break;
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:    *params = mCaps.maxTransformFeedbackSeparateComponents;    break;
      case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
          *params = static_cast<GLint>(mCaps.compressedTextureFormats.size());
          break;
      case GL_MAX_SAMPLES_ANGLE:                        *params = mCaps.maxSamples;                                     break;
      case GL_MAX_VIEWPORT_DIMS:
        {
            params[0] = mCaps.maxViewportWidth;
            params[1] = mCaps.maxViewportHeight;
        }
        break;
      case GL_COMPRESSED_TEXTURE_FORMATS:
        std::copy(mCaps.compressedTextureFormats.begin(), mCaps.compressedTextureFormats.end(), params);
        break;
      case GL_RESET_NOTIFICATION_STRATEGY_EXT:
        *params = mResetStrategy;
        break;
      case GL_NUM_SHADER_BINARY_FORMATS:
          *params = static_cast<GLint>(mCaps.shaderBinaryFormats.size());
        break;
      case GL_SHADER_BINARY_FORMATS:
        std::copy(mCaps.shaderBinaryFormats.begin(), mCaps.shaderBinaryFormats.end(), params);
        break;
      case GL_NUM_PROGRAM_BINARY_FORMATS:
          *params = static_cast<GLint>(mCaps.programBinaryFormats.size());
        break;
      case GL_PROGRAM_BINARY_FORMATS:
        std::copy(mCaps.programBinaryFormats.begin(), mCaps.programBinaryFormats.end(), params);
        break;
      case GL_NUM_EXTENSIONS:
        *params = static_cast<GLint>(mExtensionStrings.size());
        break;
      default:
        mState.getIntegerv(getData(), pname, params);
        break;
    }
}

void Context::getInteger64v(GLenum pname, GLint64 *params)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    switch (pname)
    {
      case GL_MAX_ELEMENT_INDEX:
        *params = mCaps.maxElementIndex;
        break;
      case GL_MAX_UNIFORM_BLOCK_SIZE:
        *params = mCaps.maxUniformBlockSize;
        break;
      case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
        *params = mCaps.maxCombinedVertexUniformComponents;
        break;
      case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
        *params = mCaps.maxCombinedFragmentUniformComponents;
        break;
      case GL_MAX_SERVER_WAIT_TIMEOUT:
        *params = mCaps.maxServerWaitTimeout;
        break;
      default:
        UNREACHABLE();
        break;
    }
}

bool Context::getIndexedIntegerv(GLenum target, GLuint index, GLint *data)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    // Indexed integer queries all refer to current state, so this function is a
    // mere passthrough.
    return mState.getIndexedIntegerv(target, index, data);
}

bool Context::getIndexedInteger64v(GLenum target, GLuint index, GLint64 *data)
{
    // Queries about context capabilities and maximums are answered by Context.
    // Queries about current GL state values are answered by State.
    // Indexed integer queries all refer to current state, so this function is a
    // mere passthrough.
    return mState.getIndexedInteger64v(target, index, data);
}

bool Context::getQueryParameterInfo(GLenum pname, GLenum *type, unsigned int *numParams)
{
    if (pname >= GL_DRAW_BUFFER0_EXT && pname <= GL_DRAW_BUFFER15_EXT)
    {
        *type = GL_INT;
        *numParams = 1;
        return true;
    }

    // Please note: the query type returned for DEPTH_CLEAR_VALUE in this implementation
    // is FLOAT rather than INT, as would be suggested by the GL ES 2.0 spec. This is due
    // to the fact that it is stored internally as a float, and so would require conversion
    // if returned from Context::getIntegerv. Since this conversion is already implemented
    // in the case that one calls glGetIntegerv to retrieve a float-typed state variable, we
    // place DEPTH_CLEAR_VALUE with the floats. This should make no difference to the calling
    // application.
    switch (pname)
    {
      case GL_COMPRESSED_TEXTURE_FORMATS:
        {
            *type = GL_INT;
            *numParams = static_cast<unsigned int>(mCaps.compressedTextureFormats.size());
        }
        return true;
      case GL_PROGRAM_BINARY_FORMATS_OES:
        {
            *type = GL_INT;
            *numParams = static_cast<unsigned int>(mCaps.programBinaryFormats.size());
        }
        return true;
      case GL_SHADER_BINARY_FORMATS:
        {
            *type = GL_INT;
            *numParams = static_cast<unsigned int>(mCaps.shaderBinaryFormats.size());
        }
        return true;

      case GL_MAX_VERTEX_ATTRIBS:
      case GL_MAX_VERTEX_UNIFORM_VECTORS:
      case GL_MAX_VARYING_VECTORS:
      case GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
      case GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS:
      case GL_MAX_TEXTURE_IMAGE_UNITS:
      case GL_MAX_FRAGMENT_UNIFORM_VECTORS:
      case GL_MAX_RENDERBUFFER_SIZE:
      case GL_MAX_COLOR_ATTACHMENTS_EXT:
      case GL_MAX_DRAW_BUFFERS_EXT:
      case GL_NUM_SHADER_BINARY_FORMATS:
      case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
      case GL_ARRAY_BUFFER_BINDING:
      //case GL_FRAMEBUFFER_BINDING: // equivalent to DRAW_FRAMEBUFFER_BINDING_ANGLE
      case GL_DRAW_FRAMEBUFFER_BINDING_ANGLE:
      case GL_READ_FRAMEBUFFER_BINDING_ANGLE:
      case GL_RENDERBUFFER_BINDING:
      case GL_CURRENT_PROGRAM:
      case GL_PACK_ALIGNMENT:
      case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
      case GL_UNPACK_ALIGNMENT:
      case GL_GENERATE_MIPMAP_HINT:
      case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:
      case GL_RED_BITS:
      case GL_GREEN_BITS:
      case GL_BLUE_BITS:
      case GL_ALPHA_BITS:
      case GL_DEPTH_BITS:
      case GL_STENCIL_BITS:
      case GL_ELEMENT_ARRAY_BUFFER_BINDING:
      case GL_CULL_FACE_MODE:
      case GL_FRONT_FACE:
      case GL_ACTIVE_TEXTURE:
      case GL_STENCIL_FUNC:
      case GL_STENCIL_VALUE_MASK:
      case GL_STENCIL_REF:
      case GL_STENCIL_FAIL:
      case GL_STENCIL_PASS_DEPTH_FAIL:
      case GL_STENCIL_PASS_DEPTH_PASS:
      case GL_STENCIL_BACK_FUNC:
      case GL_STENCIL_BACK_VALUE_MASK:
      case GL_STENCIL_BACK_REF:
      case GL_STENCIL_BACK_FAIL:
      case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
      case GL_STENCIL_BACK_PASS_DEPTH_PASS:
      case GL_DEPTH_FUNC:
      case GL_BLEND_SRC_RGB:
      case GL_BLEND_SRC_ALPHA:
      case GL_BLEND_DST_RGB:
      case GL_BLEND_DST_ALPHA:
      case GL_BLEND_EQUATION_RGB:
      case GL_BLEND_EQUATION_ALPHA:
      case GL_STENCIL_WRITEMASK:
      case GL_STENCIL_BACK_WRITEMASK:
      case GL_STENCIL_CLEAR_VALUE:
      case GL_SUBPIXEL_BITS:
      case GL_MAX_TEXTURE_SIZE:
      case GL_MAX_CUBE_MAP_TEXTURE_SIZE:
      case GL_SAMPLE_BUFFERS:
      case GL_SAMPLES:
      case GL_IMPLEMENTATION_COLOR_READ_TYPE:
      case GL_IMPLEMENTATION_COLOR_READ_FORMAT:
      case GL_TEXTURE_BINDING_2D:
      case GL_TEXTURE_BINDING_CUBE_MAP:
      case GL_RESET_NOTIFICATION_STRATEGY_EXT:
      case GL_NUM_PROGRAM_BINARY_FORMATS_OES:
        {
            *type = GL_INT;
            *numParams = 1;
        }
        return true;
      case GL_MAX_SAMPLES_ANGLE:
        {
            if (mExtensions.framebufferMultisample)
            {
                *type = GL_INT;
                *numParams = 1;
            }
            else
            {
                return false;
            }
        }
        return true;
      case GL_PIXEL_PACK_BUFFER_BINDING:
      case GL_PIXEL_UNPACK_BUFFER_BINDING:
        {
            if (mExtensions.pixelBufferObject)
            {
                *type = GL_INT;
                *numParams = 1;
            }
            else
            {
                return false;
            }
        }
        return true;
        case GL_PACK_ROW_LENGTH:
        case GL_PACK_SKIP_ROWS:
        case GL_PACK_SKIP_PIXELS:
            if (!mExtensions.packSubimage)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
        case GL_UNPACK_ROW_LENGTH:
        case GL_UNPACK_SKIP_ROWS:
        case GL_UNPACK_SKIP_PIXELS:
            if (!mExtensions.unpackSubimage)
            {
                return false;
            }
            *type      = GL_INT;
            *numParams = 1;
            return true;
      case GL_MAX_VIEWPORT_DIMS:
        {
            *type = GL_INT;
            *numParams = 2;
        }
        return true;
      case GL_VIEWPORT:
      case GL_SCISSOR_BOX:
        {
            *type = GL_INT;
            *numParams = 4;
        }
        return true;
      case GL_SHADER_COMPILER:
      case GL_SAMPLE_COVERAGE_INVERT:
      case GL_DEPTH_WRITEMASK:
      case GL_CULL_FACE:                // CULL_FACE through DITHER are natural to IsEnabled,
      case GL_POLYGON_OFFSET_FILL:      // but can be retrieved through the Get{Type}v queries.
      case GL_SAMPLE_ALPHA_TO_COVERAGE: // For this purpose, they are treated here as bool-natural
      case GL_SAMPLE_COVERAGE:
      case GL_SCISSOR_TEST:
      case GL_STENCIL_TEST:
      case GL_DEPTH_TEST:
      case GL_BLEND:
      case GL_DITHER:
      case GL_CONTEXT_ROBUST_ACCESS_EXT:
        {
            *type = GL_BOOL;
            *numParams = 1;
        }
        return true;
      case GL_COLOR_WRITEMASK:
        {
            *type = GL_BOOL;
            *numParams = 4;
        }
        return true;
      case GL_POLYGON_OFFSET_FACTOR:
      case GL_POLYGON_OFFSET_UNITS:
      case GL_SAMPLE_COVERAGE_VALUE:
      case GL_DEPTH_CLEAR_VALUE:
      case GL_LINE_WIDTH:
        {
            *type = GL_FLOAT;
            *numParams = 1;
        }
        return true;
      case GL_ALIASED_LINE_WIDTH_RANGE:
      case GL_ALIASED_POINT_SIZE_RANGE:
      case GL_DEPTH_RANGE:
        {
            *type = GL_FLOAT;
            *numParams = 2;
        }
        return true;
      case GL_COLOR_CLEAR_VALUE:
      case GL_BLEND_COLOR:
        {
            *type = GL_FLOAT;
            *numParams = 4;
        }
        return true;
      case GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT:
        if (!mExtensions.maxTextureAnisotropy)
        {
            return false;
        }
        *type = GL_FLOAT;
        *numParams = 1;
        return true;
    }

    if (mClientVersion < 3)
    {
        return false;
    }

    // Check for ES3.0+ parameter names
    switch (pname)
    {
      case GL_MAX_UNIFORM_BUFFER_BINDINGS:
      case GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT:
      case GL_UNIFORM_BUFFER_BINDING:
      case GL_TRANSFORM_FEEDBACK_BINDING:
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
      case GL_COPY_READ_BUFFER_BINDING:
      case GL_COPY_WRITE_BUFFER_BINDING:
      case GL_TEXTURE_BINDING_3D:
      case GL_TEXTURE_BINDING_2D_ARRAY:
      case GL_MAX_3D_TEXTURE_SIZE:
      case GL_MAX_ARRAY_TEXTURE_LAYERS:
      case GL_MAX_VERTEX_UNIFORM_BLOCKS:
      case GL_MAX_FRAGMENT_UNIFORM_BLOCKS:
      case GL_MAX_COMBINED_UNIFORM_BLOCKS:
      case GL_MAX_VERTEX_OUTPUT_COMPONENTS:
      case GL_MAX_FRAGMENT_INPUT_COMPONENTS:
      case GL_MAX_VARYING_COMPONENTS:
      case GL_VERTEX_ARRAY_BINDING:
      case GL_MAX_VERTEX_UNIFORM_COMPONENTS:
      case GL_MAX_FRAGMENT_UNIFORM_COMPONENTS:
      case GL_MIN_PROGRAM_TEXEL_OFFSET:
      case GL_MAX_PROGRAM_TEXEL_OFFSET:
      case GL_NUM_EXTENSIONS:
      case GL_MAJOR_VERSION:
      case GL_MINOR_VERSION:
      case GL_MAX_ELEMENTS_INDICES:
      case GL_MAX_ELEMENTS_VERTICES:
      case GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS:
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS:
      case GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS:
      case GL_PACK_ROW_LENGTH:
      case GL_PACK_SKIP_ROWS:
      case GL_PACK_SKIP_PIXELS:
      case GL_UNPACK_ROW_LENGTH:
      case GL_UNPACK_SKIP_ROWS:
      case GL_UNPACK_SKIP_PIXELS:
        {
            *type = GL_INT;
            *numParams = 1;
        }
        return true;

      case GL_MAX_ELEMENT_INDEX:
      case GL_MAX_UNIFORM_BLOCK_SIZE:
      case GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS:
      case GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS:
      case GL_MAX_SERVER_WAIT_TIMEOUT:
        {
            *type = GL_INT_64_ANGLEX;
            *numParams = 1;
        }
        return true;

      case GL_TRANSFORM_FEEDBACK_ACTIVE:
      case GL_TRANSFORM_FEEDBACK_PAUSED:
        {
            *type = GL_BOOL;
            *numParams = 1;
        }
        return true;

      case GL_MAX_TEXTURE_LOD_BIAS:
        {
            *type = GL_FLOAT;
            *numParams = 1;
        }
        return true;
    }

    return false;
}

bool Context::getIndexedQueryParameterInfo(GLenum target, GLenum *type, unsigned int *numParams)
{
    if (mClientVersion < 3)
    {
        return false;
    }

    switch (target)
    {
      case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
      case GL_UNIFORM_BUFFER_BINDING:
        {
            *type = GL_INT;
            *numParams = 1;
        }
        return true;
      case GL_TRANSFORM_FEEDBACK_BUFFER_START:
      case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
      case GL_UNIFORM_BUFFER_START:
      case GL_UNIFORM_BUFFER_SIZE:
        {
            *type = GL_INT_64_ANGLEX;
            *numParams = 1;
        }
    }

    return false;
}

Error Context::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    syncRendererState();
    Error error = mRenderer->drawArrays(getData(), mode, first, count);
    if (error.isError())
    {
        return error;
    }

    MarkTransformFeedbackBufferUsage(mState.getCurrentTransformFeedback());

    return Error(GL_NO_ERROR);
}

Error Context::drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    syncRendererState();
    Error error = mRenderer->drawArraysInstanced(getData(), mode, first, count, instanceCount);
    if (error.isError())
    {
        return error;
    }

    MarkTransformFeedbackBufferUsage(mState.getCurrentTransformFeedback());

    return Error(GL_NO_ERROR);
}

Error Context::drawElements(GLenum mode,
                            GLsizei count,
                            GLenum type,
                            const GLvoid *indices,
                            const IndexRange &indexRange)
{
    syncRendererState();
    return mRenderer->drawElements(getData(), mode, count, type, indices, indexRange);
}

Error Context::drawElementsInstanced(GLenum mode,
                                     GLsizei count,
                                     GLenum type,
                                     const GLvoid *indices,
                                     GLsizei instances,
                                     const IndexRange &indexRange)
{
    syncRendererState();
    return mRenderer->drawElementsInstanced(getData(), mode, count, type, indices, instances,
                                            indexRange);
}

Error Context::drawRangeElements(GLenum mode,
                                 GLuint start,
                                 GLuint end,
                                 GLsizei count,
                                 GLenum type,
                                 const GLvoid *indices,
                                 const IndexRange &indexRange)
{
    syncRendererState();
    return mRenderer->drawRangeElements(getData(), mode, start, end, count, type, indices,
                                        indexRange);
}

Error Context::flush()
{
    return mRenderer->flush();
}

Error Context::finish()
{
    return mRenderer->finish();
}

void Context::insertEventMarker(GLsizei length, const char *marker)
{
    ASSERT(mRenderer);
    mRenderer->insertEventMarker(length, marker);
}

void Context::pushGroupMarker(GLsizei length, const char *marker)
{
    ASSERT(mRenderer);
    mRenderer->pushGroupMarker(length, marker);
}

void Context::popGroupMarker()
{
    ASSERT(mRenderer);
    mRenderer->popGroupMarker();
}

void Context::recordError(const Error &error)
{
    if (error.isError())
    {
        mErrors.insert(error.getCode());
    }
}

// Get one of the recorded errors and clear its flag, if any.
// [OpenGL ES 2.0.24] section 2.5 page 13.
GLenum Context::getError()
{
    if (mErrors.empty())
    {
        return GL_NO_ERROR;
    }
    else
    {
        GLenum error = *mErrors.begin();
        mErrors.erase(mErrors.begin());
        return error;
    }
}

GLenum Context::getResetStatus()
{
    //TODO(jmadill): needs MANGLE reworking
    if (mResetStatus == GL_NO_ERROR && !mContextLost)
    {
        // mResetStatus will be set by the markContextLost callback
        // in the case a notification is sent
        if (mRenderer->testDeviceLost())
        {
            mRenderer->notifyDeviceLost();
        }
    }

    GLenum status = mResetStatus;

    if (mResetStatus != GL_NO_ERROR)
    {
        ASSERT(mContextLost);

        if (mRenderer->testDeviceResettable())
        {
            mResetStatus = GL_NO_ERROR;
        }
    }

    return status;
}

bool Context::isResetNotificationEnabled()
{
    return (mResetStrategy == GL_LOSE_CONTEXT_ON_RESET_EXT);
}

int Context::getClientVersion() const
{
    return mClientVersion;
}

const egl::Config *Context::getConfig() const
{
    return mConfig;
}

EGLenum Context::getClientType() const
{
    return mClientType;
}

EGLenum Context::getRenderBuffer() const
{
    auto framebufferIt = mFramebufferMap.find(0);
    if (framebufferIt != mFramebufferMap.end())
    {
        const Framebuffer *framebuffer              = framebufferIt->second;
        const FramebufferAttachment *backAttachment = framebuffer->getAttachment(GL_BACK);

        ASSERT(backAttachment != nullptr);
        return backAttachment->getSurface()->getRenderBuffer();
    }
    else
    {
        return EGL_NONE;
    }
}

const Caps &Context::getCaps() const
{
    return mCaps;
}

const TextureCapsMap &Context::getTextureCaps() const
{
    return mTextureCaps;
}

const Extensions &Context::getExtensions() const
{
    return mExtensions;
}

const Limitations &Context::getLimitations() const
{
    return mLimitations;
}

void Context::detachTexture(GLuint texture)
{
    // Simple pass-through to State's detachTexture method, as textures do not require
    // allocation map management either here or in the resource manager at detach time.
    // Zero textures are held by the Context, and we don't attempt to request them from
    // the State.
    mState.detachTexture(mZeroTextures, texture);
}

void Context::detachBuffer(GLuint buffer)
{
    // Buffer detachment is handled by Context, because the buffer must also be
    // attached from any VAOs in existence, and Context holds the VAO map.

    // [OpenGL ES 2.0.24] section 2.9 page 22:
    // If a buffer object is deleted while it is bound, all bindings to that object in the current context
    // (i.e. in the thread that called Delete-Buffers) are reset to zero.

    mState.removeArrayBufferBinding(buffer);

    // mark as freed among the vertex array objects
    for (auto &vaoPair : mVertexArrayMap)
    {
        vaoPair.second->detachBuffer(buffer);
    }
}

void Context::detachFramebuffer(GLuint framebuffer)
{
    // Framebuffer detachment is handled by Context, because 0 is a valid
    // Framebuffer object, and a pointer to it must be passed from Context
    // to State at binding time.

    // [OpenGL ES 2.0.24] section 4.4 page 107:
    // If a framebuffer that is currently bound to the target FRAMEBUFFER is deleted, it is as though
    // BindFramebuffer had been executed with the target of FRAMEBUFFER and framebuffer of zero.

    if (mState.removeReadFramebufferBinding(framebuffer) && framebuffer != 0)
    {
        bindReadFramebuffer(0);
    }

    if (mState.removeDrawFramebufferBinding(framebuffer) && framebuffer != 0)
    {
        bindDrawFramebuffer(0);
    }
}

void Context::detachRenderbuffer(GLuint renderbuffer)
{
    mState.detachRenderbuffer(renderbuffer);
}

void Context::detachVertexArray(GLuint vertexArray)
{
    // Vertex array detachment is handled by Context, because 0 is a valid
    // VAO, and a pointer to it must be passed from Context to State at
    // binding time.

    // [OpenGL ES 3.0.2] section 2.10 page 43:
    // If a vertex array object that is currently bound is deleted, the binding
    // for that object reverts to zero and the default vertex array becomes current.
    if (mState.removeVertexArrayBinding(vertexArray))
    {
        bindVertexArray(0);
    }
}

void Context::detachTransformFeedback(GLuint transformFeedback)
{
    mState.detachTransformFeedback(transformFeedback);
}

void Context::detachSampler(GLuint sampler)
{
    mState.detachSampler(sampler);
}

void Context::setVertexAttribDivisor(GLuint index, GLuint divisor)
{
    mState.setVertexAttribDivisor(index, divisor);
}

void Context::samplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    mResourceManager->checkSamplerAllocation(sampler);

    Sampler *samplerObject = getSampler(sampler);
    ASSERT(samplerObject);

    // clang-format off
    switch (pname)
    {
      case GL_TEXTURE_MIN_FILTER:         samplerObject->setMinFilter(static_cast<GLenum>(param));    break;
      case GL_TEXTURE_MAG_FILTER:         samplerObject->setMagFilter(static_cast<GLenum>(param));    break;
      case GL_TEXTURE_WRAP_S:             samplerObject->setWrapS(static_cast<GLenum>(param));        break;
      case GL_TEXTURE_WRAP_T:             samplerObject->setWrapT(static_cast<GLenum>(param));        break;
      case GL_TEXTURE_WRAP_R:             samplerObject->setWrapR(static_cast<GLenum>(param));        break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT: samplerObject->setMaxAnisotropy(std::min(static_cast<GLfloat>(param), getExtensions().maxTextureAnisotropy)); break;
      case GL_TEXTURE_MIN_LOD:            samplerObject->setMinLod(static_cast<GLfloat>(param));      break;
      case GL_TEXTURE_MAX_LOD:            samplerObject->setMaxLod(static_cast<GLfloat>(param));      break;
      case GL_TEXTURE_COMPARE_MODE:       samplerObject->setCompareMode(static_cast<GLenum>(param));  break;
      case GL_TEXTURE_COMPARE_FUNC:       samplerObject->setCompareFunc(static_cast<GLenum>(param));  break;
      default:                            UNREACHABLE(); break;
    }
    // clang-format on
}

void Context::samplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    mResourceManager->checkSamplerAllocation(sampler);

    Sampler *samplerObject = getSampler(sampler);
    ASSERT(samplerObject);

    // clang-format off
    switch (pname)
    {
      case GL_TEXTURE_MIN_FILTER:         samplerObject->setMinFilter(uiround<GLenum>(param));   break;
      case GL_TEXTURE_MAG_FILTER:         samplerObject->setMagFilter(uiround<GLenum>(param));   break;
      case GL_TEXTURE_WRAP_S:             samplerObject->setWrapS(uiround<GLenum>(param));       break;
      case GL_TEXTURE_WRAP_T:             samplerObject->setWrapT(uiround<GLenum>(param));       break;
      case GL_TEXTURE_WRAP_R:             samplerObject->setWrapR(uiround<GLenum>(param));       break;
      case GL_TEXTURE_MAX_ANISOTROPY_EXT: samplerObject->setMaxAnisotropy(std::min(param, getExtensions().maxTextureAnisotropy)); break;
      case GL_TEXTURE_MIN_LOD:            samplerObject->setMinLod(param);                       break;
      case GL_TEXTURE_MAX_LOD:            samplerObject->setMaxLod(param);                       break;
      case GL_TEXTURE_COMPARE_MODE:       samplerObject->setCompareMode(uiround<GLenum>(param)); break;
      case GL_TEXTURE_COMPARE_FUNC:       samplerObject->setCompareFunc(uiround<GLenum>(param)); break;
      default:                            UNREACHABLE(); break;
    }
    // clang-format on
}

GLint Context::getSamplerParameteri(GLuint sampler, GLenum pname)
{
    mResourceManager->checkSamplerAllocation(sampler);

    Sampler *samplerObject = getSampler(sampler);
    ASSERT(samplerObject);

    // clang-format off
    switch (pname)
    {
      case GL_TEXTURE_MIN_FILTER:         return static_cast<GLint>(samplerObject->getMinFilter());
      case GL_TEXTURE_MAG_FILTER:         return static_cast<GLint>(samplerObject->getMagFilter());
      case GL_TEXTURE_WRAP_S:             return static_cast<GLint>(samplerObject->getWrapS());
      case GL_TEXTURE_WRAP_T:             return static_cast<GLint>(samplerObject->getWrapT());
      case GL_TEXTURE_WRAP_R:             return static_cast<GLint>(samplerObject->getWrapR());
      case GL_TEXTURE_MAX_ANISOTROPY_EXT: return static_cast<GLint>(samplerObject->getMaxAnisotropy());
      case GL_TEXTURE_MIN_LOD:            return uiround<GLint>(samplerObject->getMinLod());
      case GL_TEXTURE_MAX_LOD:            return uiround<GLint>(samplerObject->getMaxLod());
      case GL_TEXTURE_COMPARE_MODE:       return static_cast<GLint>(samplerObject->getCompareMode());
      case GL_TEXTURE_COMPARE_FUNC:       return static_cast<GLint>(samplerObject->getCompareFunc());
      default:                            UNREACHABLE(); return 0;
    }
    // clang-format on
}

GLfloat Context::getSamplerParameterf(GLuint sampler, GLenum pname)
{
    mResourceManager->checkSamplerAllocation(sampler);

    Sampler *samplerObject = getSampler(sampler);
    ASSERT(samplerObject);

    // clang-format off
    switch (pname)
    {
      case GL_TEXTURE_MIN_FILTER:         return static_cast<GLfloat>(samplerObject->getMinFilter());
      case GL_TEXTURE_MAG_FILTER:         return static_cast<GLfloat>(samplerObject->getMagFilter());
      case GL_TEXTURE_WRAP_S:             return static_cast<GLfloat>(samplerObject->getWrapS());
      case GL_TEXTURE_WRAP_T:             return static_cast<GLfloat>(samplerObject->getWrapT());
      case GL_TEXTURE_WRAP_R:             return static_cast<GLfloat>(samplerObject->getWrapR());
      case GL_TEXTURE_MAX_ANISOTROPY_EXT: return samplerObject->getMaxAnisotropy();
      case GL_TEXTURE_MIN_LOD:            return samplerObject->getMinLod();
      case GL_TEXTURE_MAX_LOD:            return samplerObject->getMaxLod();
      case GL_TEXTURE_COMPARE_MODE:       return static_cast<GLfloat>(samplerObject->getCompareMode());
      case GL_TEXTURE_COMPARE_FUNC:       return static_cast<GLfloat>(samplerObject->getCompareFunc());
      default:                            UNREACHABLE(); return 0;
    }
    // clang-format on
}

void Context::initRendererString()
{
    std::ostringstream rendererString;
    rendererString << "ANGLE (";
    rendererString << mRenderer->getRendererDescription();
    rendererString << ")";

    mRendererString = MakeStaticString(rendererString.str());
}

const std::string &Context::getRendererString() const
{
    return mRendererString;
}

void Context::initExtensionStrings()
{
    mExtensionStrings = mExtensions.getStrings();

    std::ostringstream combinedStringStream;
    std::copy(mExtensionStrings.begin(), mExtensionStrings.end(), std::ostream_iterator<std::string>(combinedStringStream, " "));
    mExtensionString = combinedStringStream.str();
}

const std::string &Context::getExtensionString() const
{
    return mExtensionString;
}

const std::string &Context::getExtensionString(size_t idx) const
{
    return mExtensionStrings[idx];
}

size_t Context::getExtensionStringCount() const
{
    return mExtensionStrings.size();
}

void Context::initCaps(GLuint clientVersion)
{
    mCaps = mRenderer->getRendererCaps();

    mExtensions = mRenderer->getRendererExtensions();

    mLimitations = mRenderer->getRendererLimitations();

    if (clientVersion < 3)
    {
        // Disable ES3+ extensions
        mExtensions.colorBufferFloat = false;
    }

    if (clientVersion > 2)
    {
        // FIXME(geofflang): Don't support EXT_sRGB in non-ES2 contexts
        //mExtensions.sRGB = false;
    }

    // Apply implementation limits
    mCaps.maxVertexAttributes = std::min<GLuint>(mCaps.maxVertexAttributes, MAX_VERTEX_ATTRIBS);
    mCaps.maxVertexUniformBlocks = std::min<GLuint>(mCaps.maxVertexUniformBlocks, IMPLEMENTATION_MAX_VERTEX_SHADER_UNIFORM_BUFFERS);
    mCaps.maxVertexOutputComponents = std::min<GLuint>(mCaps.maxVertexOutputComponents, IMPLEMENTATION_MAX_VARYING_VECTORS * 4);

    mCaps.maxFragmentInputComponents = std::min<GLuint>(mCaps.maxFragmentInputComponents, IMPLEMENTATION_MAX_VARYING_VECTORS * 4);

    mCaps.compressedTextureFormats.clear();

    const TextureCapsMap &rendererFormats = mRenderer->getRendererTextureCaps();
    for (TextureCapsMap::const_iterator i = rendererFormats.begin(); i != rendererFormats.end(); i++)
    {
        GLenum format = i->first;
        TextureCaps formatCaps = i->second;

        const InternalFormat &formatInfo = GetInternalFormatInfo(format);

        // Update the format caps based on the client version and extensions.
        // Caps are AND'd with the renderer caps because some core formats are still unsupported in
        // ES3.
        formatCaps.texturable =
            formatCaps.texturable && formatInfo.textureSupport(clientVersion, mExtensions);
        formatCaps.renderable =
            formatCaps.renderable && formatInfo.renderSupport(clientVersion, mExtensions);
        formatCaps.filterable =
            formatCaps.filterable && formatInfo.filterSupport(clientVersion, mExtensions);

        // OpenGL ES does not support multisampling with integer formats
        if (!formatInfo.renderSupport || formatInfo.componentType == GL_INT || formatInfo.componentType == GL_UNSIGNED_INT)
        {
            formatCaps.sampleCounts.clear();
        }

        if (formatCaps.texturable && formatInfo.compressed)
        {
            mCaps.compressedTextureFormats.push_back(format);
        }

        mTextureCaps.insert(format, formatCaps);
    }
}

void Context::syncRendererState()
{
    const State::DirtyBits &dirtyBits = mState.getDirtyBits();
    if (dirtyBits.any())
    {
        mRenderer->syncState(mState, dirtyBits);
        mState.clearDirtyBits();
    }
}

void Context::syncRendererState(const State::DirtyBits &bitMask)
{
    const State::DirtyBits &dirtyBits = (mState.getDirtyBits() & bitMask);
    if (dirtyBits.any())
    {
        mRenderer->syncState(mState, dirtyBits);
        mState.clearDirtyBits(dirtyBits);
    }
}
}
