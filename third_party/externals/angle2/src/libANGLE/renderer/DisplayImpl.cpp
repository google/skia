//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.cpp: Implementation methods of egl::Display

#include "libANGLE/renderer/DisplayImpl.h"

#include "libANGLE/Surface.h"

namespace rx
{

DisplayImpl::DisplayImpl()
    : mExtensionsInitialized(false),
      mCapsInitialized(false)
{
}

DisplayImpl::~DisplayImpl()
{
    while (!mSurfaceSet.empty())
    {
        destroySurface(*mSurfaceSet.begin());
    }
}

void DisplayImpl::destroySurface(egl::Surface *surface)
{
    mSurfaceSet.erase(surface);
    surface->onDestroy();
}

const egl::DisplayExtensions &DisplayImpl::getExtensions() const
{
    if (!mExtensionsInitialized)
    {
        generateExtensions(&mExtensions);
        mExtensionsInitialized = true;
    }

    return mExtensions;
}

const egl::Caps &DisplayImpl::getCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps);
        mCapsInitialized = true;
    }

    return mCaps;
}

}
