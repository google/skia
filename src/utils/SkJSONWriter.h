/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJSONWriter_DEFINED
#define SkJSONWriter_DEFINED

#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkNoncopyable.h"
#include "include/private/base/SkTArray.h"
#include "src/base/SkUTF.h"

#include <cstring>
#include <cstdint>
#include <string>
#include <type_traits>

/**
 *  Lightweight class for writing properly structured JSON data. No random-access, everything must
 *  be generated in-order. The resulting JSON is written directly to the SkWStream supplied at
 *  construction time. Output is buffered, so writing to disk (via an SkFILEWStream) is ideal.
 *
 *  There is a basic state machine to ensure that JSON is structured correctly, and to allow for
 *  (optional) pretty formatting.
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
    enum class Mode {
        /**
         *  Output the minimal amount of text. No additional whitespace (including newlines) is
         *  generated. The resulting JSON is suitable for fast parsing and machine consumption.
         */
        kFast,

        /**
         *  Output human-readable JSON, with indented  objects and arrays, and one value per line.
         *  Slightly slower than kFast, and produces data that is somewhat larger.
         */
        kPretty
    };

    /**
     *  Construct a JSON writer that will serialize all the generated JSON to 'stream'.
     */
    SkJSONWriter(SkWStream* stream, Mode mode = Mode::kFast)
            : fBlock(new char[kBlockSize])
            , fWrite(fBlock)
            , fBlockEnd(fBlock + kBlockSize)
            , fStream(stream)
            , fMode(mode)
            , fState(State::kStart) {
        fScopeStack.push_back(Scope::kNone);
        fNewlineStack.push_back(true);
    }

    ~SkJSONWriter() {
        this->flush();
        delete[] fBlock;
        SkASSERT(fScopeStack.size() == 1);
        SkASSERT(fNewlineStack.size() == 1);
    }

    /**
     *  Force all buffered output to be flushed to the underlying stream.
     */
    void flush() {
        if (fWrite != fBlock) {
            fStream->write(fBlock, fWrite - fBlock);
            fWrite = fBlock;
        }
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
        SkASSERT(Scope::kObject == this->scope());
        SkASSERT(State::kObjectBegin == fState || State::kObjectValue == fState);
        if (State::kObjectValue == fState) {
            this->write(",", 1);
        }
        this->separator(this->multiline());
        this->write("\"", 1);
        this->write(name, strlen(name));
        this->write("\":", 2);
        fState = State::kObjectName;
    }

    /**
     *  Adds a new object. A name must be supplied when called between beginObject() and
     *  endObject(). Calls to beginObject() must be balanced by corresponding calls to endObject().
     *  By default, objects are written out with one named value per line (when in kPretty mode).
     *  This can be overridden for a particular object by passing false for multiline, this will
     *  keep the entire object on a single line. This can help with readability in some situations.
     *  In kFast mode, this parameter is ignored.
     */
    void beginObject(const char* name = nullptr, bool multiline = true) {
        this->appendName(name);
        this->beginValue(true);
        this->write("{", 1);
        fScopeStack.push_back(Scope::kObject);
        fNewlineStack.push_back(multiline);
        fState = State::kObjectBegin;
    }

    /**
     *  Ends an object that was previously started with beginObject().
     */
    void endObject() {
        SkASSERT(Scope::kObject == this->scope());
        SkASSERT(State::kObjectBegin == fState || State::kObjectValue == fState);
        bool emptyObject = State::kObjectBegin == fState;
        bool wasMultiline = this->multiline();
        this->popScope();
        if (!emptyObject) {
            this->separator(wasMultiline);
        }
        this->write("}", 1);
    }

    /**
     *  Adds a new array. A name must be supplied when called between beginObject() and
     *  endObject(). Calls to beginArray() must be balanced by corresponding calls to endArray().
     *  By default, arrays are written out with one value per line (when in kPretty mode).
     *  This can be overridden for a particular array by passing false for multiline, this will
     *  keep the entire array on a single line. This can help with readability in some situations.
     *  In kFast mode, this parameter is ignored.
     */
    void beginArray(const char* name = nullptr, bool multiline = true) {
        this->appendName(name);
        this->beginValue(true);
        this->write("[", 1);
        fScopeStack.push_back(Scope::kArray);
        fNewlineStack.push_back(multiline);
        fState = State::kArrayBegin;
    }

    /**
     *  Ends an array that was previous started with beginArray().
     */
    void endArray() {
        SkASSERT(Scope::kArray == this->scope());
        SkASSERT(State::kArrayBegin == fState || State::kArrayValue == fState);
        bool emptyArray = State::kArrayBegin == fState;
        bool wasMultiline = this->multiline();
        this->popScope();
        if (!emptyArray) {
            this->separator(wasMultiline);
        }
        this->write("]", 1);
    }

    /**
     *  Functions for adding values of various types. The single argument versions add un-named
     *  values, so must be called either
     *  - Between beginArray() and endArray()                                -or-
     *  - Between beginObject() and endObject(), after calling appendName()
     */
    void appendString(const char* value, size_t size) {
        this->beginValue();
        this->write("\"", 1);
        if (value) {
            char const * const end = value + size;
            while (value < end) {
                char const * next = value;
                SkUnichar u = SkUTF::NextUTF8(&next, end);
                switch (u) {
                    case '"': this->write("\\\"", 2); break;
                    case '\\': this->write("\\\\", 2); break;
                    case '\b': this->write("\\b", 2); break;
                    case '\f': this->write("\\f", 2); break;
                    case '\n': this->write("\\n", 2); break;
                    case '\r': this->write("\\r", 2); break;
                    case '\t': this->write("\\t", 2); break;
                    default: {
                        if (u < 0) {
                            next = value + 1;
                            SkString s("\\u");
                            s.appendHex((unsigned char)*value, 4);
                            this->write(s.c_str(), s.size());
                        } else if (u < 0x20) {
                            SkString s("\\u");
                            s.appendHex(u, 4);
                            this->write(s.c_str(), s.size());
                        } else {
                            this->write(value, next - value);
                        }
                    } break;
                }
                value = next;
            }
        }
        this->write("\"", 1);
    }
    void appendString(const SkString& value) {
        this->appendString(value.c_str(), value.size());
    }
    // Avoid the non-explicit converting constructor from char*
    template <class T, std::enable_if_t<std::is_same_v<T,std::string>,bool> = false>
    void appendString(const T& value) {
        this->appendString(value.data(), value.size());
    }
    template <size_t N> inline void appendNString(char const (&value)[N]) {
        static_assert(N > 0);
        this->appendString(value, N-1);
    }
    void appendCString(const char* value) {
        this->appendString(value, value ? strlen(value) : 0);
    }

    void appendPointer(const void* value) { this->beginValue(); this->appendf("\"%p\"", value); }
    void appendBool(bool value) {
        this->beginValue();
        if (value) {
            this->write("true", 4);
        } else {
            this->write("false", 5);
        }
    }
    void appendS32(int32_t value) { this->beginValue(); this->appendf("%d", value); }
    void appendS64(int64_t value);
    void appendU32(uint32_t value) { this->beginValue(); this->appendf("%u", value); }
    void appendU64(uint64_t value);
    void appendFloat(float value) { this->beginValue(); this->appendf("%g", value); }
    void appendDouble(double value) { this->beginValue(); this->appendf("%g", value); }
    void appendFloatDigits(float value, int digits) {
        this->beginValue();
        this->appendf("%.*g", digits, value);
    }
    void appendDoubleDigits(double value, int digits) {
        this->beginValue();
        this->appendf("%.*g", digits, value);
    }
    void appendHexU32(uint32_t value) { this->beginValue(); this->appendf("\"0x%x\"", value); }
    void appendHexU64(uint64_t value);

    void appendString(const char* name, const char* value, size_t size) {
        this->appendName(name);
        this->appendString(value, size);
    }
    void appendString(const char* name, const SkString& value) {
        this->appendName(name);
        this->appendString(value.c_str(), value.size());
    }
    // Avoid the non-explicit converting constructor from char*
    template <class T, std::enable_if_t<std::is_same_v<T,std::string>,bool> = false>
    void appendString(const char* name, const T& value) {
        this->appendName(name);
        this->appendString(value.data(), value.size());
    }
    template <size_t N> inline void appendNString(const char* name, char const (&value)[N]) {
        static_assert(N > 0);
        this->appendName(name);
        this->appendString(value, N-1);
    }
    void appendCString(const char* name, const char* value) {
        this->appendName(name);
        this->appendString(value, value ? strlen(value) : 0);
    }
