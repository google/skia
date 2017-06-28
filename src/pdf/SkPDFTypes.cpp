/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkData.h"
#include "SkDeflate.h"
#include "SkMakeUnique.h"
#include "SkPDFTypes.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkStreamPriv.h"

////////////////////////////////////////////////////////////////////////////////

SkString* pun(char* x) { return reinterpret_cast<SkString*>(x); }
const SkString* pun(const char* x) {
    return reinterpret_cast<const SkString*>(x);
}

SkPDFUnion::SkPDFUnion(Type t) : fType(t) {}

SkPDFUnion::~SkPDFUnion() {
    switch (fType) {
        case Type::kNameSkS:
        case Type::kStringSkS:
            pun(fSkString)->~SkString();
            return;
        case Type::kObjRef:
        case Type::kObject:
            SkASSERT(fObject);
            fObject->unref();
            return;
        default:
            return;
    }
}

SkPDFUnion& SkPDFUnion::operator=(SkPDFUnion&& other) {
    if (this != &other) {
        this->~SkPDFUnion();
        new (this) SkPDFUnion(std::move(other));
    }
    return *this;
}

SkPDFUnion::SkPDFUnion(SkPDFUnion&& other) {
    SkASSERT(this != &other);
    memcpy(this, &other, sizeof(*this));
    other.fType = Type::kDestroyed;
}

#if 0
SkPDFUnion SkPDFUnion::copy() const {
    SkPDFUnion u(fType);
    memcpy(&u, this, sizeof(u));
    switch (fType) {
        case Type::kNameSkS:
        case Type::kStringSkS:
            new (pun(u.fSkString)) SkString(*pun(fSkString));
            return u;
        case Type::kObjRef:
        case Type::kObject:
            SkRef(u.fObject);
            return u;
        default:
            return u;
    }
}
SkPDFUnion& SkPDFUnion::operator=(const SkPDFUnion& other) {
    return *this = other.copy();
}
SkPDFUnion::SkPDFUnion(const SkPDFUnion& other) {
    *this = other.copy();
}
#endif

bool SkPDFUnion::isName() const {
    return Type::kName == fType || Type::kNameSkS == fType;
}

#ifdef SK_DEBUG
// Most names need no escaping.  Such names are handled as static
// const strings.
bool is_valid_name(const char* n) {
    static const char kControlChars[] = "/%()<>[]{}";
    while (*n) {
        if (*n < '!' || *n > '~' || strchr(kControlChars, *n)) {
            return false;
        }
        ++n;
    }
    return true;
}
#endif  // SK_DEBUG

// Given an arbitrary string, write it as a valid name (not including
// leading slash).
static void write_name_escaped(SkWStream* o, const char* name) {
    static const char kToEscape[] = "#/%()<>[]{}";
    for (const uint8_t* n = reinterpret_cast<const uint8_t*>(name); *n; ++n) {
        uint8_t v = *n;
        if (v < '!' || v > '~' || strchr(kToEscape, v)) {
            char buffer[3] = {'#',
                              SkHexadecimalDigits::gUpper[v >> 4],
                              SkHexadecimalDigits::gUpper[v & 0xF]};
            o->write(buffer, sizeof(buffer));
        } else {
            o->write(n, 1);
        }
    }
}

