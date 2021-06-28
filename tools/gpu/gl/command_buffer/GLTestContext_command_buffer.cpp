
/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_NO_COMMAND_BUFFER

#include "include/gpu/gl/GrGLAssembleInterface.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/private/SkMutex.h"
#include "include/private/SkOnce.h"
#include "src/ports/SkOSLibrary.h"
#include "tools/gpu/gl/command_buffer/GLTestContext_command_buffer.h"

namespace {

typedef void *EGLDisplay;
typedef unsigned int EGLBoolean;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;
typedef int32_t EGLint;
typedef void* EGLNativeDisplayType;
typedef void* EGLNativeWindowType;
typedef void (*__eglMustCastToProperFunctionPointerType)(void);
#define EGL_FALSE 0
#define EGL_TRUE 1
#define EGL_OPENGL_ES2_BIT 0x0004
#define EGL_OPENGL_ES3_BIT 0x0040
#define EGL_CONTEXT_CLIENT_VERSION 0x3098
#define EGL_NO_SURFACE ((EGLSurface)0)
#define EGL_NO_DISPLAY ((EGLDisplay)0)
#define EGL_NO_CONTEXT ((EGLContext)0)
#define EGL_DEFAULT_DISPLAY ((EGLNativeDisplayType)0)
#define EGL_SURFACE_TYPE 0x3033
#define EGL_PBUFFER_BIT 0x0001
#define EGL_RENDERABLE_TYPE 0x3040
#define EGL_RED_SIZE 0x3024
#define EGL_GREEN_SIZE 0x3023
#define EGL_BLUE_SIZE 0x3022
#define EGL_ALPHA_SIZE 0x3021
#define EGL_DEPTH_SIZE 0x3025
#define EGL_STENCIL_SIZE 0x3025
#define EGL_SAMPLES 0x3031
#define EGL_SAMPLE_BUFFERS 0x3032
#define EGL_NONE 0x3038
#define EGL_WIDTH 0x3057
#define EGL_HEIGHT 0x3056
#define EGL_DRAW 0x3059
#define EGL_READ 0x305A

typedef EGLDisplay (*GetDisplayProc)(EGLNativeDisplayType display_id);
typedef EGLBoolean (*InitializeProc)(EGLDisplay dpy, EGLint *major, EGLint *minor);
typedef EGLBoolean (*TerminateProc)(EGLDisplay dpy);
typedef EGLBoolean (*ChooseConfigProc)(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
typedef EGLBoolean (*GetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
typedef EGLSurface (*CreateWindowSurfaceProc)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
typedef EGLSurface (*CreatePbufferSurfaceProc)(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list);
typedef EGLBoolean (*DestroySurfaceProc)(EGLDisplay dpy, EGLSurface surface);
typedef EGLContext (*CreateContextProc)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
typedef EGLBoolean (*DestroyContextProc)(EGLDisplay dpy, EGLContext ctx);
typedef EGLBoolean (*MakeCurrentProc)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
typedef EGLBoolean (*SwapBuffersProc)(EGLDisplay dpy, EGLSurface surface);
typedef __eglMustCastToProperFunctionPointerType (*GetProcAddressProc)(const char* procname);

static GetDisplayProc gfGetDisplay = nullptr;
static InitializeProc gfInitialize = nullptr;
static TerminateProc gfTerminate = nullptr;
static ChooseConfigProc gfChooseConfig = nullptr;
static GetConfigAttrib gfGetConfigAttrib = nullptr;
static CreateWindowSurfaceProc gfCreateWindowSurface = nullptr;
static CreatePbufferSurfaceProc gfCreatePbufferSurface = nullptr;
static DestroySurfaceProc gfDestroySurface = nullptr;
static CreateContextProc gfCreateContext = nullptr;
static DestroyContextProc gfDestroyContext = nullptr;
static MakeCurrentProc gfMakeCurrent = nullptr;
static SwapBuffersProc gfSwapBuffers = nullptr;
static GetProcAddressProc gfGetProcAddress = nullptr;

static void* gLibrary = nullptr;
static bool gfFunctionsLoadedSuccessfully = false;

static void load_command_buffer_functions() {
    if (!gLibrary) {
        static constexpr const char* libName =
#if defined _WIN32
        "command_buffer_gles2.dll";
#elif defined SK_BUILD_FOR_MAC
        "libcommand_buffer_gles2.dylib";
#else
        "libcommand_buffer_gles2.so";
#endif // defined _WIN32
        gLibrary = SkLoadDynamicLibrary(libName);
        if (gLibrary) {
            gfGetDisplay = (GetDisplayProc)SkGetProcedureAddress(gLibrary, "eglGetDisplay");
            gfInitialize = (InitializeProc)SkGetProcedureAddress(gLibrary, "eglInitialize");
            gfTerminate = (TerminateProc)SkGetProcedureAddress(gLibrary, "eglTerminate");
            gfChooseConfig = (ChooseConfigProc)SkGetProcedureAddress(gLibrary, "eglChooseConfig");
            gfGetConfigAttrib = (GetConfigAttrib)SkGetProcedureAddress(gLibrary, "eglGetConfigAttrib");
            gfCreateWindowSurface = (CreateWindowSurfaceProc)SkGetProcedureAddress(gLibrary, "eglCreateWindowSurface");
            gfCreatePbufferSurface = (CreatePbufferSurfaceProc)SkGetProcedureAddress(gLibrary, "eglCreatePbufferSurface");
            gfDestroySurface = (DestroySurfaceProc)SkGetProcedureAddress(gLibrary, "eglDestroySurface");
            gfCreateContext = (CreateContextProc)SkGetProcedureAddress(gLibrary, "eglCreateContext");
            gfDestroyContext = (DestroyContextProc)SkGetProcedureAddress(gLibrary, "eglDestroyContext");
            gfMakeCurrent = (MakeCurrentProc)SkGetProcedureAddress(gLibrary, "eglMakeCurrent");
            gfSwapBuffers = (SwapBuffersProc)SkGetProcedureAddress(gLibrary, "eglSwapBuffers");
            gfGetProcAddress = (GetProcAddressProc)SkGetProcedureAddress(gLibrary, "eglGetProcAddress");

            gfFunctionsLoadedSuccessfully =
                    gfGetDisplay && gfInitialize && gfTerminate && gfChooseConfig &&
                    gfCreateWindowSurface && gfCreatePbufferSurface && gfDestroySurface &&
                    gfCreateContext && gfDestroyContext && gfMakeCurrent && gfSwapBuffers &&
                    gfGetProcAddress;
        }
    }
}

static GrGLFuncPtr command_buffer_get_gl_proc(void* ctx, const char name[]) {
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return gfGetProcAddress(name);
}

static void load_command_buffer_once() {
    static SkOnce once;
    once(load_command_buffer_functions);
}

static sk_sp<const GrGLInterface> create_command_buffer_interface() {
    load_command_buffer_once();
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return GrGLMakeAssembledGLESInterface(gLibrary, command_buffer_get_gl_proc);
}


// The command buffer does not correctly implement eglGetCurrent. It always returns EGL_NO_<foo>.
// So we implement them ourselves and hook eglMakeCurrent to store the current values in TLS.
class TLSCurrentObjects {
public:
    static EGLDisplay CurrentDisplay() {
        if (auto objects = Get()) {
            return objects->fDisplay;
        }
        return EGL_NO_DISPLAY;
    }

    static EGLSurface CurrentSurface(EGLint readdraw) {
        if (auto objects = Get()) {
            switch (readdraw) {
                case EGL_DRAW:
                    return objects->fDrawSurface;
                case EGL_READ:
                    return objects->fReadSurface;
                default:
                    return EGL_NO_SURFACE;
            }
        }
        return EGL_NO_SURFACE;
    }

    static EGLContext CurrentContext() {
        if (auto objects = Get()) {
            return objects->fContext;
        }
        return EGL_NO_CONTEXT;
    }

    static EGLBoolean MakeCurrent(EGLDisplay display, EGLSurface draw, EGLSurface read,
                                  EGLContext ctx) {
        if (gfFunctionsLoadedSuccessfully && EGL_TRUE == gfMakeCurrent(display, draw, read, ctx)) {
            if (auto objects = Get()) {
                objects->fDisplay = display;
                objects->fDrawSurface = draw;
                objects->fReadSurface = read;
                objects->fContext = ctx;
            }
            return EGL_TRUE;
        }
        return EGL_FALSE;

    }

private:
    EGLDisplay fDisplay = EGL_NO_DISPLAY;
    EGLSurface fReadSurface = EGL_NO_SURFACE;
    EGLSurface fDrawSurface = EGL_NO_SURFACE;
    EGLContext fContext = EGL_NO_CONTEXT;

    static TLSCurrentObjects* Get() {
        static thread_local TLSCurrentObjects objects;
        return &objects;
    }
};

std::function<void()> context_restorer() {
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    auto display = TLSCurrentObjects::CurrentDisplay();
    auto dsurface = TLSCurrentObjects::CurrentSurface(EGL_DRAW);
    auto rsurface = TLSCurrentObjects::CurrentSurface(EGL_READ);
    auto context = TLSCurrentObjects::CurrentContext();
    return [display, dsurface, rsurface, context] {
        TLSCurrentObjects::MakeCurrent(display, dsurface, rsurface, context);
    };
}

}  // anonymous namespace

namespace sk_gpu_test {

CommandBufferGLTestContext::CommandBufferGLTestContext(int version,
                                                       CommandBufferGLTestContext* shareContext)
    : fContext(EGL_NO_CONTEXT), fDisplay(EGL_NO_DISPLAY), fSurface(EGL_NO_SURFACE) {

    EGLint renderableType;
    switch (version) {
        case 2:
            renderableType = EGL_OPENGL_ES2_BIT;
            break;
        case 3:
            renderableType = EGL_OPENGL_ES3_BIT;
            break;
        default:
            SkDebugf("Command Buffer: Invalid version requested (%i). Must be either 2 or 3.\n",
                     version);
            return;
    }

    EGLint configAttribs[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RENDERABLE_TYPE, renderableType,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    static const EGLint surfaceAttribs[] = {
        EGL_WIDTH, 1,
        EGL_HEIGHT, 1,
        EGL_NONE
    };

    load_command_buffer_once();
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }

    fDisplay = gfGetDisplay(EGL_DEFAULT_DISPLAY);
    if (EGL_NO_DISPLAY == fDisplay) {
        SkDebugf("Command Buffer: Could not create EGL display.\n");
        return;
    }
    if (!gfInitialize(fDisplay, nullptr, nullptr)) {
        SkDebugf("Command Buffer: Could not initialize EGL display.\n");
        this->destroyGLContext();
        return;
    }
    EGLint numConfigs;
    if (!gfChooseConfig(fDisplay, configAttribs, static_cast<EGLConfig *>(&fConfig), 1,
                        &numConfigs) || numConfigs != 1) {
        SkDebugf("Command Buffer: Could not choose EGL config.\n");
        this->destroyGLContext();
        return;
    }

    fSurface = gfCreatePbufferSurface(fDisplay,
                                        static_cast<EGLConfig>(fConfig),
                                        surfaceAttribs);

    if (EGL_NO_SURFACE == fSurface) {
        SkDebugf("Command Buffer: Could not create EGL surface.\n");
        this->destroyGLContext();
        return;
    }

    EGLint contextAttribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, version,
        EGL_NONE
    };
    EGLContext eglShareContext = shareContext
            ? reinterpret_cast<EGLContext>(shareContext->fContext) : nullptr;
    fContext = gfCreateContext(fDisplay, static_cast<EGLConfig>(fConfig), eglShareContext,
                               contextAttribs);
    if (EGL_NO_CONTEXT == fContext) {
        SkDebugf("Command Buffer: Could not create EGL context.\n");
        this->destroyGLContext();
        return;
    }

    SkScopeExit restorer(context_restorer());
    if (!TLSCurrentObjects::MakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Command Buffer: Could not make EGL context current.\n");
        this->destroyGLContext();
        return;
    }

    auto gl = create_command_buffer_interface();
    if (!gl) {
        SkDebugf("Command Buffer: Could not create CommandBuffer GL interface.\n");
        this->destroyGLContext();
        return;
    }
    if (!gl->validate()) {
        SkDebugf("Command Buffer: Could not validate CommandBuffer GL interface.\n");
        this->destroyGLContext();
        return;
    }

    // libcommand_buffer_gles2 has not always respected the EGL_CONTEXT_CLIENT_VERSION attrib.
    // Double check that we are not using an old library and we actually got the context version we
    // asked for.
    const char* versionString = (const char*)gl->fFunctions.fGetString(GR_GL_VERSION);
    const char* expectedVersion = (version == 2) ? "OpenGL ES 2.0" : "OpenGL ES 3.0";
    if (strstr(versionString, expectedVersion) != versionString) {
        SkDebugf("Command Buffer: Unexpected version.\n"
                 "           Got: \"%s\".\n"
                 "      Expected: \"%s ...\".\n",
                 versionString, expectedVersion);
        return;
    }

    this->init(std::move(gl));
}

CommandBufferGLTestContext::~CommandBufferGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void CommandBufferGLTestContext::destroyGLContext() {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (EGL_NO_DISPLAY == fDisplay) {
        return;
    }
    bool wasCurrent = false;
    if (EGL_NO_CONTEXT != fContext) {
        wasCurrent = (TLSCurrentObjects::CurrentContext() == fContext);
        gfDestroyContext(fDisplay, fContext);
        fContext = EGL_NO_CONTEXT;
    }
    if (wasCurrent) {
        // Call MakeCurrent after destroying the context, so that the EGL implementation knows that
        // the context is not used anymore after it is released from being current.This way the
        // command buffer does not need to abandon the context before destruction, and no
        // client-side errors are printed.
        TLSCurrentObjects::MakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    }

    if (EGL_NO_SURFACE != fSurface) {
        gfDestroySurface(fDisplay, fSurface);
        fSurface = EGL_NO_SURFACE;
    }
    fDisplay = EGL_NO_DISPLAY;
}

void CommandBufferGLTestContext::onPlatformMakeNotCurrent() const {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (!TLSCurrentObjects::MakeCurrent(fDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
        SkDebugf("Command Buffer: Could not null out current EGL context.\n");
    }
}

void CommandBufferGLTestContext::onPlatformMakeCurrent() const {
    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (!TLSCurrentObjects::MakeCurrent(fDisplay, fSurface, fSurface, fContext)) {
        SkDebugf("Command Buffer: Could not make EGL context current.\n");
    }
}

std::function<void()> CommandBufferGLTestContext::onPlatformGetAutoContextRestore() const {
    if (!gfFunctionsLoadedSuccessfully || TLSCurrentObjects::CurrentContext() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr CommandBufferGLTestContext::onPlatformGetProcAddress(const char *name) const {
    if (!gfFunctionsLoadedSuccessfully) {
        return nullptr;
    }
    return gfGetProcAddress(name);
}

void CommandBufferGLTestContext::presentCommandBuffer() {
    if (this->gl()) {
        this->gl()->fFunctions.fFlush();
    }

    if (!gfFunctionsLoadedSuccessfully) {
        return;
    }
    if (!gfSwapBuffers(fDisplay, fSurface)) {
        SkDebugf("Command Buffer: Could not complete gfSwapBuffers.\n");
    }
}

bool CommandBufferGLTestContext::makeCurrent() {
    return TLSCurrentObjects::MakeCurrent(fDisplay, fSurface, fSurface, fContext) != EGL_FALSE;
}

int CommandBufferGLTestContext::getStencilBits() {
    EGLint result = 0;
    gfGetConfigAttrib(fDisplay, static_cast<EGLConfig>(fConfig), EGL_STENCIL_SIZE, &result);
    return result;
}

int CommandBufferGLTestContext::getSampleCount() {
    EGLint result = 0;
    gfGetConfigAttrib(fDisplay, static_cast<EGLConfig>(fConfig), EGL_SAMPLES, &result);
    return result;
}

}  // namespace sk_gpu_test
#endif // SK_NO_COMMAND_BUFFER
