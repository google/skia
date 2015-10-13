//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence.h: Defines the gl::FenceNV and gl::FenceSync classes, which support the GL_NV_fence
// extension and GLES3 sync objects.

#ifndef LIBANGLE_FENCE_H_
#define LIBANGLE_FENCE_H_

#include "libANGLE/Error.h"
#include "libANGLE/RefCountObject.h"

#include "common/angleutils.h"

namespace rx
{
class FenceNVImpl;
class FenceSyncImpl;
}

namespace gl
{

class FenceNV final : angle::NonCopyable
{
  public:
    explicit FenceNV(rx::FenceNVImpl *impl);
    virtual ~FenceNV();

    Error set(GLenum condition);
    Error test(GLboolean *outResult);
    Error finish();

    bool isSet() const { return mIsSet; }
    GLboolean getStatus() const { return mStatus; }
    GLenum getCondition() const { return mCondition; }

  private:
    rx::FenceNVImpl *mFence;

    bool mIsSet;

    GLboolean mStatus;
    GLenum mCondition;
};

class FenceSync final : public RefCountObject
{
  public:
    explicit FenceSync(rx::FenceSyncImpl *impl, GLuint id);
    virtual ~FenceSync();

    Error set(GLenum condition, GLbitfield flags);
    Error clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult);
    Error serverWait(GLbitfield flags, GLuint64 timeout);
    Error getStatus(GLint *outResult) const;

    GLenum getCondition() const { return mCondition; }
    GLbitfield getFlags() const { return mFlags; }

  private:
    rx::FenceSyncImpl *mFence;

    GLenum mCondition;
    GLbitfield mFlags;
};

}

#endif   // LIBANGLE_FENCE_H_
