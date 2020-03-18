
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#include "tools/gpu/gl/GLTestContext.h"
#include "AvailabilityMacros.h"

#include <OpenGL/OpenGL.h>
#include <dlfcn.h>

namespace {

std::function<void()> context_restorer() {
    auto context = CGLGetCurrentContext();
    return [context] { CGLSetCurrentContext(context); };
}

class MacGLTestContext : public sk_gpu_test::GLTestContext {
public:
    MacGLTestContext(MacGLTestContext* shareContext);
    ~MacGLTestContext() override;

private:
    void destroyGLContext();

    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    CGLContextObj fContext;
    void* fGLLibrary;
};

MacGLTestContext::MacGLTestContext(MacGLTestContext* shareContext)
    : fContext(nullptr)
    , fGLLibrary(RTLD_DEFAULT) {
    // We first try to request a Radeon eGPU if one is available.
    // This will be a Radeon HD7000 and up, which includes all eGPU configs.
    // If that fails, we try again with only the base parameters.
    CGLPixelFormatAttribute attributes[] = {
        // base parameters
#if MAC_OS_X_VERSION_10_7
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
#endif
        kCGLPFADoubleBuffer,

#if MAC_OS_X_VERSION_10_8
        // eGPU parameters
        kCGLPFAAllowOfflineRenderers,  // Enables e-GPU.
        kCGLPFANoRecovery,  // Disallows software rendering.
        kCGLPFARendererID, (CGLPixelFormatAttribute)kCGLRendererATIRadeonX4000ID, // Select Radeon
#endif
        (CGLPixelFormatAttribute)NULL
    };
#if MAC_OS_X_VERSION_10_8
    static const int kFirstEGPUParameter = 3;
    SkASSERT(kCGLPFAAllowOfflineRenderers == attributes[kFirstEGPUParameter]);
#endif

    CGLPixelFormatObj pixFormat;
    GLint npix;
    CGLChoosePixelFormat(attributes, &pixFormat, &npix);

#if MAC_OS_X_VERSION_10_8
    if (nullptr == pixFormat) {
        // Move the NULL-termination up to remove the eGPU parameters and try again
        attributes[kFirstEGPUParameter] = (CGLPixelFormatAttribute)NULL;
        CGLChoosePixelFormat(attributes, &pixFormat, &npix);
    }
#endif
    if (nullptr == pixFormat) {
        SkDebugf("CGLChoosePixelFormat failed.");
        return;
    }

    CGLCreateContext(pixFormat, shareContext ? shareContext->fContext : nullptr, &fContext);
    CGLReleasePixelFormat(pixFormat);

    if (nullptr == fContext) {
        SkDebugf("CGLCreateContext failed.");
        return;
    }

    SkScopeExit restorer(context_restorer());
    CGLSetCurrentContext(fContext);

    auto gl = GrGLMakeNativeInterface();
    if (!gl) {
        SkDebugf("Context could not create GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Context could not validate GL interface.\n");
        this->destroyGLContext();
        return;
    }

    fGLLibrary = dlopen(
        "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib",
        RTLD_LAZY);

    this->init(std::move(gl));
}

MacGLTestContext::~MacGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void MacGLTestContext::destroyGLContext() {
    if (fContext) {
        if (CGLGetCurrentContext() == fContext) {
            // This will ensure that the context is immediately deleted.
            CGLSetCurrentContext(nullptr);
        }
        CGLReleaseContext(fContext);
        fContext = nullptr;
    }
    if (nullptr != fGLLibrary) {
        dlclose(fGLLibrary);
    }
}

void MacGLTestContext::onPlatformMakeNotCurrent() const {
    CGLSetCurrentContext(nullptr);
}

void MacGLTestContext::onPlatformMakeCurrent() const {
    CGLSetCurrentContext(fContext);
}

std::function<void()> MacGLTestContext::onPlatformGetAutoContextRestore() const {
    if (CGLGetCurrentContext() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr MacGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    void* handle = (nullptr == fGLLibrary) ? RTLD_DEFAULT : fGLLibrary;
    return reinterpret_cast<GrGLFuncPtr>(dlsym(handle, procName));
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI,
                                           GLTestContext* shareContext) {
    if (kGLES_GrGLStandard == forcedGpuAPI) {
        return nullptr;
    }
    MacGLTestContext* macShareContext = reinterpret_cast<MacGLTestContext*>(shareContext);
    MacGLTestContext* ctx = new MacGLTestContext(macShareContext);
    if (!ctx->isValid()) {
        delete ctx;
        return nullptr;
    }
    return ctx;
}
}  // namespace sk_gpu_test
