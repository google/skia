/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkStream.h"
#include "SkString.h"

#include <cmath>
#include <vector>

namespace skjson {

//#define SK_JSON_REPORT_ERRORS

namespace {

/*
  Value's impl side:

    -- fixed 64-bit size

    -- 8-byte aligned

    -- union of:

         bool
         int32
         float
         char[6] (short string)
         external payload pointer

     -- lower 3 bits reserved for type storage

     -- ^ this works 'cause external payload ptrs are always 8-aligned

 */
static_assert( sizeof(Value) == 8, "");
static_assert(alignof(Value) == 8, "");

static constexpr size_t kRecAlign = alignof(Value);

class ValueRec : public Value {
public:
    enum RecType {
        // no payload
        kNull        = 0x00,

        // inline payload
        kBool        = 0x01,
        kInt         = 0x02,
        kFloat       = 0x03,
        kShortString = 0x04,

        // pointer to external storage
        kString      = 0x05,
        kArray       = 0x06,
        kObject      = 0x07,
    };
    static constexpr uint64_t kTypeMask = 0x07;

    RecType getRecType() const {
        return static_cast<RecType>(this->cast<uint64_t>() & kTypeMask);
    }

    // Access the record data as T.
    template <typename T>
    const T& cast() const {
        static_assert(sizeof (T) <=  sizeof(ValueRec), "");
        static_assert(alignof(T) <= alignof(ValueRec), "");
        return *reinterpret_cast<const T*>(this);
    }

    template <typename T>
    T& cast() { return const_cast<T&>(const_cast<const ValueRec*>(this)->cast<T>()); }

    // Access the inline record payload as T.
    template <typename T>
    const T& payload() const {
        // Max usable payload size is 7 bytes.
        // We further restrict sizeof(T) for T alignment on LE machines.
        static_assert(2 * sizeof(T) <= sizeof(Value), "");
        const T* base = &this->cast<T>();

#if defined(SK_CPU_LENDIAN)
        // The uint8_t holding recType is not usable for payload.  On LE, this is the first byte.
        return *(base + 1);
#else
        return *base;
#endif
    }

    template <typename T>
    T& payload() { return const_cast<T&>(const_cast<const ValueRec*>(this)->payload<T>()); }

    // Access the pointer payload.
    template <typename T>
    const T* ptr() const {
        // Lower kTypeMask bits are reserved for type storage.
        return reinterpret_cast<T*>(this->cast<uintptr_t>() & ~kTypeMask);
    }

    // Type-bound recs only store their type.
    static ValueRec MakeTypeBound(RecType t) {
        ValueRec v;
        v.cast<uint64_t>() = t;
        SkASSERT(v.getRecType() == t);
        return v;
    }

    // Primitive recs store a type and inline primitive payload.
    template <typename T>
    static ValueRec MakePrimitive(RecType t, T src) {
        ValueRec v = MakeTypeBound(t);
        v.payload<T>() = src;
        SkASSERT(v.getRecType() == t);
        return v;
    }

    // Pointer recs store a type (in the lower 3 bits), and an 8-byte aligned pointer.
    template <typename T>
    static ValueRec MakePtr(RecType t, const T* p) {
        SkASSERT((t & kTypeMask) == t);
        SkASSERT(!(reinterpret_cast<uintptr_t>(p) & kTypeMask));

        ValueRec v = MakeTypeBound(t);
        v.cast<uintptr_t>() |= reinterpret_cast<uintptr_t>(p);

        SkASSERT(v.getRecType() == t);
        SkASSERT(v.ptr<T>() == p);

        return v;
    }

    // Vector recs point to externally allocated slabs with the following layout:
    //
    //   [size_t n] [REC_0] ... [REC_n-1] [optional extra trailing storage]
    //
    // Long strings use extra_alloc_size == 1 to store the \0 terminator.
    template <typename T, size_t extra_alloc_size = 0>
    static ValueRec MakeVector(RecType t, const T* src, size_t size, SkArenaAlloc& alloc) {
        // For zero-size arrays, we just store a nullptr.
        size_t* size_ptr = nullptr;

        if (size) {
            // The Ts are already in memory, so their size should be safeish.
            const auto total_size = sizeof(size_t) + sizeof(T) * size + extra_alloc_size;
            size_ptr = reinterpret_cast<size_t*>(alloc.makeBytesAlignedTo(total_size, kRecAlign));
            auto* data_ptr = reinterpret_cast<T*>(size_ptr + 1);
            *size_ptr = size;
            memcpy(data_ptr, src, sizeof(T) * size);
        }

        return MakePtr(t, size_ptr);
    }

