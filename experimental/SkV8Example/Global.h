/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_Global_DEFINED
#define SkV8Example_Global_DEFINED

#include <map>

#include <v8.h>

using namespace v8;

#include "SkTypes.h"
#include "SkEvent.h"

class SkOSWindow;

typedef Persistent<Function, CopyablePersistentTraits<Function> > CopyablePersistentFn;

// Provides the global isolate and context for our V8 instance.
// Also implements all the global level functions.
class Global : SkNoncopyable  {
public:
    Global(Isolate* isolate)
        : fIsolate(isolate)
        , fWindow(NULL)
        , fLastTimerID(0)
    {
        gGlobal = this;
        this->initialize();
    }
    virtual ~Global() {}

    // The script will be parsed into the context this Global contains.
    bool parseScript(const char script[]);

    Local<Context> getContext() {
        return Local<Context>::New(fIsolate, fContext);
    }

    Isolate* getIsolate() {
        return fIsolate;
    }

    void setWindow(SkOSWindow* win) {
        fWindow = win;
    }
    SkOSWindow* getWindow() {
        return fWindow;
    }

    void reportException(TryCatch* tryCatch);

private:
    void initialize();
    Handle<Context> createRootContext();
    int32_t getNextTimerID();

    static bool TimeOutProc(const SkEvent& evt);

    // Static functions that implement the global JS functions we add to
    // the context.
    static void SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Inval(const v8::FunctionCallbackInfo<Value>& args);

    Persistent<Context> fContext;
    Isolate*            fIsolate;
    SkOSWindow*         fWindow;
    static Global*      gGlobal;

    // Handle to the functions to call when a timeout triggers as indexed by id.
    std::map<int32_t, CopyablePersistentFn > fTimeouts;

    // Last timer ID generated.
    int32_t fLastTimerID;
};

#endif
