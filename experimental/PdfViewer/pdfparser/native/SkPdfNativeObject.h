/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPdfNativeObject_DEFINED
#define SkPdfNativeObject_DEFINED

#include <stdint.h>
#include <string.h>

#include "SkMatrix.h"
#include "SkPdfConfig.h"
#include "SkPdfNativeTokenizer.h"
#include "SkPdfNYI.h"
#include "SkPdfUtils.h"
#include "SkRect.h"
#include "SkString.h"
#include "SkTDArray.h"
#include "SkTDict.h"

class SkPdfDictionary;
class SkPdfStream;
class SkPdfAllocator;

// TODO(edisonn): remove these constants and clean up the code.
#define kFilteredStreamBit 0
#define kUnfilteredStreamBit 1
#define kOwnedStreamBit 2

/** \class SkPdfNativeObject
 *
 *  The SkPdfNativeObject class is used to store a pdf object. Classes that inherit it are not
 *  allowed to add fields.
 *
 *  SkPdfAllocator will allocate them in chunks and will free them in destructor.
 *
 *  You can allocate one on the stack, as long as you call reset() at the end, and any objects it
 *  points to in an allocator. But if your object is a simple one, like number, then
 *  putting it on stack will be just fine.
 *
 */
class SkPdfNativeObject {
 public:
     enum ObjectType {
         // The type will have only one of these values, but for error reporting, we make it an enum
         // so it can easily report that something was expected to be one of a few types
         kInvalid_PdfObjectType = 1 << 1,

         kBoolean_PdfObjectType = 1 << 2,
         kInteger_PdfObjectType = 1 << 3,
         kReal_PdfObjectType = 1 << 4,
         _kNumber_PdfObjectType = kInteger_PdfObjectType | kReal_PdfObjectType,
         kString_PdfObjectType = 1 << 5,
         kHexString_PdfObjectType = 1 << 6,
         _kAnyString_PdfObjectType = kString_PdfObjectType | kHexString_PdfObjectType,
         kName_PdfObjectType = 1 << 7,
         kKeyword_PdfObjectType = 1 << 8,
         _kStream_PdfObjectType = 1 << 9,  //  attached to a Dictionary, do not use
         kArray_PdfObjectType = 1 << 10,
         kDictionary_PdfObjectType = 1 << 11,
         kNull_PdfObjectType = 1 << 12,

         kReference_PdfObjectType = 1 << 13,

         kUndefined_PdfObjectType = 1 << 14,  // per 1.4 spec, if the same key appear twice in the
                                              // dictionary, the value is undefined.

         _kObject_PdfObjectType = -1,
     };

     enum DataType {
         kEmpty_Data,
         kFont_Data,
         kBitmap_Data,
     };

private:
    // TODO(edisonn): assert reset operations while in rendering! The objects should be reset
    // only when rendering is completed.
    uint32_t fInRendering : 1;
    uint32_t fUnused : 31;

    struct Reference {
        unsigned int fId;
        unsigned int fGen;
    };

    ObjectType fObjectType;

    union {
        bool fBooleanValue;
        int64_t fIntegerValue;
        // TODO(edisonn): double, float, SkScalar?
        double fRealValue;
        NotOwnedString fStr;

        SkTDArray<SkPdfNativeObject*>* fArray;
        Reference fRef;
    };
    SkTDict<SkPdfNativeObject*>* fMap;

    // TODO(edisonn): rename data with cache
    void* fData;
    DataType fDataType;

#ifdef PDF_TRACK_OBJECT_USAGE
    // Records if the object was used during rendering/proccessing. It can be used to track
    // what features are only partially implemented, by looking at what objects have not been
    // accessed.
    mutable bool fUsed;
#endif   // PDF_TRACK_OBJECT_USAGE

#ifdef PDF_TRACK_STREAM_OFFSETS
public:
    // TODO(edisonn): replace them with char* start, end - and a mechanism to register streams.
    int fStreamId;
    int fOffsetStart;
    int fOffsetEnd;
#endif  // PDF_TRACK_STREAM_OFFSETS

public:

#ifdef PDF_TRACK_STREAM_OFFSETS
    // TODO(edisonn): remove these ones.
    int streamId() const { return fStreamId; }
    int offsetStart() const { return fOffsetStart; }
    int offsetEnd() const { return fOffsetEnd; }
#endif  // PDF_TRACK_STREAM_OFFSETS

