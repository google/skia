/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSONWriter_DEFINED
#define SkJSONWriter_DEFINED

#include "SkStream.h"

#include <inttypes.h>

/**
 *  Lightweight class for writing properly structured JSON data. No random-access, everything must
 *  be generated in-order. The resulting JSON can be retrieved by calling getString().
 *
 *  This is essentially just a wrapper over SkString, with a basic state machine to ensure that
 *  JSON is structured correctly, and to allow for (optional) pretty formatting.
 *
 *  This class adheres to the RFC-4627 usage of JSON (not ECMA-404). In other words, all JSON
 *  created with this class must have a top-level object or array. Free-floating values of other
 *  types are not considered valid.
 *
 *  Note that all error checking is in the form of asserts - invalid usage in a non-debug build
 *  will simply produce invalid JSON.
 */
class SkJSONWriter : SkNoncopyable {
public:
    enum class Mode { kFast, kPretty };

    SkJSONWriter(SkWStream* stream, Mode mode = Mode::kFast)
            : fStream(stream), fMode(mode), fState(State::kStart) {
        fScopeStack.push_back(Scope::kNone);
    }

    ~SkJSONWriter() {
        SkASSERT(fScopeStack.count() == 1);
    }

    /**
     *  Append the name (key) portion of an object member. Must be called between beginObject() and
     *  endObject(). If you have both the name and value of an object member, you can simply call
     *  the two argument versions of the other append functions.
     */
    void appendName(const char* name) {
        if (!name) {
            return;
        }
        SkASSERT(Scope::kObject == scope());
        SkASSERT(State::kObjectBegin == fState || State::kObjectValue == fState);
        if (State::kObjectValue == fState) {
            fStream->write(",", 1);
        }
        this->newline();
        fStream->write("\"", 1);
        fStream->writeText(name);
        fStream->write("\":", 2);
        fState = State::kObjectName;
    }

    /**
     *  Adds a new object. A name must be supplied when called between beginObject() and
     *  endObject(). Calls to beginObject() must be balanced by corresponding calls to endObject().
     */
    void beginObject(const char* name = nullptr) {
        this->appendName(name);
        this->beginValue(true);
        fStream->write("{", 1);
        fScopeStack.push_back(Scope::kObject);
        fState = State::kObjectBegin;
    }

    /**
     *  Ends an object that was previously started with beginObject().
     */
    void endObject() {
        SkASSERT(Scope::kObject == scope());
        SkASSERT(State::kObjectBegin == fState || State::kObjectValue == fState);
        bool emptyObject = State::kObjectBegin == fState;
        this->popScope();
        if (!emptyObject) {
            this->newline();
        }
        fStream->write("}", 1);
    }

    /**
     *  Adds a new array. A name must be supplied when called between beginObject() and
     *  endObject(). Calls to beginArray() must be balanced by corresponding calls to endArray().
     */
    void beginArray(const char* name = nullptr) {
        this->appendName(name);
        this->beginValue(true);
        fStream->write("[", 1);
        fScopeStack.push_back(Scope::kArray);
        fState = State::kArrayBegin;
    }

    /**
     *  Ends an array that was previous started with beginArray().
     */
    void endArray() {
        SkASSERT(Scope::kArray == scope());
        SkASSERT(State::kArrayBegin == fState || State::kArrayValue == fState);
        bool emptyArray = State::kArrayBegin == fState;
        this->popScope();
        if (!emptyArray) {
            this->newline();
        }
        fStream->write("]", 1);
    }
    /**
     *  Functions for adding values of various types. The single argument versions add un-named
     *  values, so must be called either
     *  - Between beginArray() and endArray()                                -or-
     *  - Between beginObject() and endObject(), after calling appendName()
     */
    void appendString(const char* value) {
        this->beginValue();
        fStream->write("\"", 1);
        if (value) {
            while (*value) {
                switch (*value) {
                    case '"': fStream->write("\\\"", 2); break;
                    case '\\': fStream->write("\\\\", 2); break;
                    case '\b': fStream->write("\\b", 2); break;
                    case '\f': fStream->write("\\f", 2); break;
                    case '\n': fStream->write("\\n", 2); break;
                    case '\r': fStream->write("\\r", 2); break;
                    case '\t': fStream->write("\\t", 2); break;
                    default: fStream->write(value, 1); break;
                }
                value++;
            }
        }
        fStream->write("\"", 1);
    }

