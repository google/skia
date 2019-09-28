/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "src/gpu/gl/GrGLUtil.h"

#include <dlfcn.h>

typedef void* (*GLXGetCurrentContextProc)(void);
typedef GrGLFuncPtr (*GLXGetProcAddressProc)(const GrGLubyte* name);

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
    GrGLFuncPtr getProc(const char name[]) const {
        if (!fGetProcAddress)
            return nullptr;
        return fGetProcAddress(reinterpret_cast<const GrGLubyte*>(name));
    }
    void* getCurrentContext(void) const {
        if (!fGetCurrentContext)
            return nullptr;
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
    SkASSERT(nullptr != getter->getCurrentContext());
    return getter->getProc(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    GLXProcGetter getter;

    if (nullptr == getter.getCurrentContext()) {
        return nullptr;
    }

    return GrGLMakeAssembledInterface(&getter, glx_get);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