    size_t vectorSize(RecType t) const {
        if (this->is<NullValue>()) return 0;
        SkASSERT(this->getRecType() == t);

        const auto* size_ptr = this->ptr<const size_t>();
        return size_ptr ? *size_ptr : 0;
    }

    template <typename T>
    const T* vectorBegin(RecType t) const {
        if (this->is<NullValue>()) return nullptr;
        SkASSERT(this->getRecType() == t);

        const auto* size_ptr = this->ptr<const size_t>();
        return size_ptr ? reinterpret_cast<const T*>(size_ptr + 1) : nullptr;
    }

    template <typename T>
    const T* vectorEnd(RecType t) const {
        if (this->is<NullValue>()) return nullptr;
        SkASSERT(this->getRecType() == t);

        const auto* size_ptr = this->ptr<const size_t>();
        return size_ptr ? reinterpret_cast<const T*>(size_ptr + 1) + *size_ptr : nullptr;
    }

    // Strings have two flavors:
    //
    // -- short strings (len <= 6) -> these are stored inline, in the record
    //    (two bytes reserved for type & string len):
    //
    //        [str] [\0] [max_len - actual_len]
    //
    //    Storing [max_len - actual_len] allows the 'len' field to double-up as a
    //    null terminator when size == max_len.
    //
    // -- long strings (len > 6) -> these are externally allocated vectors (VectorRec<char>).
    //
    // The string data plus a null-char terminator are copied over.
    static constexpr size_t kMaxInlineStringSize = sizeof(Value) - 2;

    static ValueRec MakeString(const char* src, size_t size, SkArenaAlloc& alloc) {
        ValueRec v;

        if (size > kMaxInlineStringSize) {
            v = MakeVector<char, 1>(kString, src, size, alloc);
            const_cast<char *>(v.vectorBegin<char>(ValueRec::kString))[size] = '\0';
        } else {
            v = MakeTypeBound(kShortString);
            char* payload = &v.payload<char>();
            memcpy(payload, src, size);

            payload[size] = '\0';
            payload[kMaxInlineStringSize] = SkTo<char>(kMaxInlineStringSize - size);
        }
        return v;
    }

    size_t stringSize() const {
        if (this->getRecType() == ValueRec::kShortString) {
            const char* payload = &this->payload<char>();
            return kMaxInlineStringSize - SkToSizeT(payload[kMaxInlineStringSize]);
        }

        return this->vectorSize(ValueRec::kString);
    }

    const char* stringBegin() const {
        if (this->getRecType() == ValueRec::kShortString) {
            return &this->payload<char>();
        }

        return this->vectorBegin<char>(ValueRec::kString);
    }

    const char* stringEnd() const {
        if (this->getRecType() == ValueRec::kShortString) {
            const char* payload = &this->payload<char>();
            return payload + kMaxInlineStringSize - SkToSizeT(payload[kMaxInlineStringSize]);
        }

        return this->vectorEnd<char>(ValueRec::kString);
    }
};

} // namespace


// Boring public Value glue.

const Value& Value::Null() {
    static const Value g_null = ValueRec::MakeTypeBound(ValueRec::kNull);
    return g_null;
}

const Member& Member::Null() {
    static const Member g_null = { Value::Null(), Value::Null() };
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

    const auto& impl = *reinterpret_cast<const ValueRec*>(this);
    SkASSERT(static_cast<size_t>(impl.getRecType()) < SK_ARRAY_COUNT(kTypeMap));

    return kTypeMap[impl.getRecType()];
}

template <>
bool PrimitiveValue<bool, Value::Type::kBool>::operator*() const {
    const auto& impl = *reinterpret_cast<const ValueRec*>(this);

    if (impl.is<NullValue>()) return false;

    SkASSERT(impl.getRecType() == ValueRec::kBool);

    return impl.payload<bool>();
}

template <>
double PrimitiveValue<double, Value::Type::kNumber>::operator*() const {
    const auto& impl = *reinterpret_cast<const ValueRec*>(this);

    if (impl.is<NullValue>()) return 0;

    SkASSERT(impl.getRecType() == ValueRec::kInt ||
             impl.getRecType() == ValueRec::kFloat);

    return impl.getRecType() == ValueRec::kInt
        ? static_cast<double>(impl.payload<int32_t>())
        : static_cast<double>(impl.payload<float>());
}

template <>
size_t VectorValue<Value, Value::Type::kArray>::size() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorSize(ValueRec::kArray);
}

