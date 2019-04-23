/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include <dlfcn.h>

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
    GLProcGetter() {}

    GrGLFuncPtr getProc(const char name[]) const {
        return (GrGLFuncPtr) dlsym(fLoader.handle(), name);
    }

private:
    GLLoader fLoader;
};

static GrGLFuncPtr ios_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(ctx);
    const GLProcGetter* getter = (const GLProcGetter*) ctx;
    return getter->getProc(name);
}

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    GLProcGetter getter;
    return GrGLMakeAssembledGLESInterface(&getter, ios_get_gl_proc);
}

const GrGLInterface* GrGLCreateNativeInterface() { return GrGLMakeNativeInterface().release(); }
