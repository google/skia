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


#include "SkTypes.h"
#include "SkEvent.h"

class SkOSWindow;

typedef v8::Persistent<v8::Function, v8::CopyablePersistentTraits<v8::Function> > CopyablePersistentFn;

// Provides the global isolate and context for our V8 instance.
// Also implements all the global level functions.
class Global : SkNoncopyable  {
public:
    Global(v8::Isolate* isolate)
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

    v8::Local<v8::Context> getContext() {
        return v8::Local<v8::Context>::New(fIsolate, fContext);
    }

    v8::Isolate* getIsolate() {
        return fIsolate;
    }

    void setWindow(SkOSWindow* win) {
        fWindow = win;
    }
    SkOSWindow* getWindow() {
        return fWindow;
    }

    void reportException(v8::TryCatch* tryCatch);

private:
    void initialize();
    v8::Handle<v8::Context> createRootContext();
    int32_t getNextTimerID();

    static bool TimeOutProc(const SkEvent& evt);

    // Static functions that implement the global JS functions we add to
    // the context.
    static void SetTimeout(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Print(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Inval(const v8::FunctionCallbackInfo<v8::Value>& args);

    v8::Persistent<v8::Context> fContext;
    v8::Isolate*                fIsolate;
    SkOSWindow*                 fWindow;
    static Global*              gGlobal;

    // Handle to the functions to call when a timeout triggers as indexed by id.
    std::map<int32_t, CopyablePersistentFn > fTimeouts;

    // Last timer ID generated.
    int32_t fLastTimerID;
};

#endif
