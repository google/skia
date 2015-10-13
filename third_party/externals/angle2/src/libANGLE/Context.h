//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Context.h: Defines the gl::Context class, managing all GL state and performing
// rendering operations. It is the GLES2 specific implementation of EGLContext.

#ifndef LIBANGLE_CONTEXT_H_
#define LIBANGLE_CONTEXT_H_

#include "common/angleutils.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Constants.h"
#include "libANGLE/Data.h"
#include "libANGLE/Error.h"
#include "libANGLE/HandleAllocator.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/angletypes.h"

#include "angle_gl.h"

#include <string>
#include <set>
#include <map>

namespace rx
{
class Renderer;
}

namespace egl
{
class Surface;
struct Config;
}

namespace gl
{
class Compiler;
class Shader;
class Program;
class Texture;
class Framebuffer;
class Renderbuffer;
class FenceNV;
class FenceSync;
class Query;
class ResourceManager;
class Buffer;
struct VertexAttribute;
class VertexArray;
class Sampler;
class TransformFeedback;

class Context final : angle::NonCopyable
{
  public:
    Context(const egl::Config *config, int clientVersion, const Context *shareContext, rx::Renderer *renderer, bool notifyResets, bool robustAccess);

    virtual ~Context();

    void makeCurrent(egl::Surface *surface);
    void releaseSurface();

    virtual void markContextLost();
    bool isContextLost();

    // These create  and destroy methods are merely pass-throughs to
    // ResourceManager, which owns these object types
    GLuint createBuffer();
    GLuint createShader(GLenum type);
    GLuint createProgram();
    GLuint createTexture();
    GLuint createRenderbuffer();
    GLuint createSampler();
    GLuint createTransformFeedback();
    GLsync createFenceSync();

    void deleteBuffer(GLuint buffer);
    void deleteShader(GLuint shader);
    void deleteProgram(GLuint program);
    void deleteTexture(GLuint texture);
    void deleteRenderbuffer(GLuint renderbuffer);
    void deleteSampler(GLuint sampler);
    void deleteTransformFeedback(GLuint transformFeedback);
    void deleteFenceSync(GLsync fenceSync);

    // Framebuffers are owned by the Context, so these methods do not pass through
    GLuint createFramebuffer();
    void deleteFramebuffer(GLuint framebuffer);

    // NV Fences are owned by the Context.
    GLuint createFenceNV();
    void deleteFenceNV(GLuint fence);

    // Queries are owned by the Context;
    GLuint createQuery();
    void deleteQuery(GLuint query);

    // Vertex arrays are owned by the Context
    GLuint createVertexArray();
    void deleteVertexArray(GLuint vertexArray);

    void bindArrayBuffer(GLuint buffer);
    void bindElementArrayBuffer(GLuint buffer);
    void bindTexture(GLenum target, GLuint handle);
    void bindReadFramebuffer(GLuint framebuffer);
    void bindDrawFramebuffer(GLuint framebuffer);
    void bindRenderbuffer(GLuint renderbuffer);
    void bindVertexArray(GLuint vertexArray);
    void bindSampler(GLuint textureUnit, GLuint sampler);
    void bindGenericUniformBuffer(GLuint buffer);
    void bindIndexedUniformBuffer(GLuint buffer, GLuint index, GLintptr offset, GLsizeiptr size);
    void bindGenericTransformFeedbackBuffer(GLuint buffer);
    void bindIndexedTransformFeedbackBuffer(GLuint buffer, GLuint index, GLintptr offset, GLsizeiptr size);
    void bindCopyReadBuffer(GLuint buffer);
    void bindCopyWriteBuffer(GLuint buffer);
    void bindPixelPackBuffer(GLuint buffer);
    void bindPixelUnpackBuffer(GLuint buffer);
    void useProgram(GLuint program);
    void bindTransformFeedback(GLuint transformFeedback);

    Error beginQuery(GLenum target, GLuint query);
    Error endQuery(GLenum target);

    void setVertexAttribDivisor(GLuint index, GLuint divisor);

    void samplerParameteri(GLuint sampler, GLenum pname, GLint param);
    void samplerParameterf(GLuint sampler, GLenum pname, GLfloat param);
    GLint getSamplerParameteri(GLuint sampler, GLenum pname);
    GLfloat getSamplerParameterf(GLuint sampler, GLenum pname);

    Buffer *getBuffer(GLuint handle);
    FenceNV *getFenceNV(GLuint handle);
    FenceSync *getFenceSync(GLsync handle) const;
    Shader *getShader(GLuint handle) const;
    Program *getProgram(GLuint handle) const;
    Texture *getTexture(GLuint handle) const;
    Framebuffer *getFramebuffer(GLuint handle) const;
    Renderbuffer *getRenderbuffer(GLuint handle);
    VertexArray *getVertexArray(GLuint handle) const;
    Sampler *getSampler(GLuint handle) const;
    Query *getQuery(GLuint handle, bool create, GLenum type);
    TransformFeedback *getTransformFeedback(GLuint handle) const;