void SkPDFUnion::emitObject(SkWStream* stream,
                            const SkPDFObjNumMap& objNumMap) const {
    switch (fType) {
        case Type::kInt:
            stream->writeDecAsText(fIntValue);
            return;
        case Type::kColorComponent:
            SkPDFUtils::AppendColorComponent(SkToU8(fIntValue), stream);
            return;
        case Type::kBool:
            stream->writeText(fBoolValue ? "true" : "false");
            return;
        case Type::kScalar:
            SkPDFUtils::AppendScalar(fScalarValue, stream);
            return;
        case Type::kName:
            stream->writeText("/");
            SkASSERT(is_valid_name(fStaticString));
            stream->writeText(fStaticString);
            return;
        case Type::kString:
            SkASSERT(fStaticString);
            SkPDFUtils::WriteString(stream, fStaticString,
                                    strlen(fStaticString));
            return;
        case Type::kNameSkS:
            stream->writeText("/");
            write_name_escaped(stream, pun(fSkString)->c_str());
            return;
        case Type::kStringSkS:
            SkPDFUtils::WriteString(stream, pun(fSkString)->c_str(),
                                    pun(fSkString)->size());
            return;
        case Type::kObjRef:
            stream->writeDecAsText(objNumMap.getObjectNumber(fObject));
            stream->writeText(" 0 R");  // Generation number is always 0.
            return;
        case Type::kObject:
            fObject->emitObject(stream, objNumMap);
            return;
        default:
            SkDEBUGFAIL("SkPDFUnion::emitObject with bad type");
    }
}

void SkPDFUnion::addResources(SkPDFObjNumMap* objNumMap) const {
    switch (fType) {
        case Type::kInt:
        case Type::kColorComponent:
        case Type::kBool:
        case Type::kScalar:
        case Type::kName:
        case Type::kString:
        case Type::kNameSkS:
        case Type::kStringSkS:
            return;  // These have no resources.
        case Type::kObjRef:
            objNumMap->addObjectRecursively(fObject);
            return;
        case Type::kObject:
            fObject->addResources(objNumMap);
            return;
        default:
            SkDEBUGFAIL("SkPDFUnion::addResources with bad type");
    }
}

SkPDFUnion SkPDFUnion::Int(int32_t value) {
    SkPDFUnion u(Type::kInt);
    u.fIntValue = value;
    return u;
}

SkPDFUnion SkPDFUnion::ColorComponent(uint8_t value) {
    SkPDFUnion u(Type::kColorComponent);
    u.fIntValue = value;
    return u;
}

SkPDFUnion SkPDFUnion::Bool(bool value) {
    SkPDFUnion u(Type::kBool);
    u.fBoolValue = value;
    return u;
}

SkPDFUnion SkPDFUnion::Scalar(SkScalar value) {
    SkPDFUnion u(Type::kScalar);
    u.fScalarValue = value;
    return u;
}

SkPDFUnion SkPDFUnion::Name(const char* value) {
    SkPDFUnion u(Type::kName);
    SkASSERT(value);
    SkASSERT(is_valid_name(value));
    u.fStaticString = value;
    return u;
}

SkPDFUnion SkPDFUnion::String(const char* value) {
    SkPDFUnion u(Type::kString);
    SkASSERT(value);
    u.fStaticString = value;
    return u;
}

SkPDFUnion SkPDFUnion::Name(const SkString& s) {
    SkPDFUnion u(Type::kNameSkS);
    new (pun(u.fSkString)) SkString(s);
    return u;
}

SkPDFUnion SkPDFUnion::String(const SkString& s) {
    SkPDFUnion u(Type::kStringSkS);
    new (pun(u.fSkString)) SkString(s);
    return u;
}

SkPDFUnion SkPDFUnion::ObjRef(sk_sp<SkPDFObject> objSp) {
    SkPDFUnion u(Type::kObjRef);
    SkASSERT(objSp.get());
    u.fObject = objSp.release();  // take ownership into union{}
    return u;
}

SkPDFUnion SkPDFUnion::Object(sk_sp<SkPDFObject> objSp) {
    SkPDFUnion u(Type::kObject);
    SkASSERT(objSp.get());
    u.fObject = objSp.release();  // take ownership into union{}
    return u;
}

////////////////////////////////////////////////////////////////////////////////

#if 0  // Enable if needed.
void SkPDFAtom::emitObject(SkWStream* stream,
                           const SkPDFObjNumMap& objNumMap) const {
    fValue.emitObject(stream, objNumMap);
}
void SkPDFAtom::addResources(SkPDFObjNumMap* map) const {
    fValue.addResources(map);
}
#endif  // 0

