/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkData.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTime.h"

#include "pjson.h"

#include <cmath>

namespace skjson {

static_assert( sizeof(Value) == 8, "");
static_assert(alignof(Value) == 8, "");

static constexpr size_t kRecAlign = alignof(Value);

namespace {

class ValueImpl : public Value {
public:
    enum ImplType {
        kNull        = 0x00,
        kBool        = 0x01,
        kInt         = 0x02, // TODO
        kFloat       = 0x03,
        kShortString = 0x04,
        kString      = 0x05,
        kArray       = 0x06,
        kObject      = 0x07,
    };
    static constexpr uint64_t kTypeMask = 0x07;

    ImplType getImplType() const {
        return static_cast<ImplType>(this->data<uint64_t>() & kTypeMask);
    }

    template <typename T>
    const T& data() const {
        static_assert(sizeof (T) <=  sizeof(ValueImpl), "");
        static_assert(alignof(T) <= alignof(ValueImpl), "");
        return *reinterpret_cast<const T*>(this);
    }

    template <typename T>
    T& data() { return const_cast<T&>(const_cast<const ValueImpl*>(this)->data<T>()); }

    template <typename T>
    const T& payload() const {
        static_assert(2 * sizeof(T) <= sizeof(Value), "");
        const T* base = reinterpret_cast<const T*>(this);
#if defined(SK_CPU_LENDIAN)
        return *(base + 1);
#else
        return *base;
#endif
    }

    template <typename T>
    T& payload() { return const_cast<T&>(const_cast<const ValueImpl*>(this)->payload<T>()); }

    template <typename T>
    const T* ptr() const {
        return reinterpret_cast<T*>(this->data<uintptr_t>() & ~kTypeMask);
    }


    static ValueImpl MakeTypeBound(ImplType t) {
        ValueImpl v;
        v.data<uint64_t>() = t;
        SkASSERT(v.getImplType() == t);
        return v;
    }

    template <typename T>
    static ValueImpl MakePrimitive(ImplType t, T src) {
        ValueImpl v = MakeTypeBound(t);
        v.payload<T>() = src;
        SkASSERT(v.getImplType() == t);
        return v;
    }

    template <typename T>
    static ValueImpl MakeVector(ImplType t, const T* src, size_t size, SkArenaAlloc& alloc) {
        ValueImpl v;
        if (!size) {
            v.data<uint64_t>() = t;
        } else {
            // The Ts are already in memory, so their size should be safe.
            // TODO: should we worry about the extra size_t?
            const auto total_size = sizeof(size_t) + sizeof(T) * size;
            auto* size_ptr = reinterpret_cast<size_t*>(alloc.makeBytesAlignedTo(total_size,
                                                                                kRecAlign));
            auto* data_ptr = reinterpret_cast<T*>(size_ptr + 1);
            *size_ptr = size;
            memcpy(data_ptr, src, sizeof(T) * size);

            if (sizeof(void*) < sizeof(Value)) v.data<uint64_t>() = 0;
            v.data<uintptr_t>() = reinterpret_cast<uintptr_t>(size_ptr);
            SkASSERT(!(v.data<uintptr_t>() & kTypeMask));
            SkASSERT(!(v.data<uint64_t >() & kTypeMask));
            SkASSERT((t & kTypeMask) == t);
            v.data<uint64_t>() |= t;

            SkASSERT(v.ptr<size_t>() == size_ptr);
        }

        SkASSERT(v.getImplType() == t);
        return v;
    }

    static size_t VectorSize(const Value& v, ImplType t) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);
        SkASSERT(impl.getImplType() == t);

        const auto* size_ptr = impl.ptr<const size_t>();
        return size_ptr ? *size_ptr : 0;
    }

