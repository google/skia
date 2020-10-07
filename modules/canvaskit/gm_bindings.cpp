/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <string>
#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>
#include "gm/gm.h"
#include "modules/canvaskit/WasmCommon.h"

#include "include/gpu/GrDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/gpu/gl/GrGLTypes.h"

using namespace emscripten;

/**
 * Returns a JS array of strings containing the names of the registered GMs. GMs are only registered
 * when their source is included in the "link" step, not if they are in something like libgm.a.
 * The names are also logged to the console.
 */
static JSArray ListGMs() {
    SkDebugf("Listing GMs\n");
    JSArray gms = emscripten::val::array();
    for (skiagm::GMFactory fact : skiagm::GMRegistry::Range()) {
        std::unique_ptr<skiagm::GM> gm(fact());
        SkDebugf("gm %s\n", gm->getName());
        gms.call<void>("push", std::string(gm->getName()));
    }
    return gms;
}

/**
 * Sets the given WebGL context to be "current" and then creates a GrDirectContext from that
 * context.
 */
sk_sp<GrDirectContext> MakeGrContext(EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context)
{
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl context current %d\n", r);
        return nullptr;
    }
    // setup GrDirectContext
    auto interface = GrGLMakeNativeInterface();
    // setup contexts
    sk_sp<GrDirectContext> dContext(GrDirectContext::MakeGL(interface));
    return dContext;
}

EMSCRIPTEN_BINDINGS(GMs) {
    function("ListGMs", &ListGMs);
    function("MakeGrContext", &MakeGrContext);

    class_<GrDirectContext>("GrDirectContext")
        .smart_ptr<sk_sp<GrDirectContext>>("sk_sp<GrDirectContext>");
}
