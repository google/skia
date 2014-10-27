/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_DrawingMethods_DEFINED
#define SkV8Example_DrawingMethods_DEFINED

#include <v8.h>

class SkCanvas;
class Global;

// DrawingMethods contains common functionality for both Context, Image2Builder,
// and DisplayListBuiler.
class DrawingMethods {
public:
    DrawingMethods(Global* global)
            : fGlobal(global)
    {}
    virtual ~DrawingMethods() {}

    // Retrieve the SkCanvas to draw on. May return NULL.
    virtual SkCanvas* getCanvas() = 0;

    // Add the Javascript attributes and methods that DrawingMethods
    // implements to the ObjectTemplate.
    void addAttributesAndMethods(v8::Handle<v8::ObjectTemplate> tmpl);

protected:
    // Get the pointer out of obj.
    static DrawingMethods* Unwrap(v8::Handle<v8::Object> obj);

    Global* fGlobal;

private:
    // JS Attributes
    static void GetWidth(v8::Local<v8::String> name,
                         const v8::PropertyCallbackInfo<v8::Value>& info);
    static void GetHeight(v8::Local<v8::String> name,
                          const v8::PropertyCallbackInfo<v8::Value>& info);

    // JS Methods
    static void Save(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Restore(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Rotate(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Translate(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ResetTransform(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void DrawPath(const v8::FunctionCallbackInfo<v8::Value>& args);
};

#endif
