/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#ifdef SK_BUILD_FOR_MAC

#include "include/gpu/gl/GrGLInterface.h"

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/private/base/SkTemplates.h"

#include <dlfcn.h>
#include <memory>

sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    static const char kPath[] =
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib";
    std::unique_ptr<void, SkFunctionObject<dlclose>> lib(dlopen(kPath, RTLD_LAZY));
    return GrGLMakeAssembledGLInterface(lib.get(), [](void* ctx, const char* name) {
            return (GrGLFuncPtr)dlsym(ctx ? ctx : RTLD_DEFAULT, name); });
}

#endif  // SK_BUILD_FOR_MAC
