/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Path2D.h"
#include "Global.h"

Global* Path2D::gGlobal = NULL;

void Path2D::ConstructPath(const v8::FunctionCallbackInfo<Value>& args) {
    HandleScope handleScope(gGlobal->getIsolate());
    Path2D* path = new Path2D();
    args.This()->SetInternalField(
            0, External::New(gGlobal->getIsolate(), path));
}

#define ADD_METHOD(name, fn) \
    constructor->InstanceTemplate()->Set( \
            String::NewFromUtf8( \
                    global->getIsolate(), name, \
                    String::kInternalizedString), \
            FunctionTemplate::New(global->getIsolate(), fn))

// Install the constructor in the global scope so Path2Ds can be constructed
// in JS.
void Path2D::AddToGlobal(Global* global) {
    gGlobal = global;

    // Create a stack-allocated handle scope.
    HandleScope handleScope(gGlobal->getIsolate());

    Handle<Context> context = gGlobal->getContext();

    // Enter the scope so all operations take place in the scope.
    Context::Scope contextScope(context);

    Local<FunctionTemplate> constructor = FunctionTemplate::New(
            gGlobal->getIsolate(), Path2D::ConstructPath);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    ADD_METHOD("closePath", ClosePath);
    ADD_METHOD("moveTo", MoveTo);
    ADD_METHOD("lineTo", LineTo);
    ADD_METHOD("quadraticCurveTo", QuadraticCurveTo);
    ADD_METHOD("bezierCurveTo", BezierCurveTo);
    ADD_METHOD("arc", Arc);
    ADD_METHOD("rect", Rect);
    ADD_METHOD("oval", Oval);
    ADD_METHOD("conicTo", ConicTo);

    context->Global()->Set(String::NewFromUtf8(
            gGlobal->getIsolate(), "Path2D"), constructor->GetFunction());
}

Path2D* Path2D::Unwrap(const v8::FunctionCallbackInfo<Value>& args) {
    Handle<External> field = Handle<External>::Cast(
            args.This()->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<Path2D*>(ptr);
}

void Path2D::ClosePath(const v8::FunctionCallbackInfo<Value>& args) {
    Path2D* path = Unwrap(args);
    path->fSkPath.close();
}

void Path2D::MoveTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path2D* path = Unwrap(args);
    path->fSkPath.moveTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2D::LineTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path2D* path = Unwrap(args);
    path->fSkPath.lineTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2D::QuadraticCurveTo(const v8::FunctionCallbackInfo<Value>& args) {
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
    Path2D* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.quadTo(
            SkDoubleToScalar(cpx), SkDoubleToScalar(cpy),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2D::BezierCurveTo(const v8::FunctionCallbackInfo<Value>& args) {
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
    Path2D* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.cubicTo(
            SkDoubleToScalar(cp1x), SkDoubleToScalar(cp1y),
            SkDoubleToScalar(cp2x), SkDoubleToScalar(cp2y),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2D::Arc(const v8::FunctionCallbackInfo<Value>& args) {
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

    Path2D* path = Unwrap(args);
    SkRect rect = {
        SkDoubleToScalar(x-radius),
        SkDoubleToScalar(y-radius),
        SkDoubleToScalar(x+radius),
        SkDoubleToScalar(y+radius)
    };

    path->fSkPath.addArc(rect, SkRadiansToDegrees(startAngle),
                         SkRadiansToDegrees(sweepAngle));
}

void Path2D::Rect(const v8::FunctionCallbackInfo<Value>& args) {
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
    Path2D* path = Unwrap(args);
    path->fSkPath.addRect(rect);
}

void Path2D::Oval(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 4 && args.Length() != 5) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 4 or 5 args required."));
        return;
    }
    double x          = args[0]->NumberValue();
    double y          = args[1]->NumberValue();
    double radiusX    = args[2]->NumberValue();
    double radiusY    = args[3]->NumberValue();
    SkPath::Direction dir = SkPath::kCW_Direction;
    if (args.Length() == 5 && !args[4]->BooleanValue()) {
        dir = SkPath::kCCW_Direction;
    }
    Path2D* path = Unwrap(args);
    SkRect rect = {
        SkDoubleToScalar(x-radiusX),
        SkDoubleToScalar(y-radiusX),
        SkDoubleToScalar(x+radiusY),
        SkDoubleToScalar(y+radiusY)
    };

    path->fSkPath.addOval(rect, dir);
}

void Path2D::ConicTo(const v8::FunctionCallbackInfo<Value>& args) {
    if (args.Length() != 5) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 5 args required."));
        return;
    }
    double x1 = args[0]->NumberValue();
    double y1 = args[1]->NumberValue();
    double x2 = args[2]->NumberValue();
    double y2 = args[3]->NumberValue();
    double w  = args[4]->NumberValue();
    Path2D* path = Unwrap(args);

    path->fSkPath.conicTo(
            SkDoubleToScalar(x1),
            SkDoubleToScalar(y1),
            SkDoubleToScalar(x2),
            SkDoubleToScalar(y2),
            SkDoubleToScalar(w)
            );
}