    template <typename T>
    static const T* VectorBegin(const Value& v, ImplType t) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);
        SkASSERT(impl.getImplType() == t);

        const auto* size_ptr = impl.ptr<const size_t>();
        return size_ptr ? reinterpret_cast<const T*>(size_ptr + 1) : nullptr;
    }

    template <typename T>
    static const T* VectorEnd(const Value& v, ImplType t) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);
        SkASSERT(impl.getImplType() == t);

        const auto* size_ptr = impl.ptr<const size_t>();
        return size_ptr ? reinterpret_cast<const T*>(size_ptr + 1) + *size_ptr : nullptr;
    }

    static constexpr size_t kMaxInlineStringSize = sizeof(Value) - 2;

    static ValueImpl MakeString(const char* src, size_t size, SkArenaAlloc& alloc) {

        if (size > kMaxInlineStringSize)
            return MakeVector<char>(kString, src, size, alloc);

        ValueImpl v = MakeTypeBound(kShortString);
        char* payload = &v.payload<char>();
        memcpy(payload, src, size);
        if (size < kMaxInlineStringSize)
            payload[size] = '\0';
        payload[kMaxInlineStringSize] = SkTo<char>(kMaxInlineStringSize - size);

        return v;
    }

    static size_t StringSize(const Value& v) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);

        if (impl.getImplType() == ValueImpl::kShortString) {
            const char* payload = &impl.payload<char>();
            return kMaxInlineStringSize - SkToSizeT(payload[kMaxInlineStringSize]);
        }

        return VectorSize(v, ValueImpl::kString);
    }

    static const char* StringBegin(const Value& v) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);

        if (impl.getImplType() == ValueImpl::kShortString) {
            return &impl.payload<char>();
        }

        return VectorBegin<char>(v, ValueImpl::kString);
    }

    static const char* StringEnd(const Value& v) {
        const auto& impl = *reinterpret_cast<const ValueImpl*>(&v);

        if (impl.getImplType() == ValueImpl::kShortString) {
            const char* payload = &impl.payload<char>();
            return payload + kMaxInlineStringSize - SkToSizeT(payload[kMaxInlineStringSize]);
        }

        return VectorEnd<char>(v, ValueImpl::kString);
    }
};

static const Value g_null_rec = ValueImpl::MakeTypeBound(ValueImpl::kNull);

} // namespace

// Public Value glue.

const Value& Value::Null() {
    static const Value g_null = ValueImpl::MakeTypeBound(ValueImpl::kNull);
    return g_null;
}

Value::Type Value::getType() const {
    static constexpr Value::Type kTypeMap[] = {
        Value::Type::kNull,   // kNull
        Value::Type::kBool,   // kBool
        Value::Type::kNumber, // kInt
        Value::Type::kNumber, // kFloat
        Value::Type::kString, // kShortString
        Value::Type::kString, // kString
        Value::Type::kArray,  // kArray
        Value::Type::kObject, // kObject
    };

    const auto& impl = *reinterpret_cast<const ValueImpl*>(this);
    SkASSERT(static_cast<size_t>(impl.getImplType()) < SK_ARRAY_COUNT(kTypeMap));

    return kTypeMap[impl.getImplType()];
}

template <>
bool PrimitiveValue<bool, Value::Type::kBool>::operator*() const {
    const auto& impl = *reinterpret_cast<const ValueImpl*>(this);

    return impl.payload<bool>();
}

template <>
float PrimitiveValue<float, Value::Type::kNumber>::operator*() const {
    const auto& impl = *reinterpret_cast<const ValueImpl*>(this);

    SkASSERT(impl.getImplType() == ValueImpl::kInt ||
             impl.getImplType() == ValueImpl::kFloat);

    return impl.getImplType() == ValueImpl::kInt
        ? static_cast<float>(impl.payload<int32_t>()) : impl.payload<float>();
}

template <>
size_t VectorValue<Value, Value::Type::kArray>::size() const {
    return ValueImpl::VectorSize(*this, ValueImpl::kArray);
}

