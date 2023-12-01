/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/pdf/SkPDFTypes.h"

#include "include/core/SkData.h"
#include "include/core/SkExecutor.h"
#include "include/core/SkStream.h"
#include "include/private/base/SkTo.h"
#include "src/core/SkStreamPriv.h"
#include "src/pdf/SkDeflate.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFUnion.h"
#include "src/pdf/SkPDFUtils.h"

#include <new>

////////////////////////////////////////////////////////////////////////////////

SkPDFUnion::SkPDFUnion(Type t, int32_t     v) : fIntValue          (v) , fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, bool        v) : fBoolValue         (v) , fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, SkScalar    v) : fScalarValue       (v) , fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, const char* v) : fStaticString      (v) , fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, SkString    v) : fSkString(std::move(v)), fType(t) {}
SkPDFUnion::SkPDFUnion(Type t, PDFObject   v) : fObject  (std::move(v)), fType(t) {}

SkPDFUnion::~SkPDFUnion() {
    switch (fType) {
        case Type::kNameSkS:
        case Type::kByteStringSkS:
        case Type::kTextStringSkS:
            fSkString.~SkString();
            return;
        case Type::kObject:
            fObject.~PDFObject();
            return;
        default:
            return;
    }
}

SkPDFUnion::SkPDFUnion(SkPDFUnion&& that) : fType(that.fType) {
    SkASSERT(this != &that);

    switch (fType) {
        case Type::kDestroyed:
            break;
        case Type::kInt:
        case Type::kColorComponent:
        case Type::kRef:
            fIntValue = that.fIntValue;
            break;
        case Type::kBool:
            fBoolValue = that.fBoolValue;
            break;
        case Type::kColorComponentF:
        case Type::kScalar:
            fScalarValue = that.fScalarValue;
            break;
        case Type::kName:
        case Type::kByteString:
        case Type::kTextString:
            fStaticString = that.fStaticString;
            break;
        case Type::kNameSkS:
        case Type::kByteStringSkS:
        case Type::kTextStringSkS:
            new (&fSkString) SkString(std::move(that.fSkString));
            break;
        case Type::kObject:
            new (&fObject) PDFObject(std::move(that.fObject));
            break;
        default:
            SkDEBUGFAIL("SkPDFUnion::SkPDFUnion with bad type");
    }
    that.fType = Type::kDestroyed;
}

SkPDFUnion& SkPDFUnion::operator=(SkPDFUnion&& that) {
    if (this != &that) {
        this->~SkPDFUnion();
        new (this) SkPDFUnion(std::move(that));
    }
    return *this;
}

bool SkPDFUnion::isName() const {
    return Type::kName == fType || Type::kNameSkS == fType;
}

#ifdef SK_DEBUG
// Most names need no escaping.  Such names are handled as static const strings.
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

// Given an arbitrary string, write it as a valid name (not including leading slash).
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

static void write_literal_byte_string(SkWStream* wStream, const char* cin, size_t len) {
    wStream->writeText("(");
    for (size_t i = 0; i < len; i++) {
        uint8_t c = static_cast<uint8_t>(cin[i]);
        if (c < ' ' || '~' < c) {
            uint8_t octal[4] = { '\\',
                                 (uint8_t)('0' | ( c >> 6        )),
                                 (uint8_t)('0' | ((c >> 3) & 0x07)),
                                 (uint8_t)('0' | ( c       & 0x07)) };
            wStream->write(octal, 4);
        } else {
            if (c == '\\' || c == '(' || c == ')') {
                wStream->writeText("\\");
            }
            wStream->write(&c, 1);
        }
    }
    wStream->writeText(")");
}

static void write_hex_byte_string(SkWStream* wStream, const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    wStream->writeText("<");
    for (size_t i = 0; i < len; i++) {
        uint8_t c = static_cast<uint8_t>(cin[i]);
        char hexValue[2] = { SkHexadecimalDigits::gUpper[c >> 4],
                             SkHexadecimalDigits::gUpper[c & 0xF] };
        wStream->write(hexValue, 2);
    }
    wStream->writeText(">");
}

