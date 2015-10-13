//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence11.h: Defines the rx::FenceNV11 and rx::FenceSync11 classes which implement rx::FenceNVImpl and rx::FenceSyncImpl.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_

#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/FenceSyncImpl.h"

namespace rx
{
class Renderer11;

class FenceNV11 : public FenceNVImpl
{
  public:
    explicit FenceNV11(Renderer11 *renderer);
    ~FenceNV11() override;

    gl::Error set(GLenum condition) override;
    gl::Error test(GLboolean *outFinished) override;
    gl::Error finish() override;

  private:
    template<class T> friend gl::Error FenceSetHelper(T *fence);
    template<class T> friend gl::Error FenceTestHelper(T *fence, bool flushCommandBuffer, GLboolean *outFinished);

    Renderer11 *mRenderer;
    ID3D11Query *mQuery;
};

class FenceSync11 : public FenceSyncImpl
{
  public:
    explicit FenceSync11(Renderer11 *renderer);
    ~FenceSync11() override;

    gl::Error set(GLenum condition, GLbitfield flags) override;
    gl::Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult) override;
    gl::Error serverWait(GLbitfield flags, GLuint64 timeout) override;
    gl::Error getStatus(GLint *outResult) override;

  private:
    template<class T> friend gl::Error FenceSetHelper(T *fence);
    template<class T> friend gl::Error FenceTestHelper(T *fence, bool flushCommandBuffer, GLboolean *outFinished);

    Renderer11 *mRenderer;
    ID3D11Query *mQuery;
    LONGLONG mCounterFrequency;
};

}

#endif // LIBANGLE_RENDERER_D3D_D3D11_FENCE11_H_
