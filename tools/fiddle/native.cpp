/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "gl/GrGLInterface.h"
#include "gl/GrGLAssembleInterface.h"

#include <dlfcn.h>
#include <cstdio>
#include <cstring>

typedef unsigned char GLubyte;

typedef void* (*GLXGetCurrentContextProc)(void);
typedef GrGLFuncPtr (*EGLGetProcAddress)(const GLubyte* name);

class EGLLoader {
public:
    EGLLoader() {
        fLibrary = dlopen("libEGL.so.1", RTLD_LAZY);
        gLibrary = dlopen("libGLESv2.so.2", RTLD_LAZY);
        fprintf(stderr, "fLibrary: %p\n", fLibrary);
        fprintf(stderr, "gLibrary: %p\n", gLibrary);
    }
    ~EGLLoader() {
        if (fLibrary) {
            dlclose(fLibrary);
        }
        if (gLibrary) {
            dlclose(gLibrary);
        }
    }
    void* handle() const {
        return (nullptr == fLibrary) ? RTLD_DEFAULT : fLibrary;
    }
    void* handleg() const {
        return (nullptr == gLibrary) ? RTLD_DEFAULT : gLibrary;
    }
private:
    void* fLibrary;
    void* gLibrary;
};

class GLXProcGetter {
public:
    GLXProcGetter() {
    }
    GrGLFuncPtr getProc(const GLubyte* name) const {
        if (name[0] == 'e') {
            GrGLFuncPtr p = (GrGLFuncPtr) dlsym(fLoader.handle(), (const char *)name);
            fprintf(stderr, "name: %s proc: %p\n", name, p);
            return p;
        } else {
            GrGLFuncPtr p = (GrGLFuncPtr) dlsym(fLoader.handleg(), (const char *)name);
            fprintf(stderr, "name: %s proc: %p\n", name, p);
            return p;
        }
    }
private:
    EGLLoader fLoader;
};

static GrGLFuncPtr egl_get_gl_proc(void* ctx, const char name[]) {
    SkASSERT(nullptr != ctx);
    const GLXProcGetter* getter = (const GLXProcGetter*) ctx;
    return getter->getProc(reinterpret_cast<const GLubyte*>(name));
}

typedef void* EGLNativeDisplayType;
typedef void *EGLDisplay;
typedef EGLDisplay (*GetDisplayProc)(EGLNativeDisplayType display_id);
const GrGLInterface* GrGLCreateNativeInterface2() {
    GLXProcGetter getter;
    GetDisplayProc getProc = (GetDisplayProc)egl_get_gl_proc(&getter, "eglGetCurrentDisplay");
    EGLDisplay p = getProc((EGLNativeDisplayType)0);
    fprintf(stderr, "EGLDisplay: %p\n", p);

    return GrGLAssembleGLESInterface(&getter, egl_get_gl_proc);
}

