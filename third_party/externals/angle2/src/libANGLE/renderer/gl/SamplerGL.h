//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerGL.h: Defines the rx::SamplerGL class, an implementation of SamplerImpl.

#ifndef LIBANGLE_RENDERER_GL_SAMPLERGL_H_
#define LIBANGLE_RENDERER_GL_SAMPLERGL_H_

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/SamplerImpl.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;

class SamplerGL : public SamplerImpl
{
  public:
    SamplerGL(const FunctionsGL *functions, StateManagerGL *stateManager);
    ~SamplerGL() override;

    void syncState(const gl::SamplerState &samplerState) const;

    GLuint getSamplerID() const;

  private:
    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    mutable gl::SamplerState mAppliedSamplerState;
    GLuint mSamplerID;
};
}

#endif  // LIBANGLE_RENDERER_GL_SAMPLERGL_H_
