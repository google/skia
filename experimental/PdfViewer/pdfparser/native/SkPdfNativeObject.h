#ifndef EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFOBJECT_H_
#define EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFOBJECT_H_

#include <stdint.h>
#include <string.h>
#include <string>
#include "SkTDArray.h"
#include "SkTDict.h"
#include "SkRect.h"
#include "SkMatrix.h"
#include "SkString.h"

#include "SkPdfNYI.h"
#include "SkPdfConfig.h"
#include "SkPdfUtils.h"

#include "SkPdfNativeTokenizer.h"

class SkPdfDictionary;
class SkPdfStream;
class SkPdfAllocator;

// TODO(edisonn): macro it and move it to utils
SkMatrix SkMatrixFromPdfMatrix(double array[6]);


#define kFilteredStreamBit 0
#define kUnfilteredStreamBit 1
#define kOwnedStreamBit 2

class SkPdfNativeObject {
 public:
     enum ObjectType {
         kInvalid_PdfObjectType,

         kBoolean_PdfObjectType,
         kInteger_PdfObjectType,
         kReal_PdfObjectType,
         kString_PdfObjectType,
         kHexString_PdfObjectType,
         kName_PdfObjectType,
         kKeyword_PdfObjectType,
         //kStream_PdfObjectType,  //  attached to a Dictionary
         kArray_PdfObjectType,
         kDictionary_PdfObjectType,
         kNull_PdfObjectType,

         // TODO(edisonn): after the pdf has been loaded completely, resolve all references
         // try the same thing with delayed loaded ...
         kReference_PdfObjectType,

         kUndefined_PdfObjectType,  // per 1.4 spec, if the same key appear twice in the dictionary, the value is undefined
     };

     enum DataType {
         kEmpty_Data,
         kFont_Data,
         kBitmap_Data,
     };

private:
    struct Reference {
        unsigned int fId;
        unsigned int fGen;
    };

    // TODO(edisonn): add stream start, stream end, where stream is weither the file
    // or decoded/filtered pdf stream

    // TODO(edisonn): add warning/report per object
    // TODO(edisonn): add flag fUsed, to be used once the parsing is complete,
    // so we could show what parts have been proccessed, ignored, or generated errors

    ObjectType fObjectType;

    union {
        bool fBooleanValue;
        int64_t fIntegerValue;
        // TODO(edisonn): double, float? typedefed
        double fRealValue;
        NotOwnedString fStr;

        // TODO(edisonn): make sure the foorprint of fArray and fMap is small, otherwise, use pointers, or classes with up to 8 bytes in footprint
        SkTDArray<SkPdfNativeObject*>* fArray;
        Reference fRef;
    };
    SkTDict<SkPdfNativeObject*>* fMap;

    // TODO(edisonn): rename data with cache
    void* fData;
    DataType fDataType;


public:

    SkPdfNativeObject() : fObjectType(kInvalid_PdfObjectType), fMap(NULL), fData(NULL), fDataType(kEmpty_Data) {}


    inline bool hasData(DataType type) {
        return type == fDataType;
    }

    inline void* data(DataType type) {
        return type == fDataType ? fData : NULL;
    }

    inline void setData(void* data, DataType type) {
        releaseData();
        fDataType = type;
        fData = data;
    }

    void releaseData();

//    ~SkPdfNativeObject() {
//        //reset();  must be called manually!
//    }

    void reset() {
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

    ObjectType type() { return fObjectType; }

    const char* c_str() const {
        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
            case kName_PdfObjectType:
                return (const char*)fStr.fBuffer;

            default:
                // TODO(edisonn): report/warning
                return NULL;
        }
    }

    size_t lenstr() const {
        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
            case kName_PdfObjectType:
                return fStr.fBytes;

            default:
                // TODO(edisonn): report/warning
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

    static void makeInteger(int64_t value, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kInteger_PdfObjectType;
        obj->fIntegerValue = value;
    }

    static void makeReal(double value, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReal_PdfObjectType;
        obj->fRealValue = value;
    }

    static void makeNull(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kNull_PdfObjectType;
    }

    static SkPdfNativeObject makeNull() {
        SkPdfNativeObject obj;
        obj.fObjectType = kNull_PdfObjectType;
        return obj;
    }

    static SkPdfNativeObject kNull;

    static void makeNumeric(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj) {
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

    static void makeReference(unsigned int id, unsigned int gen, SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReference_PdfObjectType;
        obj->fRef.fId = id;
        obj->fRef.fGen = gen;
    }


    static void makeString(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kString_PdfObjectType);
    }

    static void makeString(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kString_PdfObjectType);
    }

