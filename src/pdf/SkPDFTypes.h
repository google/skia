/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTypes_DEFINED
#define SkPDFTypes_DEFINED

#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkTHash.h"

#include <memory>
#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

class SkData;
class SkPDFArray;

class SkPDFDict;
class SkPDFDocument;
class SkPDFObject;
class SkPDFUnion;
class SkStreamAsset;
class SkString;
class SkWStream;
struct SkPDFObjectSerializer;

struct SkPDFIndirectReference {
    int fValue = -1;
    explicit operator bool() const { return fValue != -1; }
};

inline static bool operator==(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue == v.fValue;
}

inline static bool operator!=(SkPDFIndirectReference u, SkPDFIndirectReference v) {
    return u.fValue != v.fValue;
}

/** \class SkPDFObject

    A PDF Object is the base class for primitive elements in a PDF file.  A
    common subtype is used to ease the use of indirect object references,
    which are common in the PDF format.

*/
class SkPDFObject {
public:
    SkPDFObject() = default;

    /** Subclasses must implement this method to print the object to the
     *  PDF file.
     *  @param catalog  The object catalog to use.
     *  @param stream   The writable output stream to send the output to.
     */
    virtual void emitObject(SkWStream* stream) const = 0;

    virtual ~SkPDFObject() = default;

private:
    SkPDFObject(SkPDFObject&&) = delete;
    SkPDFObject(const SkPDFObject&) = delete;
    SkPDFObject& operator=(SkPDFObject&&) = delete;
    SkPDFObject& operator=(const SkPDFObject&) = delete;
};

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFArray

    An array object in a PDF.
*/
class SkPDFArray final : public SkPDFObject {
public:
    /** Create a PDF array. Maximum length is 8191.
     */
    SkPDFArray();
    ~SkPDFArray() override;

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream) const override;

    /** The size of the array.
     */
    size_t size() const;

    /** Preallocate space for the given number of entries.
     *  @param length The number of array slots to preallocate.
     */
    void reserve(int length);

    /** Appends a value to the end of the array.
     *  @param value The value to add to the array.
     */
    void appendInt(int32_t);
    void appendColorComponent(uint8_t);
    void appendBool(bool);
    void appendScalar(SkScalar);
    void appendName(const char[]);
    void appendName(SkString);
    void appendByteString(const char[]);
    void appendTextString(const char[]);
    void appendByteString(SkString);
    void appendTextString(SkString);
    void appendObject(std::unique_ptr<SkPDFObject>&&);
    void appendRef(SkPDFIndirectReference);

private:
    std::vector<SkPDFUnion> fValues;
    void append(SkPDFUnion&& value);
};

static inline void SkPDFArray_Append(SkPDFArray* a, int v) { a->appendInt(v); }

static inline void SkPDFArray_Append(SkPDFArray* a, SkScalar v) { a->appendScalar(v); }

template <typename T, typename... Args>
static inline void SkPDFArray_Append(SkPDFArray* a, T v, Args... args) {
    SkPDFArray_Append(a, v);
    SkPDFArray_Append(a, args...);
}

static inline void SkPDFArray_Append(SkPDFArray* a) {}

template <typename... Args>
static inline std::unique_ptr<SkPDFArray> SkPDFMakeArray(Args... args) {
    std::unique_ptr<SkPDFArray> ret(new SkPDFArray());
    ret->reserve(sizeof...(Args));
    SkPDFArray_Append(ret.get(), args...);
    return ret;
}

/** \class SkPDFDict

    A dictionary object in a PDF.
*/
class SkPDFDict final : public SkPDFObject {
public:
    /** Create a PDF dictionary.
     *  @param type   The value of the Type entry, nullptr for no type.
     */
    explicit SkPDFDict(const char type[] = nullptr);

    ~SkPDFDict() override;

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream) const override;

    /** The size of the dictionary.
     */
    size_t size() const;

    /** Preallocate space for n key-value pairs */
    void reserve(int n);

    /** Add the value to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertObject(const char key[], std::unique_ptr<SkPDFObject>&&);
    void insertObject(SkString, std::unique_ptr<SkPDFObject>&&);
    void insertRef(const char key[], SkPDFIndirectReference);
    void insertRef(SkString, SkPDFIndirectReference);

    /** Add the value to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertBool(const char key[], bool value);
    void insertInt(const char key[], int32_t value);
    void insertInt(const char key[], size_t value);
    void insertScalar(const char key[], SkScalar value);
    void insertColorComponentF(const char key[], SkScalar value);
    void insertName(const char key[], const char nameValue[]);
    void insertName(const char key[], SkString nameValue);
    void insertByteString(const char key[], const char value[]);
    void insertTextString(const char key[], const char value[]);
    void insertByteString(const char key[], SkString value);
    void insertTextString(const char key[], SkString value);
    void insertUnion(const char key[], SkPDFUnion&&);

private:
    std::vector<std::pair<SkPDFUnion, SkPDFUnion>> fRecords;
};

static inline std::unique_ptr<SkPDFDict> SkPDFMakeDict(const char* type = nullptr) {
    return std::make_unique<SkPDFDict>(type);
}

enum class SkPDFSteamCompressionEnabled : bool {
    No = false,
    Yes = true,
    Default =
#ifdef SK_PDF_LESS_COMPRESSION
        No,
#else
        Yes,
#endif
};

// Exposed for unit testing.
void SkPDFWriteTextString(SkWStream* wStream, const char* cin, size_t len);
void SkPDFWriteByteString(SkWStream* wStream, const char* cin, size_t len);

SkPDFIndirectReference SkPDFStreamOut(
    std::unique_ptr<SkPDFDict> dict,
    std::unique_ptr<SkStreamAsset> stream,
    SkPDFDocument* doc,
    SkPDFSteamCompressionEnabled compress = SkPDFSteamCompressionEnabled::Default);
#endif
