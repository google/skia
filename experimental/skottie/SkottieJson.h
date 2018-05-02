/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieJson_DEFINED
#define SkottieJson_DEFINED

#include "SkJSONCPP.h"

class SkStream;
class SkString;

namespace skottie {

namespace json {

class ValueRef {
public:
    ValueRef() : fValue(&Json::Value::null) {}
    explicit ValueRef(const Json::Value& v) : fValue(&v) {}

    bool isNull()   const { return fValue->isNull();   }
    bool isObject() const { return fValue->isObject(); }
    bool isArray()  const { return fValue->isArray();  }

    template <typename T>
    bool as(T*) const;

    template <typename T>
    T asDefault(const T& defaultValue) const {
        T v;
        if (!this->as<T>(&v)) {
            v = defaultValue;
        }
        return v;
    }

    size_t size() const {
        return this->isArray() ? fValue->size() : 0;
    }

    ValueRef operator[](size_t i) const {
        return ValueRef(fValue->operator[](static_cast<Json::ArrayIndex>(i)));
    }

    ValueRef operator[](const char* key) const {
        return ValueRef(fValue->operator[](key));
    }

    bool operator==(const ValueRef& other) const { return fValue == other.fValue; }
    bool operator!=(const ValueRef& other) const { return !(*this == other); }

    template <typename F>
    void forEach(F&& f) const {
        for (const auto& prop : *fValue) {
            if (!f(ValueRef(prop)))
                break;
        }
    }

    SkString toString() const;

private:
    const Json::Value* fValue;
};

Json::Value ParseStream(SkStream*);

} // namespace json

} // namespace skottie

#endif // SkottieJson_DEFINED
