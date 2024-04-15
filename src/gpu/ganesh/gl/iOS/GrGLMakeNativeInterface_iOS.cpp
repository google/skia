/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"
#ifdef SK_BUILD_FOR_IOS

#include "include/gpu/ganesh/gl/ios/GrGLMakeIOSInterface.h"
#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/private/base/SkTemplates.h"

#include <dlfcn.h>
#include <memory>

namespace GrGLInterfaces {
sk_sp<const GrGLInterface> MakeIOS() {
    static const char kPath[] =
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib";
    std::unique_ptr<void, SkFunctionObject<dlclose>> lib(dlopen(kPath, RTLD_LAZY));
    return GrGLMakeAssembledGLESInterface(lib.get(), [](void* ctx, const char* name) {
            return (GrGLFuncPtr)dlsym(ctx ? ctx : RTLD_DEFAULT, name); });
}

}  // namespace GrGLInterfaces

#if !defined(SK_DISABLE_LEGACY_GL_MAKE_NATIVE_INTERFACE)
sk_sp<const GrGLInterface> GrGLMakeNativeInterface() {
    return GrGLInterfaces::MakeIOS();
}
#endif

#endif  // SK_BUILD_FOR_IOS
