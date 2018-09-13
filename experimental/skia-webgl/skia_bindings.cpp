#include "GrBackendSurface.h"
#include "GrContext.h"
#include "GrContextPriv.h"
#include "GrGLInterface.h"
#include "GrGLTypes.h"
#include "SkCanvas.h"
#include "SkDiscretePathEffect.h"
#include "SkFontMgr.h"
#include "SkPaint.h"
#include "SkPath.h"
#include "SkSurface.h"
#include "SkSurfaceProps.h"

#include <iostream>
#include <GL/gl.h>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/html5.h>

using namespace emscripten;


sk_sp<SkSurface> makeWebGLSurface(std::string id, int width, int height) {
    SkDebugf("alpha\n");
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

    glViewport(0, 0, width, height);
    glClearColor(1, 1, 1, 1);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // setup GrContext
    auto interface = GrGLMakeNativeInterface();

    // setup contexts
    sk_sp<GrContext> grContext(GrContext::MakeGL(interface));

    SkDebugf("supported %d\n", grContext->colorTypeSupportedAsSurface(kRGBA_8888_SkColorType));
    SkDebugf("dump %s\n", grContext->contextPriv().dump().c_str());

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
    SkDebugf("delta\n");

    sk_sp<SkSurface> surface(SkSurface::MakeFromBackendRenderTarget(grContext.get(), target,
                                                                    kBottomLeft_GrSurfaceOrigin,
                                                                    colorType, nullptr, nullptr));
    SkDebugf("iota\n");
    return surface;
}

SkPath star() {
    const SkScalar R = 115.2f, C = 128.0f;
    SkPath path;
    path.moveTo(C + R, C);
    for (int i = 1; i < 8; ++i) {
        SkScalar a = 2.6927937f * i;
        path.lineTo(C + R * cos(a), C + R * sin(a));
    }
    return path;
}

bool webGLDemo(std::string htmlID) {
    auto surface = makeWebGLSurface(htmlID, 300, 300);
    if (!surface) {
        SkDebugf("Could not make surface\n");
        return false;
    }
    auto canvas = surface->getCanvas();
    if (!canvas) {
        SkDebugf("Could not get canvas\n");
        return false;
    }

    SkPaint paint;
    paint.setPathEffect(SkDiscretePathEffect::Make(10.0f, 4.0f));
    paint.setStyle(SkPaint::kStroke_Style);
    paint.setStrokeWidth(2.0f);
    paint.setAntiAlias(true);
    paint.setColor(0xff4281A4);
    canvas->clear(SK_ColorWHITE);
    SkPath path(star());
    canvas->drawPath(path, paint);

    return true;
}


EMSCRIPTEN_BINDINGS(Skia) {
    function("webGLDemo", &webGLDemo);
}