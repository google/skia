// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef SkPDFUnion_DEFINED
#define SkPDFUnion_DEFINED

#include "src/pdf/SkPDFTypes.h"

template <class T>
class SkStorageFor {
public:
    const T& get() const { return *reinterpret_cast<const T*>(&fStore); }
    T& get() { return *reinterpret_cast<T*>(&fStore); }
    // Up to caller to keep track of status.
    template<class... Args> void init(Args&&... args) {
        new (&this->get()) T(std::forward<Args>(args)...);
    }
    void destroy() { this->get().~T(); }
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type fStore;
};

// Exposed for unit testing.
void SkPDFWriteString(SkWStream* wStream, const char* cin, size_t len);

////////////////////////////////////////////////////////////////////////////////

/**
   A SkPDFUnion is a non-virtualized implementation of the
   non-compound, non-specialized PDF Object types: Name, String,
   Number, Boolean.
 */
class SkPDFUnion {
public:
    // Move contstructor and assignment operator destroy the argument
    // and steal their references (if needed).
    SkPDFUnion(SkPDFUnion&& other);
    SkPDFUnion& operator=(SkPDFUnion&& other);

    ~SkPDFUnion();

    /** The following nine functions are the standard way of creating
        SkPDFUnion objects. */

    static SkPDFUnion Int(int32_t);

    static SkPDFUnion Int(size_t v) { return SkPDFUnion::Int(SkToS32(v)); }

    static SkPDFUnion Bool(bool);

    static SkPDFUnion Scalar(SkScalar);

    static SkPDFUnion ColorComponent(uint8_t);

    static SkPDFUnion ColorComponentF(float);

    /** These two functions do NOT take ownership of char*, and do NOT
        copy the string.  Suitable for passing in static const
        strings. For example:
          SkPDFUnion n = SkPDFUnion::Name("Length");
          SkPDFUnion u = SkPDFUnion::String("Identity"); */

    /** SkPDFUnion::Name(const char*) assumes that the passed string
        is already a valid name (that is: it has no control or
        whitespace characters).  This will not copy the name. */
    static SkPDFUnion Name(const char*);

    /** SkPDFUnion::String will encode the passed string.  This will
        not copy the name. */
    static SkPDFUnion String(const char*);

    /** SkPDFUnion::Name(SkString) does not assume that the
        passed string is already a valid name and it will escape the
        string. */
    static SkPDFUnion Name(SkString);

    /** SkPDFUnion::String will encode the passed string. */
    static SkPDFUnion String(SkString);

    static SkPDFUnion Object(std::unique_ptr<SkPDFObject>);

    static SkPDFUnion Ref(SkPDFIndirectReference);

    /** These two non-virtual methods mirror SkPDFObject's
        corresponding virtuals. */
    void emitObject(SkWStream*) const;

    bool isName() const;

private:
    union {
        int32_t fIntValue;
        bool fBoolValue;
        SkScalar fScalarValue;
        const char* fStaticString;
        SkStorageFor<SkString> fSkString;
        SkPDFObject* fObject;
    };
    enum class Type : char {
        /** It is an error to call emitObject() or addResources() on an
            kDestroyed object. */
        kDestroyed = 0,
        kInt,
        kColorComponent,
        kColorComponentF,
        kBool,
        kScalar,
        kName,
        kString,
        kNameSkS,
        kStringSkS,
        kObject,
        kRef,
    };
    Type fType;

    SkPDFUnion(Type);
    SkPDFUnion(Type, int32_t);
    SkPDFUnion(Type, bool);
    SkPDFUnion(Type, SkScalar);
    SkPDFUnion(Type, SkString);
    // We do not now need copy constructor and copy assignment, so we
    // will disable this functionality.
    SkPDFUnion& operator=(const SkPDFUnion&) = delete;
    SkPDFUnion(const SkPDFUnion&) = delete;
};
static_assert(sizeof(SkString) == sizeof(void*), "SkString_size");

#endif  // SkPDFUnion_DEFINED
