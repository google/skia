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

// Extracts a C string from a V8 Utf8Value.
const char* ToCString(const v8::String::Utf8Value& value) {
  return *value ? *value : "<string conversion failed>";
}

// Slight modification to an original function found in the V8 sample shell.cc.
void reportException(Isolate* isolate, TryCatch* try_catch) {
  HandleScope handle_scope(isolate);
  String::Utf8Value exception(try_catch->Exception());
  const char* exception_string = ToCString(exception);
  Handle<Message> message = try_catch->Message();
  if (message.IsEmpty()) {
    // V8 didn't provide any extra information about this error; just
    // print the exception.
    fprintf(stderr, "%s\n", exception_string);
  } else {
    // Print (filename):(line number): (message).
    String::Utf8Value filename(message->GetScriptResourceName());
    const char* filename_string = ToCString(filename);
    int linenum = message->GetLineNumber();
    fprintf(stderr, "%s:%i: %s\n", filename_string, linenum, exception_string);
    // Print line of source code.
    String::Utf8Value sourceline(message->GetSourceLine());
    const char* sourceline_string = ToCString(sourceline);
    fprintf(stderr, "%s\n", sourceline_string);
    // Print wavy underline.
    int start = message->GetStartColumn();
    for (int i = 0; i < start; i++) {
      fprintf(stderr, " ");
    }
    int end = message->GetEndColumn();
    for (int i = start; i < end; i++) {
      fprintf(stderr, "^");
    }
    fprintf(stderr, "\n");
    String::Utf8Value stack_trace(try_catch->StackTrace());
    if (stack_trace.length() > 0) {
      const char* stack_trace_string = ToCString(stack_trace);
      fprintf(stderr, "%s\n", stack_trace_string);
    }
  }
}

SkV8ExampleWindow::SkV8ExampleWindow(void* hwnd,
                     Isolate* isolate,
                     Handle<Context> context,
                     Handle<Script> script)
    : INHERITED(hwnd)
    , fIsolate(isolate)
{
    // Convert the Handle<> objects into Persistent<> objects using Reset().
    fContext.Reset(isolate, context);
    fScript.Reset(isolate, script);

    fRotationAngle = SkIntToScalar(0);
    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);
}

// Simple global for the Draw function.
SkCanvas* gCanvas = NULL;


// Draw is called from within V8 when the Javascript function draw() is called.
void Draw(const v8::FunctionCallbackInfo<v8::Value>& args) {
  if (NULL == gCanvas) {
    printf("Can't Draw Now.\n");
    return;
  }

  gCanvas->drawColor(SK_ColorWHITE);
  SkPaint paint;
  paint.setColor(SK_ColorRED);

  // Draw a rectangle with blue paint
  SkRect rect = {
    SkIntToScalar(10), SkIntToScalar(10),
    SkIntToScalar(128), SkIntToScalar(128)
  };
  gCanvas->drawRect(rect, paint);
}

void SkV8ExampleWindow::onDraw(SkCanvas* canvas) {
  printf("Draw\n");

  gCanvas = canvas;
  canvas->save();
  fRotationAngle += SkDoubleToScalar(0.2);
  if (fRotationAngle > SkDoubleToScalar(360.0)) {
    fRotationAngle -= SkDoubleToScalar(360.0);
  }
  canvas->rotate(fRotationAngle);

  // Create a Handle scope for temporary references.
  HandleScope handle_scope(fIsolate);

  // Create a local context from our persistent context.
  Local<Context> context =
            Local<Context>::New(fIsolate, fContext);

  // Enter the context so all operations take place within it.
  Context::Scope context_scope(context);

  TryCatch try_catch;

  // Create a local script from our persistent script.
  Local<Script> script =
            Local<Script>::New(fIsolate, fScript);

  // Run the script.
  Handle<Value> result = script->Run();

  if (result.IsEmpty()) {
    SkASSERT(try_catch.HasCaught());
    // Print errors that happened during execution.
    reportException(fIsolate, &try_catch);
  } else {
    SkASSERT(!try_catch.HasCaught());
    if (!result->IsUndefined()) {
      // If all went well and the result wasn't undefined then print
      // the returned value.
      String::Utf8Value str(result);
      const char* cstr = ToCString(str);
      printf("%s\n", cstr);
    }
  }

  canvas->restore();

  // Trigger an invalidation which should trigger another redraw to simulate
  // animation.
  this->inval(NULL);

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

// Creates a new execution environment containing the built-in
// function draw().
Handle<Context> createRootContext(Isolate* isolate) {
  // Create a template for the global object.
  Handle<ObjectTemplate> global = ObjectTemplate::New();
  // Bind the global 'draw' function to the C++ Draw callback.
  global->Set(String::NewFromUtf8(isolate, "draw"),
              FunctionTemplate::New(Draw));

  return Context::New(isolate, NULL, global);
}

SkOSWindow* create_sk_window(void* hwnd, int argc, char** argv) {
  printf("Started\n");

  // Get the default Isolate created at startup.
  Isolate* isolate = Isolate::GetCurrent();
  printf("Isolate\n");

  // Create a stack-allocated handle scope.
  HandleScope handle_scope(isolate);

  printf("Before create context\n");
  // Create a new context.
  //
  Handle<Context> context = createRootContext(isolate);

  // Enter the scope so all operations take place in the scope.
  Context::Scope context_scope(context);

  v8::TryCatch try_catch;

  // Compile the source code.
  Handle<String> source = String::NewFromUtf8(isolate, "draw();");
  printf("Before Compile\n");
  Handle<Script> script = Script::Compile(source);
  printf("After Compile\n");

  // Try running it now. It won't have a valid context, but shouldn't fail.
  script->Run();

  if (script.IsEmpty()) {
    // Print errors that happened during compilation.
    reportException(isolate, &try_catch);
    exit(1);
  }
  printf("After Exception.\n");

  // SkV8ExampleWindow will make persistent handles to hold the context and script.
  return new SkV8ExampleWindow(hwnd, isolate, context, script);
}
