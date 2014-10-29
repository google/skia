/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */

#include "Global.h"
#include "Path2DBuilder.h"
#include "Path2D.h"
#include "SkPath.h"

Global* Path2DBuilder::gGlobal = NULL;

void Path2DBuilder::ConstructPath(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::HandleScope handleScope(gGlobal->getIsolate());
    Path2DBuilder* path = new Path2DBuilder();
    args.This()->SetInternalField(
            0, v8::External::New(gGlobal->getIsolate(), path));
}

#define ADD_METHOD(name, fn) \
    constructor->InstanceTemplate()->Set( \
            v8::String::NewFromUtf8( \
                    global->getIsolate(), name, \
                    v8::String::kInternalizedString), \
            v8::FunctionTemplate::New(global->getIsolate(), fn))

// Install the constructor in the global scope so Path2DBuilders can be constructed
// in JS.
void Path2DBuilder::AddToGlobal(Global* global) {
    gGlobal = global;

    // Create a stack-allocated handle scope.
    v8::HandleScope handleScope(gGlobal->getIsolate());

    v8::Handle<v8::Context> context = gGlobal->getContext();

    // Enter the scope so all operations take place in the scope.
    v8::Context::Scope contextScope(context);

    v8::Local<v8::FunctionTemplate> constructor = v8::FunctionTemplate::New(
            gGlobal->getIsolate(), Path2DBuilder::ConstructPath);
    constructor->InstanceTemplate()->SetInternalFieldCount(1);

    ADD_METHOD("close", ClosePath);
    ADD_METHOD("moveTo", MoveTo);
    ADD_METHOD("lineTo", LineTo);
    ADD_METHOD("quadraticCurveTo", QuadraticCurveTo);
    ADD_METHOD("bezierCurveTo", BezierCurveTo);
    ADD_METHOD("arc", Arc);
    ADD_METHOD("rect", Rect);
    ADD_METHOD("oval", Oval);
    ADD_METHOD("conicTo", ConicTo);

    ADD_METHOD("finalize", Finalize);

    context->Global()->Set(v8::String::NewFromUtf8(
            gGlobal->getIsolate(), "Path2DBuilder"), constructor->GetFunction());
}

Path2DBuilder* Path2DBuilder::Unwrap(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(
            args.This()->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<Path2DBuilder*>(ptr);
}

void Path2DBuilder::ClosePath(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Path2DBuilder* path = Unwrap(args);
    path->fSkPath.close();
}

void Path2DBuilder::MoveTo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path2DBuilder* path = Unwrap(args);
    path->fSkPath.moveTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2DBuilder::LineTo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double x = args[0]->NumberValue();
    double y = args[1]->NumberValue();
    Path2DBuilder* path = Unwrap(args);
    path->fSkPath.lineTo(SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2DBuilder::QuadraticCurveTo(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    Path2DBuilder* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.quadTo(
            SkDoubleToScalar(cpx), SkDoubleToScalar(cpy),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2DBuilder::BezierCurveTo(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    Path2DBuilder* path = Unwrap(args);
    // TODO(jcgregorio) Doesn't handle the empty last path case correctly per
    // the HTML 5 spec.
    path->fSkPath.cubicTo(
            SkDoubleToScalar(cp1x), SkDoubleToScalar(cp1y),
            SkDoubleToScalar(cp2x), SkDoubleToScalar(cp2y),
            SkDoubleToScalar(x), SkDoubleToScalar(y));
}

void Path2DBuilder::Arc(const v8::FunctionCallbackInfo<v8::Value>& args) {
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

    Path2DBuilder* path = Unwrap(args);
    SkRect rect = {
        SkDoubleToScalar(x-radius),
        SkDoubleToScalar(y-radius),
        SkDoubleToScalar(x+radius),
        SkDoubleToScalar(y+radius)
    };

    path->fSkPath.addArc(rect, SkRadiansToDegrees(startAngle),
                         SkRadiansToDegrees(sweepAngle));
}

void Path2DBuilder::Rect(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    Path2DBuilder* path = Unwrap(args);
    path->fSkPath.addRect(rect);
}

void Path2DBuilder::Oval(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    Path2DBuilder* path = Unwrap(args);
    SkRect rect = {
        SkDoubleToScalar(x-radiusX),
        SkDoubleToScalar(y-radiusX),
        SkDoubleToScalar(x+radiusY),
        SkDoubleToScalar(y+radiusY)
    };

    path->fSkPath.addOval(rect, dir);
}

void Path2DBuilder::ConicTo(const v8::FunctionCallbackInfo<v8::Value>& args) {
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
    Path2DBuilder* path = Unwrap(args);

    path->fSkPath.conicTo(
            SkDoubleToScalar(x1),
            SkDoubleToScalar(y1),
            SkDoubleToScalar(x2),
            SkDoubleToScalar(y2),
            SkDoubleToScalar(w)
            );
}

void Path2DBuilder::Finalize(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Path2DBuilder* path = Unwrap(args);

    // Build Path2D from out fSkPath and return it.
    SkPath* skPath = new SkPath(path->fSkPath);

    path->fSkPath.reset();

    Path2D* pathWrap = new Path2D(skPath);

    args.GetReturnValue().Set(pathWrap->persistent());
}
