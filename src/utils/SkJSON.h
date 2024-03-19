/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSON_DEFINED
#define SkJSON_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "src/base/SkArenaAlloc.h"

#include <cstdint>
#include <cstring>
#include <string_view>

class SkString;
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
 *  They can be converted to facade types for type-specific functionality.
 *
 *  E.g.:
 *
 *     if (v.is<ArrayValue>()) {
 *         for (const auto& item : v.as<ArrayValue>()) {
 *             if (const NumberValue* n = item) {
 *                 printf("Found number: %f", **n);
 *             }
 *         }
 *     }
 *
 *     if (v.is<ObjectValue>()) {
 *         const StringValue* id = v.as<ObjectValue>()["id"];
 *         if (id) {
 *             printf("Found object ID: %s", id->begin());
 *         } else {
 *             printf("Missing object ID");
 *         }
 *     }
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
     * @return    The type of this value.
     */
    Type getType() const;

    /**
     * @return    True if the record matches the facade type T.
     */
    template <typename T>
    bool is() const { return this->getType() == T::kType; }

    /**
     * Unguarded conversion to facade types.
     *
     * @return    The record cast as facade type T&.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return *reinterpret_cast<const T*>(this);
    }

    /**
     * Guarded conversion to facade types.
     *
     * @return    The record cast as facade type T*.
     */
    template <typename T>
    operator const T*() const {
        return this->is<T>() ? &this->as<T>() : nullptr;
    }

    /**
     * @return    The string representation of this value.
     */
    SkString toString() const;

    /**
     * Helper for fluent key lookup: v["foo"]["bar"]["baz"]
     *
     * @return    The lookup result value on success, otherwise NullValue.
     */
    const Value& operator[](const char* key) const;

protected:
    /*
      Value implementation notes:

        -- fixed 64-bit size

        -- 8-byte aligned

        -- union of:

             bool
             int32
             float
             char[8] (short string storage)
             external payload (tagged) pointer

         -- lowest 3 bits reserved for tag storage

     */
    enum class Tag : uint8_t {
        // n.b.: we picked kShortString == 0 on purpose,
        // to enable certain short-string optimizations.
        kShortString                  = 0b00000000,  // inline payload
        kNull                         = 0b00000001,  // no payload
        kBool                         = 0b00000010,  // inline payload
        kInt                          = 0b00000011,  // inline payload
        kFloat                        = 0b00000100,  // inline payload
        kString                       = 0b00000101,  // ptr to external storage
        kArray                        = 0b00000110,  // ptr to external storage
        kObject                       = 0b00000111,  // ptr to external storage
    };
    inline static constexpr uint8_t kTagMask = 0b00000111;

    void init_tagged(Tag);
    void init_tagged_pointer(Tag, void*);

    Tag getTag() const {
        return static_cast<Tag>(fData8[0] & kTagMask);
    }

    // Access the record payload as T.
    //
    // Since the tag is stored in the lower bits, we skip the first word whenever feasible.
    //
    // E.g. (U == unused)
    //
    //   uint8_t
    //    -----------------------------------------------------------------------
    //   |TAG| U  |  val8  |   U    |   U    |   U    |   U    |   U    |   U    |
    //    -----------------------------------------------------------------------
    //
    //   uint16_t
    //    -----------------------------------------------------------------------
    //   |TAG|      U      |      val16      |        U        |        U        |
    //    -----------------------------------------------------------------------
    //
    //   uint32_t
    //    -----------------------------------------------------------------------
    //   |TAG|             U                 |                val32              |
    //    -----------------------------------------------------------------------
    //
    //   T* (32b)
    //    -----------------------------------------------------------------------
    //   |TAG|             U                 |             T* (32bits)           |
    //    -----------------------------------------------------------------------
    //
    //   T* (64b)
    //    -----------------------------------------------------------------------
    //   |TAG|                        T* (61bits)                                |
    //    -----------------------------------------------------------------------
    //
    template <typename T>
    const T* cast() const {
        static_assert(sizeof (T) <=  sizeof(Value), "");
        static_assert(alignof(T) <= alignof(Value), "");

        return (sizeof(T) > sizeof(*this) / 2)
                ? reinterpret_cast<const T*>(this) + 0  // need all the bits
                : reinterpret_cast<const T*>(this) + 1; // skip the first word (where the tag lives)
    }

    template <typename T>
    T* cast() { return const_cast<T*>(const_cast<const Value*>(this)->cast<T>()); }

    // Access the pointer payload.
    template <typename T>
    const T* ptr() const {
        static_assert(sizeof(uintptr_t)     == sizeof(Value) ||
                      sizeof(uintptr_t) * 2 == sizeof(Value), "");

        return (sizeof(uintptr_t) < sizeof(Value))
            // For 32-bit, pointers are stored unmodified.
            ? *this->cast<const T*>()
            // For 64-bit, we use the lower bits of the pointer as tag storage.
            : reinterpret_cast<T*>(*this->cast<uintptr_t>() & ~static_cast<uintptr_t>(kTagMask));
    }

