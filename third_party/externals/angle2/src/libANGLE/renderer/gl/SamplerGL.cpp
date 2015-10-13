//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerGL.cpp: Defines the rx::SamplerGL class, an implementation of SamplerImpl.

#include "libANGLE/renderer/gl/SamplerGL.h"

#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace
{

template <typename T>
static inline void SyncSamplerStateMember(const rx::FunctionsGL *functions,
                                          GLuint sampler,
                                          const gl::SamplerState &newState,
                                          gl::SamplerState &curState,
                                          GLenum name,
                                          T(gl::SamplerState::*samplerMember))
{
    if (curState.*samplerMember != newState.*samplerMember)
    {
        curState.*samplerMember = newState.*samplerMember;
        functions->samplerParameterf(sampler, name, static_cast<GLfloat>(curState.*samplerMember));
    }
}
}

namespace rx
{

SamplerGL::SamplerGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : SamplerImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mAppliedSamplerState(),
      mSamplerID(0)
{
    mFunctions->genSamplers(1, &mSamplerID);
}

SamplerGL::~SamplerGL()
{
    mStateManager->deleteSampler(mSamplerID);
    mSamplerID = 0;
}

void SamplerGL::syncState(const gl::SamplerState &samplerState) const
{
    // clang-format off
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_MIN_FILTER, &gl::SamplerState::minFilter);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_MAG_FILTER, &gl::SamplerState::magFilter);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_WRAP_S, &gl::SamplerState::wrapS);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_WRAP_T, &gl::SamplerState::wrapT);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_WRAP_R, &gl::SamplerState::wrapR);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_MAX_ANISOTROPY_EXT, &gl::SamplerState::maxAnisotropy);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_MIN_LOD, &gl::SamplerState::minLod);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_MAX_LOD, &gl::SamplerState::maxLod);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_COMPARE_MODE, &gl::SamplerState::compareMode);
    SyncSamplerStateMember(mFunctions, mSamplerID, samplerState, mAppliedSamplerState, GL_TEXTURE_COMPARE_FUNC, &gl::SamplerState::compareFunc);
    // clang-format on
}

GLuint SamplerGL::getSamplerID() const
{
    return mSamplerID;
}
}
