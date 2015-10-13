//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_egl.h : Defines the EGL entry points.

#ifndef LIBGLESV2_ENTRYPOINTSEGL_H_
#define LIBGLESV2_ENTRYPOINTSEGL_H_

#include <EGL/egl.h>
#include <export.h>

namespace egl
{

// EGL 1.0
ANGLE_EXPORT EGLBoolean EGLAPIENTRY ChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
ANGLE_EXPORT EGLContext EGLAPIENTRY CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroyContext(EGLDisplay dpy, EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroySurface(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY GetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY GetCurrentDisplay(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY GetCurrentSurface(EGLint readdraw);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY GetDisplay(EGLNativeDisplayType display_id);
ANGLE_EXPORT EGLint EGLAPIENTRY GetError(void);
ANGLE_EXPORT __eglMustCastToProperFunctionPointerType EGLAPIENTRY GetProcAddress(const char *procname);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY Initialize(EGLDisplay dpy, EGLint *major, EGLint *minor);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
ANGLE_EXPORT const char *EGLAPIENTRY QueryString(EGLDisplay dpy, EGLint name);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY QuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY SwapBuffers(EGLDisplay dpy, EGLSurface surface);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY Terminate(EGLDisplay dpy);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY WaitGL(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY WaitNative(EGLint engine);

// EGL 1.1
ANGLE_EXPORT EGLBoolean EGLAPIENTRY BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY SwapInterval(EGLDisplay dpy, EGLint interval);

// EGL 1.2
ANGLE_EXPORT EGLBoolean EGLAPIENTRY BindAPI(EGLenum api);
ANGLE_EXPORT EGLenum EGLAPIENTRY QueryAPI(void);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY ReleaseThread(void);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY WaitClient(void);

// EGL 1.4
ANGLE_EXPORT EGLContext EGLAPIENTRY GetCurrentContext(void);

// EGL 1.5
ANGLE_EXPORT EGLSync EGLAPIENTRY CreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroySync(EGLDisplay dpy, EGLSync sync);
ANGLE_EXPORT EGLint EGLAPIENTRY ClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY GetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value);
ANGLE_EXPORT EGLImage EGLAPIENTRY CreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY DestroyImage(EGLDisplay dpy, EGLImage image);
ANGLE_EXPORT EGLDisplay EGLAPIENTRY GetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLSurface EGLAPIENTRY CreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list);
ANGLE_EXPORT EGLBoolean EGLAPIENTRY WaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);

}

#endif // LIBGLESV2_ENTRYPOINTSEGL_H_
