/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "SkArenaAlloc.h"
#include "SkTypes.h"

class SkWStream;

namespace skjson {

/**
 *  A fast and likely non-conforming JSON parser.
 *
 *  Some known limitations/compromises:
 *
 *    -- single-precision FP numbers
 *
 *    -- missing string unescaping (no current users, could be easily added)
 *
 *
 *  Values are opaque, fixed-size (64 bits), immutable records.
 *
 *  They can be freely converted to any of the facade types for type-specific functionality.
 *
 *  Note: type checking is lazy/deferred, to facilitate chained property access - e.g.
 *
 *      if (!v.as<ObjectValue>()["foo"].as<ObjectValue>()["bar"].is<NullValue>())
 *          LOG("found v.foo.bar!");
 */
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

    /**
     * @return    The public type of this record.
     */
    Type getType() const;

    /**
     * @return    True if the record matches the facade type T.
     */
    template <typename T>
    bool is() const { return T::IsType(this->getType()); }

    /**
     * @return    The record cast as facade type T.
     *
     * Note: this is always safe, as proper typing is enforced in the facade methods.
     */
    template <typename T>
    const T& as() const {
        return *reinterpret_cast<const T*>(this->is<T>() ? this : &Value::Null());
    }

    /**
     * @return    Null value singleton.
     */
    static const Value& Null();

protected:
    uint8_t   fData8[8];
};

class NullValue final : public Value {
public:
    static bool IsType(Value::Type t) { return t == Type::kNull; }
};

template <typename T, Value::Type vtype>
class PrimitiveValue final : public Value {
public:
    static bool IsType(Value::Type t) { return t == vtype; }

    T operator *() const;
};

template <typename T, Value::Type vtype>
class VectorValue : public Value {
public:
    static bool IsType(Value::Type t) { return t == vtype; }

    size_t size() const;

    const T* begin() const;
    const T*   end() const;

    const T& operator[](size_t i) const {
        return (i < this->size()) ? *(this->begin() + i) : T::Null();
    }
};

using   BoolValue = PrimitiveValue<bool  , Value::Type::kBool  >;
using NumberValue = PrimitiveValue<double, Value::Type::kNumber>;
using StringValue =    VectorValue<char  , Value::Type::kString>;
using  ArrayValue =    VectorValue<Value , Value::Type::kArray >;

struct Member {
    StringValue fKey;
          Value fValue;

    static const Member& Null();
};

class ObjectValue final : public VectorValue<Member, Value::Type::kObject> {
public:
    const Value& operator[](const char*) const;
};

class DOM final : public SkNoncopyable {
public:
    explicit DOM(const char*);

    const Value& root() const { return *fRoot; }

    void write(SkWStream*) const;

private:
    SkArenaAlloc fAlloc;
    const Value* fRoot;
};

} // namespace skjson

#endif // SkJSON_DEFINED

