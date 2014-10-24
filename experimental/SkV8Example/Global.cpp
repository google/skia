/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#include "Global.h"

#include "SkWindow.h"
#include "SkEvent.h"


Global* Global::gGlobal = NULL;

// Extracts a C string from a V8 Utf8Value.
static const char* to_cstring(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

int32_t Global::getNextTimerID() {
    do {
        fLastTimerID++;
        if (fLastTimerID < 0) {
            fLastTimerID = 0;
        }
    } while (fTimeouts.find(fLastTimerID) != fTimeouts.end());
    return fLastTimerID;
}

// Slight modification to an original function found in the V8 sample shell.cc.
void Global::reportException(v8::TryCatch* tryCatch) {
    v8::HandleScope handleScope(fIsolate);
    v8::String::Utf8Value exception(tryCatch->Exception());
    const char* exceptionString = to_cstring(exception);
    v8::Handle<v8::Message> message = tryCatch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        fprintf(stderr, "%s\n", exceptionString);
    } else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptOrigin().ResourceName());
        const char* filenameString = to_cstring(filename);
        int linenum = message->GetLineNumber();
        fprintf(stderr,
                "%s:%i: %s\n", filenameString, linenum, exceptionString);
        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceLineString = to_cstring(sourceline);
        fprintf(stderr, "%s\n", sourceLineString);
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
        v8::String::Utf8Value stackTrace(tryCatch->StackTrace());
        if (stackTrace.length() > 0) {
            const char* stackTraceString = to_cstring(stackTrace);
            fprintf(stderr, "%s\n", stackTraceString);
        }
    }
}

// The callback that implements the JavaScript 'inval' function.
// Invalidates the current window, forcing a redraw.
//
// JS: inval();
void Global::Inval(const v8::FunctionCallbackInfo<v8::Value>& args) {
    gGlobal->getWindow()->inval(NULL);
}

// The callback that is invoked by v8 whenever the JavaScript 'print'
// function is called. Prints its arguments on stdout separated by
// spaces and ending with a newline.
//
// JS: print("foo", "bar");
void Global::Print(const v8::FunctionCallbackInfo<v8::Value>& args) {
    bool first = true;
    v8::HandleScope handleScope(args.GetIsolate());
    for (int i = 0; i < args.Length(); i++) {
        if (first) {
            first = false;
        } else {
            printf(" ");
        }
        v8::String::Utf8Value str(args[i]);
        printf("%s", to_cstring(str));
    }
    printf("\n");
    fflush(stdout);
}

// The callback that is invoked by v8 whenever the JavaScript 'setTimeout'
// function is called.
//
// JS: setTimeout(on_timeout, 500);
void Global::SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }

    // Pull out the first arg, make sure it's a function.
    if (!args[0]->IsFunction()) {
        printf("Not a function passed to setTimeout.\n");
        return;
    }
    v8::Handle<v8::Function> timeoutFn = v8::Handle<v8::Function>::Cast(args[0]);

    double delay = args[1]->NumberValue();
    int32_t id = gGlobal->getNextTimerID();

    gGlobal->fTimeouts[id].Reset(gGlobal->fIsolate, timeoutFn);

    // Create an SkEvent and add it with the right delay.
    SkEvent* evt = new SkEvent();
    evt->setTargetProc(Global::TimeOutProc);
    evt->setFast32(id);
    evt->postDelay(delay);

    args.GetReturnValue().Set(v8::Integer::New(gGlobal->fIsolate, id));
}

// Callback function for SkEvents used to implement timeouts.
bool Global::TimeOutProc(const SkEvent& evt) {
    // Create a handle scope to keep the temporary object references.
    v8::HandleScope handleScope(gGlobal->getIsolate());

    // Create a local context from our global context.
    v8::Local<v8::Context> context = gGlobal->getContext();

    // Enter the context so all the remaining operations take place there.
    v8::Context::Scope contextScope(context);

    // Set up an exception handler before calling the Process function.
    v8::TryCatch tryCatch;

    int32_t id = evt.getFast32();
    if (gGlobal->fTimeouts.find(gGlobal->fLastTimerID) == gGlobal->fTimeouts.end()) {
        printf("Not a valid timer ID.\n");
        return true;
    }

    const int argc = 0;
    v8::Local<v8::Function> onTimeout =
            v8::Local<v8::Function>::New(gGlobal->getIsolate(), gGlobal->fTimeouts[id]);
    v8::Handle<v8::Value> result = onTimeout->Call(context->Global(), argc, NULL);
    gGlobal->fTimeouts.erase(id);

    // Handle any exceptions or output.
    if (result.IsEmpty()) {
        SkASSERT(tryCatch.HasCaught());
        // Print errors that happened during execution.
        gGlobal->reportException(&tryCatch);
    } else {
        SkASSERT(!tryCatch.HasCaught());
        if (!result->IsUndefined()) {
            // If all went well and the result wasn't undefined then print the
            // returned value.
            v8::String::Utf8Value str(result);
            const char* cstr = to_cstring(str);
            printf("%s\n", cstr);
        }
    }
    return true;
}

// Creates a new execution environment containing the built-in functions.
v8::Handle<v8::Context> Global::createRootContext() {
  // Create a template for the global object.
  v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();

  global->Set(v8::String::NewFromUtf8(fIsolate, "print"),
              v8::FunctionTemplate::New(fIsolate, Global::Print));
  global->Set(v8::String::NewFromUtf8(fIsolate, "setTimeout"),
              v8::FunctionTemplate::New(fIsolate, Global::SetTimeout));
  global->Set(v8::String::NewFromUtf8(fIsolate, "inval"),
              v8::FunctionTemplate::New(fIsolate, Global::Inval));


  return v8::Context::New(fIsolate, NULL, global);
}

void Global::initialize() {
    // Create a stack-allocated handle scope.
    v8::HandleScope handleScope(fIsolate);

    // Create a new context.
    v8::Handle<v8::Context> context = this->createRootContext();

    // Make the context persistent.
    fContext.Reset(fIsolate, context);
}


// Creates the root context, parses the script into it, then stores the
// context in a global.
//
// TODO(jcgregorio) Currently only handles one script. Need to move
// createRootContext to another call that's only done once.
bool Global::parseScript(const char script[]) {

    // Create a stack-allocated handle scope.
    v8::HandleScope handleScope(fIsolate);

    // Get the global context.
    v8::Handle<v8::Context> context = this->getContext();

    // Enter the scope so all operations take place in the scope.
    v8::Context::Scope contextScope(context);

    v8::TryCatch tryCatch;

    // Compile the source code.
    v8::Handle<v8::String> source = v8::String::NewFromUtf8(fIsolate, script);
    v8::Handle<v8::Script> compiledScript = v8::Script::Compile(source);

    if (compiledScript.IsEmpty()) {
        // Print errors that happened during compilation.
        this->reportException(&tryCatch);
        return false;
    }

    // Try running it now to create the onDraw function.
    v8::Handle<v8::Value> result = compiledScript->Run();

    // Handle any exceptions or output.
    if (result.IsEmpty()) {
        SkASSERT(tryCatch.HasCaught());
        // Print errors that happened during execution.
        this->reportException(&tryCatch);
        return false;
    }

    return true;
}
