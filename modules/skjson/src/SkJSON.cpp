/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkStream.h"
#include "SkString.h"
#include "SkTo.h"

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
         char[8] (short string storage)
         external payload pointer

     -- highest 3 bits reserved for type storage

 */
static_assert( sizeof(Value) == 8, "");
static_assert(alignof(Value) == 8, "");

static constexpr size_t kRecAlign = alignof(Value);

// The current record layout assumes LE and will take some tweaking for BE.
#if defined(SK_CPU_BENDIAN)
static_assert(false, "Big-endian builds are not supported.");
#endif

class ValueRec : public Value {
public:
    static constexpr uint64_t kTypeBits  = 3,
                              kTypeShift = 64 - kTypeBits,
                              kTypeMask  = ((1ULL << kTypeBits) - 1) << kTypeShift;

    enum RecType : uint64_t {
        // We picked kShortString == 0 so that tag 0b000 and stored max_size-size (7-7=0)
        // conveniently overlap the '\0' terminator, allowing us to store a 7 character
        // C string inline.
        kShortString = 0b000ULL << kTypeShift,  // inline payload
        kNull        = 0b001ULL << kTypeShift,  // no payload
        kBool        = 0b010ULL << kTypeShift,  // inline payload
        kInt         = 0b011ULL << kTypeShift,  // inline payload
        kFloat       = 0b100ULL << kTypeShift,  // inline payload
        kString      = 0b101ULL << kTypeShift,  // ptr to external storage
        kArray       = 0b110ULL << kTypeShift,  // ptr to external storage
        kObject      = 0b111ULL << kTypeShift,  // ptr to external storage
    };

    RecType getRecType() const {
        return static_cast<RecType>(*this->cast<uint64_t>() & kTypeMask);
    }

    // Access the record data as T.
    //
    // This is also used to access the payload for inline records.  Since the record type lives in
    // the high bits, sizeof(T) must be less than sizeof(Value) when accessing inline payloads.
    //
    // E.g.
    //
    //   uint8_t
    //    -----------------------------------------------------------------------
    //   |  val8  |  val8  |  val8  |  val8  |  val8  |  val8  |  val8  |    TYPE|
    //    -----------------------------------------------------------------------
    //
    //   uint32_t
    //    -----------------------------------------------------------------------
    //   |               val32               |          unused          |    TYPE|
    //    -----------------------------------------------------------------------
    //
    //   T* (64b)
    //    -----------------------------------------------------------------------
    //   |                        T* (kTypeShift bits)                      |TYPE|
    //    -----------------------------------------------------------------------
    //
    template <typename T>
    const T* cast() const {
        static_assert(sizeof (T) <=  sizeof(ValueRec), "");
        static_assert(alignof(T) <= alignof(ValueRec), "");
        return reinterpret_cast<const T*>(this);
    }

    template <typename T>
    T* cast() { return const_cast<T*>(const_cast<const ValueRec*>(this)->cast<T>()); }

    // Access the pointer payload.
    template <typename T>
    const T* ptr() const {
        static_assert(sizeof(uintptr_t)     == sizeof(Value) ||
                      sizeof(uintptr_t) * 2 == sizeof(Value), "");

        return (sizeof(uintptr_t) < sizeof(Value))
            // For 32-bit, pointers are stored unmodified.
            ? *this->cast<const T*>()
            // For 64-bit, we use the high bits of the pointer as type storage.
            : reinterpret_cast<T*>(*this->cast<uintptr_t>() & ~kTypeMask);
    }

    // Type-bound recs only store their type.
    static ValueRec MakeTypeBound(RecType t) {
        ValueRec v;
        *v.cast<uint64_t>() = t;
        SkASSERT(v.getRecType() == t);
        return v;
    }

    // Primitive recs store a type and inline primitive payload.
    template <typename T>
    static ValueRec MakePrimitive(RecType t, T src) {
        ValueRec v = MakeTypeBound(t);
        *v.cast<T>() = src;
        SkASSERT(v.getRecType() == t);
        return v;
    }