template <>
const Value* VectorValue<Value, Value::Type::kArray>::begin() const {
    return ValueImpl::VectorBegin<Value>(*this, ValueImpl::kArray);
}

template <>
const Value* VectorValue<Value, Value::Type::kArray>::end() const {
    return ValueImpl::VectorEnd<Value>(*this, ValueImpl::kArray);
}

template <>
size_t VectorValue<Member, Value::Type::kObject>::size() const {
    return ValueImpl::VectorSize(*this, ValueImpl::kObject);
}

template <>
const Member* VectorValue<Member, Value::Type::kObject>::begin() const {
    return ValueImpl::VectorBegin<Member>(*this, ValueImpl::kObject);
}

template <>
const Member* VectorValue<Member, Value::Type::kObject>::end() const {
    return ValueImpl::VectorEnd<Member>(*this, ValueImpl::kObject);
}

template <>
size_t VectorValue<char, Value::Type::kString>::size() const {
    return ValueImpl::StringSize(*this);
}

template <>
const char* VectorValue<char, Value::Type::kString>::begin() const {
    return ValueImpl::StringBegin(*this);
}

template <>
const char* VectorValue<char, Value::Type::kString>::end() const {
    return ValueImpl::StringEnd(*this);
}

namespace {

// bit 0 (1)    - plain ASCII string character
// bit 1 (2)    - whitespace
// bit 2 (4)    - string terminator (" \0 control chars)
// bit 3 (8)    - 0-9
// bit 4 (0x10) - 0-9 e E .
alignas(256) static constexpr uint8_t g_token_flags[256] = {
 // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
    4,   4,   4,   4,   4,   4,   4,   4,     4,   6,   6,   4,   4,   6,   4,   4, // 0
    4,   4,   4,   4,   4,   4,   4,   4,     4,   4,   4,   4,   4,   4,   4,   4, // 1
    3,   1,   4,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
 0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19,  0x19,0x19,   1,   1,   1,   1,   1,   1, // 3
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 4
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   0,   1,   1,   1, // 5
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 6
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 7

 // 128-255
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
};

static inline bool is_ws(char c)          { return g_token_flags[static_cast<uint8_t>(c)] & 0x02; }
static inline bool is_sterminator(char c) { return g_token_flags[static_cast<uint8_t>(c)] & 0x04; }
static inline bool is_decimal(char c)     { return g_token_flags[static_cast<uint8_t>(c)] & 0x08; }
static inline bool is_numeric(char c)     { return g_token_flags[static_cast<uint8_t>(c)] & 0x10; }

static constexpr float g_pow10_table[63] =
{
   1.e-031f, 1.e-030f, 1.e-029f, 1.e-028f, 1.e-027f, 1.e-026f, 1.e-025f, 1.e-024f,
   1.e-023f, 1.e-022f, 1.e-021f, 1.e-020f, 1.e-019f, 1.e-018f, 1.e-017f, 1.e-016f,
   1.e-015f, 1.e-014f, 1.e-013f, 1.e-012f, 1.e-011f, 1.e-010f, 1.e-009f, 1.e-008f,
   1.e-007f, 1.e-006f, 1.e-005f, 1.e-004f, 1.e-003f, 1.e-002f, 1.e-001f, 1.e+000f,
   1.e+001f, 1.e+002f, 1.e+003f, 1.e+004f, 1.e+005f, 1.e+006f, 1.e+007f, 1.e+008f,
   1.e+009f, 1.e+010f, 1.e+011f, 1.e+012f, 1.e+013f, 1.e+014f, 1.e+015f, 1.e+016f,
   1.e+017f, 1.e+018f, 1.e+019f, 1.e+020f, 1.e+021f, 1.e+022f, 1.e+023f, 1.e+024f,
   1.e+025f, 1.e+026f, 1.e+027f, 1.e+028f, 1.e+029f, 1.e+030f, 1.e+031f
};

static inline float pow10(int32_t exp) {
    static constexpr int32_t k_exp_offset = SK_ARRAY_COUNT(g_pow10_table) / 2;

    // We only support negative exponents for now.
    SkASSERT(exp <= 0);

    return (exp >= -k_exp_offset) ? g_pow10_table[exp + k_exp_offset]
                                  : std::pow(10.0f, static_cast<float>(exp));
}

class DomParser {
public:
    explicit DomParser(SkArenaAlloc& alloc) : fAlloc(alloc) {}

