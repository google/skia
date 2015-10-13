//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Fence.cpp: Implements the gl::FenceNV and gl::FenceSync classes, which support the GL_NV_fence
// extension and GLES3 sync objects.

#include "libANGLE/Fence.h"

#include "libANGLE/renderer/FenceNVImpl.h"
#include "libANGLE/renderer/FenceSyncImpl.h"
#include "libANGLE/renderer/Renderer.h"
#include "common/utilities.h"

#include "angle_gl.h"

namespace gl
{

FenceNV::FenceNV(rx::FenceNVImpl *impl)
    : mFence(impl),
      mIsSet(false),
      mStatus(GL_FALSE),
      mCondition(GL_NONE)
{
}

FenceNV::~FenceNV()
{
    SafeDelete(mFence);
}

Error FenceNV::set(GLenum condition)
{
    Error error = mFence->set(condition);
    if (error.isError())
    {
        return error;
    }

    mCondition = condition;
    mStatus = GL_FALSE;
    mIsSet = true;

    return Error(GL_NO_ERROR);
}

Error FenceNV::test(GLboolean *outResult)
{
    // Flush the command buffer by default
    Error error = mFence->test(&mStatus);
    if (error.isError())
    {
        return error;
    }

    *outResult = mStatus;
    return Error(GL_NO_ERROR);
}

Error FenceNV::finish()
{
    ASSERT(mIsSet);

    gl::Error error = mFence->finish();
    if (error.isError())
    {
        return error;
    }

    mStatus = GL_TRUE;

    return Error(GL_NO_ERROR);
}

FenceSync::FenceSync(rx::FenceSyncImpl *impl, GLuint id)
    : RefCountObject(id),
      mFence(impl),
      mCondition(GL_NONE),
      mFlags(0)
{
}

FenceSync::~FenceSync()
{
    SafeDelete(mFence);
}

Error FenceSync::set(GLenum condition, GLbitfield flags)
{
    Error error = mFence->set(condition, flags);
    if (error.isError())
    {
        return error;
    }

    mCondition = condition;
    mFlags = flags;
    return Error(GL_NO_ERROR);
}

Error FenceSync::clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult)
{
    ASSERT(mCondition != GL_NONE);
    return mFence->clientWait(flags, timeout, outResult);
}

Error FenceSync::serverWait(GLbitfield flags, GLuint64 timeout)
{
    return mFence->serverWait(flags, timeout);
}

Error FenceSync::getStatus(GLint *outResult) const
{
    return mFence->getStatus(outResult);
}

}
