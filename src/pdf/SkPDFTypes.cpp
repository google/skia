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
#include "include/private/SkTo.h"
#include "src/core/SkStreamPriv.h"
#include "src/pdf/SkDeflate.h"
#include "src/pdf/SkPDFDocumentPriv.h"
#include "src/pdf/SkPDFUtils.h"

#include <new>

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

///////////////////////////////////////////////////////////////////////////////

SkPDFArray::SkPDFArray() = default;

SkPDFArray::~SkPDFArray() = default;

void SkPDFArray::reserve(size_t s) { fArray.reserve(s); }

size_t SkPDFArray::size() const { return fArray.size(); }

void SkPDFArray::emplace(SkPDFObject v) { fArray.push_back(std::move(v)); }

SkPDFDict::SkPDFDict() = default;

SkPDFDict::~SkPDFDict() = default;

void SkPDFDict::reserve(size_t s) { fRecords.reserve(s); }

void SkPDFDict::emplace(SkPDFObject k, SkPDFObject v) {
    fRecords.emplace_back(std::move(k), std::move(v));
}

////////////////////////////////////////////////////////////////////////////////

static void emit(bool v, SkWStream* s) { s->writeText(v ? "true" : "false"); }

static void emit(int32_t v, SkWStream* s) { s->writeDecAsText(v); }

static void emit(SkScalar v, SkWStream* s) { SkPDFUtils::AppendScalar(v, s); }

static void emit(SkPDFColorComponentU v, SkWStream* s) {
    SkPDFUtils::AppendColorComponent(v.fValue, s);
}

static void emit(SkPDFColorComponentF v, SkWStream* s) {
    SkPDFUtils::AppendColorComponentF(v.fValue, s);
}

static void emit(const SkString& v, SkWStream* s) { write_string(s, v.c_str(), v.size()); }

static void emit(const char* v, SkWStream* s) { write_string(s, v, strlen(v)); }

static void emit(const SkPDFStringName& v, SkWStream* s) {
    s->writeText("/");
    write_name_escaped(s, v.fValue.c_str());
}

static void emit(SkPDFStaticName v, SkWStream* s) {
    s->writeText("/");
    SkASSERT(is_valid_name(v.fValue));
    s->writeText(v.fValue);
}

static void emit(const std::unique_ptr<SkPDFArray>& v, SkWStream* s) {
    SkASSERT(v);
    SkPDFEmit(*v, s);
}

static void emit(const std::unique_ptr<SkPDFDict>& v, SkWStream* s) {
    SkASSERT(v);
    SkPDFEmit(*v, s);
}

static void emit(SkPDFIndirectReference v, SkWStream* s) {
    SkASSERT(v);
    s->writeDecAsText(v.fValue);
    s->writeText(" 0 R");
}

void SkPDFEmit(const SkPDFDict& dict, SkWStream* stream) {
    const auto& records = dict.elements();
    stream->writeText("<<");
    for (size_t i = 0; i < records.size(); i++) {
        const std::pair<SkPDFObject, SkPDFObject>& record = records[i];
        SkASSERT(skstd::holds_alternative<SkPDFStaticName>(record.first) ||
                 skstd::holds_alternative<SkPDFStringName>(record.first));
        SkPDFEmit(record.first, stream);
        stream->writeText(" ");
        SkPDFEmit(record.second, stream);
        if (i + 1 < records.size()) {
            stream->writeText("\n");
        }
    }
    stream->writeText(">>");
}

void SkPDFEmit(const SkPDFArray& array, SkWStream* stream) {
    const auto& vec = array.elements();
    stream->writeText("[");
    for (size_t i = 0; i < vec.size(); i++) {
        SkPDFEmit(vec[i], stream);
        if (i + 1 < vec.size()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

void SkPDFEmit(const SkPDFObject& obj, SkWStream* stream) {
    skstd::visit([stream](const auto& v) { emit(v, stream); }, obj);
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
    doc->emitStream(
            dict, [stream](SkWStream* dst) { dst->writeStream(stream, stream->getLength()); }, ref);
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