    const Value& parse(const char* p) {
        p = skip_ws(p);

        switch (*p) {
        case '{':
            goto match_object;
        case '[':
            goto match_array;
        default:
            return error(p, "invalid top-level value");
        }

match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->onPushObject();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
match_object_key:
        p = skip_ws(p);
        if (*p != '"') return error(p, "invalid object key");

        {
            const auto* string_start = p + 1;
            if (!(p = match_string(p))) return error(p, "invalid string");

            SkASSERT(p >= string_start);

            this->onObjectKey(string_start, p - string_start - 1);
        }

        p = skip_ws(p);
        if (*p != ':') return error(p, "expected ':' separator");

        ++p;

        // goto match_value;
match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return error(p, "unexpected input end");
        case '"': {
            const auto* string_start = p + 1;
            if (!(p = match_string(p))) return error(p, "invalid string");
            SkASSERT(p > string_start);
            this->onString(string_start, p - string_start - 1);
            break;
        }
        case '[':
            goto match_array;
        case 'f':
            if (!(p = match_false(p))) return error(p, "unexpected token");
            this->onFalse();
            break;
        case 'n':
            if (!(p = match_null(p))) return error(p, "unexpected token");
            this->onNull();
            break;
        case 't':
            if (!(p = match_true(p))) return error(p, "unexpected token");
            this->onTrue();
            break;
        case '{':
            goto match_object;
        default: {
            float f;
            if (!(p = match_number(p, &f))) return error(p, "unexpected token");
            this->onNumber(f);
            break;
        }
        }

        // goto match_post_value;

match_post_value:
        SkASSERT(!fScopeStack.empty());

