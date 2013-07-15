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

class SkPdfDictionary;
class SkPdfStream;
class SkPdfAllocator;

// TODO(edisonn): macro it and move it to utils
SkMatrix SkMatrixFromPdfMatrix(double array[6]);


#define kFilteredStreamBit 0
#define kUnfilteredStreamBit 1


class SkPdfObject {
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

private:
    struct NotOwnedString {
        unsigned char* fBuffer;
        size_t fBytes;
    };

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
        SkTDArray<SkPdfObject*>* fArray;
        Reference fRef;
    };
    SkTDict<SkPdfObject*>* fMap;
    void* fData;


public:

    SkPdfObject() : fObjectType(kInvalid_PdfObjectType), fMap(NULL), fData(NULL) {}

    inline void* data() {
        return fData;
    }

    inline void setData(void* data) {
        fData = data;
    }

//    ~SkPdfObject() {
//        //reset();  must be called manually!
//    }

    void reset() {
        switch (fObjectType) {
            case kArray_PdfObjectType:
                delete fArray;
                break;

            case kDictionary_PdfObjectType:
                delete fMap;
                break;

            default:
                break;
        }
        fObjectType = kInvalid_PdfObjectType;
    }

    ObjectType type() { return fObjectType; }

    const char* c_str() const {
        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
                return (const char*)fStr.fBuffer;

            default:
                // TODO(edisonn): report/warning
                return NULL;
        }
    }

    size_t len() const {
        switch (fObjectType) {
            case kString_PdfObjectType:
            case kHexString_PdfObjectType:
            case kKeyword_PdfObjectType:
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


    static void makeBoolean(bool value, SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kBoolean_PdfObjectType;
        obj->fBooleanValue = value;
    }

    static SkPdfObject makeBoolean(bool value) {
        SkPdfObject obj;
        obj.fObjectType = kBoolean_PdfObjectType;
        obj.fBooleanValue = value;
        return obj;
    }

    static void makeInteger(int64_t value, SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kInteger_PdfObjectType;
        obj->fIntegerValue = value;
    }

    static void makeReal(double value, SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReal_PdfObjectType;
        obj->fRealValue = value;
    }

    static void makeNull(SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kNull_PdfObjectType;
    }

    static SkPdfObject makeNull() {
        SkPdfObject obj;
        obj.fObjectType = kNull_PdfObjectType;
        return obj;
    }

    static SkPdfObject kNull;

    static void makeNumeric(unsigned char* start, unsigned char* end, SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        // TODO(edisonn): NYI properly
        // if has dot (impl), or exceeds max int, is real, otherwise is int
        bool isInt = true;
        for (unsigned char* current = start; current < end; current++) {
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

    static void makeReference(unsigned int id, unsigned int gen, SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kReference_PdfObjectType;
        obj->fRef.fId = id;
        obj->fRef.fGen = gen;
    }


    static void makeString(unsigned char* start, SkPdfObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kString_PdfObjectType);
    }

    static void makeString(unsigned char* start, unsigned char* end, SkPdfObject* obj) {
        makeStringCore(start, end - start, obj, kString_PdfObjectType);
    }

    static void makeString(unsigned char* start, size_t bytes, SkPdfObject* obj) {
        makeStringCore(start, bytes, obj, kString_PdfObjectType);
    }


    static void makeHexString(unsigned char* start, SkPdfObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kHexString_PdfObjectType);
    }

    static void makeHexString(unsigned char* start, unsigned char* end, SkPdfObject* obj) {
        makeStringCore(start, end - start, obj, kHexString_PdfObjectType);
    }

    static void makeHexString(unsigned char* start, size_t bytes, SkPdfObject* obj) {
        makeStringCore(start, bytes, obj, kHexString_PdfObjectType);
    }


    static void makeName(unsigned char* start, SkPdfObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kName_PdfObjectType);
    }

    static void makeName(unsigned char* start, unsigned char* end, SkPdfObject* obj) {
        makeStringCore(start, end - start, obj, kName_PdfObjectType);
    }

    static void makeName(unsigned char* start, size_t bytes, SkPdfObject* obj) {
        makeStringCore(start, bytes, obj, kName_PdfObjectType);
    }


    static void makeKeyword(unsigned char* start, SkPdfObject* obj) {
        makeStringCore(start, strlen((const char*)start), obj, kKeyword_PdfObjectType);
    }

    static void makeKeyword(unsigned char* start, unsigned char* end, SkPdfObject* obj) {
        makeStringCore(start, end - start, obj, kKeyword_PdfObjectType);
    }

    static void makeKeyword(unsigned char* start, size_t bytes, SkPdfObject* obj) {
        makeStringCore(start, bytes, obj, kKeyword_PdfObjectType);
    }



    // TODO(edisonn): make the functions to return SkPdfArray, move these functions in SkPdfArray
    static void makeEmptyArray(SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kArray_PdfObjectType;
        obj->fArray = new SkTDArray<SkPdfObject*>();
        // return (SkPdfArray*)obj;
    }

    bool appendInArray(SkPdfObject* obj) {
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

    SkPdfObject* objAtAIndex(int i) {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    SkPdfObject* removeLastInArray() {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        SkPdfObject* ret = NULL;
        fArray->pop(&ret);

        return ret;
    }


    const SkPdfObject* objAtAIndex(int i) const {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    SkPdfObject* operator[](int i) {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }

    const SkPdfObject* operator[](int i) const {
        SkASSERT(fObjectType == kArray_PdfObjectType);

        return (*fArray)[i];
    }


    // TODO(edisonn): make the functions to return SkPdfDictionary, move these functions in SkPdfDictionary
    static void makeEmptyDictionary(SkPdfObject* obj) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = kDictionary_PdfObjectType;
        obj->fMap = new SkTDict<SkPdfObject*>(1);
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
    bool set(SkPdfObject* key, SkPdfObject* value) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        // we rewrite all delimiters and white spaces with '\0', so we expect the end of name to be '\0'
        SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return set((char*)key->fStr.fBuffer, value);
    }

    bool set(const char* key, SkPdfObject* value) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);

        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        return fMap->set(key, value);
    }

    SkPdfObject* get(SkPdfObject* key) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return get((char*)key->fStr.fBuffer);
    }

    SkPdfObject* get(const char* key) {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }
        SkPdfObject* ret = NULL;
        fMap->find(key, &ret);
        return ret;
    }

    const SkPdfObject* get(SkPdfObject* key) const {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key->fObjectType == kName_PdfObjectType);

        if (key->fObjectType != kName_PdfObjectType || fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return false;
        }

        SkASSERT(key->fStr.fBuffer[key->fStr.fBytes] == '\0');

        return get((char*)key->fStr.fBuffer);
    }


    const SkPdfObject* get(const char* key) const {
        SkASSERT(fObjectType == kDictionary_PdfObjectType);
        SkASSERT(key);
        if (fObjectType != kDictionary_PdfObjectType) {
            // TODO(edisonn): report err
            return NULL;
        }
        SkPdfObject* ret = NULL;
        fMap->find(key, &ret);
        return ret;
    }

    const SkPdfObject* get(const char* key, const char* abr) const {
        const SkPdfObject* ret = get(key);
        // TODO(edisonn): / is a valid name, and it might be an abreviation, so "" should not be like NULL
        // make this distiontion in generator, and remove "" from condition
        if (ret != NULL || abr == NULL || *abr == '\0') {
            return ret;
        }
        return get(abr);
    }

    SkPdfObject* get(const char* key, const char* abr) {
        SkPdfObject* ret = get(key);
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

    // TODO(edisonn): nameValue2 and stringValue2 are used to make code generation easy,
    // but it is not a performat way to do it, since it will create an extra copy
    // remove these functions and make code generated faster
    inline std::string nameValue2() const {
        SkASSERT(fObjectType == kName_PdfObjectType);

        if (fObjectType != kName_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    inline std::string stringValue2() const {
        SkASSERT(fObjectType == kString_PdfObjectType || fObjectType == kHexString_PdfObjectType);

        if (fObjectType != kString_PdfObjectType && fObjectType != kHexString_PdfObjectType) {
            // TODO(edisonn): log err
            return "";
        }
        return (const char*)fStr.fBuffer;
    }

    inline bool boolValue() const {
        SkASSERT(fObjectType == kBoolean_PdfObjectType);

        if (fObjectType == kBoolean_PdfObjectType) {
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
            const SkPdfObject* elem = objAtAIndex(i);
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
            const SkPdfObject* elem = objAtAIndex(i);
            if (elem == NULL || !elem->isNumber()) {
                // TODO(edisonn): report error
                return SkMatrix::I();
            }
            array[i] = elem->numberValue();
        }

        return SkMatrixFromPdfMatrix(array);
    }

    bool filterStream(SkPdfAllocator* allocator);


    bool GetFilteredStreamRef(unsigned char** buffer, size_t* len, SkPdfAllocator* allocator) {
        // TODO(edisonn): add params that couls let the last filter in place if it is jpeg or png to fast load images
        if (!hasStream()) {
            return false;
        }

        filterStream(allocator);

        if (buffer) {
            *buffer = fStr.fBuffer;
        }

        if (len) {
            *len = fStr.fBytes >> 1;  // last bit
        }

        return true;
    }

    bool isStreamFiltered() const {
        return hasStream() && ((fStr.fBytes & 1) == kFilteredStreamBit);
    }

    bool GetUnfilteredStreamRef(unsigned char** buffer, size_t* len) const {
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
            *len = fStr.fBytes >> 1;  // remove slast bit
        }

        return true;
    }

    bool addStream(unsigned char* buffer, size_t len) {
        SkASSERT(!hasStream());
        SkASSERT(isDictionary());

        if (!isDictionary() || hasStream()) {
            return false;
        }

        fStr.fBuffer = buffer;
        fStr.fBytes = (len << 2) + kUnfilteredStreamBit;

        return true;
    }

    SkString toString() {
        SkString str;
        switch (fObjectType) {
            case kInvalid_PdfObjectType:
                str.append("Invalid");
                break;

            case kBoolean_PdfObjectType:
                str.appendf("Boolean: %s", fBooleanValue ? "true" : "false");
                break;

            case kInteger_PdfObjectType:
                str.appendf("Integer: %i", (int)fIntegerValue);
                break;

            case kReal_PdfObjectType:
                str.appendf("Real: %f", fRealValue);
                break;

            case kString_PdfObjectType:
                str.appendf("String, len() = %u: ", (unsigned int)fStr.fBytes);
                str.append((const char*)fStr.fBuffer, fStr.fBytes);
                break;

            case kHexString_PdfObjectType:
                str.appendf("HexString, len() = %u: ", (unsigned int)fStr.fBytes);
                str.append((const char*)fStr.fBuffer, fStr.fBytes);
                break;

            case kName_PdfObjectType:
                str.appendf("Name, len() = %u: ", (unsigned int)fStr.fBytes);
                str.append((const char*)fStr.fBuffer, fStr.fBytes);
                break;

            case kKeyword_PdfObjectType:
                str.appendf("Keyword, len() = %u: ", (unsigned int)fStr.fBytes);
                str.append((const char*)fStr.fBuffer, fStr.fBytes);
                break;

            case kArray_PdfObjectType:
                str.append("Array, size() = %i [", size());
                for (unsigned int i = 0; i < size(); i++) {
                    str.append(objAtAIndex(i)->toString());
                }
                str.append("]");
                break;

            case kDictionary_PdfObjectType:
                // TODO(edisonn): NYI
                str.append("Dictionary: NYI");
                if (hasStream()) {
                    str.append(" HAS_STREAM");
                }
                break;

            case kNull_PdfObjectType:
                str = "NULL";
                break;

            case kReference_PdfObjectType:
                str.appendf("Reference: %i %i", fRef.fId, fRef.fGen);
                break;

            case kUndefined_PdfObjectType:
                str = "Undefined";
                break;

            default:
                str = "Internal Error Object Type";
                break;
        }

        return str;
    }

private:
    static void makeStringCore(unsigned char* start, SkPdfObject* obj, ObjectType type) {
        makeStringCore(start, strlen((const char*)start), obj, type);
    }

    static void makeStringCore(unsigned char* start, unsigned char* end, SkPdfObject* obj, ObjectType type) {
        makeStringCore(start, end - start, obj, type);
    }

    static void makeStringCore(unsigned char* start, size_t bytes, SkPdfObject* obj, ObjectType type) {
        SkASSERT(obj->fObjectType == kInvalid_PdfObjectType);

        obj->fObjectType = type;
        obj->fStr.fBuffer = start;
        obj->fStr.fBytes = bytes;
    }

    bool applyFilter(const char* name, SkPdfAllocator* allocator);
    bool applyFlateDecodeFilter(SkPdfAllocator* allocator);
    bool applyDCTDecodeFilter(SkPdfAllocator* allocator);
};

class SkPdfStream : public SkPdfObject {};
class SkPdfArray : public SkPdfObject {};
class SkPdfString : public SkPdfObject {};
class SkPdfHexString : public SkPdfObject {};
class SkPdfInteger : public SkPdfObject {};
class SkPdfReal : public SkPdfObject {};
class SkPdfNumber : public SkPdfObject {};

class SkPdfName : public SkPdfObject {
    SkPdfName() : SkPdfObject() {
        SkPdfObject::makeName((unsigned char*)"", this);
    }
public:
    SkPdfName(char* name) : SkPdfObject() {
        this->makeName((unsigned char*)name, this);
    }
};

#endif  // EXPERIMENTAL_PDFVIEWER_PDFPARSER_NATIVE_SKPDFOBJECT_H_
