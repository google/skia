/*
 * Copyright 2014 Google Inc.
 *
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 */
#include <v8.h>

using namespace v8;

#include "Global.h"
#include "BaseContext.h"
#include "Path2D.h"
#include "SkCanvas.h"


BaseContext* BaseContext::Unwrap(Handle<Object> obj) {
    Handle<External> field = Handle<External>::Cast(obj->GetInternalField(0));
    void* ptr = field->Value();
    return static_cast<BaseContext*>(ptr);
}

void BaseContext::FillRect(const v8::FunctionCallbackInfo<Value>& args) {
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

void BaseContext::Save(const v8::FunctionCallbackInfo<Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->save();
}

void BaseContext::Restore(const v8::FunctionCallbackInfo<Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->restore();
}

void BaseContext::Rotate(const v8::FunctionCallbackInfo<Value>& args) {
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

void BaseContext::Translate(const v8::FunctionCallbackInfo<Value>& args) {
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

void BaseContext::ResetTransform(const v8::FunctionCallbackInfo<Value>& args) {
    BaseContext* BaseContext = Unwrap(args.This());
    SkCanvas* canvas = BaseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }

    canvas->resetMatrix();
}

void BaseContext::Stroke(const v8::FunctionCallbackInfo<Value>& args) {
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

    Handle<External> field = Handle<External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path2D* path = static_cast<Path2D*>(ptr);

    canvas->drawPath(path->getSkPath(), BaseContext->fStrokeStyle);
}

void BaseContext::Fill(const v8::FunctionCallbackInfo<Value>& args) {
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

    Handle<External> field = Handle<External>::Cast(
            args[0]->ToObject()->GetInternalField(0));
    void* ptr = field->Value();
    Path2D* path = static_cast<Path2D*>(ptr);

    canvas->drawPath(path->getSkPath(), BaseContext->fFillStyle);
}

void BaseContext::GetStyle(Local<String> name,
                         const PropertyCallbackInfo<Value>& info,
                         const SkPaint& style) {
    char buf[8];
    SkColor color = style.getColor();
    sprintf(buf, "#%02X%02X%02X", SkColorGetR(color), SkColorGetG(color),
            SkColorGetB(color));

    info.GetReturnValue().Set(String::NewFromUtf8(info.GetIsolate(), buf));
}

void BaseContext::SetStyle(Local<String> name, Local<Value> value,
                         const PropertyCallbackInfo<void>& info,
                         SkPaint& style) {
    Local<String> s = value->ToString();
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

void BaseContext::GetFillStyle(Local<String> name,
                             const PropertyCallbackInfo<Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    GetStyle(name, info, baseContext->fFillStyle);
}

void BaseContext::GetStrokeStyle(Local<String> name,
                               const PropertyCallbackInfo<Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    GetStyle(name, info, baseContext->fStrokeStyle);
}

void BaseContext::SetFillStyle(Local<String> name, Local<Value> value,
                            const PropertyCallbackInfo<void>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SetStyle(name, value, info, baseContext->fFillStyle);
}

void BaseContext::SetStrokeStyle(Local<String> name, Local<Value> value,
                               const PropertyCallbackInfo<void>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SetStyle(name, value, info, baseContext->fStrokeStyle);
}


void BaseContext::GetWidth(Local<String> name,
                         const PropertyCallbackInfo<Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SkCanvas* canvas = baseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }
    SkISize size = canvas->getDeviceSize();

    info.GetReturnValue().Set(
            Int32::New(baseContext->fGlobal->getIsolate(), size.fWidth));
}

void BaseContext::GetHeight(Local<String> name,
                         const PropertyCallbackInfo<Value>& info) {
    BaseContext* baseContext = Unwrap(info.This());
    SkCanvas* canvas = baseContext->getCanvas();
    if (NULL == canvas) {
        return;
    }
    SkISize size = canvas->getDeviceSize();

    info.GetReturnValue().Set(
            Int32::New(baseContext->fGlobal->getIsolate(), size.fHeight));
}

#define ADD_METHOD(name, fn) \
    tmpl->Set(String::NewFromUtf8( \
         fGlobal->getIsolate(), name, \
         String::kInternalizedString), \
             FunctionTemplate::New(fGlobal->getIsolate(), fn))

void BaseContext::addAttributesAndMethods(Handle<ObjectTemplate> tmpl) {
    HandleScope scope(fGlobal->getIsolate());

    // Add accessors for each of the fields of the context object.
    tmpl->SetAccessor(String::NewFromUtf8(
        fGlobal->getIsolate(), "fillStyle", String::kInternalizedString),
            GetFillStyle, SetFillStyle);
    tmpl->SetAccessor(String::NewFromUtf8(
        fGlobal->getIsolate(), "strokeStyle", String::kInternalizedString),
            GetStrokeStyle, SetStrokeStyle);
    tmpl->SetAccessor(String::NewFromUtf8(
        fGlobal->getIsolate(), "width", String::kInternalizedString),
            GetWidth);
    tmpl->SetAccessor(String::NewFromUtf8(
        fGlobal->getIsolate(), "height", String::kInternalizedString),
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