////////////////////////////////////////////////////////////////////////////////

SkPDFArray::SkPDFArray() { SkDEBUGCODE(fDumped = false;) }

SkPDFArray::~SkPDFArray() { this->drop(); }

void SkPDFArray::drop() {
    fValues.reset();
    SkDEBUGCODE(fDumped = true;)
}

int SkPDFArray::size() const { return fValues.count(); }

void SkPDFArray::reserve(int length) {
    fValues.reserve(length);
}

void SkPDFArray::emitObject(SkWStream* stream,
                            const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(!fDumped);
    stream->writeText("[");
    for (int i = 0; i < fValues.count(); i++) {
        fValues[i].emitObject(stream, objNumMap);
        if (i + 1 < fValues.count()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

void SkPDFArray::addResources(SkPDFObjNumMap* catalog) const {
    SkASSERT(!fDumped);
    for (const SkPDFUnion& value : fValues) {
        value.addResources(catalog);
    }
}

void SkPDFArray::append(SkPDFUnion&& value) {
    fValues.emplace_back(std::move(value));
}

void SkPDFArray::appendInt(int32_t value) {
    this->append(SkPDFUnion::Int(value));
}

void SkPDFArray::appendColorComponent(uint8_t value) {
    this->append(SkPDFUnion::ColorComponent(value));
}

void SkPDFArray::appendBool(bool value) {
    this->append(SkPDFUnion::Bool(value));
}

void SkPDFArray::appendScalar(SkScalar value) {
    this->append(SkPDFUnion::Scalar(value));
}

void SkPDFArray::appendName(const char name[]) {
    this->append(SkPDFUnion::Name(SkString(name)));
}

void SkPDFArray::appendName(const SkString& name) {
    this->append(SkPDFUnion::Name(name));
}

void SkPDFArray::appendString(const SkString& value) {
    this->append(SkPDFUnion::String(value));
}

void SkPDFArray::appendString(const char value[]) {
    this->append(SkPDFUnion::String(value));
}

void SkPDFArray::appendObject(sk_sp<SkPDFObject> objSp) {
    this->append(SkPDFUnion::Object(std::move(objSp)));
}

void SkPDFArray::appendObjRef(sk_sp<SkPDFObject> objSp) {
    this->append(SkPDFUnion::ObjRef(std::move(objSp)));
}

///////////////////////////////////////////////////////////////////////////////

SkPDFDict::~SkPDFDict() { this->drop(); }

void SkPDFDict::drop() {
    fRecords.reset();
    SkDEBUGCODE(fDumped = true;)
}

SkPDFDict::SkPDFDict(const char type[]) {
    SkDEBUGCODE(fDumped = false;)
    if (type) {
        this->insertName("Type", type);
    }
}

void SkPDFDict::emitObject(SkWStream* stream,
                           const SkPDFObjNumMap& objNumMap) const {
    stream->writeText("<<");
    this->emitAll(stream, objNumMap);
    stream->writeText(">>");
}

void SkPDFDict::emitAll(SkWStream* stream,
                        const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(!fDumped);
    for (int i = 0; i < fRecords.count(); i++) {
        fRecords[i].fKey.emitObject(stream, objNumMap);
        stream->writeText(" ");
        fRecords[i].fValue.emitObject(stream, objNumMap);
        if (i + 1 < fRecords.count()) {
            stream->writeText("\n");
        }
    }
}

void SkPDFDict::addResources(SkPDFObjNumMap* catalog) const {
    SkASSERT(!fDumped);
    for (int i = 0; i < fRecords.count(); i++) {
        fRecords[i].fKey.addResources(catalog);
        fRecords[i].fValue.addResources(catalog);
    }
}

int SkPDFDict::size() const { return fRecords.count(); }

void SkPDFDict::reserve(int n) {
    fRecords.reserve(n);
}

void SkPDFDict::insertObjRef(const char key[], sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::ObjRef(std::move(objSp))});
}

void SkPDFDict::insertObjRef(const SkString& key, sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::ObjRef(std::move(objSp))});
}