    static void makeString(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kString_PdfObjectType);
    }


    static void makeHexString(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kHexString_PdfObjectType);
    }

    static void makeHexString(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kHexString_PdfObjectType);
    }

    static void makeHexString(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kHexString_PdfObjectType);
    }


    static void makeName(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kName_PdfObjectType);
    }

    static void makeName(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kName_PdfObjectType);
    }

    static void makeName(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kName_PdfObjectType);
    }


    static void makeKeyword(const unsigned char* start, SkPdfNativeObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kKeyword_PdfObjectType);
    }

    static void makeKeyword(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj) {
        makeStringCore(start, end - start, obj, kKeyword_PdfObjectType);
    }

    static void makeKeyword(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj) {
        makeStringCore(start, bytes, obj, kKeyword_PdfObjectType);
    }



    // TODO(edisonn): make the functions to return SkPdfArray, move these functions in SkPdfArray
    static void makeEmptyArray(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kArray_PdfObjectType;
        obj->fArray = new SkTDArray<SkPdfNativeObject*>();
        // return (SkPdfArray*)obj;
    }

    bool appendInArray(SkPdfNativeObject* obj) {
        SkASSERT(fObjectType == kArray_PdfObjectType);
        if (fObjectType != kArray_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        fArray->push(obj);
        return true;
    }

    size_t size() const {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return fArray->count();
    }

    SkPdfNativeObject* objAtAIndex(int i) {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    SkPdfNativeObject* removeLastInArray() {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        SkPdfNativeObject* ret = NULL;
        fArray->pop(&ret);

        return ret;
    }


    const SkPdfNativeObject* objAtAIndex(int i) const {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    SkPdfNativeObject* operator[](int i) {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    const SkPdfNativeObject* operator[](int i) const {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }


    // TODO(edisonn): make the functions to return SkPdfDictionary, move these functions in SkPdfDictionary
    static void makeEmptyDictionary(SkPdfNativeObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kDictionary_PdfObjectType;
        obj->fMap = new SkTDict<SkPdfNativeObject*>(1);
        obj->fStr.fBuffer = NULL;
        obj->fStr.fBytes = 0;
    }

    // TODO(edisonn): get all the possible names from spec, and compute a hash function
    // that would create no overlaps in the same dictionary
    // or build a tree of chars that when followed goes to a unique id/index/hash
    // TODO(edisonn): generate constants like kDictFoo, kNameDict_name
    // which will be used in code
    // add function SkPdfFastNameKey key(const char* key);
    // TODO(edisonn): setting the same key twike, will make the value undefined!
    bool set(const SkPdfNativeObject* key, SkPdfNativeObject* value) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        //// we rewrite all delimiters and white spaces with '\0', so we expect the end of name to be '\0'
        //SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return set(key->fStr.fBuffer, key->fStr.fBytes, value);
    }

    bool set(const char* key, SkPdfNativeObject* value) {
        return set((const unsigned char*)key, strlen(key), value);
    }

    bool set(const unsigned char* key, size_t len, SkPdfNativeObject* value) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);

        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        return fMap->set((const char*)key, len, value);
    }

    SkPdfNativeObject* get(const SkPdfNativeObject* key) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }

        //SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return get(key->fStr.fBuffer, key->fStr.fBytes);
    }

    SkPdfNativeObject* get(const char* key) {
        return get((const unsigned char*)key, strlen(key));
    }

    SkPdfNativeObject* get(const unsigned char* key, size_t len) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }
        SkPdfNativeObject* ret = NULL;
        fMap->find((const char*)key, len, &ret);

#ifdef PDF_TRACE
        SkString _key;
        _key.append((const char*)key, len);
        printf("\nget(/%s) = %s\n", _key.c_str(), ret ? ret->toString(0, len + 9).c_str() : "_NOT_FOUND");
#endif

        return ret;
    }

    const SkPdfNativeObject* get(const SkPdfNativeObject* key) const {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }

        //SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return get(key->fStr.fBuffer, key->fStr.fBytes);
    }

    const SkPdfNativeObject* get(const char* key) const {
        return get((const unsigned char*)key, strlen(key));
    }

    const SkPdfNativeObject* get(const unsigned char* key, size_t len) const {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }
        SkPdfNativeObject* ret = NULL;
        fMap->find((const char*)key, len, &ret);

#ifdef PDF_TRACE
        SkString _key;
        _key.append((const char*)key, len);
        printf("\nget(/%s) = %s\n", _key.c_str(), ret ? ret->toString(0, len + 9).c_str() : "_NOT_FOUND");
