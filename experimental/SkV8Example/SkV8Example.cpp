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

// Extracts a C string from a V8 Utf8Value.
// TODO(jcgregrio) Currently dup'd in two files, fix.
static const char* to_cstring(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}


JsCanvas* JsCanvas::Unwrap(Handle<Object> obj) {
    Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<JsCanvas*>(ptr);
}

void JsCanvas::FillRect(const v8::FunctionCallbackInfo<Value>& args) {
    JsCanvas* jsCanvas = Unwrap(args.This());
    SkCanvas* canvas = jsCanvas->fCanvas;

    if (args.Length() != 4) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 4 arguments required."));
        return;
    }
    // TODO(jcgregorio) Really figure out the conversion from JS numbers to
    // SkScalars. Maybe test if int first? Not sure of the performance impact.
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    double w = args[2]->NumberValue();
    double h = args[3]->NumberValue();

    SkRect rect = {
        SkDoubleToScalar(x),
        SkDoubleToScalar(y),
        SkDoubleToScalar(x) + SkDoubleToScalar(w),
        SkDoubleToScalar(y) + SkDoubleToScalar(h)
    };
    canvas->drawRect(rect, jsCanvas->fFillStyle);
}

void JsCanvas::GetFillStyle(Local<String> name,
                            const PropertyCallbackInfo<Value>& info) {
    JsCanvas* jsCanvas = Unwrap(info.This());
    SkColor color = jsCanvas->fFillStyle.getColor();
    char buf[8];
    sprintf(buf, "#%02X%02X%02X", SkColorGetR(color), SkColorGetG(color),
            SkColorGetB(color));

    info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), buf));
}

void JsCanvas::SetFillStyle(Local<String> name, Local<Value> value,
                            const PropertyCallbackInfo<void>& info) {
    JsCanvas* jsCanvas = Unwrap(info.This());
    Local<String> s = value->ToString();
    if (s->Length() != 7) {
        info.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        info.GetIsolate(), "Invalid fill style format."));
        return;
    }
    char buf[8];
    s->WriteUtf8(buf, sizeof(buf));

    if (buf[0] != '#') {
        info.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        info.GetIsolate(), "Invalid fill style format."));
        return;
    }

    long color = strtol(buf+1, NULL, 16);
    jsCanvas->fFillStyle.setColor(SkColorSetA(SkColor(color), SK_AlphaOPAQUE));
}


Persistent<ObjectTemplate> JsCanvas::fCanvasTemplate;

Handle<ObjectTemplate> JsCanvas::makeCanvasTemplate() {
    EscapableHandleScope handleScope(fGlobal->getIsolate());

    Local<ObjectTemplate> result = ObjectTemplate::New();

    // Add a field to store the pointer to a JsCanvas instance.
    result->SetInternalFieldCount(1);

    // Add accessors for each of the fields of the canvas object.
    result->SetAccessor(
      String::NewFromUtf8(
        fGlobal->getIsolate(), "fillStyle", String::kInternalizedString),
      GetFillStyle, SetFillStyle);

    // Add methods.
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "fillRect",
                    String::kInternalizedString),
            FunctionTemplate::New(FillRect));

    // Return the result through the current handle scope.
    return handleScope.Escape(result);
}


// Wraps 'this' in a Javascript object.
Handle<Object> JsCanvas::wrap() {
    // Handle scope for temporary handles.
    EscapableHandleScope handleScope(fGlobal->getIsolate());

    // Fetch the template for creating JavaScript JsCanvas wrappers.
    // It only has to be created once, which we do on demand.
    if (fCanvasTemplate.IsEmpty()) {
        Handle<ObjectTemplate> raw_template = this->makeCanvasTemplate();
        fCanvasTemplate.Reset(fGlobal->getIsolate(), raw_template);
    }
    Handle<ObjectTemplate> templ =
            Local<ObjectTemplate>::New(fGlobal->getIsolate(), fCanvasTemplate);

    // Create an empty JsCanvas wrapper.
    Local<Object> result = templ->NewInstance();

    // Wrap the raw C++ pointer in an External so it can be referenced
    // from within JavaScript.
    Handle<External> canvasPtr = External::New(fGlobal->getIsolate(), this);

    // Store the canvas pointer in the JavaScript wrapper.
    result->SetInternalField(0, canvasPtr);

    // Return the result through the current handle scope.  Since each
    // of these handles will go away when the handle scope is deleted
    // we need to call Close to let one, the result, escape into the
    // outer handle scope.
    return handleScope.Escape(result);
}