    // Pointer recs store a type (in the upper kTypeBits bits) and a pointer.
    template <typename T>
    static ValueRec MakePtr(RecType t, const T* p) {
        SkASSERT((t & kTypeMask) == t);
        if (sizeof(uintptr_t) == sizeof(Value)) {
            // For 64-bit, we rely on the pointer hi bits being unused.
            SkASSERT(!(reinterpret_cast<uintptr_t>(p) & kTypeMask));
        }

        ValueRec v = MakeTypeBound(t);
        *v.cast<uintptr_t>() |= reinterpret_cast<uintptr_t>(p);

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
    // -- short strings (len <= 7) -> these are stored inline, in the record
    //    (one byte reserved for null terminator/type):
    //
    //        [str] [\0]|[max_len - actual_len]
    //
    //    Storing [max_len - actual_len] allows the 'len' field to double-up as a
    //    null terminator when size == max_len (this works 'cause kShortString == 0).
    //
    // -- long strings (len > 7) -> these are externally allocated vectors (VectorRec<char>).
    //
    // The string data plus a null-char terminator are copied over.
    static constexpr size_t kMaxInlineStringSize = sizeof(Value) - 1;

    static ValueRec MakeString(const char* src, size_t size, SkArenaAlloc& alloc) {
        ValueRec v;

        if (size > kMaxInlineStringSize) {
            v = MakeVector<char, 1>(kString, src, size, alloc);
            const_cast<char *>(v.vectorBegin<char>(ValueRec::kString))[size] = '\0';
        } else {
            v = MakeTypeBound(kShortString);
            auto* payload = v.cast<char>();
            memcpy(payload, src, size);
            payload[size] = '\0';

            const auto len_tag = SkTo<char>(kMaxInlineStringSize - size);
            // This technically overwrites the type hi bits, but is safe because
            //   1) kShortString == 0
            //   2) 0 <= len_tag <= 7
            static_assert(kShortString == 0, "please don't break this");
            payload[kMaxInlineStringSize] = len_tag;
            SkASSERT(v.getRecType() == kShortString);
        }
        return v;
    }

    size_t stringSize() const {
        if (this->getRecType() == ValueRec::kShortString) {
            const auto* payload = this->cast<char>();
            return kMaxInlineStringSize - SkToSizeT(payload[kMaxInlineStringSize]);
        }

        return this->vectorSize(ValueRec::kString);
    }

    const char* stringBegin() const {
        if (this->getRecType() == ValueRec::kShortString) {
            return this->cast<char>();
        }

        return this->vectorBegin<char>(ValueRec::kString);
    }

    const char* stringEnd() const {
        if (this->getRecType() == ValueRec::kShortString) {
            const auto* payload = this->cast<char>();
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
    static const Member g_null = { Value::Null().as<StringValue>(), Value::Null() };
    return g_null;
}

Value::Type Value::getType() const {
    static constexpr Value::Type kTypeMap[] = {
        Value::Type::kString, // kShortString
        Value::Type::kNull,   // kNull
        Value::Type::kBool,   // kBool
        Value::Type::kNumber, // kInt
        Value::Type::kNumber, // kFloat
        Value::Type::kString, // kString
        Value::Type::kArray,  // kArray
        Value::Type::kObject, // kObject
    };

    const auto& rec = *reinterpret_cast<const ValueRec*>(this);
    const auto type_index = static_cast<size_t>(rec.getRecType() >> ValueRec::kTypeShift);
    SkASSERT(type_index < SK_ARRAY_COUNT(kTypeMap));

    return kTypeMap[type_index];
}

template <>
bool PrimitiveValue<bool, Value::Type::kBool>::operator*() const {
    const auto& rec = *reinterpret_cast<const ValueRec*>(this);

    if (rec.is<NullValue>()) return false;

    SkASSERT(rec.getRecType() == ValueRec::kBool);

    return *rec.cast<bool>();
}

template <>
double PrimitiveValue<double, Value::Type::kNumber>::operator*() const {
    const auto& rec = *reinterpret_cast<const ValueRec*>(this);

    if (rec.is<NullValue>()) return 0;

    SkASSERT(rec.getRecType() == ValueRec::kInt ||
             rec.getRecType() == ValueRec::kFloat);

    return rec.getRecType() == ValueRec::kInt
        ? static_cast<double>(*rec.cast<int32_t>())
        : static_cast<double>(*rec.cast<float>());
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
        if (0 == strcmp(key, member->fKey.as<StringValue>().begin())) {
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
static constexpr uint8_t g_token_flags[256] = {
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

static inline const char* skip_ws(const char* p) {
    while (is_ws(*p)) ++p;
    return p;
}

static inline float pow10(int32_t exp) {
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

    static constexpr int32_t k_exp_offset = SK_ARRAY_COUNT(g_pow10_table) / 2;

    // We only support negative exponents for now.
    SkASSERT(exp <= 0);

    return (exp >= -k_exp_offset) ? g_pow10_table[exp + k_exp_offset]
                                  : std::pow(10.0f, static_cast<float>(exp));
}

class DOMParser {
public:
    explicit DOMParser(SkArenaAlloc& alloc)
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
            return this->error(Value::Null(), p, "invalid top-level value");
        }

    match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->pushObjectScope();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
    match_object_key:
        p = skip_ws(p);
        if (*p != '"') return this->error(Value::Null(), p, "expected object key");

        p = this->matchString(p, [this](const char* key, size_t size) {
            this->pushObjectKey(key, size);
        });
        if (!p) return Value::Null();

        p = skip_ws(p);
        if (*p != ':') return this->error(Value::Null(), p, "expected ':' separator");

        ++p;

        // goto match_value;
    match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return this->error(Value::Null(), p, "unexpected input end");
        case '"':
            p = this->matchString(p, [this](const char* str, size_t size) {
                this->pushString(str, size);
            });
            break;
        case '[':
            goto match_array;
        case 'f':
            p = this->matchFalse(p);
            break;
        case 'n':
            p = this->matchNull(p);
            break;
        case 't':
            p = this->matchTrue(p);
            break;
        case '{':
            goto match_object;
        default:
            p = this->matchNumber(p);
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
            return this->error(Value::Null(), p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

    pop_object:
        SkASSERT(*p == '}');

        if (fScopeStack.back() < 0) {
            return this->error(Value::Null(), p, "unexpected object terminator");
        }

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
                : this->error(Value::Null(), p, "trailing root garbage");
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

        if (fScopeStack.back() >= 0) {
            return this->error(Value::Null(), p, "unexpected array terminator");
        }

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
    std::vector<Value   > fValueStack;
    std::vector<intptr_t> fScopeStack;

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
        fScopeStack.push_back(SkTo<intptr_t>(fValueStack.size()));
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
        fScopeStack.push_back(-SkTo<intptr_t>(fValueStack.size()));
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

    const char* matchTrue(const char* p) {
        SkASSERT(p[0] == 't');

        if (p[1] == 'r' && p[2] == 'u' && p[3] == 'e') {
            this->pushTrue();
            return p + 4;
        }

        return this->error(nullptr, p, "invalid token");
    }

    const char* matchFalse(const char* p) {
        SkASSERT(p[0] == 'f');

        if (p[1] == 'a' && p[2] == 'l' && p[3] == 's' && p[4] == 'e') {
            this->pushFalse();
            return p + 5;
        }

        return this->error(nullptr, p, "invalid token");
    }

    const char* matchNull(const char* p) {
        SkASSERT(p[0] == 'n');

        if (p[1] == 'u' && p[2] == 'l' && p[3] == 'l') {
            this->pushNull();
            return p + 4;
        }

        return this->error(nullptr, p, "invalid token");
    }

    template <typename MatchFunc>
    const char* matchString(const char* p, MatchFunc&& func) {
        SkASSERT(*p == '"');
        const auto* s_begin = p + 1;

        // TODO: unescape
        for (p = s_begin; !is_sterminator(*p); ++p) {}

        if (*p == '"') {
            func(s_begin, p - s_begin);
            return p + 1;
        }

        return this->error(nullptr, s_begin - 1, "invalid string");
    }

    const char* matchFastFloatDecimalPart(const char* p, int sign, float f, int exp) {
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

    const char* matchFastFloatPart(const char* p, int sign, float f) {
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

        return (*p == '.') ? this->matchFastFloatDecimalPart(p + 1, sign, f, 0)
                           : nullptr;
    }

    const char* matchFast32OrFloat(const char* p) {
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
                return this->matchFastFloatDecimalPart(p, sign, n32, exp);
            }
        }

        return this->matchFastFloatPart(p, sign, n32);
    }

    const char* matchNumber(const char* p) {
        if (const auto* fast = this->matchFast32OrFloat(p)) return fast;

        // slow fallback
        char* matched;
        float f = strtof(p, &matched);
        if (matched > p) {
            this->pushFloat(f);
            return matched;
        }
        return this->error(nullptr, p, "invalid numeric token");
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
        bool first_value = true;
        for (const auto& v : array) {
            if (!first_value) stream->writeText(",");
            Write(v, stream);
            first_value = false;
        }
        stream->writeText("]");
        break;
    }
    case Value::Type::kObject:
        const auto& object = v.as<ObjectValue>();
        stream->writeText("{");
        bool first_member = true;
        for (const auto& member : object) {
            SkASSERT(member.fKey.getType() == Value::Type::kString);
            if (!first_member) stream->writeText(",");
            Write(member.fKey, stream);
            stream->writeText(":");
            Write(member.fValue, stream);
            first_member = false;
        }
        stream->writeText("}");
        break;
    }
}

} // namespace

static constexpr size_t kMinChunkSize = 4096;

DOM::DOM(const char* cstr)
    : fAlloc(kMinChunkSize) {
    DOMParser parser(fAlloc);

    fRoot = &parser.parse(cstr);
}

void DOM::write(SkWStream* stream) const {
    Write(*fRoot, stream);
}

} // namespace skjson
