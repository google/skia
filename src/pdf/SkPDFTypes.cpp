/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPDFTypes.h"

#include "SkData.h"
#include "SkDeflate.h"
#include "SkExecutor.h"
#include "SkMakeUnique.h"
#include "SkPDFDocumentPriv.h"
#include "SkPDFUnion.h"
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
        case Type::kObject:
            SkASSERT(fObject);
            delete fObject;
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
        case Type::kObject:
            fObject->emitObject(stream);
            return;
        case Type::kRef:
            SkASSERT(fIntValue >= 0);
            stream->writeDecAsText(fIntValue);
            stream->writeText(" 0 R");  // Generation number is always 0.
            return;
        default:
            SkDEBUGFAIL("SkPDFUnion::emitObject with bad type");
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

SkPDFUnion SkPDFUnion::Object(std::unique_ptr<SkPDFObject> objSp) {
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
#endif  // 0

////////////////////////////////////////////////////////////////////////////////

SkPDFArray::SkPDFArray() {}

SkPDFArray::~SkPDFArray() {}

size_t SkPDFArray::size() const { return fValues.size(); }

void SkPDFArray::reserve(int length) {
    fValues.reserve(length);
}

void SkPDFArray::emitObject(SkWStream* stream) const {
    stream->writeText("[");
    for (size_t i = 0; i < fValues.size(); i++) {
        fValues[i].emitObject(stream);
        if (i + 1 < fValues.size()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
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

void SkPDFArray::appendObject(std::unique_ptr<SkPDFObject>&& objSp) {
    this->append(SkPDFUnion::Object(std::move(objSp)));
}

void SkPDFArray::appendRef(SkPDFIndirectReference ref) {
    this->append(SkPDFUnion::Ref(ref));
}

///////////////////////////////////////////////////////////////////////////////

SkPDFDict::~SkPDFDict() {}

SkPDFDict::SkPDFDict(const char type[]) {
    if (type) {
        this->insertName("Type", type);
    }
}

void SkPDFDict::emitObject(SkWStream* stream) const {
    stream->writeText("<<");
    for (size_t i = 0; i < fRecords.size(); ++i) {
        const std::pair<SkPDFUnion, SkPDFUnion>& record = fRecords[i];
        record.first.emitObject(stream);
        stream->writeText(" ");
        record.second.emitObject(stream);
        if (i + 1 < fRecords.size()) {
            stream->writeText("\n");
        }
    }
    stream->writeText(">>");
}

size_t SkPDFDict::size() const { return fRecords.size(); }

void SkPDFDict::reserve(int n) {
    fRecords.reserve(n);
}

void SkPDFDict::insertRef(const char key[], SkPDFIndirectReference ref) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Ref(ref));
}

void SkPDFDict::insertRef(SkString key, SkPDFIndirectReference ref) {
    fRecords.emplace_back(SkPDFUnion::Name(std::move(key)), SkPDFUnion::Ref(ref));
}

void SkPDFDict::insertObject(const char key[], std::unique_ptr<SkPDFObject>&& objSp) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Object(std::move(objSp)));
}
void SkPDFDict::insertObject(SkString key, std::unique_ptr<SkPDFObject>&& objSp) {
    fRecords.emplace_back(SkPDFUnion::Name(std::move(key)),
                          SkPDFUnion::Object(std::move(objSp)));
}

void SkPDFDict::insertBool(const char key[], bool value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Bool(value));
}

void SkPDFDict::insertInt(const char key[], int32_t value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Int(value));
}

void SkPDFDict::insertInt(const char key[], size_t value) {
    this->insertInt(key, SkToS32(value));
}

void SkPDFDict::insertColorComponentF(const char key[], SkScalar value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::ColorComponentF(value));
}

void SkPDFDict::insertScalar(const char key[], SkScalar value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Scalar(value));
}

void SkPDFDict::insertName(const char key[], const char name[]) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Name(name));
}

void SkPDFDict::insertName(const char key[], SkString name) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::Name(std::move(name)));
}

void SkPDFDict::insertString(const char key[], const char value[]) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::String(value));
}

void SkPDFDict::insertString(const char key[], SkString value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::String(std::move(value)));
}

////////////////////////////////////////////////////////////////////////////////



static void serialize_stream(SkPDFDict* origDict,
                             SkStreamAsset* stream,
                             bool deflate,
                             SkPDFDocument* doc,
                             SkPDFIndirectReference ref) {
    // Code assumes that the stream starts at the beginning.
    SkASSERT(stream && stream->hasLength());

    std::unique_ptr<SkStreamAsset> tmp;
    SkPDFDict tmpDict;
    SkPDFDict& dict = origDict ? *origDict : tmpDict;
    static const size_t kMinimumSavings = strlen("/Filter_/FlateDecode_");
    if (deflate && stream->getLength() > kMinimumSavings) {
        SkDynamicMemoryWStream compressedData;
        SkDeflateWStream deflateWStream(&compressedData);
        SkStreamCopy(&deflateWStream, stream);
        deflateWStream.finalize();
        #ifdef SK_PDF_BASE85_BINARY
        {
            SkPDFUtils::Base85Encode(compressedData.detachAsStream(), &compressedData);
            tmp = compressedData.detachAsStream();
            stream = tmp.get();
            auto filters = SkPDFMakeArray();
            filters->appendName("ASCII85Decode");
            filters->appendName("FlateDecode");
            dict.insertObject("Filter", std::move(filters));
        }
        #else
        if (stream->getLength() > compressedData.bytesWritten() + kMinimumSavings) {
            tmp = compressedData.detachAsStream();
            stream = tmp.get();
            dict.insertName("Filter", "FlateDecode");
        } else {
            SkAssertResult(stream->rewind());
        }
        #endif

    }
    dict.insertInt("Length", stream->getLength());
    doc->emitStream(dict,
                    [stream](SkWStream* dst) { dst->writeStream(stream, stream->getLength()); },
                    ref);
}

SkPDFIndirectReference SkPDFStreamOut(std::unique_ptr<SkPDFDict> dict,
                                      std::unique_ptr<SkStreamAsset> content,
                                      SkPDFDocument* doc,
                                      bool deflate) {
    SkPDFIndirectReference ref = doc->reserveRef();
    if (SkExecutor* executor = doc->executor()) {
        SkPDFDict* dictPtr = dict.release();
        SkStreamAsset* contentPtr = content.release();
        // Pass ownership of both pointers into a std::function, which should
        // only be executed once.
        doc->incrementJobCount();
        executor->add([dictPtr, contentPtr, deflate, doc, ref]() {
            serialize_stream(dictPtr, contentPtr, deflate, doc, ref);
            delete dictPtr;
            delete contentPtr;
            doc->signalJobComplete();
        });
        return ref;
    }
    serialize_stream(dict.get(), content.get(), deflate, doc, ref);
    return ref;
}
