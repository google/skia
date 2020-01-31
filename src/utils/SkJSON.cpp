/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkJSON.h"

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkMalloc.h"
#include "include/utils/SkParse.h"
#include "src/utils/SkUTF.h"

#include <cmath>
#include <tuple>
#include <vector>

namespace skjson {

// #define SK_JSON_REPORT_ERRORS

static_assert( sizeof(Value) == 8, "");
static_assert(alignof(Value) == 8, "");

static constexpr size_t kRecAlign = alignof(Value);

void Value::init_tagged(Tag t) {
    memset(fData8, 0, sizeof(fData8));
    fData8[Value::kTagOffset] = SkTo<uint8_t>(t);
    SkASSERT(this->getTag() == t);
}

// Pointer values store a type (in the upper kTagBits bits) and a pointer.
void Value::init_tagged_pointer(Tag t, void* p) {
    *this->cast<uintptr_t>() = reinterpret_cast<uintptr_t>(p);

    if (sizeof(Value) == sizeof(uintptr_t)) {
        // For 64-bit, we rely on the pointer upper bits being unused/zero.
        SkASSERT(!(fData8[kTagOffset] & kTagMask));
        fData8[kTagOffset] |= SkTo<uint8_t>(t);
    } else {
        // For 32-bit, we need to zero-initialize the upper 32 bits
        SkASSERT(sizeof(Value) == sizeof(uintptr_t) * 2);
        this->cast<uintptr_t>()[kTagOffset >> 2] = 0;
        fData8[kTagOffset] = SkTo<uint8_t>(t);
    }

    SkASSERT(this->getTag()    == t);
    SkASSERT(this->ptr<void>() == p);
}

NullValue::NullValue() {
    this->init_tagged(Tag::kNull);
    SkASSERT(this->getTag() == Tag::kNull);
}

BoolValue::BoolValue(bool b) {
    this->init_tagged(Tag::kBool);
    *this->cast<bool>() = b;
    SkASSERT(this->getTag() == Tag::kBool);
}

NumberValue::NumberValue(int32_t i) {
    this->init_tagged(Tag::kInt);
    *this->cast<int32_t>() = i;
    SkASSERT(this->getTag() == Tag::kInt);
}

NumberValue::NumberValue(float f) {
    this->init_tagged(Tag::kFloat);
    *this->cast<float>() = f;
    SkASSERT(this->getTag() == Tag::kFloat);
}

// Vector recs point to externally allocated slabs with the following layout:
//
//   [size_t n] [REC_0] ... [REC_n-1] [optional extra trailing storage]
//
// Long strings use extra_alloc_size == 1 to store the \0 terminator.
//
template <typename T, size_t extra_alloc_size = 0>
static void* MakeVector(const void* src, size_t size, SkArenaAlloc& alloc) {
    // The Ts are already in memory, so their size should be safe.
    const auto total_size = sizeof(size_t) + size * sizeof(T) + extra_alloc_size;
    auto* size_ptr = reinterpret_cast<size_t*>(alloc.makeBytesAlignedTo(total_size, kRecAlign));

    *size_ptr = size;
    sk_careful_memcpy(size_ptr + 1, src, size * sizeof(T));

    return size_ptr;
}

ArrayValue::ArrayValue(const Value* src, size_t size, SkArenaAlloc& alloc) {
    this->init_tagged_pointer(Tag::kArray, MakeVector<Value>(src, size, alloc));
    SkASSERT(this->getTag() == Tag::kArray);
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
//
namespace {

// An internal string builder with a fast 8 byte short string load path
// (for the common case where the string is not at the end of the stream).
class FastString final : public Value {
public:
    FastString(const char* src, size_t size, const char* eos, SkArenaAlloc& alloc) {
        SkASSERT(src <= eos);

        if (size > kMaxInlineStringSize) {
            this->initLongString(src, size, alloc);
            SkASSERT(this->getTag() == Tag::kString);
            return;
        }

        static_assert(static_cast<uint8_t>(Tag::kShortString) == 0, "please don't break this");
        static_assert(sizeof(Value) == 8, "");

        // TODO: LIKELY
        if (src && src + 7 <= eos) {
            this->initFastShortString(src, size);
        } else {
            this->initShortString(src, size);
        }

        SkASSERT(this->getTag() == Tag::kShortString);
    }

private:
    static constexpr size_t kMaxInlineStringSize = sizeof(Value) - 1;

    void initLongString(const char* src, size_t size, SkArenaAlloc& alloc) {
        SkASSERT(size > kMaxInlineStringSize);

        this->init_tagged_pointer(Tag::kString, MakeVector<char, 1>(src, size, alloc));

        auto* data = this->cast<VectorValue<char, Value::Type::kString>>()->begin();
        const_cast<char*>(data)[size] = '\0';
    }

    void initShortString(const char* src, size_t size) {
        SkASSERT(size <= kMaxInlineStringSize);

        this->init_tagged(Tag::kShortString);
        sk_careful_memcpy(this->cast<char>(), src, size);
        // Null terminator provided by init_tagged() above (fData8 is zero-initialized).
    }

    void initFastShortString(const char* src, size_t size) {
        SkASSERT(size <= kMaxInlineStringSize);

        // Load 8 chars and mask out the tag and \0 terminator.
        uint64_t* s64 = this->cast<uint64_t>();
        memcpy(s64, src, 8);

#if defined(SK_CPU_LENDIAN)
        *s64 &= 0x00ffffffffffffffULL >> ((kMaxInlineStringSize - size) * 8);
#else
        static_assert(false, "Big-endian builds are not supported at this time.");
#endif
    }
};

} // namespace

StringValue::StringValue(const char* src, size_t size, SkArenaAlloc& alloc) {
    new (this) FastString(src, size, src, alloc);
}

ObjectValue::ObjectValue(const Member* src, size_t size, SkArenaAlloc& alloc) {
    this->init_tagged_pointer(Tag::kObject, MakeVector<Member>(src, size, alloc));
    SkASSERT(this->getTag() == Tag::kObject);
}


// Boring public Value glue.

static int inline_strcmp(const char a[], const char b[]) {
    for (;;) {
        char c = *a++;
        if (c == 0) {
            break;
        }
        if (c != *b++) {
            return 1;
        }
    }
    return *b != 0;
}

const Value& ObjectValue::operator[](const char* key) const {
    // Reverse search for duplicates resolution (policy: return last).
    const auto* begin  = this->begin();
    const auto* member = this->end();

    while (member > begin) {
        --member;
        if (0 == inline_strcmp(key, member->fKey.as<StringValue>().begin())) {
            return member->fValue;
        }
    }

    static const Value g_null = NullValue();
    return g_null;
}

namespace {

// Lexer/parser inspired by rapidjson [1], sajson [2] and pjson [3].
//
// [1] https://github.com/Tencent/rapidjson/
// [2] https://github.com/chadaustin/sajson
// [3] https://pastebin.com/hnhSTL3h


// bit 0 (0x01) - plain ASCII string character
// bit 1 (0x02) - whitespace
// bit 2 (0x04) - string terminator (" \\ \0 [control chars] **AND } ]** <- see matchString notes)
// bit 3 (0x08) - 0-9
// bit 4 (0x10) - 0-9 e E .
// bit 5 (0x20) - scope terminator (} ])
static constexpr uint8_t g_token_flags[256] = {
 // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
    4,   4,   4,   4,   4,   4,   4,   4,     4,   6,   6,   4,   4,   6,   4,   4, // 0
    4,   4,   4,   4,   4,   4,   4,   4,     4,   4,   4,   4,   4,   4,   4,   4, // 1
    3,   1,   4,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
 0x19,0x19,0x19,0x19,0x19,0x19,0x19,0x19,  0x19,0x19,   1,   1,   1,   1,   1,   1, // 3
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 4
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   4,0x25,   1,   1, // 5
    1,   1,   1,   1,   1,   0x11,1,   1,     1,   1,   1,   1,   1,   1,   1,   1, // 6
    1,   1,   1,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,0x25,   1,   1, // 7

 // 128-255
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,  0,0,0,0,0,0,0,0
};

static inline bool is_ws(char c)       { return g_token_flags[static_cast<uint8_t>(c)] & 0x02; }
static inline bool is_eostring(char c) { return g_token_flags[static_cast<uint8_t>(c)] & 0x04; }
static inline bool is_digit(char c)    { return g_token_flags[static_cast<uint8_t>(c)] & 0x08; }
static inline bool is_numeric(char c)  { return g_token_flags[static_cast<uint8_t>(c)] & 0x10; }
static inline bool is_eoscope(char c)  { return g_token_flags[static_cast<uint8_t>(c)] & 0x20; }

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
        fUnescapeBuffer.reserve(kUnescapeBufferReserve);
    }

    const Value parse(const char* p, size_t size) {
        if (!size) {
            return this->error(NullValue(), p, "invalid empty input");
        }

        const char* p_stop = p + size - 1;

        // We're only checking for end-of-stream on object/array close('}',']'),
        // so we must trim any whitespace from the buffer tail.
        while (p_stop > p && is_ws(*p_stop)) --p_stop;

        SkASSERT(p_stop >= p && p_stop < p + size);
        if (!is_eoscope(*p_stop)) {
            return this->error(NullValue(), p_stop, "invalid top-level value");
        }

        p = skip_ws(p);

        switch (*p) {
        case '{':
            goto match_object;
        case '[':
            goto match_array;
        default:
            return this->error(NullValue(), p, "invalid top-level value");
        }

    match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        this->pushObjectScope();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
    match_object_key:
        p = skip_ws(p);
        if (*p != '"') return this->error(NullValue(), p, "expected object key");

        p = this->matchString(p, p_stop, [this](const char* key, size_t size, const char* eos) {
            this->pushObjectKey(key, size, eos);
        });
        if (!p) return NullValue();

        p = skip_ws(p);
        if (*p != ':') return this->error(NullValue(), p, "expected ':' separator");

        ++p;

        // goto match_value;
    match_value:
        p = skip_ws(p);

        switch (*p) {
        case '\0':
            return this->error(NullValue(), p, "unexpected input end");
        case '"':
            p = this->matchString(p, p_stop, [this](const char* str, size_t size, const char* eos) {
                this->pushString(str, size, eos);
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

        if (!p) return NullValue();

        // goto match_post_value;
    match_post_value:
        SkASSERT(!this->inTopLevelScope());

        p = skip_ws(p);
        switch (*p) {
        case ',':
            ++p;
            if (this->inObjectScope()) {
                goto match_object_key;
            } else {
                SkASSERT(this->inArrayScope());
                goto match_value;
            }
        case ']':
            goto pop_array;
        case '}':
            goto pop_object;
        default:
            return this->error(NullValue(), p - 1, "unexpected value-trailing token");
        }

        // unreachable
        SkASSERT(false);

    pop_object:
        SkASSERT(*p == '}');

        if (this->inArrayScope()) {
            return this->error(NullValue(), p, "unexpected object terminator");
        }

        this->popObjectScope();

        // goto pop_common
    pop_common:
        SkASSERT(is_eoscope(*p));

        if (this->inTopLevelScope()) {
            SkASSERT(fValueStack.size() == 1);

            // Success condition: parsed the top level element and reached the stop token.
            return p == p_stop
                ? fValueStack.front()
                : this->error(NullValue(), p + 1, "trailing root garbage");
        }

        if (p == p_stop) {
            return this->error(NullValue(), p, "unexpected end-of-input");
        }

        ++p;

        goto match_post_value;

    match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        this->pushArrayScope();

        if (*p != ']') goto match_value;

        // goto pop_array;
    pop_array:
        SkASSERT(*p == ']');

        if (this->inObjectScope()) {
            return this->error(NullValue(), p, "unexpected array terminator");
        }

        this->popArrayScope();

        goto pop_common;

        SkASSERT(false);
        return NullValue();
    }

    std::tuple<const char*, const SkString> getError() const {
        return std::make_tuple(fErrorToken, fErrorMessage);
    }

private:
    SkArenaAlloc&         fAlloc;

    // Pending values stack.
    static constexpr size_t kValueStackReserve = 256;
    std::vector<Value>    fValueStack;

    // String unescape buffer.
    static constexpr size_t kUnescapeBufferReserve = 512;
    std::vector<char>     fUnescapeBuffer;

    // Tracks the current object/array scope, as an index into fStack:
    //
    //   - for objects: fScopeIndex =  (index of first value in scope)
    //   - for arrays : fScopeIndex = -(index of first value in scope)
    //
    // fScopeIndex == 0 IFF we are at the top level (no current/active scope).
    intptr_t              fScopeIndex = 0;

    // Error reporting.
    const char*           fErrorToken = nullptr;
    SkString              fErrorMessage;

    bool inTopLevelScope() const { return fScopeIndex == 0; }
    bool inObjectScope()   const { return fScopeIndex >  0; }
    bool inArrayScope()    const { return fScopeIndex <  0; }

    // Helper for masquerading raw primitive types as Values (bypassing tagging, etc).
    template <typename T>
    class RawValue final : public Value {
    public:
        explicit RawValue(T v) {
            static_assert(sizeof(T) <= sizeof(Value), "");
            *this->cast<T>() = v;
        }

        T operator *() const { return *this->cast<T>(); }
    };

    template <typename VectorT>
    void popScopeAsVec(size_t scope_start) {
        SkASSERT(scope_start > 0);
        SkASSERT(scope_start <= fValueStack.size());

        using T = typename VectorT::ValueT;
        static_assert( sizeof(T) >=  sizeof(Value), "");
        static_assert( sizeof(T)  %  sizeof(Value) == 0, "");
        static_assert(alignof(T) == alignof(Value), "");

        const auto scope_count = fValueStack.size() - scope_start,
                         count = scope_count / (sizeof(T) / sizeof(Value));
        SkASSERT(scope_count % (sizeof(T) / sizeof(Value)) == 0);

        const auto* begin = reinterpret_cast<const T*>(fValueStack.data() + scope_start);

        // Restore the previous scope index from saved placeholder value,
        // and instantiate as a vector of values in scope.
        auto& placeholder = fValueStack[scope_start - 1];
        fScopeIndex = *static_cast<RawValue<intptr_t>&>(placeholder);
        placeholder = VectorT(begin, count, fAlloc);

        // Drop the (consumed) values in scope.
        fValueStack.resize(scope_start);
    }

    void pushObjectScope() {
        // Save a scope index now, and then later we'll overwrite this value as the Object itself.
        fValueStack.push_back(RawValue<intptr_t>(fScopeIndex));

        // New object scope.
        fScopeIndex = SkTo<intptr_t>(fValueStack.size());
    }

    void popObjectScope() {
        SkASSERT(this->inObjectScope());
        this->popScopeAsVec<ObjectValue>(SkTo<size_t>(fScopeIndex));

        SkDEBUGCODE(
            const auto& obj = fValueStack.back().as<ObjectValue>();
            SkASSERT(obj.is<ObjectValue>());
            for (const auto& member : obj) {
                SkASSERT(member.fKey.is<StringValue>());
            }
        )
    }

    void pushArrayScope() {
        // Save a scope index now, and then later we'll overwrite this value as the Array itself.
        fValueStack.push_back(RawValue<intptr_t>(fScopeIndex));

        // New array scope.
        fScopeIndex = -SkTo<intptr_t>(fValueStack.size());
    }

    void popArrayScope() {
        SkASSERT(this->inArrayScope());
        this->popScopeAsVec<ArrayValue>(SkTo<size_t>(-fScopeIndex));

        SkDEBUGCODE(
            const auto& arr = fValueStack.back().as<ArrayValue>();
            SkASSERT(arr.is<ArrayValue>());
        )
    }

    void pushObjectKey(const char* key, size_t size, const char* eos) {
        SkASSERT(this->inObjectScope());
        SkASSERT(fValueStack.size() >= SkTo<size_t>(fScopeIndex));
        SkASSERT(!((fValueStack.size() - SkTo<size_t>(fScopeIndex)) & 1));
        this->pushString(key, size, eos);
    }

    void pushTrue() {
        fValueStack.push_back(BoolValue(true));
    }

    void pushFalse() {
        fValueStack.push_back(BoolValue(false));
    }

    void pushNull() {
        fValueStack.push_back(NullValue());
    }

    void pushString(const char* s, size_t size, const char* eos) {
        fValueStack.push_back(FastString(s, size, eos, fAlloc));
    }

    void pushInt32(int32_t i) {
        fValueStack.push_back(NumberValue(i));
    }

    void pushFloat(float f) {
        fValueStack.push_back(NumberValue(f));
    }

    template <typename T>
    T error(T&& ret_val, const char* p, const char* msg) {
#if defined(SK_JSON_REPORT_ERRORS)
        fErrorToken = p;
        fErrorMessage.set(msg);
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

    const std::vector<char>* unescapeString(const char* begin, const char* end) {
        fUnescapeBuffer.clear();

        for (const auto* p = begin; p != end; ++p) {
            if (*p != '\\') {
                fUnescapeBuffer.push_back(*p);
                continue;
            }

            if (++p == end) {
                return nullptr;
            }

            switch (*p) {
            case  '"': fUnescapeBuffer.push_back( '"'); break;
            case '\\': fUnescapeBuffer.push_back('\\'); break;
            case  '/': fUnescapeBuffer.push_back( '/'); break;
            case  'b': fUnescapeBuffer.push_back('\b'); break;
            case  'f': fUnescapeBuffer.push_back('\f'); break;
            case  'n': fUnescapeBuffer.push_back('\n'); break;
            case  'r': fUnescapeBuffer.push_back('\r'); break;
            case  't': fUnescapeBuffer.push_back('\t'); break;
            case  'u': {
                if (p + 4 >= end) {
                    return nullptr;
                }

                uint32_t hexed;
                const char hex_str[] = {p[1], p[2], p[3], p[4], '\0'};
                const auto* eos = SkParse::FindHex(hex_str, &hexed);
                if (!eos || *eos) {
                    return nullptr;
                }

                char utf8[SkUTF::kMaxBytesInUTF8Sequence];
                const auto utf8_len = SkUTF::ToUTF8(SkTo<SkUnichar>(hexed), utf8);
                fUnescapeBuffer.insert(fUnescapeBuffer.end(), utf8, utf8 + utf8_len);
                p += 4;
            } break;
            default: return nullptr;
            }
        }

        return &fUnescapeBuffer;
    }

    template <typename MatchFunc>
    const char* matchString(const char* p, const char* p_stop, MatchFunc&& func) {
        SkASSERT(*p == '"');
        const auto* s_begin = p + 1;
        bool requires_unescape = false;

        do {
            // Consume string chars.
            // This is the fast path, and hopefully we only hit it once then quick-exit below.
            for (p = p + 1; !is_eostring(*p); ++p);

            if (*p == '"') {
                // Valid string found.
                if (!requires_unescape) {
                    func(s_begin, p - s_begin, p_stop);
                } else {
                    // Slow unescape.  We could avoid this extra copy with some effort,
                    // but in practice escaped strings should be rare.
                    const auto* buf = this->unescapeString(s_begin, p);
                    if (!buf) {
                        break;
                    }

                    SkASSERT(!buf->empty());
                    func(buf->data(), buf->size(), buf->data() + buf->size() - 1);
                }
                return p + 1;
            }

            if (*p == '\\') {
                requires_unescape = true;
                ++p;
                continue;
            }

            // End-of-scope chars are special: we use them to tag the end of the input.
            // Thus they cannot be consumed indiscriminately -- we need to check if we hit the
            // end of the input.  To that effect, we treat them as string terminators above,
            // then we catch them here.
            if (is_eoscope(*p)) {
                continue;
            }

            // Invalid/unexpected char.
            break;
        } while (p != p_stop);

        // Premature end-of-input, or illegal string char.
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

        const auto decimal_scale = pow10(exp);
        if (is_numeric(*p) || !decimal_scale) {
            SkASSERT((*p == '.' || *p == 'e' || *p == 'E') || !decimal_scale);
            // Malformed input, or an (unsupported) exponent, or a collapsed decimal factor.
            return nullptr;
        }

        this->pushFloat(sign * f * decimal_scale);

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

SkString Value::toString() const {
    SkDynamicMemoryWStream wstream;
    Write(*this, &wstream);
    const auto data = wstream.detachAsData();
    // TODO: is there a better way to pass data around without copying?
    return SkString(static_cast<const char*>(data->data()), data->size());
}

static constexpr size_t kMinChunkSize = 4096;

DOM::DOM(const char* data, size_t size)
    : fAlloc(kMinChunkSize) {
    DOMParser parser(fAlloc);

    fRoot = parser.parse(data, size);
}

void DOM::write(SkWStream* stream) const {
    Write(fRoot, stream);
}

} // namespace skjson
