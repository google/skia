/*
* Copyright 2019 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef SkParticleSerialization_DEFINED
#define SkParticleSerialization_DEFINED

#include "SkReflected.h"

#include "SkJSON.h"
#include "SkJSONWriter.h"
#include "SkTArray.h"

class SkToJsonVisitor : public SkFieldVisitor {
public:
    SkToJsonVisitor(SkJSONWriter& writer) : fWriter(writer) {}

    // Primitives
    void visit(const char* name, float& f, SkField) override {
        fWriter.appendFloat(name, f);
    }
    void visit(const char* name, int& i, SkField) override {
        fWriter.appendS32(name, i);
    }
    void visit(const char* name, bool& b, SkField) override {
        fWriter.appendBool(name, b);
    }
    void visit(const char* name, SkString& s, SkField) override {
        fWriter.appendString(name, s.c_str());
    }

    // Compound types
    void visit(const char* name, SkPoint& p, SkField) override {
        fWriter.beginObject(name, false);
        fWriter.appendFloat("x", p.fX);
        fWriter.appendFloat("y", p.fY);
        fWriter.endObject();
    }

    void visit(const char* name, SkColor4f& c, SkField) override {
        fWriter.beginArray(name, false);
        fWriter.appendFloat(c.fR);
        fWriter.appendFloat(c.fG);
        fWriter.appendFloat(c.fB);
        fWriter.appendFloat(c.fA);
        fWriter.endArray();
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        fWriter.appendString("Type", e ? e->getType()->fName : "Null");
    }

    void enterObject(const char* name) override { fWriter.beginObject(name); }
    void exitObject()                  override { fWriter.endObject(); }

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) override {
        fWriter.beginArray(name);
        for (auto ptr : arr) {
            SkFieldVisitor::visit(nullptr, ptr);
        }
        fWriter.endArray();
    }

private:
    SkJSONWriter& fWriter;
};

class SkFromJsonVisitor : public SkFieldVisitor {
public:
    SkFromJsonVisitor(const skjson::Value& v) : fRoot(v) {
        fStack.push_back(&fRoot);
    }

    void visit(const char* name, float& f, SkField) override {
        TryParse(get(name), f);
    }
    void visit(const char* name, int& i, SkField) override {
        TryParse(get(name), i);
    }
    void visit(const char* name, bool& b, SkField) override {
        TryParse(get(name), b);
    }
    void visit(const char* name, SkString& s, SkField) override {
        TryParse(get(name), s);
    }

    void visit(const char* name, SkPoint& p, SkField) override {
        if (const skjson::ObjectValue* obj = get(name)) {
            TryParse((*obj)["x"], p.fX);
            TryParse((*obj)["y"], p.fY);
        }
    }

    void visit(const char* name, SkColor4f& c, SkField) override {
        const skjson::ArrayValue* arr = get(name);
        if (arr && arr->size() == 4) {
            TryParse((*arr)[0], c.fR);
            TryParse((*arr)[1], c.fG);
            TryParse((*arr)[2], c.fB);
            TryParse((*arr)[3], c.fA);
        }
    }

    void visit(const char* name, SkCurve& c, SkField field) override {
        if (get(name).is<skjson::ObjectValue>()) {
            SkFieldVisitor::visit(name, c, field);
        }
    }

    void visit(sk_sp<SkReflected>& e, const SkReflected::Type* baseType) override {
        const skjson::StringValue* typeString = get("Type");
        const char* type = typeString ? typeString->begin() : "Null";
        e = SkReflected::CreateInstance(type);
    }

    void enterObject(const char* name) override {
        fStack.push_back((const skjson::ObjectValue*)get(name));
    }
    void exitObject() override {
        fStack.pop_back();
    }

    void visit(const char* name, SkTArray<sk_sp<SkReflected>>& arr,
               const SkReflected::Type* baseType) override {
        arr.reset();

        if (const skjson::ArrayValue* arrVal = get(name)) {
            arr.reserve(arrVal->size());

            for (const skjson::ObjectValue* obj : *arrVal) {
                sk_sp<SkReflected> ptr = nullptr;
                if (obj) {
                    fStack.push_back(obj);
                    this->visit(ptr, baseType);
                    if (ptr) {
                        ptr->visitFields(this);
                    }
                    fStack.pop_back();
                }
                if (ptr && ptr->isOfType(baseType)) {
                    arr.push_back(ptr);
                }
            }
        }
    }

private:
    const skjson::Value& get(const char* name) const {
        if (const skjson::Value* cur = fStack.back()) {
            if (!name) {
                return *cur;
            } else if (cur->is<skjson::ObjectValue>()) {
                return cur->as<skjson::ObjectValue>()[name];
            }
        }
        static skjson::NullValue gNull;
        return gNull;
    }

    static bool TryParse(const skjson::Value& v, float& f) {
        if (const skjson::NumberValue* num = v) {
            f = static_cast<float>(**num);
            return true;
        }
        return false;
    }

    static bool TryParse(const skjson::Value& v, int& i) {
        if (const skjson::NumberValue* num = v) {
            double dbl = **num;
            i = static_cast<int>(dbl);
            return static_cast<double>(i) == dbl;
        }
        return false;
    }

    static bool TryParse(const skjson::Value& v, SkString& s) {
        if (const skjson::StringValue* str = v) {
            s.set(str->begin(), str->size());
            return true;
        }
        return false;
    }

    static bool TryParse(const skjson::Value& v, bool& b) {
        switch (v.getType()) {
        case skjson::Value::Type::kNumber:
            b = SkToBool(*v.as<skjson::NumberValue>());
            return true;
        case skjson::Value::Type::kBool:
            b = *v.as<skjson::BoolValue>();
            return true;
        default:
            break;
        }

        return false;
    }

    const skjson::Value& fRoot;
    SkSTArray<16, const skjson::Value*, true> fStack;
};

#endif // SkParticleSerialization_DEFINED