template <>
const Value* VectorValue<Value, Value::Type::kArray>::begin() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorBegin<Value>(ValueRec::kArray);
}

template <>
const Value* VectorValue<Value, Value::Type::kArray>::end() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorEnd<Value>(ValueRec::kArray);
}

template <>
size_t VectorValue<Member, Value::Type::kObject>::size() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorSize(ValueRec::kObject);
}

template <>
const Member* VectorValue<Member, Value::Type::kObject>::begin() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorBegin<Member>(ValueRec::kObject);
}

template <>
const Member* VectorValue<Member, Value::Type::kObject>::end() const {
    return reinterpret_cast<const ValueRec*>(this)->vectorEnd<Member>(ValueRec::kObject);
}

template <>
size_t VectorValue<char, Value::Type::kString>::size() const {
    return reinterpret_cast<const ValueRec*>(this)->stringSize();
}

template <>
const char* VectorValue<char, Value::Type::kString>::begin() const {
    return reinterpret_cast<const ValueRec*>(this)->stringBegin();
}

template <>
const char* VectorValue<char, Value::Type::kString>::end() const {
    return reinterpret_cast<const ValueRec*>(this)->stringEnd();
}

const Value& ObjectValue::operator[](const char* key) const {
    // Reverse search for duplicates resolution (policy: return last).
    const auto* begin  = this->begin();
    const auto* member = this->end();

    while (member > begin) {
        --member;
        SkASSERT(member->fKey.is<StringValue>());
        if (!strcmp(key, member->fKey.as<StringValue>().begin())) {
            return member->fValue;
        }
    }

    return Value::Null();
}

namespace {

// Lexer/parser inspired by rapidjson [1], sajson [2] and pjson [3].
//
// [1] https://github.com/Tencent/rapidjson/
// [2] https://github.com/chadaustin/sajson
// [3] https://pastebin.com/hnhSTL3h


// bit 0 (0x01) - plain ASCII string character
// bit 1 (0x02) - whitespace
// bit 2 (0x04) - string terminator (" \0 [control chars])
// bit 3 (0x08) - 0-9
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
static inline bool is_digit(char c)       { return g_token_flags[static_cast<uint8_t>(c)] & 0x08; }
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
    explicit DomParser(SkArenaAlloc& alloc)
        : fAlloc(alloc) {

        fValueStack.reserve(kValueStackReserve);
        fScopeStack.reserve(kScopeStackReserve);
    }

    const Value& parse(const char* p) {
        p = skip_ws(p);

        switch (*p) {
        case '{':
            goto match_object;
        case '[':
            goto match_array;
        default:
            return error(Value::Null(), p, "invalid top-level value");
        }

match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->pushObjectScope();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
match_object_key:
        p = skip_ws(p);
        if (*p != '"') return error(Value::Null(), p, "expected object key");

        p = match_string(p, [this](const char* key, size_t size) {
            this->pushObjectKey(key, size);
        });
        if (!p) return Value::Null();

        p = skip_ws(p);
        if (*p != ':') return error(Value::Null(), p, "expected ':' separator");

        ++p;

        // goto match_value;
match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return error(Value::Null(), p, "unexpected input end");
        case '"':
            p = match_string(p, [this](const char* str, size_t size) {
                this->pushString(str, size);
            });
            break;
        case '[':
            goto match_array;
        case 'f':
            p = match_false(p);
            break;
        case 'n':
            p = match_null(p);
            break;
        case 't':
            p = match_true(p);
            break;
        case '{':
            goto match_object;
        default:
            p = match_number(p);
            break;
        }