static void write_optimized_byte_string(SkWStream* wStream, const char* cin, size_t len,
                                        size_t literalExtras) {
    const size_t hexLength     = 2 + 2*len;
    const size_t literalLength = 2 +   len + literalExtras;
    if (literalLength <= hexLength) {
        write_literal_byte_string(wStream, cin, len);
    } else {
        write_hex_byte_string(wStream, cin, len);
    }
}

static void write_byte_string(SkWStream* wStream, const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    size_t literalExtras = 0;
    {
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            if (c < ' ' || '~' < c) {
                literalExtras += 3;
            } else if (c == '\\' || c == '(' || c == ')') {
                ++literalExtras;
            }
        }
    }
    write_optimized_byte_string(wStream, cin, len, literalExtras);
}

static void write_text_string(SkWStream* wStream, const char* cin, size_t len) {
    SkDEBUGCODE(static const size_t kMaxLen = 65535;)
    SkASSERT(len <= kMaxLen);

    bool inputIsValidUTF8 = true;
    bool inputIsPDFDocEncoding = true;
    size_t literalExtras = 0;
    {
        const char* textPtr = cin;
        const char* textEnd = cin + len;
        while (textPtr < textEnd) {
            SkUnichar unichar = SkUTF::NextUTF8(&textPtr, textEnd);
            if (unichar < 0) {
                inputIsValidUTF8 = false;
                break;
            }
            // See Table D.2 (PDFDocEncoding Character Set) in the PDF3200_2008 spec.
            // Could convert from UTF-8 to PDFDocEncoding and, if successful, use that.
            if ((0x15 < unichar && unichar < 0x20) || 0x7E < unichar) {
                inputIsPDFDocEncoding = false;
                break;
            }
            if (unichar < ' ' || '~' < unichar) {
                literalExtras += 3;
            } else if (unichar == '\\' || unichar == '(' || unichar == ')') {
                ++literalExtras;
            }
        }
    }

    if (!inputIsValidUTF8) {
        SkDebugf("Invalid UTF8: %.*s\n", (int)len, cin);
        wStream->writeText("<>");
        return;
    }

    if (inputIsPDFDocEncoding) {
        write_optimized_byte_string(wStream, cin, len, literalExtras);
        return;
    }

    wStream->writeText("<FEFF");
    const char* textPtr = cin;
    const char* textEnd = cin + len;
    while (textPtr < textEnd) {
        SkUnichar unichar = SkUTF::NextUTF8(&textPtr, textEnd);
        SkPDFUtils::WriteUTF16beHex(wStream, unichar);
    }
    wStream->writeText(">");
}

