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

#include "gl/GrGLUtil.h"
#include "gl/GrGLDefines.h"
#include "gl/GrGLInterface.h"
#include "SkApplication.h"
#include "SkDraw.h"
#include "SkGpuDevice.h"
#include "SkGraphics.h"


void application_init() {
    SkGraphics::Init();
    SkEvent::Init();
}

void application_term() {
    SkEvent::Term();
    SkGraphics::Term();
}

SkV8ExampleWindow::SkV8ExampleWindow(void* hwnd)
    : INHERITED(hwnd) {

    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);
}


void SkV8ExampleWindow::onDraw(SkCanvas* canvas) {
  printf("Draw\n");

  canvas->drawColor(SK_ColorWHITE);
  SkPaint paint;
  paint.setColor(SK_ColorRED);

  // Draw a rectangle with blue paint
  SkRect rect = {
    SkIntToScalar(10), SkIntToScalar(10),
    SkIntToScalar(128), SkIntToScalar(128)
  };
  canvas->drawRect(rect, paint);

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

  // Get the default Isolate created at startup.
  Isolate* isolate = Isolate::GetCurrent();

  // Create a stack-allocated handle scope.
  HandleScope handle_scope(isolate);

  // Create a new context.
  Handle<Context> context = Context::New(isolate);

  // Here's how you could create a Persistent handle to the context, if needed.
  Persistent<Context> persistent_context(isolate, context);

  // Enter the created context for compiling and
  // running the hello world script.
  Context::Scope context_scope(context);

  // Create a string containing the JavaScript source code.
  Handle<String> source = String::New("'Hello' + ', World!'");

  // Compile the source code.
  Handle<Script> script = Script::Compile(source);

  // Run the script to get the result.
  Handle<Value> result = script->Run();

  // The persistent handle needs to be eventually disposed.
  persistent_context.Dispose();

  // Convert the result to an ASCII string and print it.
  String::AsciiValue ascii(result);
  printf("%s\n", *ascii);

    return new SkV8ExampleWindow(hwnd);
}
