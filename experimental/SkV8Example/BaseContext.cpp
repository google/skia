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
#include "BaseContext.h"
#include "Path2D.h"
#include "SkCanvas.h"


BaseContext* BaseContext::Unwrap(v8::Handle<v8::Object> obj) {
    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<BaseContext*>(ptr);
}

void BaseContext::FillRect(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

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
    canvas->drawRect(rect, BaseContext->fFillStyle);
}

void BaseContext::Save(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->save();
}

void BaseContext::Restore(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->restore();
}

void BaseContext::Rotate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
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

void BaseContext::Translate(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
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

void BaseContext::ResetTransform(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->resetMatrix();
}

void BaseContext::Stroke(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 arguments required."));
        return;
    }

    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path2D* path = static_cast<Path2D*>(ptr);

    canvas->drawPath(path->getSkPath(), BaseContext->fStrokeStyle);
}

void BaseContext::Fill(const v8::FunctionCallbackInfo<v8::Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    if (args.Length() != 1) {
        args.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        args.GetIsolate(), "Error: 1 arguments required."));
        return;
    }

    v8::Handle<v8::External> field = v8::Handle<v8::External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path2D* path = static_cast<Path2D*>(ptr);

    canvas->drawPath(path->getSkPath(), BaseContext->fFillStyle);
}

void BaseContext::GetStyle(v8::Local<v8::String> name,
                         const v8::PropertyCallbackInfo<v8::Value>& info,
                         const SkPaint& style) {
    char buf[8];
    SkColor color = style.getColor();
    sprintf(buf, "#%02X%02X%02X", SkColorGetR(color), SkColorGetG(color),
            SkColorGetB(color));

    info.GetReturnValue().Set(v8::String::NewFromUtf8(info.GetIsolate(), buf));
}

void BaseContext::SetStyle(v8::Local<v8::String> name, v8::Local<v8::Value> value,
                         const v8::PropertyCallbackInfo<void>& info,
                         SkPaint& style) {
    v8::Local<v8::String> s = value->ToString();
    if (s->Length() != 7 && s->Length() != 9) {
        info.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        info.GetIsolate(),
                        "Invalid fill style format length."));
        return;
    }
    char buf[10];
    s->WriteUtf8(buf, sizeof(buf));

    if (buf[0] != '#') {
        info.GetIsolate()->ThrowException(
                v8::String::NewFromUtf8(
                        info.GetIsolate(), "Invalid fill style format."));
        return;
    }

    // Colors can be RRGGBBAA, but SkColor uses ARGB.
    long color = strtol(buf+1, NULL, 16);
    uint32_t alpha = SK_AlphaOPAQUE;
    if (s->Length() == 9) {
        alpha = color & 0xFF;
        color >>= 8;
    }
    style.setColor(SkColorSetA(SkColor(color), alpha));
}

void BaseContext::GetFillStyle(v8::Local<v8::String> name,
                             const v8::PropertyCallbackInfo<v8::Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    GetStyle(name, info, baseContext->fFillStyle);
}

void BaseContext::GetStrokeStyle(v8::Local<v8::String> name,
                               const v8::PropertyCallbackInfo<v8::Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    GetStyle(name, info, baseContext->fStrokeStyle);
}

void BaseContext::SetFillStyle(v8::Local<v8::String> name, v8::Local<v8::Value> value,
                            const v8::PropertyCallbackInfo<void>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SetStyle(name, value, info, baseContext->fFillStyle);
}

void BaseContext::SetStrokeStyle(v8::Local<v8::String> name, v8::Local<v8::Value> value,
                               const v8::PropertyCallbackInfo<void>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SetStyle(name, value, info, baseContext->fStrokeStyle);
}


void BaseContext::GetWidth(v8::Local<v8::String> name,
                         const v8::PropertyCallbackInfo<v8::Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SkCanvas* canvas = baseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }
    SkISize size = canvas->getDeviceSize();

    info.GetReturnValue().Set(
            v8::Int32::New(baseContext->fGlobal->getIsolate(), size.fWidth));
}

void BaseContext::GetHeight(v8::Local<v8::String> name,
                         const v8::PropertyCallbackInfo<v8::Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SkCanvas* canvas = baseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }
    SkISize size = canvas->getDeviceSize();

    info.GetReturnValue().Set(
            v8::Int32::New(baseContext->fGlobal->getIsolate(), size.fHeight));
}

#define ADD_METHOD(name, fn) \
    tmpl->Set(v8::String::NewFromUtf8( \
         fGlobal->getIsolate(), name, \
         v8::String::kInternalizedString), \
             v8::FunctionTemplate::New(fGlobal->getIsolate(), fn))

void BaseContext::addAttributesAndMethods(v8::Handle<v8::ObjectTemplate> tmpl) {
    v8::HandleScope scope(fGlobal->getIsolate());

    // Add accessors for each of the fields of the context object.
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "fillStyle", v8::String::kInternalizedString),
            GetFillStyle, SetFillStyle);
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "strokeStyle", v8::String::kInternalizedString),
            GetStrokeStyle, SetStrokeStyle);
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "width", v8::String::kInternalizedString),
            GetWidth);
    tmpl->SetAccessor(v8::String::NewFromUtf8(
        fGlobal->getIsolate(), "height", v8::String::kInternalizedString),
            GetHeight);

    // Add methods.
    ADD_METHOD("fillRect", FillRect);
    ADD_METHOD("stroke", Stroke);
    ADD_METHOD("fill", Fill);
    ADD_METHOD("rotate", Rotate);
    ADD_METHOD("save", Save);
    ADD_METHOD("restore", Restore);
    ADD_METHOD("translate", Translate);
    ADD_METHOD("resetTransform", ResetTransform);
}
