/*
 * Copyright 2013 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_DEFINED
#define SkV8Example_DEFINED

#include <v8.h>

#include "SkWindow.h"

using namespace v8;

class SkCanvas;
class JsCanvas;

class SkV8ExampleWindow : public SkOSWindow {
public:
    SkV8ExampleWindow(void* hwnd, JsCanvas* canvas);

protected:
    virtual void onDraw(SkCanvas* canvas) SK_OVERRIDE;

#ifdef SK_BUILD_FOR_WIN
    virtual void onHandleInval(const SkIRect&) SK_OVERRIDE;
#endif

private:
    typedef SkOSWindow INHERITED;
    SkScalar fRotationAngle;
    JsCanvas* fJsCanvas;
};


// Provides the canvas implementation in JS, and the OnDraw() method in C++
// that's used to bridge from C++ to JS. Should be used in JS as:
//
//  function onDraw(canvas) {
//   canvas.drawRect();
//   canvas.inval();
//  }
//
class JsCanvas {
public:
    JsCanvas(Isolate* isolate)
            : fIsolate(isolate)
            , fCanvas(NULL)
            , fWindow(NULL)
    {}
    ~JsCanvas();

    // Parse the script.
    bool initialize(const char script[]);

    // Call this with the SkCanvas you want onDraw to draw on.
    void onDraw(SkCanvas* canvas, SkOSWindow* window);

private:
    // Implementation of the canvas.drawRect() JS function.
    static void drawRect(const v8::FunctionCallbackInfo<Value>& args);

    // Implementation of the canvas.inval() JS function.
    static void inval(const v8::FunctionCallbackInfo<Value>& args);

    // Get the pointer out of obj.
    static JsCanvas* unwrap(Handle<Object> obj);

    // Create a template for JS object associated with JsCanvas, called lazily
    // by Wrap() and the results are stored in fCanvasTemplate;
    Handle<ObjectTemplate> makeCanvasTemplate();

    // Wrap the 'this' pointer into an Object. Can be retrieved via Unwrap.
    Handle<Object> wrap();

    Isolate* fIsolate;

    // Only valid when inside OnDraw().
    SkCanvas* fCanvas;

    // Only valid when inside OnDraw().
    SkOSWindow* fWindow;

    // The context that the script will be parsed into.
    Persistent<Context> fContext;

    // A handle to the onDraw function defined in the script.
    Persistent<Function> fOnDraw;

    // The template for what a canvas object looks like. The canvas object is
    // what's passed into the JS onDraw() function.
    static Persistent<ObjectTemplate> fCanvasTemplate;
};

#endif