        if (!p) return Value::Null();

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
            return error(Value::Null(), p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

pop_object:
        SkASSERT(*p == '}');

        if (fScopeStack.back() < 0)
            return error(Value::Null(), p, "unexpected object terminator");

        this->popObjectScope();

        // goto pop_common
pop_common:
        SkASSERT(*p == '}' || *p == ']');

        ++p;

        if (fScopeStack.empty()) {
            SkASSERT(fValueStack.size() == 1);
            auto* root = fAlloc.make<Value>();
            *root = fValueStack.front();

            // Stop condition: parsed the top level element and there is no trailing garbage.
            return *skip_ws(p) == '\0'
                ? *root
                : error(Value::Null(), p, "trailing root garbage");
        }

        goto match_post_value;

match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        this->pushArrayScope();

        if (*p != ']') goto match_value;

        // goto pop_array;
pop_array:
        SkASSERT(*p == ']');

        if (fScopeStack.back() >= 0)
            return error(Value::Null(), p, "unexpected array terminator");

        this->popArrayScope();

        goto pop_common;

        SkASSERT(false);
        return Value::Null();
    }

    const SkString& getError() const {
        return fError;
    }

private:
    SkArenaAlloc&                 fAlloc;

    static constexpr size_t kValueStackReserve = 256;
    static constexpr size_t kScopeStackReserve = 128;
    std::vector<Value  > fValueStack;
    std::vector<ssize_t> fScopeStack;

    SkString             fError;

    template <typename T>
    void popScopeAsVec(ValueRec::RecType type, size_t scope_start) {
        SkASSERT(scope_start > 0);
        SkASSERT(scope_start <= fValueStack.size());

        static_assert( sizeof(T) >=  sizeof(Value), "");
        static_assert( sizeof(T)  %  sizeof(Value) == 0, "");
        static_assert(alignof(T) == alignof(Value), "");

        const auto scope_count = fValueStack.size() - scope_start,
                         count = scope_count / (sizeof(T) / sizeof(Value));
        SkASSERT(scope_count % (sizeof(T) / sizeof(Value)) == 0);

        const auto* begin = reinterpret_cast<const T*>(fValueStack.data() + scope_start);

        // Instantiate the placeholder value added in onPush{Object/Array}.
        fValueStack[scope_start - 1] = ValueRec::MakeVector<T>(type, begin, count, fAlloc);

        // Drop the current scope.
        fScopeStack.pop_back();
        fValueStack.resize(scope_start);
    }

    void pushObjectScope() {
        // Object placeholder.
        fValueStack.emplace_back();

        // Object scope marker (size).
        fScopeStack.push_back(SkTo<ssize_t>(fValueStack.size()));
    }

    void popObjectScope() {
        const auto scope_start = fScopeStack.back();
        SkASSERT(scope_start > 0);
        this->popScopeAsVec<Member>(ValueRec::kObject, SkTo<size_t>(scope_start));

        SkDEBUGCODE(
            const auto& obj = fValueStack.back().as<ObjectValue>();
            SkASSERT(obj.is<ObjectValue>());
            for (const auto& member : obj) {
                SkASSERT(member.fKey.is<StringValue>());
            }
        )
    }

    void pushArrayScope() {
        // Array placeholder.
        fValueStack.emplace_back();

        // Array scope marker (-size).
        fScopeStack.push_back(-SkTo<ssize_t>(fValueStack.size()));
    }

    void popArrayScope() {
        const auto scope_start = -fScopeStack.back();
        SkASSERT(scope_start > 0);
        this->popScopeAsVec<Value>(ValueRec::kArray, SkTo<size_t>(scope_start));

        SkDEBUGCODE(
            const auto& arr = fValueStack.back().as<ArrayValue>();
            SkASSERT(arr.is<ArrayValue>());
        )
    }

    void pushObjectKey(const char* key, size_t size) {
        SkASSERT(fScopeStack.back() >= 0);
        SkASSERT(fValueStack.size() >= SkTo<size_t>(fScopeStack.back()));
        SkASSERT(!((fValueStack.size() - SkTo<size_t>(fScopeStack.back())) & 1));
        this->pushString(key, size);
    }

    void pushTrue() {
        fValueStack.push_back(ValueRec::MakePrimitive<bool>(ValueRec::kBool, true));
    }

    void pushFalse() {
        fValueStack.push_back(ValueRec::MakePrimitive<bool>(ValueRec::kBool, false));
    }

    void pushNull() {
        fValueStack.push_back(ValueRec::MakeTypeBound(ValueRec::kNull));
    }

    void pushString(const char* s, size_t size) {
        fValueStack.push_back(ValueRec::MakeString(s, size, fAlloc));
    }

