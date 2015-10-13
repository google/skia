//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer.cpp: Implements EGL dependencies for creating and destroying Renderer instances.

#include "common/utilities.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/Renderer.h"

#include <EGL/eglext.h>

namespace rx
{
Renderer::Renderer() : mCapsInitialized(false)
{
}

Renderer::~Renderer()
{
}

void Renderer::ensureCapsInitialized() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions, &mLimitations);
        mCapsInitialized = true;
    }
}

const gl::Caps &Renderer::getRendererCaps() const
{
    ensureCapsInitialized();

    return mCaps;
}

const gl::TextureCapsMap &Renderer::getRendererTextureCaps() const
{
    ensureCapsInitialized();

    return mTextureCaps;
}

const gl::Extensions &Renderer::getRendererExtensions() const
{
    ensureCapsInitialized();

    return mExtensions;
}

const gl::Limitations &Renderer::getRendererLimitations() const
{
    ensureCapsInitialized();

    return mLimitations;
}

}
