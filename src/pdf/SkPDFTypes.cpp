/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFTypes.h"

#include "SkData.h"
#include "SkDeflate.h"
#include "SkMakeUnique.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkTo.h"

#include <new>

////////////////////////////////////////////////////////////////////////////////

SkPDFUnion::SkPDFUnion(Type t) : fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, int32_t v)  : fIntValue    (v), fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, bool v)     : fBoolValue   (v), fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, SkScalar v) : fScalarValue (v), fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, SkString v) : fType(t) { fSkString.init(std::move(v)); }

SkPDFUnion::~SkPDFUnion() {
    switch (fType) {
        case Type::kNameSkS:
        case Type::kStringSkS:
            fSkString.destroy();
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
            u.fSkString.init(fSkString.get());
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

static void write_string(SkWStream* wStream, const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    size_t extraCharacterCount = 0;
    for (size_t i = 0; i < len; i++) {
        if (cin[i] > '~' || cin[i] < ' ') {
            extraCharacterCount += 3;
        } else if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
            ++extraCharacterCount;
        }
    }
    if (extraCharacterCount <= len) {
        wStream->writeText("(");
        for (size_t i = 0; i < len; i++) {
            if (cin[i] > '~' || cin[i] < ' ') {
                uint8_t c = static_cast<uint8_t>(cin[i]);
                uint8_t octal[4] = { '\\',
                                     (uint8_t)('0' | ( c >> 6        )),
                                     (uint8_t)('0' | ((c >> 3) & 0x07)),
                                     (uint8_t)('0' | ( c       & 0x07)) };
                wStream->write(octal, 4);
            } else {
                if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
                    wStream->writeText("\\");
                }
                wStream->write(&cin[i], 1);
            }
        }
        wStream->writeText(")");
    } else {
        wStream->writeText("<");
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            char hexValue[2] = { SkHexadecimalDigits::gUpper[c >> 4],
                                 SkHexadecimalDigits::gUpper[c & 0xF] };
            wStream->write(hexValue, 2);
        }
        wStream->writeText(">");
    }
}

void SkPDFWriteString(SkWStream* wStream, const char* cin, size_t len) {
    write_string(wStream, cin, len);
}

void SkPDFUnion::emitObject(SkWStream* stream) const {
    switch (fType) {
        case Type::kInt:
            stream->writeDecAsText(fIntValue);
            return;
        case Type::kColorComponent:
            SkPDFUtils::AppendColorComponent(SkToU8(fIntValue), stream);
            return;
        case Type::kColorComponentF:
            SkPDFUtils::AppendColorComponentF(fScalarValue, stream);
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
            write_string(stream, fStaticString, strlen(fStaticString));
            return;
        case Type::kNameSkS:
            stream->writeText("/");
            write_name_escaped(stream, fSkString.get().c_str());
            return;
        case Type::kStringSkS:
            write_string(stream, fSkString.get().c_str(), fSkString.get().size());
            return;
        case Type::kObjRef:
            stream->writeDecAsText(fObject->fIndirectReference.fValue);
            stream->writeText(" 0 R");  // Generation number is always 0.
            return;
        case Type::kObject:
            fObject->emitObject(stream);
            return;
        case Type::kRef:
            stream->writeDecAsText(fIntValue);
            stream->writeText(" 0 R");  // Generation number is always 0.
            return;
        default:
            SkDEBUGFAIL("SkPDFUnion::emitObject with bad type");
    }
}

