/*
 * Copyright 2010 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPDFTypes_DEFINED
#define SkPDFTypes_DEFINED

#include "SkRefCnt.h"
#include "SkScalar.h"
#include "SkTHash.h"
#include "SkTo.h"
#include "SkTypes.h"
#include "SkMakeUnique.h"

#include <new>
#include <type_traits>
#include <utility>
#include <vector>
#include <memory>

class SkData;
class SkPDFArray;
class SkPDFCanon;
class SkPDFDict;
class SkPDFDocument;
class SkPDFObject;
class SkPDFUnion;
class SkStreamAsset;
class SkString;
class SkWStream;
struct SkPDFObjectSerializer;

#ifdef SK_PDF_IMAGE_STATS
    #include <atomic>
#endif

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

/** \class SkPDFObject

    A PDF Object is the base class for primitive elements in a PDF file.  A
    common subtype is used to ease the use of indirect object references,
    which are common in the PDF format.

*/
class SkPDFObject : public SkRefCnt {
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
    void appendString(const char[]);
    void appendString(SkString);
    void appendObject(sk_sp<SkPDFObject>);
    void appendRef(SkPDFIndirectReference);

private:
    std::vector<SkPDFUnion> fValues;
    void append(SkPDFUnion&& value);
};

static inline void SkPDFArray_Append(SkPDFArray* a, int v) { a->appendInt(v); }

static inline void SkPDFArray_Append(SkPDFArray* a, SkScalar v) { a->appendScalar(v); }

template <typename T, typename... Args>
inline void SkPDFArray_Append(SkPDFArray* a, T v, Args... args) {
    SkPDFArray_Append(a, v);
    SkPDFArray_Append(a, args...);
}

template <typename... Args>
inline sk_sp<SkPDFArray> SkPDFMakeArray(Args... args) {
    auto ret = sk_make_sp<SkPDFArray>();
    ret->reserve(sizeof...(Args));
    SkPDFArray_Append(ret.get(), args...);
    return ret;
}

/** \class SkPDFDict

    A dictionary object in a PDF.
*/
class SkPDFDict : public SkPDFObject {
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
    void insertObject(const char key[], sk_sp<SkPDFObject>);
    void insertObject(SkString, sk_sp<SkPDFObject>);
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
    void insertString(const char key[], const char value[]);
    void insertString(const char key[], SkString value);

    /** Emit the dictionary, without the "<<" and ">>".
     */
    void emitAll(SkWStream* stream) const;

private:
    std::vector<std::pair<SkPDFUnion, SkPDFUnion>> fRecords;
};

#ifdef SK_PDF_LESS_COMPRESSION
    static constexpr bool kSkPDFDefaultDoDeflate = false;
#else
    static constexpr bool kSkPDFDefaultDoDeflate = true;
#endif

SkPDFIndirectReference SkPDFStreamOut(sk_sp<SkPDFDict> dict,
                                      std::unique_ptr<SkStreamAsset> stream,
                                      SkPDFDocument* doc,
                                      bool deflate = kSkPDFDefaultDoDeflate);

////////////////////////////////////////////////////////////////////////////////

#ifdef SK_PDF_IMAGE_STATS
extern std::atomic<int> gDrawImageCalls;
extern std::atomic<int> gJpegImageObjects;
extern std::atomic<int> gRegularImageObjects;
extern void SkPDFImageDumpStats();
#endif // SK_PDF_IMAGE_STATS

#endif