        p = skip_ws(p);
        switch (*p) {
        case ',':
            ++p;
            if (fScopeStack.back() >= 0) {
                goto match_object_key;
            } else {
                goto match_value;
            }
        case ']':
            goto pop_array;
        case '}':
            goto pop_object;
        default:
            return error(p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

pop_object:
        SkASSERT(*p == '}');

        if (fScopeStack.back() < 0)
            return error(p, "unexpected object terminator");

        this->onPopObject();

        // goto pop_common
pop_common:
        SkASSERT(*p == '}' || *p == ']');

        ++p;

        if (fScopeStack.empty()) {
            SkASSERT(fValueStack.count() == 1);
            auto* root = fAlloc.make<Value>();
            *root = fValueStack.front();

            // Stop condition: parsed the top level element and there is no trailing garbage.
            p = skip_ws(p);
            return *p == '\0' ? *root : g_null_rec;
        }

        goto match_post_value;

match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        this->onPushArray();

        if (*p != ']') goto match_value;

        // goto pop_array;
pop_array:
        SkASSERT(*p == ']');

        if (fScopeStack.back() >= 0)
            return error(p, "unexpected array terminator");

        this->onPopArray();

        goto pop_common;

        SkASSERT(false);
        return g_null_rec;
    }

private:
    SkArenaAlloc&                 fAlloc;
    SkSTArray<256, Value  , true> fValueStack;
    SkSTArray<128, int32_t, true> fScopeStack;

    void onPushObject() {
        fScopeStack.push_back(fValueStack.count());
    }

    void onPopObject() {
        SkASSERT(fScopeStack.back() >= 0);
        const auto scope_start = fScopeStack.back();
        SkASSERT(scope_start <= fValueStack.count());

        static_assert(alignof(Value) == alignof(Member), "");
        const Member* member_begin =
            reinterpret_cast<const Member*>(fValueStack.begin() + scope_start);

        const auto scope_count = SkToSizeT(fValueStack.count() - scope_start);
        SkASSERT(!(scope_count & 1));
        const auto member_count = scope_count / 2;

        const Value object =
            ValueImpl::MakeVector<Member>(ValueImpl::kObject, member_begin, member_count, fAlloc);

        fScopeStack.pop_back();
        fValueStack.pop_back_n(scope_count);
        fValueStack.push_back(object);
    }

    void onPushArray() {
        fScopeStack.push_back(-fValueStack.count() - 1);
    }

    void onPopArray() {
        SkASSERT(fScopeStack.back() < 0);
        const auto scope_start = -(fScopeStack.back() + 1);
        SkASSERT(scope_start <= fValueStack.count());

        const auto* value_begin = fValueStack.begin() + scope_start;
        const auto  value_count = SkToU32(fValueStack.count() - scope_start);
        const Value array =
            ValueImpl::MakeVector<Value>(ValueImpl::kArray, value_begin, value_count, fAlloc);

        fScopeStack.pop_back();
        fValueStack.pop_back_n(value_count);
        fValueStack.push_back(array);
    }

    void onObjectKey(const char* key, size_t size) {
        SkASSERT(fScopeStack.back() >= 0);
        SkASSERT(fValueStack.count() >= fScopeStack.back());
        SkASSERT(!((fValueStack.count() - fScopeStack.back()) & 1));
        this->onString(key, size);
    }

    void onTrue() {
        fValueStack.push_back(ValueImpl::MakePrimitive<bool>(ValueImpl::kBool, true));
    }

    void onFalse() {
        fValueStack.push_back(ValueImpl::MakePrimitive<bool>(ValueImpl::kBool, false));
    }

    void onNull() {
        fValueStack.push_back(ValueImpl::MakeTypeBound(ValueImpl::kNull));
    }

    void onString(const char* s, size_t size) {
        fValueStack.push_back(ValueImpl::MakeString(s, size, fAlloc));
    }

    void onNumber(float n) {
        fValueStack.push_back(ValueImpl::MakePrimitive<float>(ValueImpl::kFloat, n));
    }

    const Value& error(const char* p, const char* msg) {
        SkDebugf("*** Parse error [%s]: %s\n", msg, p);
        return g_null_rec;
    }

    static inline const char* skip_ws(const char* p) {
        while (is_ws(*p)) ++p;
        return p;
    }

    static const char* match_true(const char* p) {
        SkASSERT(p[0] == 't');

        return (p[1] == 'r' && p[2] == 'u' && p[3] == 'e') ? p + 4 : nullptr;
    }

    static const char* match_false(const char* p) {
        SkASSERT(p[0] == 'f');

        return (p[1] == 'a' && p[2] == 'l' && p[3] == 's' && p[4] == 'e') ? p + 5 : nullptr;
    }

    static const char* match_null(const char* p) {
        SkASSERT(p[0] == 'n');

        return (p[1] == 'u' && p[2] == 'l' && p[3] == 'l') ? p + 4 : nullptr;
    }

    static const char* match_string(const char* p) {
        SkASSERT(*p == '"');

        // TODO: unescape
        for (p = p + 1; !is_sterminator(*p); ++p) {}

        return p + (*p == '"');
    }

    const char* match_fast_float_decimal_part(const char* p, float* n,
                                              int sign, float f, int exp) {
        SkASSERT(exp <= 0);

        for (;;) {
            if (!is_decimal(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
            if (!is_decimal(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
        }

        if (is_numeric(*p)) {
            SkASSERT(*p == '.' || *p == 'e' || *p == 'E');
            // We either have malformed input, or an (unsupported) exponent.
            return nullptr;
        }

        *n = sign * f * pow10(exp);

        return p;
    }

    const char* match_fast_float_part(const char* p, float* n, int sign, float f) {
        for (;;) {
            if (!is_decimal(*p)) break;
            f = f * 10.f + (*p++ - '0');
            if (!is_decimal(*p)) break;
            f = f * 10.f + (*p++ - '0');
        }

        if (!is_numeric(*p)) {
            // Matched (integral) float.
            *n = sign * f;
            return p;
        }

        return (*p == '.') ? match_fast_float_decimal_part(p + 1, n, sign, f, 0)
                           : nullptr;
    }

    const char* match_fast_32_or_float(const char* p, float* n) {
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        }

        const auto* digits_start = p;

        int32_t n32 = 0;

        // This is the largest absolute int32 value we can handle before
        // risking overflow *on the next digit* (214748363).
        static constexpr int32_t kMaxInt32 = (std::numeric_limits<int32_t>::max() - 9) / 10;

        if (is_decimal(*p)) {
            n32 = (*p++ - '0');
            for (;;) {
                if (!is_decimal(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0');
            }
        }

        if (!is_numeric(*p)) {
            // Did we actually match any digits?
            if (p > digits_start) {
                *n = static_cast<float>(sign * n32);
                return p;
            }
            return nullptr;
        }

        if (*p == '.') {
            const auto* decimals_start = ++p;

            int exp = 0;

            for (;;) {
                if (!is_decimal(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
                if (!is_decimal(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
            }

            if (!is_numeric(*p)) {
                // Did we actually match any digits?
                if (p > decimals_start) {
                    *n = sign * n32 * pow10(exp);
                    return p;
                }
                return nullptr;
            }

            if (n32 > kMaxInt32) {
                // we ran out on n32 bits
                return match_fast_float_decimal_part(p, n, sign, n32, exp);
            }
        }

        return match_fast_float_part(p, n, sign, n32);
    }

    const char* match_number(const char* p, float* n) {
        if (const auto* fast = match_fast_32_or_float(p, n)) return fast;

        // slow fallback
        char* matched;

        *n = strtod(p, &matched);
        return matched > p ? matched : nullptr;
    }

    SkSTArray<256, char, true> fStringBuffer;
};

void Write(const Value& v, SkWStream* stream) {
    switch (v.getType()) {
    case Value::Type::kNull:
        stream->writeText("null");
        break;
    case Value::Type::kBool:
        stream->writeText(*v.as<BoolValue>() ? "true" : "false");
        break;
    case Value::Type::kNumber:
        stream->writeScalarAsText(*v.as<NumberValue>());
        break;
    case Value::Type::kString:
        stream->writeText("\"");
        stream->writeText(v.as<StringValue>().begin());
        stream->writeText("\"");
        break;
    case Value::Type::kArray: {
        const auto& array = v.as<ArrayValue>();
        stream->writeText("[");
        const auto* end = array.end();
        for (const auto* v = array.begin(); v != end; ++v) {
            Write(*v, stream);
            if (v != end - 1) {
                stream->writeText(",");
            }
        }
        stream->writeText("]");
        break;
    }
    case Value::Type::kObject:
        const auto& object = v.as<ObjectValue>();
        stream->writeText("{");
        const auto* end = object.end();
        for (const auto* member = object.begin(); member != end; ++member) {
            SkASSERT(member->fKey.getType() == Value::Type::kString);
            Write(member->fKey, stream);
            stream->writeText(":");
            Write(member->fValue, stream);
            if (member != end - 1) {
                stream->writeText(",");
            }
        }
        stream->writeText("}");
        break;
    }
}

} // namespace

static constexpr size_t kMinChunkSize = 4096;

Dom::Dom(const char* cstr)
    : fAlloc(kMinChunkSize) {
    DomParser parser(fAlloc);

    fRoot = &parser.parse(cstr);
}

void Dom::write(SkWStream* stream) const {
    Write(*fRoot, stream);
}

} // namespace skjson
