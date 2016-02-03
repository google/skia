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
#include "SkStream.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTHash.h"
#include "SkTypes.h"

class SkPDFObjNumMap;
class SkPDFObject;
class SkPDFSubstituteMap;

#ifdef SK_PDF_IMAGE_STATS
#include "SkAtomics.h"
#endif

/** \class SkPDFObject

    A PDF Object is the base class for primitive elements in a PDF file.  A
    common subtype is used to ease the use of indirect object references,
    which are common in the PDF format.

*/
class SkPDFObject : public SkRefCnt {
public:
    /** Subclasses must implement this method to print the object to the
     *  PDF file.
     *  @param catalog  The object catalog to use.
     *  @param stream   The writable output stream to send the output to.
     */
    // TODO(halcanary): make this method const
    virtual void emitObject(SkWStream* stream,
                            const SkPDFObjNumMap& objNumMap,
                            const SkPDFSubstituteMap& substitutes) const = 0;

    /**
     *  Adds all transitive dependencies of this object to the
     *  catalog.  Implementations should respect the catalog's object
     *  substitution map.
     */
    virtual void addResources(SkPDFObjNumMap* catalog,
                              const SkPDFSubstituteMap& substitutes) const {}

private:
    typedef SkRefCnt INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/**
   A SkPDFUnion is a non-virtualized implementation of the
   non-compound, non-specialized PDF Object types: Name, String,
   Number, Boolean.
 */
class SkPDFUnion {
public:
    // u.move() is analogous to std::move(u). It returns an rvalue.
    SkPDFUnion move() { return static_cast<SkPDFUnion&&>(*this); }
    // Move contstructor and assignemnt operator destroy the argument
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

    /** These two functions do NOT take ownership of ptr, and do NOT
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

    /** SkPDFUnion::Name(const SkString&) does not assume that the
        passed string is already a valid name and it will escape the
        string. */
    static SkPDFUnion Name(const SkString&);

    /** SkPDFUnion::String will encode the passed string. */
    static SkPDFUnion String(const SkString&);

    /** This function DOES take ownership of the object. E.g.
          SkAutoTUnref<SkPDFDict> dict(new SkPDFDict);
          dict->insert(.....);
          SkPDFUnion u = SkPDFUnion::Object(dict.detach()) */
    static SkPDFUnion Object(SkPDFObject*);

    /** This function DOES take ownership of the object. E.g.
          SkAutoTUnref<SkPDFBitmap> image(
                 SkPDFBitmap::Create(fCanon, bitmap));
          SkPDFUnion u = SkPDFUnion::ObjRef(image.detach()) */
    static SkPDFUnion ObjRef(SkPDFObject*);

    /** These two non-virtual methods mirror SkPDFObject's
        corresponding virtuals. */
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const;
    void addResources(SkPDFObjNumMap*, const SkPDFSubstituteMap&) const;

    bool isName() const;

private:
    union {
        int32_t fIntValue;
        bool fBoolValue;
        SkScalar fScalarValue;
        const char* fStaticString;
        char fSkString[sizeof(SkString)];
        SkPDFObject* fObject;
    };
    enum class Type : char {
        /** It is an error to call emitObject() or addResources() on an
            kDestroyed object. */
        kDestroyed = 0,
        kInt,
        kBool,
        kScalar,
        kName,
        kString,
        kNameSkS,
        kStringSkS,
        kObjRef,
        kObject,
    };
    Type fType;

    SkPDFUnion(Type);
    // We do not now need copy constructor and copy assignment, so we
    // will disable this functionality.
    SkPDFUnion& operator=(const SkPDFUnion&) = delete;
    SkPDFUnion(const SkPDFUnion&) = delete;
};
static_assert(sizeof(SkString) == sizeof(void*), "SkString_size");

////////////////////////////////////////////////////////////////////////////////

#if 0  // Enable if needed.
/** This class is a SkPDFUnion with SkPDFObject virtuals attached.
    The only use case of this is when a non-compound PDF object is
    referenced indirectly. */
class SkPDFAtom final : public SkPDFObject {
public:
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap,
                    const SkPDFSubstituteMap& substitutes) final;
    void addResources(SkPDFObjNumMap*, const SkPDFSubstituteMap&) const final;
    SkPDFAtom(SkPDFUnion&& v) : fValue(v.move()) {}

private:
    const SkPDFUnion fValue;
    typedef SkPDFObject INHERITED;
};
#endif  // 0

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFArray

    An array object in a PDF.
*/
class SkPDFArray final : public SkPDFObject {
public:
    static const int kMaxLen = 8191;

    /** Create a PDF array. Maximum length is 8191.
     */
    SkPDFArray();
    virtual ~SkPDFArray();

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap,
                    const SkPDFSubstituteMap& substitutes) const override;
    void addResources(SkPDFObjNumMap*,
                      const SkPDFSubstituteMap&) const override;

    /** The size of the array.
     */
    int size() const;

    /** Preallocate space for the given number of entries.
     *  @param length The number of array slots to preallocate.
     */
    void reserve(int length);

    /** Appends a value to the end of the array.
     *  @param value The value to add to the array.
     */
    void appendInt(int32_t);
    void appendBool(bool);
    void appendScalar(SkScalar);
    void appendName(const char[]);
    void appendName(const SkString&);
    void appendString(const char[]);
    void appendString(const SkString&);
    /** appendObject and appendObjRef take ownership of the passed object */
    void appendObject(SkPDFObject*);
    void appendObjRef(SkPDFObject*);