void SkPDFDict::insertObject(const char key[], sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Object(std::move(objSp))});
}
void SkPDFDict::insertObject(const SkString& key, sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Object(std::move(objSp))});
}

void SkPDFDict::insertBool(const char key[], bool value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Bool(value)});
}

void SkPDFDict::insertInt(const char key[], int32_t value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Int(value)});
}

void SkPDFDict::insertInt(const char key[], size_t value) {
    this->insertInt(key, SkToS32(value));
}

void SkPDFDict::insertScalar(const char key[], SkScalar value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Scalar(value)});
}

void SkPDFDict::insertName(const char key[], const char name[]) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Name(name)});
}

void SkPDFDict::insertName(const char key[], const SkString& name) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Name(name)});
}

void SkPDFDict::insertString(const char key[], const char value[]) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::String(value)});
}

void SkPDFDict::insertString(const char key[], const SkString& value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::String(value)});
}

////////////////////////////////////////////////////////////////////////////////

SkPDFSharedStream::SkPDFSharedStream(std::unique_ptr<SkStreamAsset> data)
    : fAsset(std::move(data)) {
    SkASSERT(fAsset);
}

SkPDFSharedStream::~SkPDFSharedStream() { this->drop(); }

void SkPDFSharedStream::drop() {
    fAsset = nullptr;;
    fDict.drop();
}

#ifdef SK_PDF_LESS_COMPRESSION
void SkPDFSharedStream::emitObject(
        SkWStream* stream,
        const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fAsset);
    std::unique_ptr<SkStreamAsset> dup(fAsset->duplicate());
    SkASSERT(dup && dup->hasLength());
    size_t length = dup->getLength();
    stream->writeText("<<");
    fDict.emitAll(stream, objNumMap);
    stream->writeText("\n");
    SkPDFUnion::Name("Length").emitObject(stream, objNumMap);
    stream->writeText(" ");
    SkPDFUnion::Int(length).emitObject(stream, objNumMap);
    stream->writeText("\n>>stream\n");
    SkStreamCopy(stream, dup.get());
    stream->writeText("\nendstream");
}
#else
void SkPDFSharedStream::emitObject(
        SkWStream* stream,
        const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fAsset);
    SkDynamicMemoryWStream buffer;
    SkDeflateWStream deflateWStream(&buffer);
    // Since emitObject is const, this function doesn't change the dictionary.
    std::unique_ptr<SkStreamAsset> dup(fAsset->duplicate());  // Cheap copy
    SkASSERT(dup);
    SkStreamCopy(&deflateWStream, dup.get());
    deflateWStream.finalize();
    size_t length = buffer.bytesWritten();
    stream->writeText("<<");
    fDict.emitAll(stream, objNumMap);
    stream->writeText("\n");
    SkPDFUnion::Name("Length").emitObject(stream, objNumMap);
    stream->writeText(" ");
    SkPDFUnion::Int(length).emitObject(stream, objNumMap);
    stream->writeText("\n");
    SkPDFUnion::Name("Filter").emitObject(stream, objNumMap);
    stream->writeText(" ");
    SkPDFUnion::Name("FlateDecode").emitObject(stream, objNumMap);
    stream->writeText(">>");
    stream->writeText(" stream\n");
    buffer.writeToAndReset(stream);
    stream->writeText("\nendstream");
}
#endif

void SkPDFSharedStream::addResources(
        SkPDFObjNumMap* catalog) const {
    SkASSERT(fAsset);
    fDict.addResources(catalog);
}


////////////////////////////////////////////////////////////////////////////////