#define DEFINE_NAMED_APPEND(function, type) \
    void function(const char* name, type value) { this->appendName(name); this->function(value); }

    /**
     *  Functions for adding named values of various types. These add a name field, so must be
     *  called between beginObject() and endObject().
     */
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

    void appendFloatDigits(const char* name, float value, int digits) {
        this->appendName(name);
        this->appendFloatDigits(value, digits);
    }
    void appendDoubleDigits(const char* name, double value, int digits) {
        this->appendName(name);
        this->appendDoubleDigits(value, digits);
    }

private:
    enum {
        // Using a 32k scratch block gives big performance wins, but we diminishing returns going
        // any larger. Even with a 1MB block, time to write a large (~300 MB) JSON file only drops
        // another ~10%.
        kBlockSize = 32 * 1024,
    };

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

    void appendf(const char* fmt, ...) SK_PRINTF_LIKE(2, 3);

    void beginValue(bool structure = false) {
        SkASSERT(State::kObjectName == fState ||
                 State::kArrayBegin == fState ||
                 State::kArrayValue == fState ||
                 (structure && State::kStart == fState));
        if (State::kArrayValue == fState) {
            this->write(",", 1);
        }
        if (Scope::kArray == this->scope()) {
            this->separator(this->multiline());
        } else if (Scope::kObject == this->scope() && Mode::kPretty == fMode) {
            this->write(" ", 1);
        }
        // We haven't added the value yet, but all (non-structure) callers emit something
        // immediately, so transition state, to simplify the calling code.
        if (!structure) {
            fState = Scope::kArray == this->scope() ? State::kArrayValue : State::kObjectValue;
        }
    }

    void separator(bool multiline) {
        if (Mode::kPretty == fMode) {
            if (multiline) {
                this->write("\n", 1);
                for (int i = 0; i < fScopeStack.size() - 1; ++i) {
                    this->write("   ", 3);
                }
            } else {
                this->write(" ", 1);
            }
        }
    }

    void write(const char* buf, size_t length) {
        if (static_cast<size_t>(fBlockEnd - fWrite) < length) {
            // Don't worry about splitting writes that overflow our block.
            this->flush();
        }
        if (length > kBlockSize) {
            // Send particularly large writes straight through to the stream (unbuffered).
            fStream->write(buf, length);
        } else {
            memcpy(fWrite, buf, length);
            fWrite += length;
        }
    }

    Scope scope() const {
        SkASSERT(!fScopeStack.empty());
        return fScopeStack.back();
    }

    bool multiline() const {
        SkASSERT(!fNewlineStack.empty());
        return fNewlineStack.back();
    }

    void popScope() {
        fScopeStack.pop_back();
        fNewlineStack.pop_back();
        switch (this->scope()) {
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

    char* fBlock;
    char* fWrite;
    char* fBlockEnd;

    SkWStream* fStream;
    Mode fMode;
    State fState;
    skia_private::STArray<16, Scope, true> fScopeStack;
    skia_private::STArray<16, bool, true> fNewlineStack;
};

#endif
