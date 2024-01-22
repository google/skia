/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkCanvas.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/gpu/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/gl/GrGLInterface.h"
#include "include/encode/SkWebpEncoder.h"

#if defined(__linux__)
#include "include/gpu/gl/glx/GrGLMakeGLXInterface.h"

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#endif

#include <cstdio>

#if defined(__linux__)

// Set up an X Display that can be rendered to GL. This will not be visible while
// the program runs. It is cribbed from how Skia's tooling sets itself up (e.g. viewer).
bool initialize_gl_linux() {
    Display* display = XOpenDisplay(nullptr);
    if (!display) {
        printf("Could not open an X display\n");
        return false;
    }
    static int constexpr kChooseFBConfigAtt[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DOUBLEBUFFER, True,
        GLX_STENCIL_SIZE, 8,
        None
    };
    int n;
    GLXFBConfig* fbConfig = glXChooseFBConfig(display, DefaultScreen(display), kChooseFBConfigAtt, &n);
    XVisualInfo* visualInfo;
    if (n > 0) {
        visualInfo = glXGetVisualFromFBConfig(display, *fbConfig);
    } else {
        // For some reason glXChooseVisual takes a non-const pointer to the attributes.
        int chooseVisualAtt[] = {
            GLX_RGBA,
            GLX_DOUBLEBUFFER,
            GLX_STENCIL_SIZE, 8,
            None
        };
        visualInfo = glXChooseVisual(display, DefaultScreen(display), chooseVisualAtt);
    }
    if (!visualInfo) {
        printf("Could not get X visualInfo\n");
        return false;
    }
    GLXContext glContext = glXCreateContext(display, visualInfo, nullptr, GL_TRUE);
    if (!glContext) {
        printf("Could not make GL X context\n");
        return false;
    }
    Colormap colorMap = XCreateColormap(display,
                                        RootWindow(display, visualInfo->screen),
                                        visualInfo->visual,
                                        AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = colorMap;
    swa.event_mask = 0;
    Window window = XCreateWindow(display,
                            RootWindow(display, visualInfo->screen),
                            0, 0, // x, y
                            1280, 960, // width, height
                            0, // border width
                            visualInfo->depth,
                            InputOutput,
                            visualInfo->visual,
                            CWEventMask | CWColormap,
                            &swa);

    if (!glXMakeCurrent(display, window, glContext)) {
        printf("Could not set GL X context to be the created one\n");
        return false;
    }
    return true;
}
#endif  // defined(__linux__)

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Usage: %s <name.webp>\n", argv[0]);
        return 1;
    }

    SkFILEWStream output(argv[1]);
    if (!output.isValid()) {
        printf("Cannot open output file %s\n", argv[1]);
        return 1;
    }

    GrContextOptions opts;
    opts.fSuppressPrints = true;
#if defined(__linux__)
    if (!initialize_gl_linux()) {
        return 1;
    }
    sk_sp<const GrGLInterface> interface = GrGLInterfaces::MakeGLX();
#endif
    if (!interface) {
        printf("Could not make GL interface\n");
        return 1;
    }

    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeGL(interface, opts);
    if (!ctx) {
        printf("Could not make GrDirectContext\n");
        return 1;
    }
    printf("Context made, now to make the surface\n");

    SkImageInfo imageInfo =
            SkImageInfo::Make(200, 400, kRGBA_8888_SkColorType, kPremul_SkAlphaType);

    sk_sp<SkSurface> surface =
            SkSurfaces::RenderTarget(ctx.get(), skgpu::Budgeted::kYes, imageInfo);
    if (!surface) {
        printf("Could not make surface from GL DirectContext\n");
        return 1;
    }

    SkCanvas* canvas = surface->getCanvas();
    canvas->clear(SK_ColorRED);
    SkRRect rrect = SkRRect::MakeRectXY(SkRect::MakeLTRB(10, 20, 50, 70), 10, 10);

    SkPaint paint;
    paint.setColor(SK_ColorBLUE);
    paint.setAntiAlias(true);

    canvas->drawRRect(rrect, paint);

    ctx->flush();

    printf("Drew to surface, now doing readback\n");
    sk_sp<SkImage> img = surface->makeImageSnapshot();
    sk_sp<SkData> webp = SkWebpEncoder::Encode(ctx.get(), img.get(), {});
    if (!webp) {
        printf("Readback of pixels (or encoding) failed\n");
        return 1;
    }
    output.write(webp->data(), webp->size());
    output.fsync();

    return 0;
}
