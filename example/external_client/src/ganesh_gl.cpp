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
#include "include/gpu/ganesh/GrDirectContext.h"
#include "include/gpu/ganesh/SkSurfaceGanesh.h"
#include "include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "include/gpu/ganesh/gl/GrGLInterface.h"
#include "include/encode/SkWebpEncoder.h"

#if defined(__linux__)
#include "include/gpu/ganesh/gl/glx/GrGLMakeGLXInterface.h"

#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#endif

#if defined(__APPLE__) && TARGET_OS_MAC == 1
#include "include/gpu/ganesh/gl/mac/GrGLMakeMacInterface.h"

#include "gl_context_helper.h"
#endif

#if defined(_MSC_VER)
#include <windows.h>
#include "include/gpu/ganesh/gl/win/GrGLMakeWinInterface.h"
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

#if defined(_MSC_VER)

// This was mostly taken from
// https://skia.googlesource.com/skia/+/f725a5ba8a29ec7c271805624d54755ce34968a1/tools/gpu/gl/win/CreatePlatformGLTestContext_win.cpp#57
bool initialize_gl_win() {
    HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(nullptr);
    WNDCLASS wc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hbrBackground = nullptr;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = (WNDPROC) DefWindowProc;
    wc.lpszClassName = TEXT("external_client");
    wc.lpszMenuName = nullptr;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

    ATOM cls = RegisterClass(&wc);
    if (!cls) {
        printf("Could not register window class.\n");
        return false;
    }

    HWND window;
    if (!(window = CreateWindow(TEXT("external_client"),
                                 TEXT("my-window"),
                                 CS_OWNDC,
                                 0, 0, 1, 1,
                                 nullptr, nullptr,
                                 hInstance, nullptr))) {
        printf("Could not create window.\n");
        return false;
    }

    HDC deviceContext;
    if (!(deviceContext = GetDC(window))) {
        printf("Could not get device context.\n");
        return false;
    }

    // Taken from https://www.khronos.org/opengl/wiki/Creating_an_OpenGL_Context_(WGL)
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    int pixelFormat = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pixelFormat, &pfd);

    HGLRC glrc;
    if (!(glrc = wglCreateContext(deviceContext))) {
        printf("Could not create rendering context.\n");
        return false;
    }

    if (!(wglMakeCurrent(deviceContext, glrc))) {
        printf("Could not set the context.\n");
        return false;
    }
    return true;
}
#endif  // defined(_MSC_VER)

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
    sk_sp<const GrGLInterface> iface = GrGLInterfaces::MakeGLX();
#elif defined(__APPLE__) && TARGET_OS_MAC == 1
    if (!initialize_gl_mac()) {
        return 1;
    }
    sk_sp<const GrGLInterface> iface = GrGLInterfaces::MakeMac();
#elif defined(_MSC_VER)
    if (!initialize_gl_win()) {
        return 1;
    }
    sk_sp<const GrGLInterface> iface = GrGLInterfaces::MakeWin();
#endif
    if (!iface) {
        printf("Could not make GL interface\n");
        return 1;
    }

    sk_sp<GrDirectContext> ctx = GrDirectContexts::MakeGL(iface, opts);
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
