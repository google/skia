
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "include/core/SkTypes.h"

#include "include/gpu/ganesh/gl/mac/GrGLMakeMacInterface.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <OpenGL/OpenGL.h>
#include <dlfcn.h>

// All of NSOpenGL is deprecated.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"

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
        kCGLPFAOpenGLProfile,
        (CGLPixelFormatAttribute) kCGLOGLPVersion_3_2_Core,
        kCGLPFADoubleBuffer,

        // eGPU parameters
        kCGLPFAAllowOfflineRenderers,  // Enables e-GPU.
        kCGLPFANoRecovery,  // Disallows software rendering.
        kCGLPFARendererID, (CGLPixelFormatAttribute)kCGLRendererATIRadeonX4000ID, // Select Radeon
        (CGLPixelFormatAttribute)NULL
    };
    static const int kFirstEGPUParameter = 3;
    SkASSERT(kCGLPFAAllowOfflineRenderers == attributes[kFirstEGPUParameter]);

    CGLPixelFormatObj pixFormat;
    GLint npix;
    CGLChoosePixelFormat(attributes, &pixFormat, &npix);

    if (nullptr == pixFormat) {
        // Move the NULL-termination up to remove the eGPU parameters and try again
        attributes[kFirstEGPUParameter] = (CGLPixelFormatAttribute)NULL;
        CGLChoosePixelFormat(attributes, &pixFormat, &npix);
    }
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

    auto gl = GrGLInterfaces::MakeMac();
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

#pragma clang diagnostic pop

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
