//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// global_state.cpp : Implements functions for querying the thread-local GL and EGL state.

#include "libGLESv2/global_state.h"

#include "libANGLE/Context.h"
#include "libANGLE/Error.h"

#include "common/debug.h"
#include "common/platform.h"
#include "common/tls.h"

namespace
{

static TLSIndex currentTLS = TLS_INVALID_INDEX;

struct Current
{
    EGLint error;
    EGLenum API;
    egl::Display *display;
    egl::Surface *drawSurface;
    egl::Surface *readSurface;
    gl::Context *context;
};

Current *AllocateCurrent()
{
    ASSERT(currentTLS != TLS_INVALID_INDEX);
    if (currentTLS == TLS_INVALID_INDEX)
    {
        return NULL;
    }

    Current *current = new Current();
    current->error = EGL_SUCCESS;
    current->API = EGL_OPENGL_ES_API;
    current->display = reinterpret_cast<egl::Display*>(EGL_NO_DISPLAY);
    current->drawSurface = reinterpret_cast<egl::Surface*>(EGL_NO_SURFACE);
    current->readSurface = reinterpret_cast<egl::Surface*>(EGL_NO_SURFACE);
    current->context = reinterpret_cast<gl::Context*>(EGL_NO_CONTEXT);

    if (!SetTLSValue(currentTLS, current))
    {
        ERR("Could not set thread local storage.");
        return NULL;
    }

    return current;
}

void DeallocateCurrent()
{
    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));
    SafeDelete(current);
    SetTLSValue(currentTLS, NULL);
}

Current *GetCurrentData()
{
    // Create a TLS index if one has not been created for this DLL
    if (currentTLS == TLS_INVALID_INDEX)
    {
        currentTLS = CreateTLSIndex();
    }

    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));

    // ANGLE issue 488: when the dll is loaded after thread initialization,
    // thread local storage (current) might not exist yet.
    return (current ? current : AllocateCurrent());
}

#ifdef ANGLE_PLATFORM_WINDOWS
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        currentTLS = CreateTLSIndex();
        if (currentTLS == TLS_INVALID_INDEX)
        {
            return FALSE;
        }
        AllocateCurrent();
        break;

      case DLL_THREAD_ATTACH:
        AllocateCurrent();
        break;

      case DLL_THREAD_DETACH:
        DeallocateCurrent();
        break;

      case DLL_PROCESS_DETACH:
        DeallocateCurrent();
        if (currentTLS != TLS_INVALID_INDEX)
        {
            DestroyTLSIndex(currentTLS);
            currentTLS = TLS_INVALID_INDEX;
        }
        break;
    }

    return TRUE;
}
#endif

}

namespace gl
{

Context *GetGlobalContext()
{
    Current *current = GetCurrentData();

    return current->context;
}

Context *GetValidGlobalContext()
{
    gl::Context *context = GetGlobalContext();
    if (context)
    {
        if (context->isContextLost())
        {
            context->recordError(gl::Error(GL_OUT_OF_MEMORY, "Context has been lost."));
            return nullptr;
        }
        else
        {
            return context;
        }
    }
    return nullptr;
}

}

namespace egl
{

void SetGlobalError(const Error &error)
{
    Current *current = GetCurrentData();

    current->error = error.getCode();
}

EGLint GetGlobalError()
{
    Current *current = GetCurrentData();

    return current->error;
}

EGLenum GetGlobalAPI()
{
    Current *current = GetCurrentData();

    return current->API;
}

void SetGlobalAPI(EGLenum API)
{
    Current *current = GetCurrentData();

    current->API = API;
}

void SetGlobalDisplay(Display *dpy)
{
    Current *current = GetCurrentData();

    current->display = dpy;
}

Display *GetGlobalDisplay()
{
    Current *current = GetCurrentData();

    return current->display;
}

void SetGlobalDrawSurface(Surface *surface)
{
    Current *current = GetCurrentData();

    current->drawSurface = surface;
}

Surface *GetGlobalDrawSurface()
{
    Current *current = GetCurrentData();

    return current->drawSurface;
}

void SetGlobalReadSurface(Surface *surface)
{
    Current *current = GetCurrentData();

    current->readSurface = surface;
}

Surface *GetGlobalReadSurface()
{
    Current *current = GetCurrentData();

    return current->readSurface;
}

void SetGlobalContext(gl::Context *context)
{
    Current *current = GetCurrentData();

    current->context = context;
}

gl::Context *GetGlobalContext()
{
    Current *current = GetCurrentData();

    return current->context;
}

}