private:
    SkTDArray<SkPDFUnion> fValues;
    void append(SkPDFUnion&& value);
    typedef SkPDFObject INHERITED;
};

/** \class SkPDFDict

    A dictionary object in a PDF.
*/
class SkPDFDict : public SkPDFObject {
public:
    /** Create a PDF dictionary. Maximum number of entries is 4095.
     */
    SkPDFDict();

    /** Create a PDF dictionary with a Type entry.
     *  @param type   The value of the Type entry.
     */
    explicit SkPDFDict(const char type[]);

    virtual ~SkPDFDict();

    // The SkPDFObject interface.
    void emitObject(SkWStream* stream,
                    const SkPDFObjNumMap& objNumMap,
                    const SkPDFSubstituteMap& substitutes) const override;
    void addResources(SkPDFObjNumMap*,
                      const SkPDFSubstituteMap&) const override;

    /** The size of the dictionary.
     */
    int size() const;

    /** Add the value to the dictionary with the given key.  Takes
     *  ownership of the object.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertObject(const char key[], SkPDFObject* value);
    void insertObject(const SkString& key, SkPDFObject* value);
    void insertObjRef(const char key[], SkPDFObject* value);
    void insertObjRef(const SkString& key, SkPDFObject* value);

    /** Add the value to the dictionary with the given key.
     *  @param key   The text of the key for this dictionary entry.
     *  @param value The value for this dictionary entry.
     */
    void insertBool(const char key[], bool value);
    void insertInt(const char key[], int32_t value);
    void insertInt(const char key[], size_t value);
    void insertScalar(const char key[], SkScalar value);
    void insertName(const char key[], const char nameValue[]);
    void insertName(const char key[], const SkString& nameValue);
    void insertString(const char key[], const char value[]);
    void insertString(const char key[], const SkString& value);

    /** Remove all entries from the dictionary.
     */
    void clear();

    /** Emit the dictionary, without the "<<" and ">>".
     */
    void emitAll(SkWStream* stream,
                 const SkPDFObjNumMap& objNumMap,
                 const SkPDFSubstituteMap& substitutes) const;

private:
    struct Record {
        SkPDFUnion fKey;
        SkPDFUnion fValue;
    };
    SkTDArray<Record> fRecords;
    static const int kMaxLen = 4095;

    void set(SkPDFUnion&& name, SkPDFUnion&& value);

    typedef SkPDFObject INHERITED;
};

/** \class SkPDFSharedStream

    This class takes an asset and assumes that it is backed by
    long-lived shared data (for example, an open file
    descriptor). That is: no memory savings can be made by holding on
    to a compressed version instead.
 */
class SkPDFSharedStream final : public SkPDFObject {
public:
    // Takes ownership of asset.
    SkPDFSharedStream(SkStreamAsset* data) : fAsset(data), fDict(new SkPDFDict) { SkASSERT(data); }
    SkPDFDict* dict() { return fDict; }
    void emitObject(SkWStream*,
                    const SkPDFObjNumMap&,
                    const SkPDFSubstituteMap&) const override;
    void addResources(SkPDFObjNumMap*,
                      const SkPDFSubstituteMap&) const override;

private:
    SkAutoTDelete<SkStreamAsset> fAsset;
    SkAutoTUnref<SkPDFDict> fDict;
    typedef SkPDFObject INHERITED;
};

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFObjNumMap

    The PDF Object Number Map manages object numbers.  It is used to
    create the PDF cross reference table.
*/
class SkPDFObjNumMap : SkNoncopyable {
public:
    /** Add the passed object to the catalog.
     *  @param obj         The object to add.
     *  @return True iff the object was not already added to the catalog.
     */
    bool addObject(SkPDFObject* obj);

    /** Add the passed object to the catalog, as well as all its dependencies.
     *  @param obj   The object to add.  If nullptr, this is a noop.
     *  @param subs  Will be passed to obj->addResources().
     */
    void addObjectRecursively(SkPDFObject* obj, const SkPDFSubstituteMap& subs);

    /** Get the object number for the passed object.
     *  @param obj         The object of interest.
     */
    int32_t getObjectNumber(SkPDFObject* obj) const;

    const SkTDArray<SkPDFObject*>& objects() const { return fObjects; }

private:
    SkTDArray<SkPDFObject*> fObjects;
    SkTHashMap<SkPDFObject*, int32_t> fObjectNumbers;
};

////////////////////////////////////////////////////////////////////////////////

/** \class SkPDFSubstituteMap

    The PDF Substitute Map manages substitute objects and owns the
    substitutes.
*/
class SkPDFSubstituteMap : SkNoncopyable {
public:
    ~SkPDFSubstituteMap();
    /** Set substitute object for the passed object.
        Refs substitute.
     */
    void setSubstitute(SkPDFObject* original, SkPDFObject* substitute);

    /** Find and return any substitute object set for the passed object. If
     *  there is none, return the passed object.
     */
    SkPDFObject* getSubstitute(SkPDFObject* object) const;

    SkPDFObject* operator()(SkPDFObject* o) const {
        return this->getSubstitute(o);
    }

private:
    SkTHashMap<SkPDFObject*, SkPDFObject*> fSubstituteMap;
};

#ifdef SK_PDF_IMAGE_STATS
extern SkAtomic<int> gDrawImageCalls;
extern SkAtomic<int> gJpegImageObjects;
extern SkAtomic<int> gRegularImageObjects;
extern void SkPDFImageDumpStats();
#endif // SK_PDF_IMAGE_STATS

#endif
