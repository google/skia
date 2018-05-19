/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkData.h"
#include "SkString.h"
#include "SkTArray.h"
#include "SkTime.h"

#include "pjson.h"

namespace skjson {

using NumberType = double;

enum class Type : uint8_t {
    kNull,
    kBool,
    kNumber,
    kInlineString,
    kString,
    kArray,
    kObject,
};

struct alignas(void*) alignas(NumberType) Value::Rec {
    char fPayload[15];
    Type fType;

    Rec() {} // uninitialized
    explicit Rec(Type t) : fType(t) {}

    template <typename T>
    T& as() {
        static_assert(sizeof(T) <= sizeof(Rec), "");
        static_assert(alignof(T) <= alignof(Rec), "");

        SkASSERT(fType == T::type);
        return *reinterpret_cast<T*>(this);
    }
};

static_assert(sizeof(Value::Rec) == 16, "");
static_assert(alignof(Value::Rec) == 8, "");

struct NullRec {
    static constexpr Type type = Type::kNull;

    static Value::Rec Make() {
        return Value::Rec(Type::kNull);
    }
};

struct BoolRec {
    static constexpr Type type = Type::kBool;
    bool fValue;

    static Value::Rec Make(bool v) {
        Value::Rec rec(Type::kBool);
        rec.as<BoolRec>().fValue = v;

        return rec;
    }
};

struct NumberRec {
    static constexpr Type type = Type::kNumber;
    NumberType fValue;

    static Value::Rec Make(NumberType v) {
        Value::Rec rec(Type::kNumber);
        rec.as<NumberRec>().fValue = v;

        return rec;
    }
};

struct InlineStringRec {
    static constexpr Type       type = Type::kInlineString;
    static constexpr size_t max_size = 14;

    char    fData[max_size];
    uint8_t fSizeInv;

    static Value::Rec Make(const char* s, size_t size) {
        SkASSERT(size <= max_size);

        Value::Rec rec(type);
        memcpy(rec.as<InlineStringRec>().fData, s, size);
        rec.as<InlineStringRec>().fSizeInv = SkToU8(max_size - size);
        if (size < max_size) {
            rec.as<InlineStringRec>().fData[size] = '\0';
        }

        return rec;
    }
};

struct StringRec {
    static constexpr Type type = Type::kString;
    const char* fValue;
    uint32_t    fSize;

    static Value::Rec Make(const char* s, size_t size, SkArenaAlloc& alloc) {
        SkASSERT(SkTFitsIn<uint32_t>(size));
        SkASSERT(size < std::numeric_limits<size_t>::max());

        auto* data = alloc.makeArrayDefault<char>(size + 1);
        memcpy(data, s, size);
        data[size] = '\0';

        Value::Rec rec(Type::kString);
        rec.as<StringRec>().fValue = data;
        rec.as<StringRec>().fSize  = SkToU32(size);

        return rec;
    }
};

struct ArrayRec {
    static constexpr Type type = Type::kArray;
    const Value::Rec* fValues;
    uint32_t          fSize;

    static Value::Rec Make(const Value::Rec* recs, size_t size, SkArenaAlloc& alloc) {
        auto* storage = alloc.makeArrayDefault<Value::Rec>(size);
        memcpy(storage, recs, size * sizeof(Value::Rec));

        Value::Rec rec(type);
        rec.as<ArrayRec>().fValues = storage;
        rec.as<ArrayRec>().fSize   = SkToU32(size);

        return rec;
    }
};

struct ObjectRec {
    static constexpr Type type = Type::kObject;
    const Value::Rec* fValues;
    uint32_t          fSize;