SkPDFStream:: SkPDFStream(sk_sp<SkData> data) {
    this->setData(skstd::make_unique<SkMemoryStream>(std::move(data)));
}

SkPDFStream::SkPDFStream(std::unique_ptr<SkStreamAsset> stream) {
    this->setData(std::move(stream));
}

SkPDFStream::SkPDFStream() {}

SkPDFStream::~SkPDFStream() {}

void SkPDFStream::addResources(SkPDFObjNumMap* catalog) const {
    SkASSERT(fCompressedData);
    fDict.addResources(catalog);
}

void SkPDFStream::drop() {
    fCompressedData.reset(nullptr);
    fDict.drop();
}

void SkPDFStream::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap& objNumMap) const {
    SkASSERT(fCompressedData);
    fDict.emitObject(stream, objNumMap);
    // duplicate (a cheap operation) preserves const on fCompressedData.
    std::unique_ptr<SkStreamAsset> dup(fCompressedData->duplicate());
    SkASSERT(dup);
    SkASSERT(dup->hasLength());
    stream->writeText(" stream\n");
    stream->writeStream(dup.get(), dup->getLength());
    stream->writeText("\nendstream");
}

void SkPDFStream::setData(std::unique_ptr<SkStreamAsset> stream) {
    SkASSERT(!fCompressedData);  // Only call this function once.
    SkASSERT(stream);
    // Code assumes that the stream starts at the beginning.

    #ifdef SK_PDF_LESS_COMPRESSION
    fCompressedData = std::move(stream);
    SkASSERT(fCompressedData && fCompressedData->hasLength());
    fDict.insertInt("Length", fCompressedData->getLength());
    #else

    SkASSERT(stream->hasLength());
    SkDynamicMemoryWStream compressedData;
    SkDeflateWStream deflateWStream(&compressedData);
    if (stream->getLength() > 0) {
        SkStreamCopy(&deflateWStream, stream.get());
    }
    deflateWStream.finalize();
    size_t compressedLength = compressedData.bytesWritten();
    size_t originalLength = stream->getLength();

    if (originalLength <= compressedLength + strlen("/Filter_/FlateDecode_")) {
        SkAssertResult(stream->rewind());
        fCompressedData = std::move(stream);
        fDict.insertInt("Length", originalLength);
        return;
    }
    fCompressedData = compressedData.detachAsStream();
    fDict.insertName("Filter", "FlateDecode");
    fDict.insertInt("Length", compressedLength);
    #endif
}

////////////////////////////////////////////////////////////////////////////////

bool SkPDFObjNumMap::addObject(SkPDFObject* obj) {
    if (fObjectNumbers.find(obj)) {
        return false;
    }
    fObjectNumbers.set(obj, fObjectNumbers.count() + 1);
    fObjects.emplace_back(sk_ref_sp(obj));
    return true;
}

void SkPDFObjNumMap::addObjectRecursively(SkPDFObject* obj) {
    if (obj && this->addObject(obj)) {
        obj->addResources(this);
    }
}

int32_t SkPDFObjNumMap::getObjectNumber(SkPDFObject* obj) const {
    int32_t* objectNumberFound = fObjectNumbers.find(obj);
    SkASSERT(objectNumberFound);
    return *objectNumberFound;
}

#ifdef SK_PDF_IMAGE_STATS
SkAtomic<int> gDrawImageCalls(0);
SkAtomic<int> gJpegImageObjects(0);
SkAtomic<int> gRegularImageObjects(0);

void SkPDFImageDumpStats() {
    SkDebugf("\ntotal PDF drawImage/drawBitmap calls: %d\n"
             "total PDF jpeg images: %d\n"
             "total PDF regular images: %d\n",
             gDrawImageCalls.load(),
             gJpegImageObjects.load(),
             gRegularImageObjects.load());
}
#endif // SK_PDF_IMAGE_STATS
