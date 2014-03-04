/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_BaseContext_DEFINED
#define SkV8Example_BaseContext_DEFINED

#include <v8.h>

#include "SkPaint.h"

using namespace v8;

class SkCanvas;
class Global;

// BaseContext contains common functionality for both JsContext
// and DisplayList.
class BaseContext {
public:
    BaseContext(Global* global)
            : fGlobal(global)
    {
        fFillStyle.setColor(SK_ColorBLACK);
        fFillStyle.setAntiAlias(true);
        fFillStyle.setStyle(SkPaint::kFill_Style);
        fStrokeStyle.setColor(SK_ColorBLACK);
        fStrokeStyle.setAntiAlias(true);
        fStrokeStyle.setStyle(SkPaint::kStroke_Style);
    }
    virtual ~BaseContext() {}

    // Retrieve the SkCanvas to draw on. May return NULL.
    virtual SkCanvas* getCanvas() = 0;

    // Add the Javascript attributes and methods that BaseContext implements to the ObjectTemplate.
    void addAttributesAndMethods(Handle<ObjectTemplate> tmpl);

protected:
    // Get the pointer out of obj.
    static BaseContext* Unwrap(Handle<Object> obj);

    Global* fGlobal;
    SkPaint fFillStyle;
    SkPaint fStrokeStyle;

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
};

#endif