    void pushInt32(int32_t i) {
        fValueStack.push_back(ValueRec::MakePrimitive<int32_t>(ValueRec::kInt, i));
    }

    void pushFloat(float f) {
        fValueStack.push_back(ValueRec::MakePrimitive<float>(ValueRec::kFloat, f));
    }

    template <typename T>
    const T& error(const T& ret_val, const char* p, const char* msg) {
#if defined(SK_JSON_REPORT_ERRORS)
        static constexpr size_t kMaxContext = 128;
        fError = SkStringPrintf("%s: >", msg);
        fError.append(p, std::min(strlen(p), kMaxContext));
#endif
        return ret_val;
    }

    static inline const char* skip_ws(const char* p) {
        while (is_ws(*p)) ++p;
        return p;
    }

    const char* match_true(const char* p) {
        SkASSERT(p[0] == 't');

        if (p[1] == 'r' && p[2] == 'u' && p[3] == 'e') {
            this->pushTrue();
            return p + 4;
        }

        return error(nullptr, p, "invalid token");
    }

    const char* match_false(const char* p) {
        SkASSERT(p[0] == 'f');

        if (p[1] == 'a' && p[2] == 'l' && p[3] == 's' && p[4] == 'e') {
            this->pushFalse();
            return p + 5;
        }

        return error(nullptr, p, "invalid token");
    }

    const char* match_null(const char* p) {
        SkASSERT(p[0] == 'n');

        if (p[1] == 'u' && p[2] == 'l' && p[3] == 'l') {
            this->pushNull();
            return p + 4;
        }

        return error(nullptr, p, "invalid token");
    }

    template <typename MatchFunc>
    const char* match_string(const char* p, MatchFunc&& func) {
        SkASSERT(*p == '"');
        const auto* s_begin = p + 1;

        // TODO: unescape
        for (p = s_begin; !is_sterminator(*p); ++p) {}

        if (*p == '"') {
            func(s_begin, p - s_begin);
            return p + 1;
        }

        return error(nullptr, s_begin - 1, "invalid string");
    }

    const char* match_fast_float_decimal_part(const char* p, int sign, float f, int exp) {
        SkASSERT(exp <= 0);

        for (;;) {
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0'); --exp;
        }

        if (is_numeric(*p)) {
            SkASSERT(*p == '.' || *p == 'e' || *p == 'E');
            // We either have malformed input, or an (unsupported) exponent.
            return nullptr;
        }

        this->pushFloat(sign * f * pow10(exp));

        return p;
    }

    const char* match_fast_float_part(const char* p, int sign, float f) {
        for (;;) {
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0');
            if (!is_digit(*p)) break;
            f = f * 10.f + (*p++ - '0');
        }

        if (!is_numeric(*p)) {
            // Matched (integral) float.
            this->pushFloat(sign * f);
            return p;
        }

        return (*p == '.') ? match_fast_float_decimal_part(p + 1, sign, f, 0)
                           : nullptr;
    }

    const char* match_fast_32_or_float(const char* p) {
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

        if (is_digit(*p)) {
            n32 = (*p++ - '0');
            for (;;) {
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0');
            }
        }

        if (!is_numeric(*p)) {
            // Did we actually match any digits?
            if (p > digits_start) {
                this->pushInt32(sign * n32);
                return p;
            }
            return nullptr;
        }

        if (*p == '.') {
            const auto* decimals_start = ++p;

            int exp = 0;

            for (;;) {
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
                if (!is_digit(*p) || n32 > kMaxInt32) break;
                n32 = n32 * 10 + (*p++ - '0'); --exp;
            }

            if (!is_numeric(*p)) {
                // Did we actually match any digits?
                if (p > decimals_start) {
                    this->pushFloat(sign * n32 * pow10(exp));
                    return p;
                }
                return nullptr;
            }

            if (n32 > kMaxInt32) {
                // we ran out on n32 bits
                return match_fast_float_decimal_part(p, sign, n32, exp);
            }
        }

        return match_fast_float_part(p, sign, n32);
    }

    const char* match_number(const char* p) {
        if (const auto* fast = match_fast_32_or_float(p)) return fast;

        // slow fallback
        char* matched;
        float f = strtof(p, &matched);
        if (matched > p) {
            this->pushFloat(f);
            return matched;
        }
        return error(nullptr, p, "invalid numeric token");
    }
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
