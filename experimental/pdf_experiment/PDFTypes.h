// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.
#ifndef PDFTypes_DEFINED
#define PDFTypes_DEFINED

#include "SkTypes.h"

#include "SimpleString.h"
#include "IRef.h"

class PDFDocument;
class SkArenaAlloc;
class SkWStream;

struct PDFObject {
    virtual void emit(SkWStream* stream) const = 0;
protected:
    ~PDFObject() = default;
};

// Numbers:
struct PDFInt final : public PDFObject {
    PDFInt(int v) : fValue(v) {}
    int fValue;
    void emit(SkWStream* stream) const override;
};

struct PDFColorComponent final : public PDFObject {
    PDFColorComponent(float v) : fValue(v) {}
    float fValue;
    void emit(SkWStream* stream) const override;
};

struct PDFFloat final : public PDFObject {
    PDFFloat(float v) : fValue(v) {}
    float fValue;
    void emit(SkWStream* stream) const override;
};

// bool
struct PDFBool final : public PDFObject {
    PDFBool(bool v) : fValue(v) {}
    bool fValue;
    void emit(SkWStream* stream) const override;
};

// string
struct PDFString final : public PDFObject {
    SimpleString fValue;
    void emit(SkWStream* stream) const override;
};
    
// name
struct PDFName final : public PDFObject {
    PDFName(SimpleString v) : fValue(v) {}
    SimpleString fValue;
    void emit(SkWStream* stream) const override;
};

// indirect reference
struct PDFIndirectReference final : public PDFObject {
    PDFIndirectReference(IRef v) : fValue(v) {}
    IRef fValue;
    void emit(SkWStream* stream) const override;
};

// [ list ]
class PDFListBase : public PDFObject {
public:
    PDFListBase(PDFObject** l, size_t s) : fValues(l), fSize(s) {}
    void emit(SkWStream* stream) const final; 

protected:
    PDFObject** fValues;
    size_t fSize;
    ~PDFListBase() = default;
};

//a dynamic list
class PDFListImpl : public PDFListBase {
public:
    PDFListImpl& add(PDFObject* value) {
        SkASSERT(fSize < fMaxSize);
        fValues[fSize++] = value;
        return *this;
    }

protected:
    PDFListImpl(PDFObject** l, size_t m) : PDFListBase(l, 0), fMaxSize(m) {}
    ~PDFListImpl() = default;

private:
    size_t fMaxSize;
};

//class PDFListImpl : public PDFObject {
//public:
//    PDFListImpl& add(PDFObject* value) {
//        SkASSERT(fSize < fMaxSize);
//        fValues[fSize++] = value;
//        return *this;
//    }
//    void emit(SkWStream* stream) const final; 
//
//protected:
//    PDFListImpl(PDFObject** l, size_t m) : fValues(l), fMaxSize(m) {}
//    ~PDFListImpl() = default;
//
//private:
//    PDFObject** fValues;
//    size_t fMaxSize;
//    size_t fSize = 0;
//};

class PDFList final : public PDFListImpl {
public:
    static PDFList* Make(SkArenaAlloc* arena, size_t maximumSize);
    PDFList(SkArenaAlloc* arena, size_t maximumSize);
};

template <size_t N>
class PDFListST final : public PDFListImpl {
public:
    PDFListST() : PDFListImpl(fStorage, N) {}
private:
    PDFObject* fStorage[N];
};

// <<dict>> 
class PDFDictImpl : public PDFObject {
public:
    PDFDictImpl& add(SimpleString key, PDFObject* value) {
        SkASSERT(fSize < fMaxSize);
        fRecords[fSize++] = Record{key, value};
        return *this;
    }
    PDFDictImpl& add(const char* key, PDFObject* value) {
        return this->add(StaticString(key), value);
    }
    void innerEmit(SkWStream* stream) const;
    void emit(SkWStream* stream) const final;

protected:
    struct Record {
        SimpleString fKey;
        PDFObject* fValue;
    };
    PDFDictImpl(Record* r, size_t m) : fRecords(r), fMaxSize(m) {}
    ~PDFDictImpl() = default;

private:
    Record* fRecords;
    size_t fMaxSize;
    size_t fSize = 0;
};

class PDFDict final : public PDFDictImpl {
public:
    PDFDict(SkArenaAlloc* arena, size_t maximumSize);
    static PDFDict* Make(SkArenaAlloc* arena, size_t maximumSize);
};

template <size_t N>
class PDFDictST final : public PDFDictImpl {
public:
    PDFDictST() : PDFDictImpl(fStorage, N) {}
private:
    PDFDict::Record fStorage[N];
};
#endif  // PDFTypes_DEFINED