void SkPDFUnion::addResources(SkPDFObjNumMap* objNumMap) const {
    switch (fType) {
        case Type::kInt:
        case Type::kColorComponent:
        case Type::kColorComponentF:
        case Type::kBool:
        case Type::kScalar:
        case Type::kName:
        case Type::kString:
        case Type::kNameSkS:
        case Type::kStringSkS:
        case Type::kRef:
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

SkPDFUnion SkPDFUnion::Int(int32_t value) { return SkPDFUnion(Type::kInt, value); }

SkPDFUnion SkPDFUnion::ColorComponent(uint8_t value) {
    return SkPDFUnion(Type::kColorComponent, (int32_t)value);
}

SkPDFUnion SkPDFUnion::ColorComponentF(float value) {
    return SkPDFUnion(Type::kColorComponentF, (SkScalar)value);
}

SkPDFUnion SkPDFUnion::Bool(bool value) {
    return SkPDFUnion(Type::kBool, value);
}

SkPDFUnion SkPDFUnion::Scalar(SkScalar value) {
    return SkPDFUnion(Type::kScalar, value);
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

SkPDFUnion SkPDFUnion::Name(SkString s) { return SkPDFUnion(Type::kNameSkS, std::move(s)); }

SkPDFUnion SkPDFUnion::String(SkString s) { return SkPDFUnion(Type::kStringSkS, std::move(s)); }

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

SkPDFUnion SkPDFUnion::Ref(SkPDFIndirectReference ref) {
    return SkASSERT(ref.fValue > 0), SkPDFUnion(Type::kRef, (int32_t)ref.fValue);
}

////////////////////////////////////////////////////////////////////////////////

#if 0  // Enable if needed.
void SkPDFAtom::emitObject(SkWStream* stream) const {
    fValue.emitObject(stream);
}
void SkPDFAtom::addResources(SkPDFObjNumMap* map) const {
    fValue.addResources(map);
}
#endif  // 0

////////////////////////////////////////////////////////////////////////////////

SkPDFArray::SkPDFArray() { SkDEBUGCODE(fDumped = false;) }

SkPDFArray::~SkPDFArray() { this->drop(); }

void SkPDFArray::drop() {
    fValues = std::vector<SkPDFUnion>();
    SkDEBUGCODE(fDumped = true;)
}

size_t SkPDFArray::size() const { return fValues.size(); }

void SkPDFArray::reserve(int length) {
    fValues.reserve(length);
}

void SkPDFArray::emitObject(SkWStream* stream) const {
    SkASSERT(!fDumped);
    stream->writeText("[");
    for (size_t i = 0; i < fValues.size(); i++) {
        fValues[i].emitObject(stream);
        if (i + 1 < fValues.size()) {
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

void SkPDFArray::appendName(SkString name) {
    this->append(SkPDFUnion::Name(std::move(name)));
}

void SkPDFArray::appendString(SkString value) {
    this->append(SkPDFUnion::String(std::move(value)));
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

void SkPDFArray::appendRef(SkPDFIndirectReference ref) {
    this->append(SkPDFUnion::Ref(ref));
}

///////////////////////////////////////////////////////////////////////////////

SkPDFDict::~SkPDFDict() { this->drop(); }

void SkPDFDict::drop() {
    fRecords = std::vector<SkPDFDict::Record>();
    SkDEBUGCODE(fDumped = true;)
}

SkPDFDict::SkPDFDict(const char type[]) {
    SkDEBUGCODE(fDumped = false;)
    if (type) {
        this->insertName("Type", type);
    }
}

void SkPDFDict::emitObject(SkWStream* stream) const {
    stream->writeText("<<");
    this->emitAll(stream);
    stream->writeText(">>");
}

void SkPDFDict::emitAll(SkWStream* stream) const {
    SkASSERT(!fDumped);
    for (size_t i = 0; i < fRecords.size(); i++) {
        fRecords[i].fKey.emitObject(stream);
        stream->writeText(" ");
        fRecords[i].fValue.emitObject(stream);
        if (i + 1 < fRecords.size()) {
            stream->writeText("\n");
        }
    }
}

void SkPDFDict::addResources(SkPDFObjNumMap* catalog) const {
    SkASSERT(!fDumped);
    for (size_t i = 0; i < fRecords.size(); i++) {
        fRecords[i].fKey.addResources(catalog);
        fRecords[i].fValue.addResources(catalog);
    }
}

size_t SkPDFDict::size() const { return fRecords.size(); }

void SkPDFDict::reserve(int n) {
    fRecords.reserve(n);
}

void SkPDFDict::insertRef(const char key[], SkPDFIndirectReference ref) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Ref(ref)});
}

void SkPDFDict::insertRef(SkString key, SkPDFIndirectReference ref) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(std::move(key)), SkPDFUnion::Ref(ref)});
}

void SkPDFDict::insertObjRef(const char key[], sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::ObjRef(std::move(objSp))});
}