    SkPdfNativeObject() : fInRendering(0)
                        , fObjectType(kInvalid_PdfObjectType)
                        , fMap(NULL)
                        , fData(NULL)
                        , fDataType(kEmpty_Data)
#ifdef PDF_TRACK_OBJECT_USAGE
                        , fUsed(false)
#endif   // PDF_TRACK_OBJECT_USAGE

#ifdef PDF_TRACK_STREAM_OFFSETS
                        , fStreamId(-1)
                        , fOffsetStart(-1)
                        , fOffsetEnd(-1)
#endif  // PDF_TRACK_STREAM_OFFSETS
    {}

    // Used to verify if a form is used in rendering, to check for infinite loops.
    bool inRendering() const { return fInRendering != 0; }
    void startRendering() {fInRendering = 1;}
    void doneRendering() {fInRendering = 0;}

    // Each object can cache one entry associated with it.
    // for example a SkPdfImage could cache an SkBitmap, of a SkPdfFont, could cache a SkTypeface.
    inline bool hasData(DataType type) {
        return type == fDataType;
    }

    // returns the cached value
    inline void* data(DataType type) {
        return type == fDataType ? fData : NULL;
    }

    // Stores something in the cache
    inline void setData(void* data, DataType type) {
        releaseData();
        fDataType = type;
        fData = data;
    }

    // destroys the cache
    void releaseData();

    // TODO(edisonn): add an assert that reset was called
//    ~SkPdfNativeObject() {
//        //reset();  must be called manually! Normally, will be called by allocator destructor.
//    }

    // Resets a pdf object, deleting all resources directly referenced.
    // It will not reset/delete indirect resources.
    // (e.g. it deletes only the array holding pointers to objects, but does not del the objects)
    void reset() {
        SkPdfMarkObjectUnused();

        switch (fObjectType) {
            case kArray_PdfObjectType:
                delete fArray;
                break;

            case kDictionary_PdfObjectType:
                delete fMap;
                if (isStreamOwned()) {
                    delete[] fStr.fBuffer;
                    fStr.fBuffer = NULL;
                    fStr.fBytes = 0;
                }
                break;

            default:
                break;
        }
        fObjectType = kInvalid_PdfObjectType;
        releaseData();
    }

    // returns the object type (Null, Integer, String, Dictionary, ... )
    // It does not specify what type of dictionary we have.
    ObjectType type() {
        SkPdfMarkObjectUsed();

        return fObjectType;
    }

