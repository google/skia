//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.h : Defines functions for querying the thread-local GL and EGL state.

#ifndef LIBGLESV2_GLOBALSTATE_H_
#define LIBGLESV2_GLOBALSTATE_H_

#include <EGL/egl.h>

namespace gl
{
class Context;

Context *GetGlobalContext();
Context *GetValidGlobalContext();

}

namespace egl
{
class Error;
class Display;
class Surface;

void SetGlobalError(const Error &error);
EGLint GetGlobalError();

void SetGlobalAPI(EGLenum API);
EGLenum GetGlobalAPI();

void SetGlobalDisplay(Display *dpy);
Display *GetGlobalDisplay();

void SetGlobalDrawSurface(Surface *surface);
Surface *GetGlobalDrawSurface();

void SetGlobalReadSurface(Surface *surface);
Surface *GetGlobalReadSurface();

void SetGlobalContext(gl::Context *context);
gl::Context *GetGlobalContext();

}

#endif // LIBGLESV2_GLOBALSTATE_H_