void SkPDFDict::insertObjRef(SkString key, sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(std::move(key)),
                                 SkPDFUnion::ObjRef(std::move(objSp))});
}

void SkPDFDict::insertObject(const char key[], sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Object(std::move(objSp))});
}
void SkPDFDict::insertObject(SkString key, sk_sp<SkPDFObject> objSp) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(std::move(key)),
                                 SkPDFUnion::Object(std::move(objSp))});
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

void SkPDFDict::insertColorComponentF(const char key[], SkScalar value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::ColorComponentF(value)});
}

void SkPDFDict::insertScalar(const char key[], SkScalar value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Scalar(value)});
}

void SkPDFDict::insertName(const char key[], const char name[]) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Name(name)});
}

void SkPDFDict::insertName(const char key[], SkString name) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::Name(std::move(name))});
}

void SkPDFDict::insertString(const char key[], const char value[]) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::String(value)});
}

void SkPDFDict::insertString(const char key[], SkString value) {
    fRecords.emplace_back(Record{SkPDFUnion::Name(key), SkPDFUnion::String(std::move(value))});
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
void SkPDFSharedStream::emitObject(SkWStream* stream) const {
    SkASSERT(fAsset);
    std::unique_ptr<SkStreamAsset> dup(fAsset->duplicate());
    SkASSERT(dup && dup->hasLength());
    size_t length = dup->getLength();
    stream->writeText("<<");
    fDict.emitAll(stream);
    stream->writeText("\n");
    SkPDFUnion::Name("Length").emitObject(stream);
    stream->writeText(" ");
    SkPDFUnion::Int(length).emitObject(stream);
    stream->writeText("\n>>stream\n");
    SkStreamCopy(stream, dup.get());
    stream->writeText("\nendstream");
}
#else
void SkPDFSharedStream::emitObject(SkWStream* stream) const {
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
    fDict.emitAll(stream);
    stream->writeText("\n");
    SkPDFUnion::Name("Length").emitObject(stream);
    stream->writeText(" ");
    SkPDFUnion::Int(length).emitObject(stream);
    stream->writeText("\n");
    SkPDFUnion::Name("Filter").emitObject(stream);
    stream->writeText(" ");
    SkPDFUnion::Name("FlateDecode").emitObject(stream);
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

SkPDFStream::SkPDFStream(sk_sp<SkData> data) {
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

void SkPDFStream::emitObject(SkWStream* stream) const {
    SkASSERT(fCompressedData);
    fDict.emitObject(stream);
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

void SkPDFObjNumMap::addObjectRecursively(SkPDFObject* obj) {
    if (obj && obj->fIndirectReference.fValue == -1) {
        obj->fIndirectReference.fValue = fNextObjectNumber++;
        fObjects.emplace_back(sk_ref_sp(obj));
        obj->addResources(this);
    }
}

#ifdef SK_PDF_IMAGE_STATS
std::atomic<int> gDrawImageCalls(0);
std::atomic<int> gJpegImageObjects(0);
std::atomic<int> gRegularImageObjects(0);

void SkPDFImageDumpStats() {
    SkDebugf("\ntotal PDF drawImage/drawBitmap calls: %d\n"
             "total PDF jpeg images: %d\n"
             "total PDF regular images: %d\n",
             gDrawImageCalls.load(),
             gJpegImageObjects.load(),
             gRegularImageObjects.load());
}
#endif // SK_PDF_IMAGE_STATS