    // Gives quick access to the buffer's address of a string/keyword/name
    const char* c_str() const {
        SkPdfMarkObjectUsed();

        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
            case kName_PdfObjectType:
                return (const char*)fStr.fBuffer;

            default:
                // TODO(edisonn): report/warning/assert?
                return NULL;
        }
    }

    // Gives quick access to the length of a string/keyword/name
    size_t lenstr() const {
        SkPdfMarkObjectUsed();

        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
            case kName_PdfObjectType:
                return fStr.fBytes;

            default:
                // TODO(edisonn): report/warning/assert?
                return 0;
        }
    }


    // TODO(edisonn): NYI
    SkPdfDate& dateValue() const {
        static SkPdfDate nyi;
        return nyi;
    }

    // TODO(edisonn): NYI
    SkPdfFunction& functionValue() const {
        static SkPdfFunction nyi;
        return nyi;
    }

    // TODO(edisonn): NYI
    SkPdfFileSpec& fileSpecValue() const {
        static SkPdfFileSpec nyi;
        return nyi;
    }

    // TODO(edisonn): NYI
    SkPdfTree& treeValue() const {
        static SkPdfTree nyi;
        return nyi;
    }

    // Creates a Boolean object. Assumes and asserts that it was never initialized.
    static void makeBoolean(bool value, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kBoolean_PdfObjectType;
        obj->fBooleanValue = value;
    }

    static SkPdfNativeObject makeBoolean(bool value) {
        SkPdfNativeObject obj;

        obj.fObjectType = kBoolean_PdfObjectType;
        obj.fBooleanValue = value;
        return obj;
    }

    // Creates an Integer object. Assumes and asserts that it was never initialized.
    static void makeInteger(int64_t value, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kInteger_PdfObjectType;
        obj->fIntegerValue = value;
    }

    // Creates a Real object. Assumes and asserts that it was never initialized.
    static void makeReal(double value, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReal_PdfObjectType;
        obj->fRealValue = value;
    }

    // Creates a Null object. Assumes and asserts that it was never initialized.
    static void makeNull(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kNull_PdfObjectType;
    }

    static SkPdfNativeObject makeNull() {
        SkPdfNativeObject obj;

        obj.fObjectType = kNull_PdfObjectType;
        return obj;
    }

    // TODO(edisonn): this might not woirk well in Chrome
    static SkPdfNativeObject kNull;

    // Creates a Numeric object from a string. Assumes and asserts that it was never initialized.
    static void makeNumeric(const unsigned char* start, const unsigned char* end,
                            SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        // TODO(edisonn): NYI properly
        // if has dot (impl), or exceeds max int, is real, otherwise is int
        bool isInt = true;
        for (const unsigned char* current = start; current < end; current++) {
            if (*current == '.') {
                isInt = false;
                break;
            }
            // TODO(edisonn): report parse issue with numbers like "24asdasd123"
        }
        if (isInt) {
            makeInteger(atol((const char*)start), obj);
        } else {
            makeReal(atof((const char*)start), obj);
        }
    }

    // Creates a Reference object. Assumes and asserts that it was never initialized.
    static void makeReference(unsigned int id, unsigned int gen, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReference_PdfObjectType;
        obj->fRef.fId = id;
        obj->fRef.fGen = gen;
    }

    // Creates a Reference object. Resets the object before use.
    static void resetAndMakeReference(unsigned int id, unsigned int gen, SkPdfNativeObject* obj) {
        obj->reset();
        makeReference(id, gen, obj);
    }

    // Creates a String object. Assumes and asserts that it was never initialized.
    static void makeString(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kString_PdfObjectType);
    }

    // Creates a String object. Assumes and asserts that it was never initialized.
    static void makeString(const unsigned char* start, const unsigned char* end,
                           SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kString_PdfObjectType);
    }

    // Creates a String object. Assumes and asserts that it was never initialized.
    static void makeString(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kString_PdfObjectType);
    }

    // Creates a HexString object. Assumes and asserts that it was never initialized.
    static void makeHexString(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kHexString_PdfObjectType);
    }

    // Creates a HexString object. Assumes and asserts that it was never initialized.
    static void makeHexString(const unsigned char* start, const unsigned char* end,
                              SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kHexString_PdfObjectType);
    }

    // Creates a HexString object. Assumes and asserts that it was never initialized.
    static void makeHexString(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kHexString_PdfObjectType);
    }

    // Creates a Name object. Assumes and asserts that it was never initialized.
    static void makeName(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kName_PdfObjectType);
    }

    // Creates a Name object. Assumes and asserts that it was never initialized.
    static void makeName(const unsigned char* start, const unsigned char* end,
                         SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kName_PdfObjectType);
    }

    // Creates a Name object. Assumes and asserts that it was never initialized.
    static void makeName(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kName_PdfObjectType);
    }

    // Creates a Keyword object. Assumes and asserts that it was never initialized.
    static void makeKeyword(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kKeyword_PdfObjectType);
    }

    // Creates a Keyword object. Assumes and asserts that it was never initialized.
    static void makeKeyword(const unsigned char* start, const unsigned char* end,
                            SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kKeyword_PdfObjectType);
    }

    // Creates a Keyword object. Assumes and asserts that it was never initialized.
    static void makeKeyword(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kKeyword_PdfObjectType);
    }

    // Creates an empty Array object. Assumes and asserts that it was never initialized.
    static void makeEmptyArray(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kArray_PdfObjectType;
        obj->fArray = new SkTDArray<SkPdfNativeObject*>();
    }

    // Appends an object into the array. Assumes <this> is an array.
    bool appendInArray(SkPdfNativeObject* obj) {
        SkASSERT(fObjectType == kArray_PdfObjectType);
        if (fObjectType != kArray_PdfObjectType) {
            // TODO(edisonn): report/warning/assert?
            return false;
        }

        fArray->push(obj);
        return true;
    }

    // Returns the size of an array.
    size_t size() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        return fArray->count();
    }

    // Returns one object of an array, by index.
    SkPdfNativeObject* objAtAIndex(int i) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    // Returns one object of an array, by index.
    const SkPdfNativeObject* objAtAIndex(int i) const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    // Returns one object of an array, by index.
    SkPdfNativeObject* operator[](int i) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    const SkPdfNativeObject* operator[](int i) const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    // Removes the last object in the array.
    SkPdfNativeObject* removeLastInArray() {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kArray_PdfObjectType);

        SkPdfNativeObject* ret = NULL;
        fArray->pop(&ret);

        return ret;
    }

    // Creates an empty Dictionary object. Assumes and asserts that it was never initialized.
    static void makeEmptyDictionary(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kDictionary_PdfObjectType;
        obj->fMap = new SkTDict<SkPdfNativeObject*>(1);
        obj->fStr.fBuffer = NULL;
        obj->fStr.fBytes = 0;
    }

    // TODO(edisonn): perf: get all the possible names from spec, and compute a hash function
    // that would create no overlaps in the same dictionary
    // or build a tree of chars that when followed goes to a unique id/index/hash
    // TODO(edisonn): generate constants like kDictFoo, kNameDict_name
    // which will be used in code
    // add function SkPdfFastNameKey key(const char* key);
    // TODO(edisonn): setting the same key twice, will make the value undefined!

    // this[key] = value;
    bool set(const SkPdfNativeObject* key, SkPdfNativeObject* value) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert?
            return false;
        }

        return set(key->fStr.fBuffer, key->fStr.fBytes, value);
    }

    // this[key] = value;
    bool set(const char* key, SkPdfNativeObject* value) {
        SkPdfMarkObjectUsed();

        return set((const unsigned char*)key, strlen(key), value);
    }

    // this[key] = value;
    bool set(const unsigned char* key, size_t len, SkPdfNativeObject* value) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);

        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return false;
        }

        return fMap->set((const char*)key, len, value);
    }

    // Returns an object from a Dictionary, identified by it's name.
    SkPdfNativeObject* get(const SkPdfNativeObject* key) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return NULL;
        }

        return get(key->fStr.fBuffer, key->fStr.fBytes);
    }

    // Returns an object from a Dictionary, identified by it's name.
    SkPdfNativeObject* get(const char* key) {
        SkPdfMarkObjectUsed();

        return get((const unsigned char*)key, strlen(key));
    }

    // Returns an object from a Dictionary, identified by it's name.
    SkPdfNativeObject* get(const unsigned char* key, size_t len) {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return NULL;
        }
        SkPdfNativeObject* ret = NULL;
        fMap->find((const char*)key, len, &ret);

