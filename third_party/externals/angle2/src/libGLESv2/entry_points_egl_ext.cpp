//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_ext.cpp : Implements the EGL extension entry points.

#include "libGLESv2/entry_points_egl_ext.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/Display.h"
#include "libANGLE/Device.h"
#include "libANGLE/Surface.h"
#include "libANGLE/validationEGL.h"

#include "common/debug.h"

namespace egl
{

// EGL_ANGLE_query_surface_pointer
EGLBoolean EGLAPIENTRY QuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, void **value = 0x%0.8p)",
          dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (!display->getExtensions().querySurfacePointer)
    {
        SetGlobalError(Error(EGL_SUCCESS));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // validate the attribute parameter
    switch (attribute)
    {
      case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
        if (!display->getExtensions().surfaceD3DTexture2DShareHandle)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        break;
      case EGL_DXGI_KEYED_MUTEX_ANGLE:
        if (!display->getExtensions().keyedMutex)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        break;
      default:
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    error = eglSurface->querySurfacePointerANGLE(attribute, value);
    SetGlobalError(error);
    return (error.isError() ? EGL_FALSE : EGL_TRUE);
}


// EGL_NV_post_sub_buffer
EGLBoolean EGLAPIENTRY PostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint x = %d, EGLint y = %d, EGLint width = %d, EGLint height = %d)", dpy, surface, x, y, width, height);

    if (x < 0 || y < 0 || width < 0 || height < 0)
    {
        SetGlobalError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    Error error = ValidateSurface(display, eglSurface);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    if (display->isDeviceLost())
    {
        SetGlobalError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        SetGlobalError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if (!display->getExtensions().postSubBuffer)
    {
        // Spec is not clear about how this should be handled.
        SetGlobalError(Error(EGL_SUCCESS));
        return EGL_TRUE;
    }

    error = eglSurface->postSubBuffer(x, y, width, height);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

// EGL_EXT_platform_base
EGLDisplay EGLAPIENTRY GetPlatformDisplayEXT(EGLenum platform, void *native_display, const EGLint *attrib_list)
{
    EVENT("(EGLenum platform = %d, void* native_display = 0x%0.8p, const EGLint* attrib_list = 0x%0.8p)",
          platform, native_display, attrib_list);

    const ClientExtensions &clientExtensions = Display::getClientExtensions();

    switch (platform)
    {
      case EGL_PLATFORM_ANGLE_ANGLE:
        if (!clientExtensions.platformANGLE)
        {
            SetGlobalError(Error(EGL_BAD_PARAMETER));
            return EGL_NO_DISPLAY;
        }
        break;

      default:
        SetGlobalError(Error(EGL_BAD_CONFIG));
        return EGL_NO_DISPLAY;
    }

    EGLint platformType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
    EGLint deviceType = EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE;
    bool majorVersionSpecified = false;
    bool minorVersionSpecified = false;
    bool enableAutoTrimSpecified = false;
    bool deviceTypeSpecified = false;

    if (attrib_list)
    {
        for (const EGLint *curAttrib = attrib_list; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            switch (curAttrib[0])
            {
              case EGL_PLATFORM_ANGLE_TYPE_ANGLE:
                switch (curAttrib[1])
                {
                  case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
                    break;

                  case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
                  case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
                    if (!clientExtensions.platformANGLED3D)
                    {
                        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
                        return EGL_NO_DISPLAY;
                    }
                    break;

                  case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
                  case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                    if (!clientExtensions.platformANGLEOpenGL)
                    {
                        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
                        return EGL_NO_DISPLAY;
                    }
                    break;

                  default:
                    SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
                    return EGL_NO_DISPLAY;
                }
                platformType = curAttrib[1];
                break;

              case EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE:
                if (curAttrib[1] != EGL_DONT_CARE)
                {
                    majorVersionSpecified = true;
                }
                break;

              case EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE:
                if (curAttrib[1] != EGL_DONT_CARE)
                {
                    minorVersionSpecified = true;
                }
                break;

              case EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE:
                switch (curAttrib[1])
                {
                  case EGL_TRUE:
                  case EGL_FALSE:
                    break;
                  default:
                    SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
                    return EGL_NO_DISPLAY;
                }
                enableAutoTrimSpecified = true;
                break;

              case EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE:
                switch (curAttrib[1])
                {
                  case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
                  case EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE:
                  case EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE:
                    deviceTypeSpecified = true;
                    break;

                  case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
                    // This is a hidden option, accepted by the OpenGL back-end.
                    break;

                  default:
                    SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
                    return EGL_NO_DISPLAY;
                }
                deviceType = curAttrib[1];
                break;

              default:
                break;
            }
        }
    }

    if (!majorVersionSpecified && minorVersionSpecified)
    {
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_NO_DISPLAY;
    }

    if (deviceType == EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE &&
        platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE, "EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE requires a device type of "
                                                "EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE."));
        return EGL_NO_DISPLAY;
    }

    if (enableAutoTrimSpecified &&
        platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE, "EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE requires a device type of "
                                                "EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE."));
        return EGL_NO_DISPLAY;
    }

    if (deviceTypeSpecified &&
        platformType != EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE &&
        platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
    {
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE, "EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE requires a device type of "
                                                "EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE or EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE."));
        return EGL_NO_DISPLAY;
    }

    SetGlobalError(Error(EGL_SUCCESS));

    EGLNativeDisplayType displayId = static_cast<EGLNativeDisplayType>(native_display);
    return Display::getDisplay(displayId, AttributeMap(attrib_list));
}

