/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrGLInterface.h"
#include "GrGLTypes.h"
#include "SkCanvas.h"
#include "SkCanvas.h"
#include "SkDiscretePathEffect.h"
#include "SkFontMgr.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkPathEffect.h"
#include "SkScalar.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"

#include <iostream>
#include <GL/gl.h>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

using namespace emscripten;

using JSColor = int32_t;

// Wraps the WebGL context in an SkSurface and returns it.
sk_sp<SkSurface> getWebGLSurface(std::string id, int width, int height) {
    // Context configurations
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = true;
    attrs.premultipliedAlpha = true;
    attrs.majorVersion = 1;
    attrs.enableExtensionsByDefault = true;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context(id.c_str(), &attrs);
    if (context < 0) {
        printf("failed to create webgl context %d\n", context);
        return nullptr;
    }
    EMSCRIPTEN_RESULT r = emscripten_webgl_make_context_current(context);
    if (r < 0) {
        printf("failed to make webgl current %d\n", r);
        return nullptr;
    }

    glClearColor(0, 0, 0, 0);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup GrContext
    auto interface = GrGLMakeNativeInterface();

    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));

    // Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
    // render to it
    GrGLint buffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &buffer);
    GrGLFramebufferInfo info;
    info.fFBOID = (GrGLuint) buffer;
    SkColorType colorType;

    info.fFormat = GL_RGBA8;
    colorType = kRGBA_8888_SkColorType;


    GrBackendRenderTarget target(width, height, 0, 8, info);

    SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, &props));
    return surface;
}

EMSCRIPTEN_BINDINGS(Skia) {
    function("_getWebGLSurface", &getWebGLSurface, allow_raw_pointers());
    function("MakeSkDiscretePathEffect", &SkDiscretePathEffect::Make, allow_raw_pointers());

    class_<SkSurface>("SkSurface")
        .smart_ptr<sk_sp<SkSurface>>("sk_sp<SkSurface>")
        .function("width", &SkSurface::width)
        .function("height", &SkSurface::height)
        .function("getCanvas", &SkSurface::getCanvas, allow_raw_pointers());

    class_<SkCanvas>("SkCanvas")
        .constructor<>()
        .function("clear", optional_override([](SkCanvas& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a lambda to change it out.
            return self.clear(SkColor(color));
        }))
        .function("drawPaint", &SkCanvas::drawPaint)
        .function("drawPath", &SkCanvas::drawPath)
        .function("drawRect", &SkCanvas::drawRect)
        .function("flush", &SkCanvas::flush)
        .function("save", &SkCanvas::save)
        .function("translate", &SkCanvas::translate);

    class_<SkPaint>("SkPaint")
        .constructor<>()
        .function("setAntiAlias", &SkPaint::setAntiAlias)
        .function("setColor", optional_override([](SkPaint& self, JSColor color)->void {
            // JS side gives us a signed int instead of an unsigned int for color
            // Add a lambda to change it out.
            return self.setColor(SkColor(color));
        }))
        .function("setPathEffect", &SkPaint::setPathEffect)
        .function("setShader", &SkPaint::setShader)
        .function("setStrokeWidth", &SkPaint::setStrokeWidth)
        .function("setStyle", &SkPaint::setStyle)
        .function("setTextSize", &SkPaint::setTextSize);

    class_<SkPathEffect>("SkPathEffect")
        .smart_ptr<sk_sp<SkPathEffect>>("sk_sp<SkPathEffect>");

    enum_<SkPaint::Style>("PaintStyle")
        .value("FILL",              SkPaint::Style::kFill_Style)
        .value("STROKE",            SkPaint::Style::kStroke_Style)
        .value("STROKE_AND_FILL",   SkPaint::Style::kStrokeAndFill_Style);


    //TODO make these chainable like PathKit
    class_<SkPath>("SkPath")
        .constructor<>()
        .constructor<const SkPath&>()
        .function("moveTo", select_overload<SkPath&(SkScalar, SkScalar)>(&SkPath::moveTo))
        .function("lineTo", select_overload<SkPath&(SkScalar, SkScalar)>(&SkPath::lineTo))
        .function("close", &SkPath::close);


}