void JsCanvas::onDraw(SkCanvas* canvas) {
    // Record canvas and window in this.
    fCanvas = canvas;

    // Create a handle scope to keep the temporary object references.
    HandleScope handleScope(fGlobal->getIsolate());

    // Create a local context from our global context.
    Local<Context> context = fGlobal->getContext();

    // Enter the context so all the remaining operations take place there.
    Context::Scope contextScope(context);

    // Wrap the C++ this pointer in a JavaScript wrapper.
    Handle<Object> canvasObj = this->wrap();

    // Set up an exception handler before calling the Process function.
    TryCatch tryCatch;

    // Invoke the process function, giving the global object as 'this'
    // and one argument, this JsCanvas.
    const int argc = 1;
    Handle<Value> argv[argc] = { canvasObj };
    Local<Function> onDraw =
            Local<Function>::New(fGlobal->getIsolate(), fOnDraw);
    Handle<Value> result = onDraw->Call(context->Global(), argc, argv);

    // Handle any exceptions or output.
    if (result.IsEmpty()) {
        SkASSERT(tryCatch.HasCaught());
        // Print errors that happened during execution.
        fGlobal->reportException(&tryCatch);
    } else {
        SkASSERT(!tryCatch.HasCaught());
        if (!result->IsUndefined()) {
            // If all went well and the result wasn't undefined then print
            // the returned value.
            String::Utf8Value str(result);
            const char* cstr = to_cstring(str);
            printf("%s\n", cstr);
        }
    }
}

// Fetch the onDraw function from the global context.
bool JsCanvas::initialize() {

    // Create a stack-allocated handle scope.
    HandleScope handleScope(fGlobal->getIsolate());

    // Create a local context from our global context.
    Local<Context> context = fGlobal->getContext();

    // Enter the scope so all operations take place in the scope.
    Context::Scope contextScope(context);

    v8::TryCatch try_catch;

    Handle<String> fn_name = String::NewFromUtf8(
        fGlobal->getIsolate(), "onDraw");
    Handle<Value> fn_val = context->Global()->Get(fn_name);

    if (!fn_val->IsFunction()) {
        printf("Not a function.\n");
        return false;
    }

    // It is a function; cast it to a Function.
    Handle<Function> fn_fun = Handle<Function>::Cast(fn_val);

    // Store the function in a Persistent handle, since we also want that to
    // remain after this call returns.
    fOnDraw.Reset(fGlobal->getIsolate(), fn_fun);

    return true;
}


SkV8ExampleWindow::SkV8ExampleWindow(void* hwnd, JsCanvas* canvas)
    : INHERITED(hwnd)
    , fJsCanvas(canvas)
{
    this->setConfig(SkBitmap::kARGB_8888_Config);
    this->setVisibleP(true);
    this->setClipToBounds(false);
}

void SkV8ExampleWindow::onDraw(SkCanvas* canvas) {

    canvas->save();
    canvas->drawColor(SK_ColorWHITE);

    // Now jump into JS and call the onDraw(canvas) method defined there.
    fJsCanvas->onDraw(canvas);

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

    if (!global->parseScript(script)) {
        exit(1);
    }

    JsCanvas* jsCanvas = new JsCanvas(global);

    if (!jsCanvas->initialize()) {
        printf("Failed to initialize.\n");
        exit(1);
    }
    SkV8ExampleWindow* win = new SkV8ExampleWindow(hwnd, jsCanvas);
    global->setWindow(win);
    return win;
}
