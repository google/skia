/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "SkArenaAlloc.h"
#include "SkEndian.h"
#include "SkTypes.h"

class SkWStream;

namespace skjson {

class alignas(8) Value {
public:
    enum class Type {
        kNull,
        kBool,
        kNumber,
        kString,
        kArray,
        kObject,
    };

    Type getType() const;

    template <typename T>
    bool is() const { return T::IsType(this->getType()); }

    template <typename T>
    const T& as() const {
        return *reinterpret_cast<const T*>(this);
    }

    static const Value& Null();

protected:
    uint8_t   fData8[8];
};

template <Value::Type vtype>
class TypeBoundValue : public Value {
public:
    static bool IsType(Value::Type t) { return t == vtype; }
};

template <typename T, Value::Type vtype>
class PrimitiveValue : public TypeBoundValue<vtype> {
public:
    T operator *() const;
};

template <typename T, Value::Type vtype>
class VectorValue : public TypeBoundValue<vtype> {
public:
    size_t size() const;

    const T* begin() const;
    const T*   end() const;

    const T& operator[](size_t i) const {
        SkASSERT(i < this->size());
        return *(this->begin() + i);
    }
};

struct Member { Value fKey, fValue; };

using   NullValue = TypeBoundValue<Value::Type::kNull>;
using   BoolValue = PrimitiveValue<bool , Value::Type::kBool  >;
using NumberValue = PrimitiveValue<float, Value::Type::kNumber>;
using StringValue =    VectorValue<char , Value::Type::kString>;
using  ArrayValue =    VectorValue<Value, Value::Type::kArray >;

class ObjectValue final : public VectorValue<Member, Value::Type::kObject> {
public:
    const Value& operator[](const char*) const;
};

class Dom final : public SkNoncopyable {
public:
    explicit Dom(const char*);

    const Value& root() const { return *fRoot; }

    void write(SkWStream*) const;

private:
    SkArenaAlloc fAlloc;
    const Value* fRoot;
};

} // namespace skjson

#endif // SkJSON_DEFINED