    static Value::Rec Make(const Value::Rec* recs, size_t size, SkArenaAlloc& alloc) {
        SkASSERT(!(size % 1));

        auto* storage = alloc.makeArrayDefault<Value::Rec>(size);
        memcpy(storage, recs, size * sizeof(Value::Rec));

        Value::Rec rec(type);
        rec.as<ObjectRec>().fValues = storage;
        rec.as<ObjectRec>().fSize   = SkToU32(size);

        return rec;
    }
};

namespace {

// bit 0 (1)    - set if: plain ASCII string character
// bit 1 (2)    - set if: whitespace
// bit 2 (4)    - set if: 0-9
// bit 4 (0x10) - set if: 0-9 e E .
static constexpr uint8_t g_token_flags[256] = {
 // 0    1    2    3    4    5    6    7      8    9    A    B    C    D    E    F
    0,   0,   0,   0,   0,   0,   0,   0,     0,   2,   2,   0,   0,   2,   0,   0, // 0
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, // 1
    3,   1,   0,   1,   1,   1,   1,   1,     1,   1,   1,   1,   1,   1,   0x11,1, // 2
 0x15,0x15,0x15,0x15,0x15,0x15,0x15,0x15,  0x15,0x15,   1,   1,   1,   1,   1,   1, // 3
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

static inline bool is_ws(char c)      { return g_token_flags[SkToU8(c)] & 0x02; }
static inline bool is_decimal(char c) { return g_token_flags[SkToU8(c)] & 0x04; }
static inline bool is_numeric(char c) { return g_token_flags[SkToU8(c)] & 0x10; }

static constexpr double g_pow10_table[63] =
{
   1.e-031,1.e-030,1.e-029,1.e-028,1.e-027,1.e-026,1.e-025,1.e-024,
   1.e-023,1.e-022,1.e-021,1.e-020,1.e-019,1.e-018,1.e-017,1.e-016,
   1.e-015,1.e-014,1.e-013,1.e-012,1.e-011,1.e-010,1.e-009,1.e-008,
   1.e-007,1.e-006,1.e-005,1.e-004,1.e-003,1.e-002,1.e-001,1.e+000,
   1.e+001,1.e+002,1.e+003,1.e+004,1.e+005,1.e+006,1.e+007,1.e+008,
   1.e+009,1.e+010,1.e+011,1.e+012,1.e+013,1.e+014,1.e+015,1.e+016,
   1.e+017,1.e+018,1.e+019,1.e+020,1.e+021,1.e+022,1.e+023,1.e+024,
   1.e+025,1.e+026,1.e+027,1.e+028,1.e+029,1.e+030,1.e+031
};

static inline double pow10(int32_t exp) {
    static constexpr int32_t k_exp_offset = sizeof(g_pow10_table) / (2 * sizeof(double));

    // We only support negative exponents for now.
    SkASSERT(exp <= 0);

    return (exp >= -k_exp_offset) ? g_pow10_table[exp + k_exp_offset]
                                  : std::pow(10, exp);
}

template <typename Parser>
class Lexer {
public:
    bool consume(const char* p, Parser& parser) {
        p = skip_ws(p);

        switch (*p) {
        case '{':
            fNestingStack.push_back('{');
            goto match_object;
        case '[':
            fNestingStack.push_back('[');
            goto match_array;
        default:
            return error(p, "invalid top-level value");
        }

match_object:
        SkASSERT(*p == '{');
        p = skip_ws(p + 1);

        parser.onPushObject();

        if (*p == '}') goto pop_object;

        // goto match_object_key;
match_object_key:
        p = skip_ws(p);
        if (*p != '"') return error(p, "invalid object key");

        {
            const auto* string_start = p + 1;
            if (!(p = match_string(p))) return error(p, "invalid string");

            SkASSERT(p >= string_start);
            parser.onObjectKey(string_start, p - string_start - 1);
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
            parser.onString(string_start, p - string_start - 1);
            break;
        }
        case '[':
            fNestingStack.push_back('[');
            goto match_array;
        case 'f':
            if (!(p = match_false(p))) return error(p, "unexpected token");
            parser.onFalse();
            break;
        case 'n':
            if (!(p = match_null(p))) return error(p, "unexpected token");
            parser.onNull();
            break;
        case 't':
            if (!(p = match_true(p))) return error(p, "unexpected token");
            parser.onTrue();
            break;
        case '{':
            fNestingStack.push_back('{');
            goto match_object;
        default: {
            NumberType n;
            if (!(p = match_number(p, &n))) return error(p, "unexpected token");
            parser.onNumber(n);
            break;
        }
        }

        // goto match_post_value;

match_post_value:
        SkASSERT(!fNestingStack.empty());

        p = skip_ws(p);
        switch (*p) {
        case ',':
            ++p;
            if (fNestingStack.back() == '{') {
                goto match_object_key;
            } else {
                SkASSERT(fNestingStack.back() == '[');
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
        SkASSERT(!fNestingStack.empty());

        if (fNestingStack.back() != '{')
            return error(p, "unexpected object terminator");

        parser.onPopObject();

        // goto pop_common
pop_common:
        SkASSERT(*p == '}' || *p == ']');

        fNestingStack.pop_back();
        ++p;

        if (fNestingStack.empty()) {
            // Stop condition: parsed the top level element and there is no trailing garbage.
            p = skip_ws(p);
            return *p == '\0';
        }

        goto match_post_value;

match_array:
        SkASSERT(*p == '[');
        p = skip_ws(p + 1);

        parser.onPushArray();

        if (*p != ']') goto match_value;

        // goto pop_array;
pop_array:
        SkASSERT(*p == ']');
        SkASSERT(!fNestingStack.empty());

        if (fNestingStack.back() != '[')
            return error(p, "unexpected array terminator");

        parser.onPopArray();

        goto pop_common;

        SkASSERT(false);
        return false;
    }

private:
    SkSTArray <256, char, true> fNestingStack;

    bool error(const char* p, const char* msg) {
        SkDebugf("*** Parse error: %s\n", msg);
        return false;
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

        for (p = p + 1; *p; ++p) {
            // TODO: unescape?
            if (*p == '"') return p + 1;
        }

        return nullptr;
    }

    const char* match_fast_double_decimal_part(const char* p, NumberType* n,
                                               int sign, double d, int exp) {
        SkASSERT(exp <= 0);

        for (;;) {
            if (!is_decimal(*p)) break;
            d = d * 10.0 + (*p++ - '0'); --exp;
            if (!is_decimal(*p)) break;
            d = d * 10.0 + (*p++ - '0'); --exp;
        }

        if (is_numeric(*p)) {
            SkASSERT(*p == '.' || *p == 'e' || *p == 'E');
            // We either have malformed input, or an (unsupported) exponent.
            return nullptr;
        }

        *n = sign * d * pow10(exp);

        return p;
    }

    const char* match_fast_double_part(const char* p, NumberType* n, int sign, double d) {
        for (;;) {
            if (!is_decimal(*p)) break;
            d = d * 10.0 + (*p++ - '0');
            if (!is_decimal(*p)) break;
            d = d * 10.0 + (*p++ - '0');
        }

        if (!is_numeric(*p)) {
            // Matched (integral) double.
            *n = sign * d;
            return p;
        }

        return (*p == '.') ? match_fast_double_decimal_part(p + 1, n, sign, d, 0)
                           : nullptr;
    }

    const char* match_fast_32_or_double(const char* p, NumberType* n) {
        int sign = 1;
        if (*p == '-') {
            sign = -1;
            ++p;
        }

        const auto* digits_start = p;

        int32_t n32 = 0;
        static constexpr int32_t kMaxInt32 = (std::numeric_limits<int32_t>::max() - 9) / 10;
        // We're good up to 9 digits.
        for (;;) {
            if (!is_decimal(*p) || n32 > kMaxInt32) break;
            n32 = n32 * 10 + (*p++ - '0');
            if (!is_decimal(*p) || n32 > kMaxInt32) break;
            n32 = n32 * 10 + (*p++ - '0');
        }

        if (!is_numeric(*p)) {
            // Did we actually match any digits?
            if (p > digits_start) {
                *n = static_cast<double>(sign * n32);
                return p;
            }
            return nullptr;
        }

        if (*p == '.') {
            const auto* decimals_start = ++p;

            int exp = 0;

            // This is the largest absolute int32 value we can handle before
            // risking overflow *on the next digit* (214748363).
            static constexpr int32_t kMaxInt32 = (std::numeric_limits<int32_t>::max() - 9) / 10;

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
                return match_fast_double_decimal_part(p, n, sign, n32, exp);
            }
        }

        return match_fast_double_part(p, n, sign, n32);
    }

    const char* match_number(const char* p, NumberType* n) {
        if (const auto* fast = match_fast_32_or_double(p, n)) return fast;

        // slow fallback
        char* matched;

        *n = strtod(p, &matched);
        return matched > p ? matched : nullptr;
    }

    SkSTArray<256, char, true> fStringBuffer;
};

class MemoryStream final {
public:
    MemoryStream(const char* data, size_t size) : fCurrent(data), fEnd(data + size) {}

    const char* peek(size_t count = 1) const {
        return fCurrent + count <= fEnd ? fCurrent : nullptr;
    }

    void skip(size_t count = 1) {
        fCurrent += count;
        SkASSERT(fCurrent <= fEnd);
    }

private:
    const char* fCurrent;
    const char* fEnd;
};

class TestParser final {
public:
    bool onPushObject() {
        this->indent();
        SkDebugf("[push object]\n");
        fIndent++;

        return true;
    }

    bool onPopObject() {
        SkASSERT(fIndent > 0);
        fIndent--;

        this->indent();
        SkDebugf("[pop object]\n");

        return true;
    }

    bool onObjectKey(const char* s, size_t size) {
        SkString str(s, size);

        this->indent();
        SkDebugf("[key]: \"%s\"\n", str.c_str());

        return true;
    }

    bool onPushArray() {
        this->indent();
        SkDebugf("[push array]\n");
        fIndent++;

        return true;
    }

    bool onPopArray() {
        SkASSERT(fIndent > 0);
        fIndent--;

        this->indent();
        SkDebugf("[pop array]\n");

        return true;
    }

    bool onTrue() {
        this->indent();
        SkDebugf("TRUE\n");

        return true;
    }

    bool onFalse() {
        this->indent();
        SkDebugf("FALSE\n");

        return true;
    }

    bool onNull() {
        this->indent();
        SkDebugf("NULL\n");

        return true;
    }

    bool onString(const char* s, size_t size) {
        SkString str(s, size);

        this->indent();
        SkDebugf("\"%s\"\n", str.c_str());

        return true;
    }

    bool onNumber(double d) {
        this->indent();
        SkDebugf("%g\n", d);

        return true;
    }

private:
    void indent() const {
        static constexpr unsigned kIndentSize = 2;
        for (unsigned i = 0; i < fIndent * kIndentSize; ++i) {
            SkDebugf(" ");
        }
    }

    unsigned fIndent = 0;
};

class DomParser final {
public:
    DomParser(SkArenaAlloc& alloc) : fAlloc(alloc) {}

    bool onPushObject() {
        fScopeStack.push_back(fRecStack.count());
        return true;
    }

    bool onPopObject() {
        SkASSERT(!fScopeStack.empty());
        SkASSERT(fRecStack.count() >= fScopeStack.back());

        const auto* scoped_begin    = fRecStack.begin() + fScopeStack.back();
        const auto  scoped_count    = SkToU32(fRecStack.count() - fScopeStack.back());
        const Value::Rec object_rec = ObjectRec::Make(scoped_begin, scoped_count, fAlloc);

        fScopeStack.pop_back();
        fRecStack.pop_back_n(scoped_count);
        fRecStack.push_back(std::move(object_rec));

        return true;
    }

    bool onObjectKey(const char* s, size_t size) {
        SkASSERT(fRecStack.count() >= fScopeStack.back());
        SkASSERT(!((fRecStack.count() - fScopeStack.back()) & 1));
        return this->onString(s, size);
    }

    bool onPushArray() {
        fScopeStack.push_back(fRecStack.count());
        return true;
    }

    bool onPopArray() {
        SkASSERT(!fScopeStack.empty());
        SkASSERT(fRecStack.count() >= fScopeStack.back());

        const auto* scoped_begin   = fRecStack.begin() + fScopeStack.back();
        const auto  scoped_count   = SkToU32(fRecStack.count() - fScopeStack.back());
        const Value::Rec array_rec = ArrayRec::Make(scoped_begin, scoped_count, fAlloc);

        fScopeStack.pop_back();
        fRecStack.pop_back_n(scoped_count);
        fRecStack.push_back(std::move(array_rec));

        return true;
    }

    bool onTrue() {
        fRecStack.push_back(BoolRec::Make(true));
        return true;
    }

    bool onFalse() {
        fRecStack.push_back(BoolRec::Make(false));
        return true;
    }

    bool onNull() {
        fRecStack.push_back(NullRec::Make());
        return true;
    }

    bool onString(const char* s, size_t size) {
        if (size <= InlineStringRec::max_size) {
            fRecStack.push_back(InlineStringRec::Make(s, size));
            return true;
        }

        if (!SkTFitsIn<uint32_t>(size) || size == std::numeric_limits<size_t>::max()) {
            // TODO: log error?
            return false;
        }

        fRecStack.push_back(StringRec::Make(s, size, fAlloc));
        return true;
    }

    bool onNumber(double d) {
        fRecStack.push_back(NumberRec::Make(static_cast<NumberType>(d)));
        return true;
    }

private:
    friend class skjson::Dom;

    SkArenaAlloc& fAlloc;

    SkSTArray<128, Value::Rec, true> fRecStack;
    SkSTArray< 64, int       , true> fScopeStack;
};

} // namespace

static constexpr size_t kMinChunkSize = 4096;

Dom::Dom() : fAlloc(kMinChunkSize) {}

Dom::Dom(const char* data, size_t size)
    : fAlloc(kMinChunkSize) {

    const auto cstring_data = SkData::MakeUninitialized(size + 1);
    auto* cstr = static_cast<char*>(cstring_data->writable_data());
    memcpy(cstr, data, size);
    cstr[size] = '\0';

    DomParser parser(fAlloc);

    Lexer<DomParser> lexer;

    pjson::document doc;

    const auto t0 = SkTime::GetMSecs();
    if (!lexer.consume(cstr, parser)) {
        SkDebugf("*** Parsing failed!\n");
        return;
    }
    const auto t1 = SkTime::GetMSecs();

    const auto res1 = doc.deserialize_in_place(cstr);

    const auto t2 = SkTime::GetMSecs();

    if (1) {
        SkDebugf("* rec stack: %d\n", parser.fRecStack.count());
        SkDebugf("** skson: [%gms, %d], pjson: [%gms]\n", t1 - t0, res1, t2 - t1);
    }

    SkASSERT(parser.fRecStack.count() == 1);
}

bool Parse(const char* data, size_t size) {
    const auto cstring_data = SkData::MakeUninitialized(size + 1);
    auto* cstr = static_cast<char*>(cstring_data->writable_data());
    memcpy(cstr, data, size);
    cstr[size] = '\0';

    TestParser parser;
    Lexer<TestParser> lex;

    return lex.consume(cstr, parser);
}

} // namespace skjson