#ifdef PDF_TRACE
        SkString _key;
        _key.append((const char*)key, len);
        printf("\nget(/%s) = %s\n", _key.c_str(),
               ret ? ret->toString(0, len + 9).c_str() : "_NOT_FOUND");
#endif

        return ret;
    }

    // Returns an object from a Dictionary, identified by it's name.
    const SkPdfNativeObject* get(const SkPdfNativeObject* key) const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return NULL;
        }

        return get(key->fStr.fBuffer, key->fStr.fBytes);
    }

    // Returns an object from a Dictionary, identified by it's name.
    const SkPdfNativeObject* get(const char* key) const {
        SkPdfMarkObjectUsed();

        return get((const unsigned char*)key, strlen(key));
    }

    // Returns an object from a Dictionary, identified by it's name.
    const SkPdfNativeObject* get(const unsigned char* key, size_t len) const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return NULL;
        }
        SkPdfNativeObject* ret = NULL;
        fMap->find((const char*)key, len, &ret);

#ifdef PDF_TRACE
        SkString _key;
        _key.append((const char*)key, len);
        printf("\nget(/%s) = %s\n", _key.c_str(),
               ret ? ret->toString(0, len + 9).c_str() : "_NOT_FOUND");
#endif

        return ret;
    }

    // Returns an object from a Dictionary, identified by it's name.
    const SkPdfNativeObject* get(const char* key, const char* abr) const {
        SkPdfMarkObjectUsed();

        const SkPdfNativeObject* ret = get(key);
        // TODO(edisonn): remove  || *abr == '\0' and pass NULL in the _autogen files instead.
        if (ret != NULL || abr == NULL || *abr == '\0') {
            return ret;
        }
        return get(abr);
    }

    // Returns an object from a Dictionary, identified by it's name.
    SkPdfNativeObject* get(const char* key, const char* abr) {
        SkPdfMarkObjectUsed();

        SkPdfNativeObject* ret = get(key);
        // TODO(edisonn): remove  || *abr == '\0' and pass NULL in the _autogen files instead.
        if (ret != NULL || abr == NULL || *abr == '\0') {
            return ret;
        }
        return get(abr);
    }

    // Casts the object to a Dictionary. Asserts if the object is not a Dictionary.
    SkPdfDictionary* asDictionary() {
        SkPdfMarkObjectUsed();

        SkASSERT(isDictionary());
        if (!isDictionary()) {
            return NULL;
        }
        return (SkPdfDictionary*) this;
    }

    // Casts the object to a Dictionary. Asserts if the object is not a Dictionary.
    const SkPdfDictionary* asDictionary() const {
        SkPdfMarkObjectUsed();

        SkASSERT(isDictionary());
        if (!isDictionary()) {
            return NULL;
        }
        return (SkPdfDictionary*) this;
    }


    // Returns true if the object is a Reference.
    bool isReference() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kReference_PdfObjectType;
    }

    // Returns true if the object is a Boolean.
    bool isBoolean() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kBoolean_PdfObjectType;
    }

    // Returns true if the object is an Integer.
    bool isInteger() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kInteger_PdfObjectType;
    }