    Texture *getTargetTexture(GLenum target) const;
    Texture *getSamplerTexture(unsigned int sampler, GLenum type) const;

    Compiler *getCompiler() const;

    bool isSampler(GLuint samplerName) const;

    void getBooleanv(GLenum pname, GLboolean *params);
    void getFloatv(GLenum pname, GLfloat *params);
    void getIntegerv(GLenum pname, GLint *params);
    void getInteger64v(GLenum pname, GLint64 *params);

    bool getIndexedIntegerv(GLenum target, GLuint index, GLint *data);
    bool getIndexedInteger64v(GLenum target, GLuint index, GLint64 *data);

    bool getQueryParameterInfo(GLenum pname, GLenum *type, unsigned int *numParams);
    bool getIndexedQueryParameterInfo(GLenum target, GLenum *type, unsigned int *numParams);

    Error drawArrays(GLenum mode, GLint first, GLsizei count);
    Error drawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount);

    Error drawElements(GLenum mode,
                       GLsizei count,
                       GLenum type,
                       const GLvoid *indices,
                       const IndexRange &indexRange);
    Error drawElementsInstanced(GLenum mode,
                                GLsizei count,
                                GLenum type,
                                const GLvoid *indices,
                                GLsizei instances,
                                const IndexRange &indexRange);
    Error drawRangeElements(GLenum mode,
                            GLuint start,
                            GLuint end,
                            GLsizei count,
                            GLenum type,
                            const GLvoid *indices,
                            const IndexRange &indexRange);

    Error flush();
    Error finish();

    void insertEventMarker(GLsizei length, const char *marker);
    void pushGroupMarker(GLsizei length, const char *marker);
    void popGroupMarker();

    void recordError(const Error &error);

    GLenum getError();
    GLenum getResetStatus();
    virtual bool isResetNotificationEnabled();

    virtual int getClientVersion() const;

    const egl::Config *getConfig() const;
    EGLenum getClientType() const;
    EGLenum getRenderBuffer() const;

    const Caps &getCaps() const;
    const TextureCapsMap &getTextureCaps() const;
    const Extensions &getExtensions() const;
    const Limitations &getLimitations() const;

    const std::string &getRendererString() const;

    const std::string &getExtensionString() const;
    const std::string &getExtensionString(size_t idx) const;
    size_t getExtensionStringCount() const;

    rx::Renderer *getRenderer() { return mRenderer; }

    State &getState() { return mState; }
    const State &getState() const { return mState; }

    const Data &getData() const { return mData; }
    void syncRendererState();
    void syncRendererState(const State::DirtyBits &bitMask);

  private:
    void detachBuffer(GLuint buffer);
    void detachTexture(GLuint texture);
    void detachFramebuffer(GLuint framebuffer);
    void detachRenderbuffer(GLuint renderbuffer);
    void detachVertexArray(GLuint vertexArray);
    void detachTransformFeedback(GLuint transformFeedback);
    void detachSampler(GLuint sampler);

    void initRendererString();
    void initExtensionStrings();

    void initCaps(GLuint clientVersion);

    // Caps to use for validation
    Caps mCaps;
    TextureCapsMap mTextureCaps;
    Extensions mExtensions;
    Limitations mLimitations;

    // Shader compiler
    Compiler *mCompiler;

    rx::Renderer *const mRenderer;
    State mState;

    int mClientVersion;

    const egl::Config *mConfig;
    EGLenum mClientType;

    TextureMap mZeroTextures;

    typedef std::map<GLuint, Framebuffer*> FramebufferMap;
    FramebufferMap mFramebufferMap;
    HandleAllocator mFramebufferHandleAllocator;

    typedef std::map<GLuint, FenceNV*> FenceNVMap;
    FenceNVMap mFenceNVMap;
    HandleAllocator mFenceNVHandleAllocator;

    typedef std::map<GLuint, Query*> QueryMap;
    QueryMap mQueryMap;
    HandleAllocator mQueryHandleAllocator;

    typedef std::map<GLuint, VertexArray*> VertexArrayMap;
    VertexArrayMap mVertexArrayMap;
    HandleAllocator mVertexArrayHandleAllocator;

    BindingPointer<TransformFeedback> mTransformFeedbackZero;
    typedef std::map<GLuint, TransformFeedback*> TransformFeedbackMap;
    TransformFeedbackMap mTransformFeedbackMap;
    HandleAllocator mTransformFeedbackAllocator;

    std::string mRendererString;
    std::string mExtensionString;
    std::vector<std::string> mExtensionStrings;

    // Recorded errors
    typedef std::set<GLenum> ErrorSet;
    ErrorSet mErrors;

    // Current/lost context flags
    bool mHasBeenCurrent;
    bool mContextLost;
    GLenum mResetStatus;
    GLenum mResetStrategy;
    bool mRobustAccess;
    egl::Surface *mCurrentSurface;

    ResourceManager *mResourceManager;

    // Cache the Data object to avoid re-calling the constructor
    Data mData;
};

}

#endif   // LIBANGLE_CONTEXT_H_
