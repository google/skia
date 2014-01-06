
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

#include "Global.h"
#include "JsContext.h"
#include "Path.h"
#include "SkCanvas.h"


// Extracts a C string from a V8 Utf8Value.
// TODO(jcgregrio) Currently dup'd in two files, fix.
static const char* to_cstring(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

JsContext* JsContext::Unwrap(Handle<Object> obj) {
    Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<JsContext*>(ptr);
}

void JsContext::FillRect(const v8::FunctionCallbackInfo<Value>& args) {
    JsContext* jsContext = Unwrap(args.This());
    SkCanvas* canvas = jsContext->fCanvas;

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
    jsContext->fFillStyle.setStyle(SkPaint::kFill_Style);
    canvas->drawRect(rect, jsContext->fFillStyle);
}

void JsContext::Translate(const v8::FunctionCallbackInfo<Value>& args) {
    JsContext* jsContext = Unwrap(args.This());
    SkCanvas* canvas = jsContext->fCanvas;

    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double dx = args[0]->NumberValue();
    double dy = args[1]->NumberValue();
    canvas->translate(SkDoubleToScalar(dx), SkDoubleToScalar(dy));
}

void JsContext::ResetTransform(const v8::FunctionCallbackInfo<Value>& args) {
    JsContext* jsContext = Unwrap(args.This());
    SkCanvas* canvas = jsContext->fCanvas;

    canvas->resetMatrix();
}

void JsContext::Stroke(const v8::FunctionCallbackInfo<Value>& args) {
    JsContext* jsContext = Unwrap(args.This());
    SkCanvas* canvas = jsContext->fCanvas;

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 arguments required."));
        return;
    }

    Handle<External> field = Handle<External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path* path = static_cast<Path*>(ptr);

    jsContext->fFillStyle.setStyle(SkPaint::kStroke_Style);
    canvas->drawPath(path->getSkPath(), jsContext->fFillStyle);
}


void JsContext::Fill(const v8::FunctionCallbackInfo<Value>& args) {
    JsContext* jsContext = Unwrap(args.This());
    SkCanvas* canvas = jsContext->fCanvas;

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 arguments required."));
        return;
    }

    Handle<External> field = Handle<External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path* path = static_cast<Path*>(ptr);

    jsContext->fFillStyle.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(path->getSkPath(), jsContext->fFillStyle);
}


void JsContext::GetFillStyle(Local<String> name,
                            const PropertyCallbackInfo<Value>& info) {
    JsContext* jsContext = Unwrap(info.This());
    SkColor color = jsContext->fFillStyle.getColor();
    char buf[8];
    sprintf(buf, "#%02X%02X%02X", SkColorGetR(color), SkColorGetG(color),
            SkColorGetB(color));

    info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), buf));
}

void JsContext::SetFillStyle(Local<String> name, Local<Value> value,
                            const PropertyCallbackInfo<void>& info) {
    JsContext* jsContext = Unwrap(info.This());
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
    jsContext->fFillStyle.setColor(SkColorSetA(SkColor(color), SK_AlphaOPAQUE));
}


void JsContext::GetWidth(Local<String> name,
                         const PropertyCallbackInfo<Value>& info) {
    JsContext* jsContext = Unwrap(info.This());
    SkISize size = jsContext->fCanvas->getDeviceSize();

    info.GetReturnValue().Set(Int32::New(size.fWidth));
}

void JsContext::GetHeight(Local<String> name,
                         const PropertyCallbackInfo<Value>& info) {
    JsContext* jsContext = Unwrap(info.This());
    SkISize size = jsContext->fCanvas->getDeviceSize();

    info.GetReturnValue().Set(Int32::New(size.fHeight));
}


Persistent<ObjectTemplate> JsContext::gContextTemplate;

Handle<ObjectTemplate> JsContext::makeContextTemplate() {
    EscapableHandleScope handleScope(fGlobal->getIsolate());

    Local<ObjectTemplate> result = ObjectTemplate::New();

    // Add a field to store the pointer to a JsContext instance.
    result->SetInternalFieldCount(1);

    // Add accessors for each of the fields of the context object.
    result->SetAccessor(String::NewFromUtf8(
            fGlobal->getIsolate(), "fillStyle", String::kInternalizedString),
                        GetFillStyle, SetFillStyle);
    result->SetAccessor(String::NewFromUtf8(
            fGlobal->getIsolate(), "width", String::kInternalizedString),
                        GetWidth);
    result->SetAccessor(String::NewFromUtf8(
            fGlobal->getIsolate(), "height", String::kInternalizedString),
                        GetHeight);

    // Add methods.
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "fillRect",
                    String::kInternalizedString),
            FunctionTemplate::New(FillRect));
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "stroke",
                    String::kInternalizedString),
            FunctionTemplate::New(Stroke));
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "fill",
                    String::kInternalizedString),
            FunctionTemplate::New(Fill));
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "translate",
                    String::kInternalizedString),
            FunctionTemplate::New(Translate));
    result->Set(
            String::NewFromUtf8(
                    fGlobal->getIsolate(), "resetTransform",
                    String::kInternalizedString),
            FunctionTemplate::New(ResetTransform));

    // Return the result through the current handle scope.
    return handleScope.Escape(result);
}


// Wraps 'this' in a Javascript object.
Handle<Object> JsContext::wrap() {
    // Handle scope for temporary handles.
    EscapableHandleScope handleScope(fGlobal->getIsolate());

    // Fetch the template for creating JavaScript JsContext wrappers.
    // It only has to be created once, which we do on demand.
    if (gContextTemplate.IsEmpty()) {
        Handle<ObjectTemplate> raw_template = this->makeContextTemplate();
        gContextTemplate.Reset(fGlobal->getIsolate(), raw_template);
    }
    Handle<ObjectTemplate> templ =
            Local<ObjectTemplate>::New(fGlobal->getIsolate(), gContextTemplate);

    // Create an empty JsContext wrapper.
    Local<Object> result = templ->NewInstance();

    // Wrap the raw C++ pointer in an External so it can be referenced
    // from within JavaScript.
    Handle<External> contextPtr = External::New(fGlobal->getIsolate(), this);

    // Store the context pointer in the JavaScript wrapper.
    result->SetInternalField(0, contextPtr);

    // Return the result through the current handle scope.  Since each
    // of these handles will go away when the handle scope is deleted
    // we need to call Close to let one, the result, escape into the
    // outer handle scope.
    return handleScope.Escape(result);
}

void JsContext::onDraw(SkCanvas* canvas) {
    // Record canvas and window in this.
    fCanvas = canvas;

    // Create a handle scope to keep the temporary object references.
    HandleScope handleScope(fGlobal->getIsolate());

    // Create a local context from our global context.
    Local<Context> context = fGlobal->getContext();

    // Enter the context so all the remaining operations take place there.
    Context::Scope contextScope(context);

    // Wrap the C++ this pointer in a JavaScript wrapper.
    Handle<Object> contextObj = this->wrap();

    // Set up an exception handler before calling the Process function.
    TryCatch tryCatch;

    // Invoke the process function, giving the global object as 'this'
    // and one argument, this JsContext.
    const int argc = 1;
    Handle<Value> argv[argc] = { contextObj };
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
bool JsContext::initialize() {

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