private:
    // Returns true if the object is a Real number.
    bool isReal() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kReal_PdfObjectType;
    }

public:
    // Returns true if the object is a Number (either Integer or Real).
    bool isNumber() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kInteger_PdfObjectType || fObjectType == kReal_PdfObjectType;
    }

    // Returns true if the object is a R keyword (used to identify references, e.g. "10 3 R".
    bool isKeywordReference() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kKeyword_PdfObjectType && fStr.fBytes == 1 && fStr.fBuffer[0] == 'R';
    }

    // Returns true if the object is a Keyword.
    bool isKeyword() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kKeyword_PdfObjectType;
    }

    // Returns true if the object is a given Keyword.
    bool isKeyword(const char* keyword) const {
        SkPdfMarkObjectUsed();

        if (!isKeyword()) {
            return false;
        }

        if (strlen(keyword) != fStr.fBytes) {
            return false;
        }

        if (strncmp(keyword, (const char*)fStr.fBuffer, fStr.fBytes) != 0) {
            return false;
        }

        return true;
    }

    // Returns true if the object is a Name.
    bool isName() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kName_PdfObjectType;
    }

    // Returns true if the object is a given Name.
    bool isName(const char* name) const {
        SkPdfMarkObjectUsed();

        return fObjectType == kName_PdfObjectType &&
                fStr.fBytes == strlen(name) &&
                strncmp((const char*)fStr.fBuffer, name, fStr.fBytes) == 0;
    }

    // Returns true if the object is an Array.
    bool isArray() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kArray_PdfObjectType;
    }

    // Returns true if the object is a Date.
    // TODO(edisonn): NYI
    bool isDate() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType;
    }

    // Returns true if the object is a Dictionary.
    bool isDictionary() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kDictionary_PdfObjectType;
    }

    // Returns true if the object is a Date.
    // TODO(edisonn): NYI
    bool isFunction() const {
        SkPdfMarkObjectUsed();

        return false;  // NYI
    }

    // Returns true if the object is a Rectangle.
    bool isRectangle() const {
        SkPdfMarkObjectUsed();

        // TODO(edisonn): add also that each of these 4 objects are numbers.
        return fObjectType == kArray_PdfObjectType && fArray->count() == 4;
    }

    // TODO(edisonn): Design: decide if we should use hasStream or isStream
    // Returns true if the object has a stream associated with it.
    bool hasStream() const {
        SkPdfMarkObjectUsed();

        return isDictionary() && fStr.fBuffer != NULL;
    }

    // Returns the stream associated with the dictionary. As of now, it casts this to Stream.
    const SkPdfStream* getStream() const {
        SkPdfMarkObjectUsed();

        return hasStream() ? (const SkPdfStream*)this : NULL;
    }

    // Returns the stream associated with the dictionary. As of now, it casts this to Stream.
    SkPdfStream* getStream() {
        SkPdfMarkObjectUsed();

        return hasStream() ? (SkPdfStream*)this : NULL;
    }

    // Returns true if the object is a String or HexString.
    bool isAnyString() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType;
    }

    // Returns true if the object is a HexString.
    bool isHexString() const {
        SkPdfMarkObjectUsed();

        return fObjectType == kHexString_PdfObjectType;
    }

    // Returns true if the object is a Matrix.
    bool isMatrix() const {
        SkPdfMarkObjectUsed();

        // TODO(edisonn): add also that each of these 6 objects are numbers.
        return fObjectType == kArray_PdfObjectType && fArray->count() == 6;
    }

    // Returns the int value stored in the object. Assert if the object is not an Integer.
    inline int64_t intValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kInteger_PdfObjectType);

        if (fObjectType != kInteger_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return 0;
        }
        return fIntegerValue;
    }

