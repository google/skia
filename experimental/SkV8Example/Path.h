/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#ifndef SkV8Example_Path_DEFINED
#define SkV8Example_Path_DEFINED

#include <v8.h>

#include "SkPath.h"
#include "SkTypes.h"

class Global;

class Path : SkNoncopyable {
public:
    Path() : fSkPath() {}
    virtual ~Path() {}

    const SkPath& getSkPath() { return fSkPath; }

    // The JS Path constuctor implementation.
    static void ConstructPath(const v8::FunctionCallbackInfo<v8::Value>& args);

    // Add the Path JS constructor to the global context.
    static void AddToGlobal(Global* global);

    // Path JS methods.
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
private:
    SkPath fSkPath;

    static Path* Unwrap(const v8::FunctionCallbackInfo<v8::Value>& args);

    static Global* gGlobal;
};

#endif