void SkPDFWriteTextString(SkWStream* wStream, const char* cin, size_t len) {
    write_text_string(wStream, cin, len);
}
void SkPDFWriteByteString(SkWStream* wStream, const char* cin, size_t len) {
    write_byte_string(wStream, cin, len);
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
        case Type::kByteString:
            SkASSERT(fStaticString);
            write_byte_string(stream, fStaticString, strlen(fStaticString));
            return;
        case Type::kTextString:
            SkASSERT(fStaticString);
            write_text_string(stream, fStaticString, strlen(fStaticString));
            return;
        case Type::kNameSkS:
            stream->writeText("/");
            write_name_escaped(stream, fSkString.c_str());
            return;
        case Type::kByteStringSkS:
            write_byte_string(stream, fSkString.c_str(), fSkString.size());
            return;
        case Type::kTextStringSkS:
            write_text_string(stream, fSkString.c_str(), fSkString.size());
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

SkPDFUnion SkPDFUnion::Int(int32_t value) {
    return SkPDFUnion(Type::kInt, value);
}

SkPDFUnion SkPDFUnion::ColorComponent(uint8_t value) {
    return SkPDFUnion(Type::kColorComponent,  SkTo<int32_t>(value));
}

SkPDFUnion SkPDFUnion::ColorComponentF(float value) {
    return SkPDFUnion(Type::kColorComponentF, SkFloatToScalar(value));
}

SkPDFUnion SkPDFUnion::Bool(bool value) {
    return SkPDFUnion(Type::kBool, value);
}

SkPDFUnion SkPDFUnion::Scalar(SkScalar value) {
    return SkPDFUnion(Type::kScalar, value);
}

SkPDFUnion SkPDFUnion::Name(const char* value) {
    SkASSERT(value);
    SkASSERT(is_valid_name(value));
    return SkPDFUnion(Type::kName, value);
}

SkPDFUnion SkPDFUnion::ByteString(const char* value) {
    SkASSERT(value);
    return SkPDFUnion(Type::kByteString, value);
}

SkPDFUnion SkPDFUnion::TextString(const char* value) {
    SkASSERT(value);
    return SkPDFUnion(Type::kTextString, value);
}

SkPDFUnion SkPDFUnion::Name(SkString s) {
    return SkPDFUnion(Type::kNameSkS, std::move(s));
}

SkPDFUnion SkPDFUnion::ByteString(SkString s) {
    return SkPDFUnion(Type::kByteStringSkS, std::move(s));
}

SkPDFUnion SkPDFUnion::TextString(SkString s) {
    return SkPDFUnion(Type::kTextStringSkS, std::move(s));
}

SkPDFUnion SkPDFUnion::Object(std::unique_ptr<SkPDFObject> objSp) {
    SkASSERT(objSp.get());
    return SkPDFUnion(Type::kObject, std::move(objSp));
}

SkPDFUnion SkPDFUnion::Ref(SkPDFIndirectReference ref) {
    SkASSERT(ref.fValue > 0);
    return SkPDFUnion(Type::kRef, SkTo<int32_t>(ref.fValue));
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

void SkPDFArray::appendByteString(SkString value) {
    this->append(SkPDFUnion::ByteString(std::move(value)));
}

void SkPDFArray::appendTextString(SkString value) {
    this->append(SkPDFUnion::TextString(std::move(value)));
}

void SkPDFArray::appendByteString(const char value[]) {
    this->append(SkPDFUnion::ByteString(value));
}

void SkPDFArray::appendTextString(const char value[]) {
    this->append(SkPDFUnion::TextString(value));
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

void SkPDFDict::insertByteString(const char key[], const char value[]) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::ByteString(value));
}

void SkPDFDict::insertTextString(const char key[], const char value[]) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::TextString(value));
}

void SkPDFDict::insertByteString(const char key[], SkString value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::ByteString(std::move(value)));
}

void SkPDFDict::insertTextString(const char key[], SkString value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), SkPDFUnion::TextString(std::move(value)));
}

void SkPDFDict::insertUnion(const char key[], SkPDFUnion&& value) {
    fRecords.emplace_back(SkPDFUnion::Name(key), std::move(value));
}

////////////////////////////////////////////////////////////////////////////////



static void serialize_stream(SkPDFDict* origDict,
                             SkStreamAsset* stream,
                             SkPDFSteamCompressionEnabled compress,
                             SkPDFDocument* doc,
                             SkPDFIndirectReference ref) {
    // Code assumes that the stream starts at the beginning.
    SkASSERT(stream && stream->hasLength());

    std::unique_ptr<SkStreamAsset> tmp;
    SkPDFDict tmpDict;
    SkPDFDict& dict = origDict ? *origDict : tmpDict;
    static const size_t kMinimumSavings = strlen("/Filter_/FlateDecode_");
    if (doc->metadata().fCompressionLevel != SkPDF::Metadata::CompressionLevel::None &&
        compress == SkPDFSteamCompressionEnabled::Yes &&
        stream->getLength() > kMinimumSavings)
    {
        SkDynamicMemoryWStream compressedData;
        SkDeflateWStream deflateWStream(&compressedData,SkToInt(doc->metadata().fCompressionLevel));
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
                                      SkPDFSteamCompressionEnabled compress) {
    SkPDFIndirectReference ref = doc->reserveRef();
    if (SkExecutor* executor = doc->executor()) {
        SkPDFDict* dictPtr = dict.release();
        SkStreamAsset* contentPtr = content.release();
        // Pass ownership of both pointers into a std::function, which should
        // only be executed once.
        doc->incrementJobCount();
        executor->add([dictPtr, contentPtr, compress, doc, ref]() {
            serialize_stream(dictPtr, contentPtr, compress, doc, ref);
            delete dictPtr;
            delete contentPtr;
            doc->signalJobComplete();
        });
        return ref;
    }
    serialize_stream(dict.get(), content.get(), compress, doc, ref);
    return ref;
}
