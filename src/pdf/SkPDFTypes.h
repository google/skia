/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTypes_DEFINED
#define SkPDFTypes_DEFINED

#include "include/core/SkScalar.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTo.h"
#include "src/pdf/SkVariant.h"

#include <cstdint>
#include <memory>
#include <vector>

class SkData;

class SkPDFDocument;
class SkStreamAsset;
class SkWStream;

// "PDF includes eight basic types of objects: Boolean values, Integer and Real
//  numbers, Strings, Names, Arrays, Dictionaries, Streams, and the null object.
//  Objects may be labelled so that they can be referred to by other objects. A
//  labelled object is called an indirect object."
//
// We do not need null objects.  Streams are handled separately.

// The following utility structs are used to differentiate data types with the same representation.
struct SkPDFColorComponentU {
    uint8_t fValue;
};

struct SkPDFColorComponentF {
    float fValue;
};

struct SkPDFStaticName {
    const char* fValue;
};

struct SkPDFStringName {
    SkString fValue;
};

struct SkPDFIndirectReference {
    int fValue = -1;
    explicit operator bool() { return fValue != -1; }
};

inline static bool operator==(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue == v.fValue;
}
inline static bool operator!=(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue != v.fValue;
}

// Use SkPDFName() to constsruct either type of SkPDFXXXXXName structs.
inline SkPDFStringName SkPDFName(SkString name) { return SkPDFStringName{std::move(name)}; }
inline SkPDFStaticName SkPDFName(const char* name) { return SkPDFStaticName{name}; }

class SkPDFArray;  // vector of pdf objects
class SkPDFDict;   // vector of key, value pairs, values are pdf objects

class SkPDFObject : public skstd::variant<bool,                         // Boolean
                                          int32_t,                      // Integer number
                                          SkScalar,                     // Real number
                                          SkPDFColorComponentU,         // Real number less precise
                                          SkPDFColorComponentF,         // Real number less precise
                                          SkString,                     // String
                                          const char*,                  // String
                                          SkPDFStringName,              // Name
                                          SkPDFStaticName,              // Name
                                          std::unique_ptr<SkPDFArray>,  // Array
                                          std::unique_ptr<SkPDFDict>,   // Dictionary
                                          SkPDFIndirectReference        // Indirect object
                                          > {
public:
    void emit(SkWStream*) const;
};

static_assert(sizeof(std::unique_ptr<SkPDFArray>) == sizeof(char*));

class SkPDFArray : public std::vector<SkPDFObject> {
public:
    void emit(SkWStream*) const;
    template <typename T> void append(T v) { this->push_back(SkPDFObject{std::move(v)}); }

    // The following inline functions can be replaced in the calling code in a later commit.
    void appendInt(int32_t v) { this->append(v); }
    void appendColorComponent(uint8_t v) { this->append(SkPDFColorComponentU{v}); }
    void appendBool(bool v) { this->append(v); }
    void appendScalar(SkScalar v) { this->append(v); }
    void appendName(const char v[]) { this->append(SkPDFName(v)); }
    void appendName(SkString v) { this->append(SkPDFName(std::move(v))); }
    void appendString(const char v[]) { this->append(v); }
    void appendString(SkString v) { this->append(std::move(v)); }
    void appendRef(SkPDFIndirectReference v) { this->append(v); }
    void appendObject(std::unique_ptr<SkPDFArray>&& v) { this->append(std::move(v)); }
    void appendObject(std::unique_ptr<SkPDFDict>&& v) { this->append(std::move(v)); }
};

template <typename T, typename... Args>
static inline void SkPDFArray_Append(SkPDFArray* a, T v, Args... args) {
    a->append(std::move(v));
    SkPDFArray_Append(a, args...);
}

static inline void SkPDFArray_Append(SkPDFArray* a) {}

template <typename... Args> static inline std::unique_ptr<SkPDFArray> SkPDFMakeArray(Args... args) {
    std::unique_ptr<SkPDFArray> ret(new SkPDFArray());
    ret->reserve(sizeof...(Args));
    SkPDFArray_Append(ret.get(), args...);
    return ret;
}

class SkPDFDict : public std::vector<std::pair<SkPDFObject, SkPDFObject>> {
public:
    explicit SkPDFDict(const char type[] = nullptr) {
        if (type) {
            this->insertName("Type", type);
        }
    }
    void emit(SkWStream*) const;
    // @param key is either const char[] or SkString
    // @param value is any type SkPDFObject can hold.
    template <typename K, typename V> void insert(K key, V value) {
        this->emplace_back(SkPDFObject{SkPDFName(std::move(key))}, SkPDFObject{std::move(value)});
    }

    // The following inline functions can be replaced in the calling code in a later commit.
    void insertObject(const char key[], std::unique_ptr<SkPDFArray>&& value) {
        this->insert(key, std::move(value));
    }
    void insertObject(SkString key, std::unique_ptr<SkPDFArray>&& value) {
        this->insert(std::move(key), std::move(value));
    }
    void insertObject(const char key[], std::unique_ptr<SkPDFDict>&& value) {
        this->insert(key, std::move(value));
    }
    void insertObject(SkString key, std::unique_ptr<SkPDFDict>&& value) {
        this->insert(std::move(key), std::move(value));
    }
    void insertRef(const char key[], SkPDFIndirectReference value) { this->insert(key, value); }
    void insertRef(SkString key, SkPDFIndirectReference value) {
        this->insert(std::move(key), value);
    }
    void insertBool(const char key[], bool value) { this->insert(key, value); }
    void insertInt(const char key[], int32_t value) { this->insert(key, value); }
    void insertInt(const char key[], size_t value) { this->insert(key, SkToS32(value)); }
    void insertScalar(const char key[], SkScalar value) { this->insert(key, value); }
    void insertName(const char key[], const char value[]) { this->insert(key, SkPDFName(value)); }
    void insertName(const char key[], SkString value) {
        this->insert(key, SkPDFName(std::move(value)));
    }
    void insertString(const char key[], const char value[]) { this->insert(key, value); }
    void insertString(const char key[], SkString value) { this->insert(key, std::move(value)); }
    void insertColorComponentF(const char key[], SkScalar value) {
        this->insert(key, SkPDFColorComponentF{value});
    }
};

static inline std::unique_ptr<SkPDFDict> SkPDFMakeDict(const char* type = nullptr) {
    return std::make_unique<SkPDFDict>(type);
}

// Exposed for unit testing.
void SkPDFWriteString(SkWStream* wStream, const char* cin, size_t len);

#ifdef SK_PDF_LESS_COMPRESSION
static constexpr bool kSkPDFDefaultDoDeflate = false;
#else
static constexpr bool kSkPDFDefaultDoDeflate = true;
#endif

SkPDFIndirectReference SkPDFStreamOut(std::unique_ptr<SkPDFDict> dict,
                                      std::unique_ptr<SkStreamAsset> stream,
                                      SkPDFDocument* doc,
                                      bool deflate = kSkPDFDefaultDoDeflate);
#endif