    void appendPointer(const void* value) { this->beginValue(); this->appendf("\"%p\"", value); }
    void appendBool(bool value) {
        this->beginValue();
        if (value) {
            fStream->write("true", 4);
        } else {
            fStream->write("false", 5);
        }
    }
    void appendS32(int32_t value) { this->beginValue(); this->appendf("%d", value); }
    void appendS64(int64_t value) { this->beginValue(); this->appendf("%" PRId64, value); }
    void appendU32(uint32_t value) { this->beginValue(); this->appendf("%u", value); }
    void appendU64(uint64_t value) { this->beginValue(); this->appendf("%" PRIu64, value); }
    void appendFloat(float value) { this->beginValue(); this->appendf("%f", value);; }
    void appendDouble(double value) { this->beginValue(); this->appendf("%f", value); }
    void appendHexU32(uint32_t value) { this->beginValue(); this->appendf("\"0x%x\"", value); }
    void appendHexU64(uint64_t value) { this->beginValue(); this->appendf("\"0x%" PRIx64 "\"", value); }

#define DEFINE_NAMED_APPEND(function, type) \
    void function(const char* name, type value) { this->appendName(name); this->function(value); }

    /**
     *  Functions for adding named values of various types. These add a name field, so must be
     *  called between beginObject() and endObject().
     */
    DEFINE_NAMED_APPEND(appendString, const char *)
    DEFINE_NAMED_APPEND(appendPointer, const void *)
    DEFINE_NAMED_APPEND(appendBool, bool)
    DEFINE_NAMED_APPEND(appendS32, int32_t)
    DEFINE_NAMED_APPEND(appendS64, int64_t)
    DEFINE_NAMED_APPEND(appendU32, uint32_t)
    DEFINE_NAMED_APPEND(appendU64, uint64_t)
    DEFINE_NAMED_APPEND(appendFloat, float)
    DEFINE_NAMED_APPEND(appendDouble, double)
    DEFINE_NAMED_APPEND(appendHexU32, uint32_t)
    DEFINE_NAMED_APPEND(appendHexU64, uint64_t)

#undef DEFINE_NAMED_APPEND

private:
    enum class Scope {
        kNone,
        kObject,
        kArray
    };

    enum class State {
        kStart,
        kEnd,
        kObjectBegin,
        kObjectName,
        kObjectValue,
        kArrayBegin,
        kArrayValue,
    };

    void appendf(const char* fmt, ...) {
        const int kBufferSize = 1024;
        char buffer[kBufferSize];
        va_list argp;
        va_start(argp, fmt);
#ifdef SK_BUILD_FOR_WIN
        int length = _vsnprintf_s(buffer, kBufferSize, _TRUNCATE, fmt, argp);
#else
        int length = vsnprintf(buffer, kBufferSize, fmt, argp);
#endif
        SkASSERT(length >= 0 && length < kBufferSize);
        va_end(argp);
        fStream->write(buffer, length);
    }

    void beginValue(bool structure = false) {
        SkASSERT(State::kObjectName == fState ||
                 State::kArrayBegin == fState ||
                 State::kArrayValue == fState ||
                 (structure && State::kStart == fState));
        if (State::kArrayValue == fState) {
            fStream->write(",", 1);
        }
        if (Scope::kArray == scope()) {
            newline();
        } else if (Scope::kObject == scope() && Mode::kPretty == fMode) {
            fStream->write(" ", 1);
        }
        // We haven't added the value yet, but all (non-structure) callers emit something
        // immediately, so transition state, to simplify the calling code.
        if (!structure) {
            fState = Scope::kArray == scope() ? State::kArrayValue : State::kObjectValue;
        }
    }

    void newline() {
        if (Mode::kPretty == fMode) {
            fStream->write("\n", 1);
            for (int i = 0; i < fScopeStack.count() - 1; ++i) {
                fStream->write("   ", 3);
            }
        }
    }

    Scope scope() const {
        SkASSERT(!fScopeStack.empty());
        return fScopeStack.back();
    }

    void popScope() {
        fScopeStack.pop_back();
        switch (scope()) {
            case Scope::kNone:
                fState = State::kEnd;
                break;
            case Scope::kObject:
                fState = State::kObjectValue;
                break;
            case Scope::kArray:
                fState = State::kArrayValue;
                break;
            default:
                SkDEBUGFAIL("Invalid scope");
                break;
        }
    }

    SkWStream* fStream;
    Mode fMode;
    State fState;
    SkSTArray<16, Scope, true> fScopeStack;
};

#endif