private:
    // Returns the real value stored in the object. Assert if the object is not a Real.
    inline double realValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kReal_PdfObjectType);

        if (fObjectType != kReal_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return 0;
        }
        return fRealValue;
    }

public:
    // Returns the numeric value stored in the object. Assert if the object is not a Real
    // or an Integer.
    inline double numberValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(isNumber());

        if (!isNumber()) {
            // TODO(edisonn): report/warn/assert.
            return 0;
        }
        return fObjectType == kReal_PdfObjectType ? fRealValue : fIntegerValue;
    }

    // Returns the numeric value stored in the object as a scalar. Assert if the object is not
    // a Realor an Integer.
    inline SkScalar scalarValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(isNumber());

        if (!isNumber()) {
            // TODO(edisonn): report/warn/assert.
            return SkIntToScalar(0);
        }
        return fObjectType == kReal_PdfObjectType ? SkDoubleToScalar(fRealValue) :
                                                    SkIntToScalar(fIntegerValue);
    }

    // Returns the id of the referenced object. Assert if the object is not a Reference.
    int referenceId() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kReference_PdfObjectType);
        return fRef.fId;
    }

    // Returns the generation of the referenced object. Assert if the object is not a Reference.
    int referenceGeneration() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kReference_PdfObjectType);
        return fRef.fGen;
    }

    // Returns the buffer of a Name object. Assert if the object is not a Name.
    inline const char* nameValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kName_PdfObjectType);

        if (fObjectType != kName_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    // Returns the buffer of a (Hex)String object. Assert if the object is not a (Hex)String.
    inline const char* stringValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType);

        if (fObjectType != kString_PdfObjectType && fObjectType != kHexString_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    // Returns the storage of any type that can hold a form of string.
    inline NotOwnedString strRef() {
        SkPdfMarkObjectUsed();

        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
            case kName_PdfObjectType:
                return fStr;

            default:
                // TODO(edisonn): report/warning
                return NotOwnedString();
        }
    }

    // TODO(edisonn): nameValue2 and stringValue2 are used to make code generation easy,
    // but it is not a performat way to do it, since it will create an extra copy
    // remove these functions and make code generated faster
    inline SkString nameValue2() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kName_PdfObjectType);

        if (fObjectType != kName_PdfObjectType) {
            // TODO(edisonn): log err
            return SkString();
        }
        return SkString((const char*)fStr.fBuffer, fStr.fBytes);
    }

    // Returns an SkString with the value of the (Hex)String object.
    inline SkString stringValue2() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType);

        if (fObjectType != kString_PdfObjectType && fObjectType != kHexString_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return SkString();
        }
        return SkString((const char*)fStr.fBuffer, fStr.fBytes);
    }

    // Returns the boolean of the Bool object. Assert if the object is not a Bool.
    inline bool boolValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(fObjectType == kBoolean_PdfObjectType);

        if (fObjectType != kBoolean_PdfObjectType) {
            // TODO(edisonn): report/warn/assert.
            return false;
        }
        return fBooleanValue;
    }

    // Returns the rectangle of the Rectangle object. Assert if the object is not a Rectangle.
    SkRect rectangleValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(isRectangle());
        if (!isRectangle()) {
            return SkRect::MakeEmpty();
        }

        double array[4];
        for (int i = 0; i < 4; i++) {
            // TODO(edisonn): version where we could resolve references?
            const SkPdfNativeObject* elem = objAtAIndex(i);
            if (elem == NULL || !elem->isNumber()) {
                // TODO(edisonn): report/warn/assert.
                return SkRect::MakeEmpty();
            }
            array[i] = elem->numberValue();
        }

        return SkRect::MakeLTRB(SkDoubleToScalar(array[0]),
                                SkDoubleToScalar(array[1]),
                                SkDoubleToScalar(array[2]),
                                SkDoubleToScalar(array[3]));
    }

    // Returns the matrix of the Matrix object. Assert if the object is not a Matrix.
    SkMatrix matrixValue() const {
        SkPdfMarkObjectUsed();

        SkASSERT(isMatrix());
        if (!isMatrix()) {
            return SkMatrix::I();
        }

        double array[6];
        for (int i = 0; i < 6; i++) {
            // TODO(edisonn): version where we could resolve references?
            const SkPdfNativeObject* elem = objAtAIndex(i);
            if (elem == NULL || !elem->isNumber()) {
                // TODO(edisonn): report/warn/assert.
                return SkMatrix::I();
            }
            array[i] = elem->numberValue();
        }

        return SkMatrixFromPdfMatrix(array);
    }

    // Runs all the filters of this stream, except the last one, if it is a DCT.
    // Returns false on failure.
    bool filterStream();

    // Runs all the filters of this stream, except the last one, if it is a DCT, a gives back
    // the buffer and the length. The object continues to own the buffer.
    // Returns false on failure.
    bool GetFilteredStreamRef(unsigned char const** buffer, size_t* len) {
        SkPdfMarkObjectUsed();

        // TODO(edisonn): add params that could let the last filter in place
        // if it is jpeg or png to fast load images.
        if (!hasStream()) {
            return false;
        }

        filterStream();

        if (buffer) {
            *buffer = fStr.fBuffer;
        }

        if (len) {
            *len = fStr.fBytes >> 2;  // last 2 bits - TODO(edisonn): clean up.
        }

        return true;
    }

    // Returns true if the stream is already filtered.
    bool isStreamFiltered() const {
        SkPdfMarkObjectUsed();

        return hasStream() && ((fStr.fBytes & 1) == kFilteredStreamBit);
    }

    // Returns true if this object own the buffer, or false if an Allocator own it.
    bool isStreamOwned() const {
        SkPdfMarkObjectUsed();

        return hasStream() && ((fStr.fBytes & 2) == kOwnedStreamBit);
    }

    // Gives back the original buffer and the length. The object continues to own the buffer.
    // Returns false if the stream is already filtered.
    bool GetUnfilteredStreamRef(unsigned char const** buffer, size_t* len) const {
        SkPdfMarkObjectUsed();

        if (isStreamFiltered()) {
            return false;
        }

        if (!hasStream()) {
            return false;
        }

        if (buffer) {
            *buffer = fStr.fBuffer;
        }

        if (len) {
            *len = fStr.fBytes >> 2;  // remove last 2 bits - TODO(edisonn): clean up.
        }

        return true;
    }

    // Add a stream to this Dictionarry. Asserts we do not have yet a stream.
    bool addStream(const unsigned char* buffer, size_t len) {
        SkPdfMarkObjectUsed();

        SkASSERT(!hasStream());
        SkASSERT(isDictionary());

        if (!isDictionary() || hasStream()) {
            return false;
        }

        fStr.fBuffer = buffer;
        fStr.fBytes = (len << 2) + kUnfilteredStreamBit;

        return true;
    }

    static void appendSpaces(SkString* str, int level) {
        for (int i = 0 ; i < level; i++) {
            str->append(" ");
        }
    }

    static void append(SkString* str, const char* data, size_t len, const char* prefix = "\\x") {
        for (unsigned int i = 0 ; i < len; i++) {
            if (data[i] == kNUL_PdfWhiteSpace) {
                str->append(prefix);
                str->append("00");
            } else if (data[i] == kHT_PdfWhiteSpace) {
                str->append(prefix);
                str->append("09");
            } else if (data[i] == kLF_PdfWhiteSpace) {
                str->append(prefix);
                str->append("0A");
            } else if (data[i] == kFF_PdfWhiteSpace) {
                str->append(prefix);
                str->append("0C");
            } else if (data[i] == kCR_PdfWhiteSpace) {
                str->append(prefix);
                str->append("0D");
            } else {
                str->append(data + i, 1);
            }
        }
    }

    // Returns the string representation of the object value.
    SkString toString(int firstRowLevel = 0, int level = 0) {
        SkString str;
        appendSpaces(&str, firstRowLevel);
        switch (fObjectType) {
            case kInvalid_PdfObjectType:
                str.append("__Invalid");
                break;

            case kBoolean_PdfObjectType:
                str.appendf("%s", fBooleanValue ? "true" : "false");
                break;

            case kInteger_PdfObjectType:
                str.appendf("%i", (int)fIntegerValue);
                break;

            case kReal_PdfObjectType:
                str.appendf("%f", fRealValue);
                break;

            case kString_PdfObjectType:
                str.append("\"");
                append(&str, (const char*)fStr.fBuffer, fStr.fBytes);
                str.append("\"");
                break;

            case kHexString_PdfObjectType:
                str.append("<");
                for (unsigned int i = 0 ; i < fStr.fBytes; i++) {
                    str.appendf("%02x", (unsigned int)fStr.fBuffer[i]);
                }
                str.append(">");
                break;

            case kName_PdfObjectType:
                str.append("/");
                append(&str, (const char*)fStr.fBuffer, fStr.fBytes, "#");
                break;

            case kKeyword_PdfObjectType:
                append(&str, (const char*)fStr.fBuffer, fStr.fBytes);
                break;

            case kArray_PdfObjectType:
                str.append("[\n");
                for (unsigned int i = 0; i < size(); i++) {
                    str.append(objAtAIndex(i)->toString(level + 1, level + 1));
                    if (i < size() - 1) {
                        str.append(",");
                    }
                    str.append("\n");
                }
                appendSpaces(&str, level);
                str.append("]");
                break;

            case kDictionary_PdfObjectType: {
                    SkTDict<SkPdfNativeObject*>::Iter iter(*fMap);
                    SkPdfNativeObject* obj = NULL;
                    const char* key = NULL;
                    str.append("<<\n");
                    while ((key = iter.next(&obj)) != NULL) {
                        appendSpaces(&str, level + 2);
                        str.appendf("/%s %s\n", key,
                                    obj->toString(0, level + (int) strlen(key) + 4).c_str());
                    }
                    appendSpaces(&str, level);
                    str.append(">>");
                    if (hasStream()) {
                        const unsigned char* stream = NULL;
                        size_t length = 0;
                        if (GetFilteredStreamRef(&stream, &length)) {
                            str.append("stream\n");
                            append(&str, (const char*)stream, length > 256 ? 256 : length);
                            str.append("\nendstream");
                        } else {
                            str.append("stream STREAM_ERROR endstream");
                        }
                    }
                }
                break;

            case kNull_PdfObjectType:
                str = "NULL";
                break;

            case kReference_PdfObjectType:
                str.appendf("%i %i R", fRef.fId, fRef.fGen);
                break;

            case kUndefined_PdfObjectType:
                str = "Undefined";
                break;

            default:
                str = "Error";
                break;
        }

        return str;
    }

