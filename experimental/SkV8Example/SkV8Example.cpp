/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#include <v8.h>

using namespace v8;

#include "SkV8Example.h"
#include "Global.h"
#include "JsContext.h"
#include "Path.h"

#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "SkApplication.h"
#include "SkCommandLineFlags.h"
#include "SkData.h"
#include "SkDraw.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"
#include "SkScalar.h"


DEFINE_string2(infile, i, NULL, "Name of file to load JS from.\n");

void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}


SkV8ExampleWindow::SkV8ExampleWindow(void* hwnd, JsContext* context)
    : INHERITED(hwnd)
    , fJsContext(context)
{
    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);
}

void SkV8ExampleWindow::onDraw(SkCanvas* canvas) {

    canvas->save();
    canvas->drawColor(SK_ColorWHITE);

    // Now jump into JS and call the onDraw(canvas) method defined there.
    fJsContext->onDraw(canvas);

    canvas->restore();

    INHERITED::onDraw(canvas);
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

    SkCommandLineFlags::Parse(argc, argv);

    // Get the default Isolate created at startup.
    Isolate* isolate = Isolate::GetCurrent();
    Global* global = new Global(isolate);

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
    Path::AddToGlobal(global);

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
