/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#if defined(SK_BUILD_FOR_MAC)


#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"

#include <dlfcn.h>

typedef void* (*CGLGetCurrentContextProc)(void);

class GLLoader {
public:
    GLLoader() {
        fLibrary = dlopen(
                    "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib",
                    RTLD_LAZY);
    }

    ~GLLoader() {
        if (fLibrary) {
            dlclose(fLibrary);
        }
    }

    void* handle() const {
        return nullptr == fLibrary ? RTLD_DEFAULT : fLibrary;
    }

private:
    void* fLibrary;
};

class GLProcGetter {
public:
    GLProcGetter() {
        fGetCurrentContext = (CGLGetCurrentContextProc)getProc("CGLGetCurrentContext");
    }

    bool isInitialized() const {
        return SkToBool(fLoader.handle() && fGetCurrentContext);
    }

    GrGLFuncPtr getProc(const char name[]) const {
        return (GrGLFuncPtr) dlsym(fLoader.handle(), name);
    }

    void* getCurrentContext() const {
        if (!fGetCurrentContext)
            return nullptr;
        return fGetCurrentContext();
    }
private:
    GLLoader fLoader;
    CGLGetCurrentContextProc fGetCurrentContext;
};

static GrGLFuncPtr mac_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(ctx);
    const GLProcGetter* getter = (const GLProcGetter*) ctx;
    SkASSERT(getter->getCurrentContext());
    return getter->getProc(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    GLProcGetter getter;
    if (!getter.isInitialized())
        return nullptr;
    if (!getter.getCurrentContext())
        return nullptr;
    return GrGLMakeAssembledGLInterface(&getter, mac_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }

#endif//defined(SK_BUILD_FOR_MAC)
