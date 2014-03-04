
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
#include "Path2D.h"
#include "SkCanvas.h"


// Extracts a C string from a V8 Utf8Value.
// TODO(jcgregrio) Currently dup'd in two files, fix.
static const char* to_cstring(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

Persistent<ObjectTemplate> JsContext::gContextTemplate;

// Wraps 'this' in a Javascript object.
Handle<Object> JsContext::wrap() {
    // Handle scope for temporary handles.
    EscapableHandleScope handleScope(fGlobal->getIsolate());

    // Fetch the template for creating JavaScript JsContext wrappers.
    // It only has to be created once, which we do on demand.
    if (gContextTemplate.IsEmpty()) {
        Local<ObjectTemplate> localTemplate = ObjectTemplate::New();

        // Add a field to store the pointer to a JsContext instance.
        localTemplate->SetInternalFieldCount(1);

        this->addAttributesAndMethods(localTemplate);

        gContextTemplate.Reset(fGlobal->getIsolate(), localTemplate);
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
