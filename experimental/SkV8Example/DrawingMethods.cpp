/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#include <v8.h>

#include "Global.h"
#include "DrawingMethods.h"
#include "Path2D.h"
#include "SkCanvas.h"
#include "SkPaint.h"


DrawingMethods* DrawingMethods::Unwrap(v8::Handle<v8::Object> obj) {
    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<DrawingMethods*>(ptr);
}


void DrawingMethods::Save(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->save();
}

void DrawingMethods::Restore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->restore();
}

void DrawingMethods::Rotate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 arguments required."));
        return;
    }
    double angle = args[0]->NumberValue();
    canvas->rotate(SkRadiansToDegrees(angle));
}

void DrawingMethods::Translate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    if (args.Length() != 2) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 2 arguments required."));
        return;
    }
    double dx = args[0]->NumberValue();
    double dy = args[1]->NumberValue();
    canvas->translate(SkDoubleToScalar(dx), SkDoubleToScalar(dy));
}

void DrawingMethods::ResetTransform(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->resetMatrix();
}

void DrawingMethods::DrawPath(const v8::FunctionCallbackInfo<v8::Value>& args) {
    DrawingMethods* drawingMethods = Unwrap(args.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 argument required."));
        return;
    }

    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path2D* path = static_cast<Path2D*>(ptr);
    if (NULL == path) {
        return;
    }
    // TODO(jcgregorio) Add support for Paint2D parameter after Paint2D is
    // implemented.
    SkPaint fillStyle;
    fillStyle.setColor(SK_ColorBLACK);
    fillStyle.setAntiAlias(true);
    fillStyle.setStyle(SkPaint::kFill_Style);
    canvas->drawPath(*(path->path()), fillStyle);
}


void DrawingMethods::GetWidth(v8::Local<v8::String> name,
        const v8::PropertyCallbackInfo<v8::Value>& info) {
    DrawingMethods* drawingMethods = Unwrap(info.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    info.GetReturnValue().Set(
            v8::Int32::New(
                drawingMethods->fGlobal->getIsolate(), canvas->imageInfo().width()));
}

void DrawingMethods::GetHeight(v8::Local<v8::String> name,
        const v8::PropertyCallbackInfo<v8::Value>& info) {
    DrawingMethods* drawingMethods = Unwrap(info.This());
    SkCanvas* canvas = drawingMethods->getCanvas();
    if (NULL == canvas) {
        return;
    }

    info.GetReturnValue().Set(
            v8::Int32::New(
                drawingMethods->fGlobal->getIsolate(), canvas->imageInfo().height()));
}

#define ADD_METHOD(name, fn) \
    tmpl->Set(v8::String::NewFromUtf8( \
         fGlobal->getIsolate(), name, \
         v8::String::kInternalizedString), \
             v8::FunctionTemplate::New(fGlobal->getIsolate(), fn))

void DrawingMethods::addAttributesAndMethods(v8::Handle<v8::ObjectTemplate> tmpl) {
    v8::HandleScope scope(fGlobal->getIsolate());

    // Add accessors for each of the fields of the context object.
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "width", v8::String::kInternalizedString),
            GetWidth);
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "height", v8::String::kInternalizedString),
            GetHeight);

    // Add methods.
    ADD_METHOD("save", Save);
    ADD_METHOD("restore", Restore);
    ADD_METHOD("rotate", Rotate);
    ADD_METHOD("translate", Translate);
    ADD_METHOD("resetTransform", ResetTransform);

    ADD_METHOD("drawPath", DrawPath);
}
