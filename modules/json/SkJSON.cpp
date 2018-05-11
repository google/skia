/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJSON.h"

#include "SkString.h"
#include "SkTArray.h"

namespace skjson {

namespace {

template <typename Stream>
class Lexer {
public:
    bool consume(Stream& s) {
        // We expect exactly one top-level value in the stream.
        // TODO: are there other interpretations for top-level JSON validity?
        return this->consume_optional_ws(s)
            && this->consume_value(s)
            && this->consume_optional_ws(s)
            && !s.peek();
    }

private:
    static inline bool is_ws(char c) {
        return c == ' ' || c == '\n' || c == '\r' || c == '\t';
    }

    static bool consume_optional_ws(Stream& s) {
        while (const auto* c = s.peek()) {
            if (!is_ws(*c)) break;
            s.skip();
        }
        return true;
    }

    static bool consume(Stream& s, char ch) {
        const char* c = s.peek();
        if (c && *c == ch) {
            s.skip();
            return true;
        }
        return false;
    }

    bool consume_value(Stream& s) {
        const auto* c = s.peek();
        if (!c) {
            return false;
        }

        switch (*c) {
        case '"':
            return this->consume_string(s, emit_string_value);
        case '[':
            return this->consume_array(s);
        case 'f':
            return this->match_false(s);
        case 'n':
            return this->match_null(s);
        case 't':
            return this->match_true(s);
        case '{':
            return this->consume_object(s);
        default:
            break;
        }

        // TODO: log error
        printf("*** unexpectd token: >%c<\n", *c);
        return false;
    }

    bool match_true(Stream& s) {
        SkASSERT(*s.peek() == 't');
        s.skip();

        if (this->consume(s, 'r') &&
            this->consume(s, 'u') &&
            this->consume(s, 'e')) {
            this->emit_true_value();
            return true;
        }

        return false;
    }

    bool match_false(Stream& s) {
        SkASSERT(*s.peek() == 'f');
        s.skip();

        if (this->consume(s, 'a') &&
            this->consume(s, 'l') &&
            this->consume(s, 's') &&
            this->consume(s, 'e')) {
            this->emit_false_value();
            return true;
        }

        return false;
    }

    bool match_null(Stream& s) {
        SkASSERT(*s.peek() == 'n');
        s.skip();

        if (this->consume(s, 'u') &&
            this->consume(s, 'l') &&
            this->consume(s, 'l')) {
            this->emit_null_value();
            return true;
        }

        return false;
    }

    bool consume_string(Stream& s, void(*emitter)(const char*, size_t)) {
        SkASSERT(*s.peek() == '"');
        s.skip();

        // TODO: does this interfere with the reserved mem?
        fStringBuffer.resize_back(0);

        while (const auto* c = s.peek()) {
            s.skip();
            if (*c == '"') {
                emitter(fStringBuffer.begin(), fStringBuffer.count());
                return true;
            }

            // TODO: escaped chars
            fStringBuffer.push_back(*c);
        }

        return false;
    }

    template <typename MatchElementFunc>
    bool match_list(Stream& s, char terminator, MatchElementFunc&& Func) {
        size_t size = 0;
        bool trailing_comma = false;

        this->consume_optional_ws(s);
        while (!this->consume(s, terminator)) {
            if (size > 0 && !trailing_comma) {
                // missing comma separator
                return false;
            }

            if (!Func(s)) {
                // invalid element
                return false;
            }

            this->consume_optional_ws(s);
            trailing_comma = this->consume(s, ',');
            this->consume_optional_ws(s);

            size++;
        }

        // trailing comma ?
        return !trailing_comma;
    }

    bool consume_object(Stream& s) {
        SkASSERT(*s.peek() == '{');
        s.skip();

        this->emit_object_begin();

        if (!this->match_list(s, '}', [this](Stream& s) {
            return this->consume_string(s, emit_object_key)
                && this->consume_optional_ws(s)
                && this->consume(s, ':')
                && this->consume_optional_ws(s)
                && this->consume_value(s);
        })) {
            return false;
        }

        this->emit_object_end();

        return true;
    }

    bool consume_array(Stream& s) {
        SkASSERT(*s.peek() == '[');
        s.skip();

        this->emit_array_begin();

        if (!this->match_list(s, ']', [this](Stream& s) {
            return this->consume_value(s);
        })) {
            return false;
        }

        this->emit_array_end();

        return true;
    }

    static void emit_true_value() {
        // testing
        printf("[TRUE]\n");
    }

    static void emit_false_value() {
        // testing
        printf("[FALSE]\n");
    }

    static void emit_null_value() {
        // testing
        printf("[NULL]\n");
    }

    static void emit_string_value(const char* s, size_t size) {
        // testing
        SkString string(s, size);
        printf("[string] : \"%s\"\n", string.c_str());
    }

    static void emit_object_begin() {
        // testing
        printf("[begin_object]\n");
    }

    static void emit_object_end() {
        // testing
        printf("[end_object]\n");
    }

    static void emit_object_key(const char* s, size_t size) {
        // testing
        SkString string(s, size);
        printf("[object key] : \"%s\"\n", string.c_str());
    }

    static void emit_array_begin() {
        // testing
        printf("[begin_array]\n");
    }

    static void emit_array_end() {
        // testing
        printf("[end_array]\n");
    }

    SkSTArray<256, char, true> fStringBuffer;
};

class MemoryStream final {
public:
    MemoryStream(const char* data, size_t size) : fCurrent(data), fEnd(data + size) {}

    const char* peek() const { return fCurrent < fEnd ? fCurrent : nullptr; }

    void skip(size_t count = 1) {
        fCurrent += count;
        SkASSERT(fCurrent <= fEnd);
    }

private:
    const char* fCurrent;
    const char* fEnd;
};

} // namespace

bool Parse(const char* str, size_t size) {
    MemoryStream stream(str, size);
    Lexer<MemoryStream> lex;

    return lex.consume(stream);
}

} // namespace skjson