#endif

        return ret;
    }

    const SkPdfNativeObject* get(const char* key, const char* abr) const {
        const SkPdfNativeObject* ret = get(key);
        // TODO(edisonn): / is a valid name, and it might be an abreviation, so "" should not be like NULL
        // make this distiontion in generator, and remove "" from condition
        if (ret != NULL || abr == NULL || *abr == '\0') {
            return ret;
        }
        return get(abr);
    }

    SkPdfNativeObject* get(const char* key, const char* abr) {
        SkPdfNativeObject* ret = get(key);
        // TODO(edisonn): / is a valid name, and it might be an abreviation, so "" should not be like NULL
        // make this distiontion in generator, and remove "" from condition
        if (ret != NULL || abr == NULL || *abr == '\0') {
            return ret;
        }
        return get(abr);
    }

    SkPdfDictionary* asDictionary() {
        SkASSERT(isDictionary());
        if (!isDictionary()) {
            return NULL;
        }
        return (SkPdfDictionary*) this;
    }

    const SkPdfDictionary* asDictionary() const {
        SkASSERT(isDictionary());
        if (!isDictionary()) {
            return NULL;
        }
        return (SkPdfDictionary*) this;
    }


    bool isReference() const {
        return fObjectType == kReference_PdfObjectType;
    }

    bool isBoolean() const {
        return fObjectType == kBoolean_PdfObjectType;
    }

    bool isInteger() const {
        return fObjectType == kInteger_PdfObjectType;
    }
private:
    bool isReal() const {
        return fObjectType == kReal_PdfObjectType;
    }
