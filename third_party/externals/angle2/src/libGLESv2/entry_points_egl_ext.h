//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl_ext.h : Defines the EGL extension entry points.

#ifndef LIBGLESV2_ENTRYPOINTSEGLEXT_H_
#define LIBGLESV2_ENTRYPOINTSEGLEXT_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <export.h>

namespace egl
{

// EGL_ANGLE_query_surface_pointer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY QuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value);

// EGL_NV_post_sub_buffer
ANGLE_EXPORT EGLBoolean EGLAPIENTRY PostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height);

// EGL_EXT_platform_base
ANGLE_EXPORT EGLDisplay EGLAPIENTRY GetPlatformDisplayEXT(EGLenum platform, void *native_display, const EGLint *attrib_list);

// EGL_EXT_device_query
ANGLE_EXPORT EGLBoolean EGLAPIENTRY QueryDisplayAttribEXT(EGLDisplay dpy, EGLint attribute, EGLAttrib *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY QueryDeviceAttribEXT(EGLDeviceEXT device, EGLint attribute, EGLAttrib *value);
ANGLE_EXPORT const char * EGLAPIENTRY QueryDeviceStringEXT(EGLDeviceEXT device, EGLint name);

// EGL_KHR_image_base/EGL_KHR_image
ANGLE_EXPORT EGLImageKHR EGLAPIENTRY CreateImageKHR(EGLDisplay dpy,
                                                    EGLContext ctx,
                                                    EGLenum target,
                                                    EGLClientBuffer buffer,
                                                    const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroyImageKHR(EGLDisplay dpy, EGLImageKHR image);
}

#endif // LIBGLESV2_ENTRYPOINTSEGLEXT_H_