// EGL_EXT_device_query
EGLBoolean EGLAPIENTRY QueryDeviceAttribEXT(EGLDeviceEXT device, EGLint attribute, EGLAttrib *value)
{
    EVENT("(EGLDeviceEXT device = 0x%0.8p, EGLint attribute = %d, EGLAttrib *value = 0x%0.8p)",
          device, attribute, value);

    Device *dev = static_cast<Device*>(device);
    if (dev == EGL_NO_DEVICE_EXT)
    {
        SetGlobalError(Error(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    Display *display = dev->getDisplay();
    Error error(EGL_SUCCESS);

    if (!display->getExtensions().deviceQuery)
    {
        SetGlobalError(Error(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    // validate the attribute parameter
    switch (attribute)
    {
      case EGL_D3D11_DEVICE_ANGLE:
        if (!dev->getExtensions().deviceD3D || dev->getType() != EGL_D3D11_DEVICE_ANGLE)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        error = dev->getDevice(value);
        break;
      case EGL_D3D9_DEVICE_ANGLE:
        if (!dev->getExtensions().deviceD3D || dev->getType() != EGL_D3D9_DEVICE_ANGLE)
        {
            SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
            return EGL_FALSE;
        }
        error = dev->getDevice(value);
        break;
      default:
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    SetGlobalError(error);
    return (error.isError() ? EGL_FALSE : EGL_TRUE);
}

// EGL_EXT_device_query
const char * EGLAPIENTRY QueryDeviceStringEXT(EGLDeviceEXT device, EGLint name)
{
    EVENT("(EGLDeviceEXT device = 0x%0.8p, EGLint name = %d)",
          device, name);

    Device *dev = static_cast<Device*>(device);
    if (dev == EGL_NO_DEVICE_EXT)
    {
        SetGlobalError(Error(EGL_BAD_DEVICE_EXT));
        return nullptr;
    }

    const char *result;
    switch (name)
    {
      case EGL_EXTENSIONS:
        result = dev->getExtensionString().c_str();
        break;
      default:
        SetGlobalError(Error(EGL_BAD_DEVICE_EXT));
        return nullptr;
    }

    SetGlobalError(Error(EGL_SUCCESS));
    return result;
}

// EGL_EXT_device_query
EGLBoolean EGLAPIENTRY QueryDisplayAttribEXT(EGLDisplay dpy, EGLint attribute, EGLAttrib *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint attribute = %d, EGLAttrib *value = 0x%0.8p)",
          dpy, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Error error(EGL_SUCCESS);

    if (!display->getExtensions().deviceQuery)
    {
        SetGlobalError(Error(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    // validate the attribute parameter
    switch (attribute)
    {
      case EGL_DEVICE_EXT:
        *value = reinterpret_cast<EGLAttrib>(display->getDevice());
        break;

      default:
        SetGlobalError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    SetGlobalError(error);
    return (error.isError() ? EGL_FALSE : EGL_TRUE);
}

ANGLE_EXPORT EGLImageKHR EGLAPIENTRY CreateImageKHR(EGLDisplay dpy,
                                                    EGLContext ctx,
                                                    EGLenum target,
                                                    EGLClientBuffer buffer,
                                                    const EGLint *attrib_list)
{
    EVENT(
        "(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p, EGLenum target = 0x%X, "
        "EGLClientBuffer buffer = 0x%0.8p, const EGLAttrib *attrib_list = 0x%0.8p)",
        dpy, ctx, target, buffer, attrib_list);

    Display *display     = static_cast<Display *>(dpy);
    gl::Context *context = static_cast<gl::Context *>(ctx);
    AttributeMap attributes(attrib_list);

    Error error = ValidateCreateImageKHR(display, context, target, buffer, attributes);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_IMAGE;
    }

    Image *image = nullptr;
    error = display->createImage(context, target, buffer, attributes, &image);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_NO_IMAGE;
    }

    return static_cast<EGLImage>(image);
}

ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroyImageKHR(EGLDisplay dpy, EGLImageKHR image)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLImage image = 0x%0.8p)", dpy, image);

    Display *display = static_cast<Display *>(dpy);
    Image *img       = static_cast<Image *>(image);

    Error error = ValidateDestroyImageKHR(display, img);
    if (error.isError())
    {
        SetGlobalError(error);
        return EGL_FALSE;
    }

    display->destroyImage(img);

    return EGL_TRUE;
}
}
