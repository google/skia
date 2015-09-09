/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#include <v8.h>
#include <include/libplatform/libplatform.h>

#include "SkV8Example.h"
#include "Global.h"
#include "JsContext.h"
#include "Path2D.h"
#include "Path2DBuilder.h"

#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "GrRenderTarget.h"
#include "GrContext.h"
#include "SkApplication.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDraw.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"
#include "SkScalar.h"
#include "SkSurface.h"


DEFINE_string2(infile, i, NULL, "Name of file to load JS from.\n");
DEFINE_bool(gpu, true, "Use the GPU for rendering.");

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
}

SkV8ExampleWindow::SkV8ExampleWindow(void* hwnd, JsContext* context)
    : INHERITED(hwnd)
    , fJsContext(context)
#if SK_SUPPORT_GPU
    , fCurContext(NULL)
    , fCurIntf(NULL)
    , fCurRenderTarget(NULL)
    , fCurSurface(NULL)
#endif
{
    this->setColorType(kBGRA_8888_SkColorType);
    this->setVisibleP(true);
    this->setClipToBounds(false);

#if SK_SUPPORT_GPU
    this->windowSizeChanged();
#endif
}

SkV8ExampleWindow::~SkV8ExampleWindow() {
#if SK_SUPPORT_GPU
    SkSafeUnref(fCurContext);
    SkSafeUnref(fCurIntf);
    SkSafeUnref(fCurRenderTarget);
    SkSafeUnref(fCurSurface);
#endif
}

#if SK_SUPPORT_GPU
void SkV8ExampleWindow::windowSizeChanged() {
    if (FLAGS_gpu) {
        SkOSWindow::AttachmentInfo attachmentInfo;
        bool result = this->attach(
                SkOSWindow::kNativeGL_BackEndType, 0, &attachmentInfo);
        if (!result) {
            printf("Failed to attach.");
            exit(1);
        }

        fCurIntf = GrGLCreateNativeInterface();
        fCurContext = GrContext::Create(
                kOpenGL_GrBackend, (GrBackendContext) fCurIntf);
        if (NULL == fCurIntf || NULL == fCurContext) {
            printf("Failed to initialize GL.");
            exit(1);
        }

        GrBackendRenderTargetDesc desc;
        desc.fWidth = SkScalarRoundToInt(this->width());
        desc.fHeight = SkScalarRoundToInt(this->height());
        desc.fConfig = kSkia8888_GrPixelConfig;
        desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
        desc.fSampleCnt = attachmentInfo.fSampleCount;
        desc.fStencilBits = attachmentInfo.fStencilBits;
        GrGLint buffer;
        GR_GL_GetIntegerv(fCurIntf, GR_GL_FRAMEBUFFER_BINDING, &buffer);
        desc.fRenderTargetHandle = buffer;

        SkSafeUnref(fCurRenderTarget);
        fCurRenderTarget = fCurContext->wrapBackendRenderTarget(desc);
        SkSafeUnref(fCurSurface);
        fCurSurface = SkSurface::NewRenderTargetDirect(fCurRenderTarget);
    }
}
#endif

#if SK_SUPPORT_GPU
SkSurface* SkV8ExampleWindow::createSurface() {
    if (FLAGS_gpu) {
        // Increase the ref count since callers of createSurface put the
        // results in a SkAutoTUnref.
        fCurSurface->ref();
        return fCurSurface;
    } else {
        return this->INHERITED::createSurface();
    }
}
#endif

void SkV8ExampleWindow::onSizeChange() {
    this->INHERITED::onSizeChange();

#if SK_SUPPORT_GPU
    this->windowSizeChanged();
#endif
}

Global* global = NULL;

void SkV8ExampleWindow::onDraw(SkCanvas* canvas) {

    canvas->save();
    canvas->drawColor(SK_ColorWHITE);

    // Now jump into JS and call the onDraw(canvas) method defined there.
    fJsContext->onDraw(canvas);

    canvas->restore();

    this->INHERITED::onDraw(canvas);

#if SK_SUPPORT_GPU
    if (FLAGS_gpu) {
        fCurContext->flush();
        this->present();
    }
#endif
}

#ifdef SK_BUILD_FOR_WIN
void SkV8ExampleWindow::onHandleInval(const SkIRect& rect) {
    RECT winRect;
    winRect.top = rect.top();
    winRect.bottom = rect.bottom();
    winRect.right = rect.right();
    winRect.left = rect.left();
    InvalidateRect((HWND)this->getHWND(), &winRect, false);
}
#endif


SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
    printf("Started\n");

    v8::V8::SetFlagsFromCommandLine(&argc, argv, true);
    SkCommandLineFlags::Parse(argc, argv);

    v8::V8::InitializeICU();
    v8::Platform* platform = v8::platform::CreateDefaultPlatform();
    v8::V8::InitializePlatform(platform);
    v8::V8::Initialize();

    v8::Isolate* isolate = v8::Isolate::New();
    v8::Isolate::Scope isolate_scope(isolate);
    v8::HandleScope handle_scope(isolate);
    isolate->Enter();

    global = new Global(isolate);


    // Set up things to look like a browser by creating
    // a console object that invokes our print function.
    const char* startupScript =
            "function Console() {};                   \n"
            "Console.prototype.log = function() {     \n"
            "  var args = Array.prototype.slice.call(arguments).join(' '); \n"
            "  print(args);                      \n"
            "};                                       \n"
            "console = new Console();                 \n";

    if (!global->parseScript(startupScript)) {
        printf("Failed to parse startup script: %s.\n", FLAGS_infile[0]);
        exit(1);
    }

    const char* script =
            "function onDraw(canvas) {              \n"
            "    canvas.fillStyle = '#00FF00';      \n"
            "    canvas.fillRect(20, 20, 100, 100); \n"
            "    canvas.inval();                    \n"
            "}                                      \n";

    SkAutoTUnref<SkData> data;
    if (FLAGS_infile.count()) {
        data.reset(SkData::NewFromFileName(FLAGS_infile[0]));
        script = static_cast<const char*>(data->data());
    }
    if (NULL == script) {
        printf("Could not load file: %s.\n", FLAGS_infile[0]);
        exit(1);
    }
    Path2DBuilder::AddToGlobal(global);
    Path2D::AddToGlobal(global);

    if (!global->parseScript(script)) {
        printf("Failed to parse file: %s.\n", FLAGS_infile[0]);
        exit(1);
    }


    JsContext* jsContext = new JsContext(global);

    if (!jsContext->initialize()) {
        printf("Failed to initialize.\n");
        exit(1);
    }
    SkV8ExampleWindow* win = new SkV8ExampleWindow(hwnd, jsContext);
    global->setWindow(win);

    return win;
}
