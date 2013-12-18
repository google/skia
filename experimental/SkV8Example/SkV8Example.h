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
#include "SkPaint.h"

using namespace v8;

class SkCanvas;
class JsCanvas;
class Global;

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
    JsCanvas* fJsCanvas;
};


// Provides the canvas implementation in JS, and the OnDraw() method in C++
// that's used to bridge from C++ to JS. Should be used in JS as:
//
//  function onDraw(canvas) {
//    canvas.fillStyle="#FF0000";
//    canvas.fillRect(x, y, w, h);
//  }
class JsCanvas {
public:
    JsCanvas(Global* global)
            : fGlobal(global)
            , fCanvas(NULL)
    {
        fFillStyle.setColor(SK_ColorRED);
    }
    ~JsCanvas();

    // Parse the script.
    bool initialize();

    // Call this with the SkCanvas you want onDraw to draw on.
    void onDraw(SkCanvas* canvas);

private:
    // Implementation of the canvas.fillStyle field.
    static void GetFillStyle(Local<String> name,
                             const PropertyCallbackInfo<Value>& info);
    static void SetFillStyle(Local<String> name, Local<Value> value,
                             const PropertyCallbackInfo<void>& info);

    // Implementation of the canvas.fillRect() JS function.
    static void FillRect(const v8::FunctionCallbackInfo<Value>& args);

    // Get the pointer out of obj.
    static JsCanvas* Unwrap(Handle<Object> obj);

    // Create a template for JS object associated with JsCanvas, called lazily
    // by Wrap() and the results are stored in fCanvasTemplate;
    Handle<ObjectTemplate> makeCanvasTemplate();

    // Wrap the 'this' pointer into an Object. Can be retrieved via Unwrap.
    Handle<Object> wrap();

    Global* fGlobal;

    // Only valid when inside OnDraw().
    SkCanvas* fCanvas;

    SkPaint fFillStyle;

    // A handle to the onDraw function defined in the script.
    Persistent<Function> fOnDraw;

    // The template for what a canvas object looks like. The canvas object is
    // what's passed into the JS onDraw() function.
    static Persistent<ObjectTemplate> fCanvasTemplate;
};

#endif
