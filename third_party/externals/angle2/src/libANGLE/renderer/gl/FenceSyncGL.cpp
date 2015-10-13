//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// FenceSyncGL.cpp: Implements the class methods for FenceSyncGL.

#include "libANGLE/renderer/gl/FenceSyncGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"

namespace rx
{

FenceSyncGL::FenceSyncGL(const FunctionsGL *functions)
    : FenceSyncImpl(),
      mFunctions(functions),
      mSyncObject(0)
{
    ASSERT(mFunctions);
}

FenceSyncGL::~FenceSyncGL()
{
    if (mSyncObject != 0)
    {
        mFunctions->deleteSync(mSyncObject);
    }
}

gl::Error FenceSyncGL::set(GLenum condition, GLbitfield flags)
{
    ASSERT(condition == GL_SYNC_GPU_COMMANDS_COMPLETE && flags == 0);
    mSyncObject = mFunctions->fenceSync(condition, flags);
    if (mSyncObject == 0)
    {
        // if glFenceSync fails, it returns 0.
        return gl::Error(GL_OUT_OF_MEMORY, "glFenceSync failed to create a GLsync object.");
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::clientWait(GLbitfield flags, GLuint64 timeout, GLenum *outResult)
{
    ASSERT(mSyncObject != 0);
    *outResult = mFunctions->clientWaitSync(mSyncObject, flags, timeout);
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::serverWait(GLbitfield flags, GLuint64 timeout)
{
    ASSERT(mSyncObject != 0);
    mFunctions->waitSync(mSyncObject, flags, timeout);
    return gl::Error(GL_NO_ERROR);
}

gl::Error FenceSyncGL::getStatus(GLint *outResult)
{
    ASSERT(mSyncObject != 0);
    mFunctions->getSynciv(mSyncObject, GL_SYNC_STATUS, 1, nullptr, outResult);
    return gl::Error(GL_NO_ERROR);
}

}
