//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeviceD3D.cpp: D3D implementation of egl::Device

#include "libANGLE/renderer/d3d/DeviceD3D.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

#include "libANGLE/Device.h"
#include "libANGLE/Display.h"

#include <EGL/eglext.h>

namespace rx
{

DeviceD3D::DeviceD3D(rx::RendererD3D *renderer)
    : mRenderer(renderer)
{
}

egl::Error DeviceD3D::getDevice(EGLAttrib *value)
{
    *value = reinterpret_cast<EGLAttrib>(mRenderer->getD3DDevice());
    if (*value == 0)
    {
        return egl::Error(EGL_BAD_DEVICE_EXT);
    }
    return egl::Error(EGL_SUCCESS);
}

EGLint DeviceD3D::getType()
{
    switch (mRenderer->getRendererClass())
    {
      case RENDERER_D3D11:
        return EGL_D3D11_DEVICE_ANGLE;
      case RENDERER_D3D9:
        return EGL_D3D9_DEVICE_ANGLE;
      default:
        UNREACHABLE();
        return EGL_NONE;
    }
}

void DeviceD3D::generateExtensions(egl::DeviceExtensions *outExtensions) const
{
    outExtensions->deviceD3D = true;
}

}