public:
    bool isNumber() const {
        return fObjectType == kInteger_PdfObjectType || fObjectType == kReal_PdfObjectType;
    }

    bool isKeywordReference() const {
        return fObjectType == kKeyword_PdfObjectType && fStr.fBytes == 1 && fStr.fBuffer[0] == 'R';
    }

    bool isKeyword() const {
        return fObjectType == kKeyword_PdfObjectType;
    }

    bool isKeyword(const char* keyword) const {
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

    bool isName() const {
        return fObjectType == kName_PdfObjectType;
    }

    bool isName(const char* name) const {
        return fObjectType == kName_PdfObjectType && fStr.fBytes == strlen(name) && strncmp((const char*)fStr.fBuffer, name, fStr.fBytes) == 0;
    }

    bool isArray() const {
        return fObjectType == kArray_PdfObjectType;
    }

    bool isDate() const {
        return fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType;
    }

    bool isDictionary() const {
        return fObjectType == kDictionary_PdfObjectType;
    }

    bool isFunction() const {
        return false;  // NYI
    }

    bool isRectangle() const {
        return fObjectType == kArray_PdfObjectType && fArray->count() == 4; // NYI + and elems are numbers
    }

    // TODO(edisonn): has stream .. or is stream ... TBD
    bool hasStream() const {
        return isDictionary() && fStr.fBuffer != NULL;
    }

    // TODO(edisonn): has stream .. or is stream ... TBD
    const SkPdfStream* getStream() const {
        return hasStream() ? (const SkPdfStream*)this : NULL;
    }

    SkPdfStream* getStream() {
        return hasStream() ? (SkPdfStream*)this : NULL;
    }

    bool isAnyString() const {
        return fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType;
    }

    bool isHexString() const {
        return fObjectType == kHexString_PdfObjectType;
    }

    bool isMatrix() const {
        return fObjectType == kArray_PdfObjectType && fArray->count() == 6; // NYI + and elems are numbers
    }

    inline int64_t intValue() const {
        SkASSERT(fObjectType == kInteger_PdfObjectType);

        if (fObjectType != kInteger_PdfObjectType) {
            // TODO(edisonn): log err
            return 0;
        }
        return fIntegerValue;
    }
private:
    inline double realValue() const {
        SkASSERT(fObjectType == kReal_PdfObjectType);

        if (fObjectType != kReal_PdfObjectType) {
            // TODO(edisonn): log err
            return 0;
        }
        return fRealValue;
    }
public:
    inline double numberValue() const {
        SkASSERT(isNumber());

        if (!isNumber()) {
            // TODO(edisonn): log err
            return 0;
        }
        return fObjectType == kReal_PdfObjectType ? fRealValue : fIntegerValue;
    }

    inline SkScalar scalarValue() const {
        SkASSERT(isNumber());

        if (!isNumber()) {
            // TODO(edisonn): log err
            return SkIntToScalar(0);
        }
        return fObjectType == kReal_PdfObjectType ? SkDoubleToScalar(fRealValue) :
                                                    SkIntToScalar(fIntegerValue);
    }

    int referenceId() const {
        SkASSERT(fObjectType == kReference_PdfObjectType);
        return fRef.fId;
    }

    int referenceGeneration() const {
        SkASSERT(fObjectType == kReference_PdfObjectType);
        return fRef.fGen;
    }

    inline const char* nameValue() const {
        SkASSERT(fObjectType == kName_PdfObjectType);

        if (fObjectType != kName_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    inline const char* stringValue() const {
        SkASSERT(fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType);

        if (fObjectType != kString_PdfObjectType && fObjectType != kHexString_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    inline NotOwnedString strRef() {
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
    inline std::string nameValue2() const {
        SkASSERT(fObjectType == kName_PdfObjectType);

        if (fObjectType != kName_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return std::string((const char*)fStr.fBuffer, fStr.fBytes);
    }

    inline std::string stringValue2() const {
        SkASSERT(fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType);

        if (fObjectType != kString_PdfObjectType && fObjectType != kHexString_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return std::string((const char*)fStr.fBuffer, fStr.fBytes);
    }

    inline bool boolValue() const {
        SkASSERT(fObjectType == kBoolean_PdfObjectType);

        if (fObjectType != kBoolean_PdfObjectType) {
            // TODO(edisonn): log err
            return false;
        }
        return fBooleanValue;
    }

    SkRect rectangleValue() const {
        SkASSERT(isRectangle());
        if (!isRectangle()) {
            return SkRect::MakeEmpty();
        }

        double array[4];
        for (int i = 0; i < 4; i++) {
            // TODO(edisonn): version where we could resolve references?
            const SkPdfNativeObject* elem = objAtAIndex(i);
            if (elem == NULL || !elem->isNumber()) {
                // TODO(edisonn): report error
                return SkRect::MakeEmpty();
            }
            array[i] = elem->numberValue();
        }

        return SkRect::MakeLTRB(SkDoubleToScalar(array[0]),
                                SkDoubleToScalar(array[1]),
                                SkDoubleToScalar(array[2]),
                                SkDoubleToScalar(array[3]));
    }

    SkMatrix matrixValue() const {
        SkASSERT(isMatrix());
        if (!isMatrix()) {
            return SkMatrix::I();
        }

        double array[6];
        for (int i = 0; i < 6; i++) {
            // TODO(edisonn): version where we could resolve references?
            const SkPdfNativeObject* elem = objAtAIndex(i);
            if (elem == NULL || !elem->isNumber()) {
                // TODO(edisonn): report error
                return SkMatrix::I();
            }
            array[i] = elem->numberValue();
        }

        return SkMatrixFromPdfMatrix(array);
    }

    bool filterStream();


    bool GetFilteredStreamRef(unsigned char const** buffer, size_t* len) {
        // TODO(edisonn): add params that couls let the last filter in place if it is jpeg or png to fast load images
        if (!hasStream()) {
            return false;
        }

        filterStream();

        if (buffer) {
            *buffer = fStr.fBuffer;
        }

        if (len) {
            *len = fStr.fBytes >> 2;  // last 2 bits
        }

        return true;
    }

    bool isStreamFiltered() const {
        return hasStream() && ((fStr.fBytes & 1) == kFilteredStreamBit);
    }

    bool isStreamOwned() const {
        return hasStream() && ((fStr.fBytes & 2) == kOwnedStreamBit);
    }

    bool GetUnfilteredStreamRef(unsigned char const** buffer, size_t* len) const {
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
            *len = fStr.fBytes >> 2;  // remove last 2 bits
        }

        return true;
    }

    bool addStream(const unsigned char* buffer, size_t len) {
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
                        str.appendf("/%s %s\n", key, obj->toString(0, level + strlen(key) + 4).c_str());
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
    static void makeStringCore(const unsigned char* start, SkPdfNativeObject* obj, ObjectType type) {
        makeStringCore(start, strlen((const char*)start), obj, type);
    }

    static void makeStringCore(const unsigned char* start, const unsigned char* end, SkPdfNativeObject* obj, ObjectType type) {
        makeStringCore(start, end - start, obj, type);
    }

    static void makeStringCore(const unsigned char* start, size_t bytes, SkPdfNativeObject* obj, ObjectType type) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = type;
        obj->fStr.fBuffer = start;
        obj->fStr.fBytes = bytes;
    }

    bool applyFilter(const char* name);
    bool applyFlateDecodeFilter();
    bool applyDCTDecodeFilter();
};

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

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFOBJECT_H_
