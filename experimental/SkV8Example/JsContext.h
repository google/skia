/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_JsContext_DEFINED
#define SkV8Example_JsContext_DEFINED

#include <v8.h>

#include "SkPaint.h"

using namespace v8;

class SkCanvas;
class Global;

// Provides the canvas context implementation in JS, and the OnDraw() method in
// C++ that's used to bridge from C++ to JS. Should be used in JS as:
//
//  function onDraw(context) {
//    context.fillStyle="#FF0000";
//    context.fillRect(x, y, w, h);
//  }
class JsContext {
public:
    JsContext(Global* global)
            : fGlobal(global)
            , fCanvas(NULL)
    {
        fFillStyle.setColor(SK_ColorBLACK);
        fFillStyle.setAntiAlias(true);
        fFillStyle.setStyle(SkPaint::kFill_Style);
        fStrokeStyle.setColor(SK_ColorBLACK);
        fStrokeStyle.setAntiAlias(true);
        fStrokeStyle.setStyle(SkPaint::kStroke_Style);
    }
    ~JsContext();

    // Parse the script.
    bool initialize();

    // Call this with the SkCanvas you want onDraw to draw on.
    void onDraw(SkCanvas* canvas);

private:
    static void GetStyle(Local<String> name,
                         const PropertyCallbackInfo<Value>& info,
                         const SkPaint& style);
    static void SetStyle(Local<String> name, Local<Value> value,
                         const PropertyCallbackInfo<void>& info,
                         SkPaint& style);
    // JS Attributes
    static void GetFillStyle(Local<String> name,
                         const PropertyCallbackInfo<Value>& info);
    static void SetFillStyle(Local<String> name, Local<Value> value,
                         const PropertyCallbackInfo<void>& info);
    static void GetStrokeStyle(Local<String> name,
                         const PropertyCallbackInfo<Value>& info);
    static void SetStrokeStyle(Local<String> name, Local<Value> value,
                         const PropertyCallbackInfo<void>& info);
    static void GetWidth(Local<String> name,
                         const PropertyCallbackInfo<Value>& info);
    static void GetHeight(Local<String> name,
                          const PropertyCallbackInfo<Value>& info);

    // JS Methods
    static void FillRect(const v8::FunctionCallbackInfo<Value>& args);
    static void Stroke(const v8::FunctionCallbackInfo<Value>& args);
    static void Fill(const v8::FunctionCallbackInfo<Value>& args);
    static void Rotate(const v8::FunctionCallbackInfo<Value>& args);
    static void Save(const v8::FunctionCallbackInfo<Value>& args);
    static void Restore(const v8::FunctionCallbackInfo<Value>& args);
    static void Translate(const v8::FunctionCallbackInfo<Value>& args);
    static void ResetTransform(const v8::FunctionCallbackInfo<Value>& args);

    // Get the pointer out of obj.
    static JsContext* Unwrap(Handle<Object> obj);

    // Create a template for JS object associated with JsContext, called lazily
    // by Wrap() and the results are stored in gContextTemplate;
    Handle<ObjectTemplate> makeContextTemplate();

    // Wrap the 'this' pointer into an Object. Can be retrieved via Unwrap.
    Handle<Object> wrap();

    Global* fGlobal;

    // Only valid when inside OnDraw().
    SkCanvas* fCanvas;

    SkPaint fFillStyle;
    SkPaint fStrokeStyle;

    // A handle to the onDraw function defined in the script.
    Persistent<Function> fOnDraw;

    // The template for what a canvas context object looks like. The canvas
    // context object is what's passed into the JS onDraw() function.
    static Persistent<ObjectTemplate> gContextTemplate;
};

#endif