private:
    static void makeStringCore(const unsigned char* start, SkPdfNativeObject* obj,
                               ObjectType type) {
        makeStringCore(start, strlen((const char*)start), obj, type);
    }

    static void makeStringCore(const unsigned char* start, const unsigned char* end,
                               SkPdfNativeObject* obj, ObjectType type) {
        makeStringCore(start, end - start, obj, type);
    }

    static void makeStringCore(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj,
                               ObjectType type) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = type;
        obj->fStr.fBuffer = start;
        obj->fStr.fBytes = bytes;
    }

    bool applyFilter(const char* name);
    bool applyFlateDecodeFilter();
    bool applyDCTDecodeFilter();
};

// These classes are provided for convenience. You still have to make sure an SkPdfInteger
// is indeed an Integer.
class SkPdfStream : public SkPdfNativeObject {};
class SkPdfArray : public SkPdfNativeObject {};
class SkPdfString : public SkPdfNativeObject {};
class SkPdfHexString : public SkPdfNativeObject {};
class SkPdfInteger : public SkPdfNativeObject {};
class SkPdfReal : public SkPdfNativeObject {};
class SkPdfNumber : public SkPdfNativeObject {};

class SkPdfName : public SkPdfNativeObject {
    SkPdfName() : SkPdfNativeObject() {
        SkPdfNativeObject::makeName((const unsigned char*)"", this);
    }
public:
    SkPdfName(char* name) : SkPdfNativeObject() {
        this->makeName((const unsigned char*)name, this);
    }
};

#endif  // SkPdfNativeObject
