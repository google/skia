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
    bool operator==(const SkPDFIndirectReference& v) const { return fValue == v.fValue; }
    bool operator!=(const SkPDFIndirectReference& v) const { return fValue != v.fValue; }
};

// Use SkPDFName() to construct either type of SkPDFxxxxxName structs.
inline SkPDFStringName SkPDFName(SkString name) { return SkPDFStringName{std::move(name)}; }
inline SkPDFStaticName SkPDFName(const char* name) { return SkPDFStaticName{name}; }

class SkPDFArray;  // vector of pdf objects
class SkPDFDict;   // vector of key, value pairs, values are pdf objects

using SkPDFObject = skstd::variant<bool,                         // Boolean
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
                                   >;

static_assert(sizeof(SkString) == sizeof(char*));
static_assert(sizeof(std::unique_ptr<SkPDFArray>) == sizeof(char*));

class SkPDFArray {
public:
    SkPDFArray();
    SkPDFArray(const SkPDFArray&) = delete;
    SkPDFArray& operator=(const SkPDFArray&) = delete;
    ~SkPDFArray();
    const std::vector<SkPDFObject>& elements() const { return fArray; }
    void emplace(SkPDFObject);
    void reserve(size_t);
    size_t size() const;
    template <typename T> void append(T v) { this->emplace(SkPDFObject{std::move(v)}); }

private:
    std::vector<SkPDFObject> fArray;
};

template <typename T, typename... Args>
static inline void SkPDFArray_Append(SkPDFArray* a, T&& v, Args... args) {
    a->append(std::move(v));
    SkPDFArray_Append(a, args...);
}

static inline void SkPDFArray_Append(SkPDFArray* a) {}

template <typename... Args> static inline std::unique_ptr<SkPDFArray> SkPDFMakeArray(Args... args) {
    auto ret = std::make_unique<SkPDFArray>();
    ret->reserve(sizeof...(Args));
    SkPDFArray_Append(ret.get(), args...);
    return ret;
}

class SkPDFDict {
public:
    SkPDFDict();
    SkPDFDict(const SkPDFDict&) = delete;
    ~SkPDFDict();
    SkPDFDict& operator=(const SkPDFDict&) = delete;
    const std::vector<std::pair<SkPDFObject, SkPDFObject>>& elements() const { return fRecords; }
    void emplace(SkPDFObject k, SkPDFObject v);
    void reserve(size_t);
    // @param key is either const char[] or SkString
    // @param value is any type SkPDFObject can hold.
    template <typename K, typename V> void insert(K key, V value) {
        this->emplace(SkPDFObject{SkPDFName(std::move(key))}, SkPDFObject{std::move(value)});
    }

private:
    std::vector<std::pair<SkPDFObject, SkPDFObject>> fRecords;
};

void SkPDFEmit(const SkPDFDict&, SkWStream*);
void SkPDFEmit(const SkPDFArray&, SkWStream*);

// Exposed for unit testing.
void SkPDFEmit(const SkPDFObject&, SkWStream*);

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
