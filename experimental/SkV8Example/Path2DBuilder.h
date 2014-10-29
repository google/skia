/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_Path2DBuilder_DEFINED
#define SkV8Example_Path2DBuilder_DEFINED

#include <v8.h>

#include "SkPath.h"
#include "SkTypes.h"

class Global;

class Path2DBuilder : SkNoncopyable {
public:
    Path2DBuilder() : fSkPath() {}
    virtual ~Path2DBuilder() {}

    const SkPath& getSkPath() { return fSkPath; }

    // The JS Path2DBuilder constuctor implementation.
    static void ConstructPath(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Add the Path2DBuilder JS constructor to the global context.
    static void AddToGlobal(Global* global);

    // Path2DBuilder JS methods.
    static void ClosePath(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void MoveTo(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void LineTo(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void QuadraticCurveTo(
            const v8::FunctionCallbackInfo<v8::Value>& args);
    static void BezierCurveTo(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Arc(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Rect(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Oval(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void ConicTo(const v8::FunctionCallbackInfo<v8::Value>& args);
    static void Finalize(const v8::FunctionCallbackInfo<v8::Value>& args);
private:
    SkPath fSkPath;

    static Path2DBuilder* Unwrap(const v8::FunctionCallbackInfo<v8::Value>& args);

    static Global* gGlobal;
};

#endif
