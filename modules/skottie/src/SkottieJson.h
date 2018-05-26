/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieJson_DEFINED
#define SkottieJson_DEFINED

#include "SkRefCnt.h"

#include "rapidjson/document.h"

class SkData;
class SkStream;
class SkString;

namespace skottie {

namespace json {

class ValueRef {
public:
    ValueRef() : fValue(nullptr) {}
    ValueRef(const rapidjson::Value& v) : fValue(v.IsNull() ? nullptr : &v) {}

    bool isNull()   const { return !fValue;   }
    bool isObject() const { return fValue && fValue->IsObject(); }
    bool isArray()  const { return fValue && fValue->IsArray();  }

    template <typename T>
    bool to(T*) const;

    template <typename T>
    T toDefault(const T& defaultValue) const {
        T v;
        if (!this->to<T>(&v)) {
            v = defaultValue;
        }
        return v;
    }

    size_t size() const;
    ValueRef operator[](size_t i) const;
    ValueRef operator[](const char* key) const;

    bool operator==(const ValueRef& other) const { return fValue == other.fValue; }
    bool operator!=(const ValueRef& other) const { return !(*this == other); }

    const rapidjson::Value* begin() const;
    const rapidjson::Value* end() const;

    SkString toString() const;

private:
    const rapidjson::Value*       fValue;
};

// Container for the json DOM
class Document {
public:
    explicit Document(SkStream*);

    ValueRef root() const { return fDocument; }

private:
    sk_sp<SkData>       fData;     // raw data
    rapidjson::Document fDocument; // in-place json DOM
};

} // namespace json

} // namespace skottie

#endif // SkottieJson_DEFINED
