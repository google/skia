//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_test_instantiate.cpp: Adds support for filtering parameterized
// tests by platform, so we skip unsupported configs.

#include "test_utils/angle_test_instantiate.h"

#include <map>
#include <iostream>

#include "EGLWindow.h"
#include "OSWindow.h"
#include "test_utils/angle_test_configs.h"

namespace angle
{

bool IsPlatformAvailable(const PlatformParameters &param)
{
    switch (param.getRenderer())
    {
      case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
        break;

      case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
#ifndef ANGLE_ENABLE_D3D9
        return false;
#endif
        break;

      case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
#ifndef ANGLE_ENABLE_D3D11
        return false;
#endif
        break;

      case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
      case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
#ifndef ANGLE_ENABLE_OPENGL
        return false;
#endif
        break;

      default:
        UNREACHABLE();
        break;
    }

    static std::map<PlatformParameters, bool> paramAvailabilityCache;
    auto iter = paramAvailabilityCache.find(param);
    if (iter != paramAvailabilityCache.end())
    {
        return iter->second;
    }
    else
    {
        OSWindow *osWindow = CreateOSWindow();
        bool result = osWindow->initialize("CONFIG_TESTER", 1, 1);

        if (result)
        {
            EGLWindow *eglWindow =
                new EGLWindow(param.majorVersion, param.minorVersion, param.eglParameters);
            result = eglWindow->initializeGL(osWindow);

            eglWindow->destroyGL();
            SafeDelete(eglWindow);
        }

        osWindow->destroy();
        SafeDelete(osWindow);

        paramAvailabilityCache[param] = result;

        if (!result)
        {
            std::cout << "Skipping tests using configuration " << param << " because it is not available." << std::endl;
        }

        return result;
    }
}

}
