//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ResourceManager.h : Defines the ResourceManager class, which tracks objects
// shared by multiple GL contexts.

#ifndef LIBANGLE_RESOURCEMANAGER_H_
#define LIBANGLE_RESOURCEMANAGER_H_

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/HandleAllocator.h"

#include <map>

namespace rx
{
class ImplFactory;
}

namespace gl
{
class Buffer;
struct Data;
class FenceSync;
struct Limitations;
class Program;
class Renderbuffer;
class Sampler;
class Shader;
class Texture;

class ResourceManager : angle::NonCopyable
{
  public:
    explicit ResourceManager(rx::ImplFactory *factory);
    ~ResourceManager();

    void addRef();
    void release();

    GLuint createBuffer();
    GLuint createShader(const gl::Limitations &rendererLimitations, GLenum type);
    GLuint createProgram();
    GLuint createTexture();
    GLuint createRenderbuffer();
    GLuint createSampler();
    GLuint createFenceSync();

    void deleteBuffer(GLuint buffer);
    void deleteShader(GLuint shader);
    void deleteProgram(GLuint program);
    void deleteTexture(GLuint texture);
    void deleteRenderbuffer(GLuint renderbuffer);
    void deleteSampler(GLuint sampler);
    void deleteFenceSync(GLuint fenceSync);

    Buffer *getBuffer(GLuint handle);
    Shader *getShader(GLuint handle);
    Program *getProgram(GLuint handle) const;
    Texture *getTexture(GLuint handle);
    Renderbuffer *getRenderbuffer(GLuint handle);
    Sampler *getSampler(GLuint handle);
    FenceSync *getFenceSync(GLuint handle);

    void setRenderbuffer(GLuint handle, Renderbuffer *renderbuffer);

    void checkBufferAllocation(GLuint handle);
    void checkTextureAllocation(GLuint handle, GLenum type);
    void checkRenderbufferAllocation(GLuint handle);
    void checkSamplerAllocation(GLuint sampler);

    bool isSampler(GLuint sampler);

  private:
    void createTextureInternal(GLuint handle);

    rx::ImplFactory *mFactory;
    std::size_t mRefCount;

    typedef std::map<GLuint, Buffer*> BufferMap;
    BufferMap mBufferMap;
    HandleAllocator mBufferHandleAllocator;

    typedef std::map<GLuint, Shader*> ShaderMap;
    ShaderMap mShaderMap;

    typedef std::map<GLuint, Program*> ProgramMap;
    ProgramMap mProgramMap;
    HandleAllocator mProgramShaderHandleAllocator;

    typedef std::map<GLuint, Texture*> TextureMap;
    TextureMap mTextureMap;
    HandleAllocator mTextureHandleAllocator;

    typedef std::map<GLuint, Renderbuffer*> RenderbufferMap;
    RenderbufferMap mRenderbufferMap;
    HandleAllocator mRenderbufferHandleAllocator;

    typedef std::map<GLuint, Sampler*> SamplerMap;
    SamplerMap mSamplerMap;
    HandleAllocator mSamplerHandleAllocator;

    typedef std::map<GLuint, FenceSync*> FenceMap;
    FenceMap mFenceSyncMap;
    HandleAllocator mFenceSyncHandleAllocator;
};

}

#endif // LIBANGLE_RESOURCEMANAGER_H_
