/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"
#include "gl/GrGLUtil.h"

#include <dlfcn.h>

typedef unsigned char GLubyte;

typedef void* (*GLXGetCurrentContextProc)(void);
typedef GrGLFuncPtr (*GLXGetProcAddressProc)(const GLubyte* name);

class GLXLoader {
public:
    GLXLoader() {
        fLibrary = dlopen("libGL.so.1", RTLD_LAZY);
    }
    ~GLXLoader() {
        if (fLibrary) {
            dlclose(fLibrary);
        }
    }
    void* handle() const {
        return (nullptr == fLibrary) ? RTLD_DEFAULT : fLibrary;
    }
private:
    void* fLibrary;
};

class GLXProcGetter {
public:
    GLXProcGetter() {
        fGetCurrentContext = (GLXGetCurrentContextProc) dlsym(fLoader.handle(), "glXGetCurrentContext");
        fGetProcAddress = (GLXGetProcAddressProc) dlsym(fLoader.handle(), "glXGetProcAddress");
    }
    GrGLFuncPtr getProc(const GLubyte* name) const {
        return fGetProcAddress(name);
    }
    void* getCurrentContext(void) const {
        return fGetCurrentContext();
    }
private:
    GLXLoader fLoader;
    GLXGetCurrentContextProc fGetCurrentContext;
    GLXGetProcAddressProc fGetProcAddress;
};

static GrGLFuncPtr glx_get(void* ctx, const char name[]) {
    // Avoid calling glXGetProcAddress() for EGL procs.
    // We don't expect it to ever succeed, but somtimes it returns non-null anyway.
    if (0 == strncmp(name, "egl", 3)) {
        return nullptr;
    }

    SkASSERT(nullptr != ctx);
    const GLXProcGetter* getter = (const GLXProcGetter*) ctx;
    SkASSERT(nullptr != getter->getCurrentContext())
    return getter->getProc(reinterpret_cast<const GLubyte*>(name));
}

const GrGLInterface* GrGLCreateNativeInterface() {
    GLXProcGetter getter;

    if (nullptr == getter.getCurrentContext()) {
        return nullptr;
    }

    return GrGLAssembleInterface(&getter, glx_get);
}
