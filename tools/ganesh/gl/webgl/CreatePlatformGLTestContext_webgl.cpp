/*
 * Copyright 2026 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/gpu/ganesh/gl/GrGLMakeWebGLInterface.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/SkLog.h"
#include "tools/ganesh/gl/GLTestContext.h"

#include <emscripten/html5_webgl.h>

namespace {

std::function<void()> context_restorer() {
    const EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_get_current_context();
    return [context] { emscripten_webgl_make_context_current(context); };
}

// On the web, threads run in web workers, and OffscreenCanvas is the interface used for rendering
// inside web workers. emscripten_webgl_create_context() isn't used because it looks for a canvas in
// the DOM, and workers have no DOM.
EM_JS(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE, createOffscreenContextJs, (), {
    // Smallest possible default framebuffer, which tests don't (currently?) render to.
    const offscreenCanvas = new OffscreenCanvas(1, 1);
    return GL.createContext(offscreenCanvas, { majorVersion : 2 });
})

// Destroying via emscripten_webgl_destroy_context just marks the context for GC. Since our tests
// create and destroy contexts quickly, they can pile up while waiting for GC. Chrome limits the
// number of active contexts and evicts the oldest when the limit is reached, even if it's still in
// use. See <https://cs.chromium.org/ForciblyLoseOldestContext>.
//
// "Losing" the context explicitly frees its slot immediately, see
// <https://registry.khronos.org/webgl/extensions/WEBGL_lose_context/> namely: "This is the
// recommended mechanism for applications to programmatically halt their use of the WebGL API"
EM_JS(void, loseContextJs, (EMSCRIPTEN_WEBGL_CONTEXT_HANDLE handle), {
    GL.getContext(handle).GLctx.getExtension('WEBGL_lose_context')?.loseContext();
})

class WebGLTestContext : public sk_gpu_test::GLTestContext {
public:
    WebGLTestContext();
    ~WebGLTestContext() override;

private:
    void destroyGLContext();
    void onPlatformMakeNotCurrent() const override;
    void onPlatformMakeCurrent() const override;
    std::function<void()> onPlatformGetAutoContextRestore() const override;
    GrGLFuncPtr onPlatformGetProcAddress(const char*) const override;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE fContext;
};

WebGLTestContext::WebGLTestContext() : fContext(0) {
    fContext = createOffscreenContextJs();
    if (fContext == 0) {
        SKIA_LOG_E("createOffscreenContextJs failed");
        return;
    }

    SkScopeExit restorer(context_restorer());
    if (EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(fContext);
        res != EMSCRIPTEN_RESULT_SUCCESS) {
        SKIA_LOG_E("emscripten_webgl_make_context_current failed: %d", res);
        this->destroyGLContext();
        return;
    }

    // Needed for GPU workarounds.
    if (!emscripten_webgl_enable_extension(fContext, "WEBGL_debug_renderer_info")) {
        SKIA_LOG_W("WEBGL_debug_renderer_info extension not available");
    }

    sk_sp<const GrGLInterface> gl = GrGLInterfaces::MakeWebGL();
    if (!gl) {
        SKIA_LOG_E("Context could not create GL interface");
        this->destroyGLContext();
        return;
    } else if (!gl->validate()) {
        SKIA_LOG_E("Context could not validate GL interface");
        this->destroyGLContext();
        return;
    }

    this->init(std::move(gl));
}

WebGLTestContext::~WebGLTestContext() {
    this->teardown();
    this->destroyGLContext();
}

void WebGLTestContext::destroyGLContext() {
    if (fContext) {
        if (fContext == emscripten_webgl_get_current_context()) {
            if (auto res = emscripten_webgl_make_context_current(0);
                res != EMSCRIPTEN_RESULT_SUCCESS) {
                SKIA_LOG_E("destroyGLContext: emscripten_webgl_make_context_current failed: %d",
                           res);
            }
        }

        // Comes first because emscripten_webgl_destroy_context invalidates the handle
        loseContextJs(fContext);

        if (auto res = emscripten_webgl_destroy_context(fContext);
            res != EMSCRIPTEN_RESULT_SUCCESS) {
            SKIA_LOG_E("destroyGLContext: emscripten_webgl_destroy_context failed: %d", res);
        }

        fContext = 0;
    }
}

void WebGLTestContext::onPlatformMakeNotCurrent() const {
    if (auto res = emscripten_webgl_make_context_current(0); res != EMSCRIPTEN_RESULT_SUCCESS) {
        SKIA_LOG_E("onPlatformMakeNotCurrent: emscripten_webgl_make_context_current failed: %d",
                   res);
    }
}

void WebGLTestContext::onPlatformMakeCurrent() const {
    if (auto res = emscripten_webgl_make_context_current(fContext);
        res != EMSCRIPTEN_RESULT_SUCCESS) {
        SKIA_LOG_E("onPlatformMakeCurrent: emscripten_webgl_make_context_current failed: %d", res);
    }
}

std::function<void()> WebGLTestContext::onPlatformGetAutoContextRestore() const {
    if (emscripten_webgl_get_current_context() == fContext) {
        return nullptr;
    }
    return context_restorer();
}

GrGLFuncPtr WebGLTestContext::onPlatformGetProcAddress(const char* procName) const {
    // emscripten advises not using `*glGetProcAddress()` since it statically links in all GL entry
    // points, and calling a function returned via this route would be slower.
    // <https://emscripten.org/docs/optimizing/Optimizing-WebGL.html>
    return nullptr;
}

}  // anonymous namespace

namespace sk_gpu_test {
GLTestContext* CreatePlatformGLTestContext(GrGLStandard forcedGpuAPI, GLTestContext* shareContext) {
    if (!(forcedGpuAPI == kWebGL_GrGLStandard || forcedGpuAPI == kNone_GrGLStandard)) {
        return nullptr;
    }

    // WebGL has no share groups.
    if (shareContext) {
        return nullptr;
    }

    WebGLTestContext* ctx = new WebGLTestContext();
    if (!ctx->isValid()) {
        SKIA_LOG_E("CreatePlatformGLTestContext: webgl context validation failed");
        delete ctx;
        return nullptr;
    }

    return ctx;
}
}  // namespace sk_gpu_test