private:
    inline static constexpr size_t kValueSize = 8;

    uint8_t fData8[kValueSize];

#if !defined(SK_CPU_LENDIAN)
    // The current value layout assumes LE and will take some tweaking for BE.
    static_assert(false, "Big-endian builds are not supported at this time.");
#endif
};

class NullValue final : public Value {
public:
    inline static constexpr Type kType = Type::kNull;

    NullValue();
};

class BoolValue final : public Value {
public:
    inline static constexpr Type kType = Type::kBool;

    explicit BoolValue(bool);

    bool operator *() const {
        SkASSERT(this->getTag() == Tag::kBool);
        return *this->cast<bool>();
    }
};

class NumberValue final : public Value {
public:
    inline static constexpr Type kType = Type::kNumber;

    explicit NumberValue(int32_t);
    explicit NumberValue(float);

    double operator *() const {
        SkASSERT(this->getTag() == Tag::kInt ||
                 this->getTag() == Tag::kFloat);

        return this->getTag() == Tag::kInt
            ? static_cast<double>(*this->cast<int32_t>())
            : static_cast<double>(*this->cast<float>());
    }
};

template <typename T, Value::Type vtype>
class VectorValue : public Value {
public:
    using ValueT = T;
    inline static constexpr Type kType = vtype;

    size_t size() const {
        SkASSERT(this->getType() == kType);
        return *this->ptr<size_t>();
    }

    const T* begin() const {
        SkASSERT(this->getType() == kType);
        const auto* size_ptr = this->ptr<size_t>();
        return reinterpret_cast<const T*>(size_ptr + 1);
    }

    const T* end() const {
        SkASSERT(this->getType() == kType);
        const auto* size_ptr = this->ptr<size_t>();
        return reinterpret_cast<const T*>(size_ptr + 1) + *size_ptr;
    }

    const T& operator[](size_t i) const {
        SkASSERT(this->getType() == kType);
        SkASSERT(i < this->size());

        return *(this->begin() + i);
    }
};

class ArrayValue final : public VectorValue<Value, Value::Type::kArray> {
public:
    ArrayValue(const Value* src, size_t size, SkArenaAlloc& alloc);
};

class StringValue final : public Value {
public:
    inline static constexpr Type kType = Type::kString;

    StringValue();
    StringValue(const char* src, SkArenaAlloc& alloc);
    StringValue(const char* src, size_t size, SkArenaAlloc& alloc);

    size_t size() const {
        switch (this->getTag()) {
        case Tag::kShortString:
            // We don't bother storing a length for short strings on the assumption
            // that strlen is fast in this case.  If this becomes problematic, we
            // can either go back to storing (7-len) in the tag byte or write a fast
            // short_strlen.
            return strlen(this->cast<char>());
        case Tag::kString:
            return this->cast<VectorValue<char, Value::Type::kString>>()->size();
        default:
            return 0;
        }
    }

    const char* begin() const {
        return this->getTag() == Tag::kShortString
            ? this->cast<char>()
            : this->cast<VectorValue<char, Value::Type::kString>>()->begin();
    }

    const char* end() const {
        return this->getTag() == Tag::kShortString
            ? strchr(this->cast<char>(), '\0')
            : this->cast<VectorValue<char, Value::Type::kString>>()->end();
    }

    std::string_view str() const {
        return std::string_view(this->begin(), this->size());
    }
};

struct Member {
    StringValue fKey;
          Value fValue;
};

class ObjectValue final : public VectorValue<Member, Value::Type::kObject> {
public:
    ObjectValue(const Member* src, size_t size, SkArenaAlloc& alloc);

    const Value& operator[](const char* key) const {
        static const Value gNullValue = NullValue();

        const auto* member = this->find(key);
        return member
            ? member->fValue
            : gNullValue;
    }

    // Writable access to the value associated with the given key.
    // If the key is not present, it is added with a default NullValue.
    Value& writable(const char* key, SkArenaAlloc&) const;

private:
    const Member* find(const char*) const;
};

class DOM final : public SkNoncopyable {
public:
    DOM(const char*, size_t);

    const Value& root() const { return fRoot; }

    void write(SkWStream*) const;

private:
    SkArenaAlloc fAlloc;
    Value        fRoot;
};

inline Value::Type Value::getType() const {
    switch (this->getTag()) {
    case Tag::kNull:        return Type::kNull;
    case Tag::kBool:        return Type::kBool;
    case Tag::kInt:         return Type::kNumber;
    case Tag::kFloat:       return Type::kNumber;
    case Tag::kShortString: return Type::kString;
    case Tag::kString:      return Type::kString;
    case Tag::kArray:       return Type::kArray;
    case Tag::kObject:      return Type::kObject;
    }

    SkASSERT(false); // unreachable
    return Type::kNull;
}

inline const Value& Value::operator[](const char* key) const {
    static const Value gNullValue = NullValue();

    return this->is<ObjectValue>()
        ? this->as<ObjectValue>()[key]
        : gNullValue;
}

} // namespace skjson

#endif // SkJSON_DEFINED
