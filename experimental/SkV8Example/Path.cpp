/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Path.h"
#include "Global.h"

Global* Path::gGlobal = NULL;

void Path::ConstructPath(const v8::FunctionCallbackInfo<Value>& args) {
    HandleScope handleScope(gGlobal->getIsolate());
    Path* path = new Path();
    args.This()->SetInternalField(0, External::New(path));
}

#define ADD_METHOD(name, fn) \
    constructor->InstanceTemplate()->Set( \
            String::NewFromUtf8( \
                    global->getIsolate(), name, \
                    String::kInternalizedString), \
            FunctionTemplate::New(fn))

// Install the constructor in the global scope so Paths can be constructed
// in JS.
void Path::AddToGlobal(Global* global) {
    gGlobal = global;

    // Create a stack-allocated handle scope.
    HandleScope handleScope(gGlobal->getIsolate());

    Handle<Context> context = gGlobal->getContext();

    // Enter the scope so all operations take place in the scope.
    Context::Scope contextScope(context);

    Local<FunctionTemplate> constructor = FunctionTemplate::New(
            Path::ConstructPath);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    ADD_METHOD("close", ClosePath);
    ADD_METHOD("moveTo", MoveTo);
    ADD_METHOD("lineTo", LineTo);
    ADD_METHOD("quadraticCurveTo", QuadraticCurveTo);
    ADD_METHOD("bezierCurveTo", BezierCurveTo);
    ADD_METHOD("arc", Arc);
    ADD_METHOD("rect", Rect);

    context->Global()->Set(String::New("Path"), constructor->GetFunction());
}

Path* Path::Unwrap(const v8::FunctionCallbackInfo<Value>& args) {
    Handle<External> field = Handle<External>::Cast(
            args.This()->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<Path*>(ptr);
}

void Path::ClosePath(const v8::FunctionCallbackInfo<Value>& args) {
    Path* path = Unwrap(args);
    path->fSkPath.close();
}

void Path::MoveTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path* path = Unwrap(args);
    path->fSkPath.moveTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path::LineTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path* path = Unwrap(args);
    path->fSkPath.lineTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path::QuadraticCurveTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 4) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 4 arguments required."));
        return;
    }
    double cpx = args[0]->NumberValue();
    double cpy = args[1]->NumberValue();
    double x = args[2]->NumberValue();
    double y = args[3]->NumberValue();
    Path* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.quadTo(
            SkDoubleToScalar(cpx), SkDoubleToScalar(cpy),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path::BezierCurveTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 6) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 6 arguments required."));
        return;
    }
    double cp1x = args[0]->NumberValue();
    double cp1y = args[1]->NumberValue();
    double cp2x = args[2]->NumberValue();
    double cp2y = args[3]->NumberValue();
    double x = args[4]->NumberValue();
    double y = args[5]->NumberValue();
    Path* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.cubicTo(
            SkDoubleToScalar(cp1x), SkDoubleToScalar(cp1y),
            SkDoubleToScalar(cp2x), SkDoubleToScalar(cp2y),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path::Arc(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 5 && args.Length() != 6) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 5 or 6 args required."));
        return;
    }
    double x          = args[0]->NumberValue();
    double y          = args[1]->NumberValue();
    double radius     = args[2]->NumberValue();
    double startAngle = args[3]->NumberValue();
    double endAngle   = args[4]->NumberValue();
    bool antiClockwise = false;
    if (args.Length() == 6) {
       antiClockwise = args[5]->BooleanValue();
    }
    double sweepAngle;
    if (!antiClockwise) {
      sweepAngle = endAngle - startAngle;
    } else {
      sweepAngle = startAngle - endAngle;
      startAngle = endAngle;
    }

    Path* path = Unwrap(args);
    SkRect rect = {
        SkDoubleToScalar(x-radius),
        SkDoubleToScalar(y-radius),
        SkDoubleToScalar(x+radius),
        SkDoubleToScalar(y+radius)
    };

    path->fSkPath.addArc(rect, SkRadiansToDegrees(startAngle),
                         SkRadiansToDegrees(sweepAngle));
}

void Path::Rect(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 4) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 4 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    double w = args[2]->NumberValue();
    double h = args[3]->NumberValue();

    SkRect rect = {
        SkDoubleToScalar(x),
        SkDoubleToScalar(y),
        SkDoubleToScalar(x) + SkDoubleToScalar(w),
        SkDoubleToScalar(y) + SkDoubleToScalar(h)
    };
    Path* path = Unwrap(args);
    path->fSkPath.addRect(rect);
}

