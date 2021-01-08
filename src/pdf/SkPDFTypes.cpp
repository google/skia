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

////////////////////////////////////////////////////////////////////////////////

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

void SkPDFArray::emitObject(SkWStream* stream) const {
    stream->writeText("[");
    for (size_t i = 0; i < this->size(); i++) {
        (*this)[i].emitObject(stream);
        if (i + 1 < this->size()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

void SkPDFDict::emitObject(SkWStream* stream) const {
    stream->writeText("<<");
    for (size_t i = 0; i < this->size(); i++) {
        const std::pair<SkPDFObject, SkPDFObject>& record = (*this)[i];
        SkASSERT(std::holds_alternative<SkPDFStaticName>(record.first) ||
                 std::holds_alternative<SkPDFStringName>(record.first));
        record.first.emitObject(stream);
        stream->writeText(" ");
        record.second.emitObject(stream);
        if (i + 1 < this->size()) {
            stream->writeText("\n");
        }
    }
    stream->writeText(">>");
}

template <class> inline constexpr bool always_false_v = false;

void SkPDFObject::emitObject(SkWStream* stream) const {
    std::visit(
            [stream](auto&& arg) {
                using T = typename std::decay<decltype(arg)>::type;
                if constexpr (std::is_same_v<T, bool>) {
                    stream->writeText(arg ? "true" : "false");
                } else if constexpr (std::is_same_v<T, int32_t>) {
                    stream->writeDecAsText(arg);
                } else if constexpr (std::is_same_v<T, SkScalar>) {
                    SkPDFUtils::AppendScalar(arg, stream);
                } else if constexpr (std::is_same_v<T, SkPDFColorComponentU>) {
                    SkPDFUtils::AppendColorComponent(arg.fValue, stream);
                } else if constexpr (std::is_same_v<T, SkPDFColorComponentF>) {
                    SkPDFUtils::AppendColorComponentF(arg.fValue, stream);
                } else if constexpr (std::is_same_v<T, SkString>) {
                    write_string(stream, arg.c_str(), arg.size());
                } else if constexpr (std::is_same_v<T, const char*>) {
                    write_string(stream, arg, strlen(arg));
                } else if constexpr (std::is_same_v<T, SkPDFStringName>) {
                    stream->writeText("/");
                    write_name_escaped(stream, arg.fValue.c_str());
                } else if constexpr (std::is_same_v<T, SkPDFStaticName>) {
                    stream->writeText("/");
                    SkASSERT(is_valid_name(arg.fValue));
                    stream->writeText(arg.fValue);
                } else if constexpr (std::is_same_v<T, std::unique_ptr<SkPDFArray>>) {
                    SkASSERT(arg);
                    arg->emitObject(stream);
                } else if constexpr (std::is_same_v<T, std::unique_ptr<SkPDFDict>>) {
                    SkASSERT(arg);
                    arg->emitObject(stream);
                } else if constexpr (std::is_same_v<T, SkPDFIndirectReference>) {
                    SkASSERT(arg.fValue >= 0);
                    stream->writeDecAsText(arg.fValue);
                    stream->writeText(" 0 R");  // Generation number is always 0.
                    return;
                } else {
                    static_assert(always_false_v<std::decay_t<decltype(arg)>>,
                                  "non-exhaustive visitor!");
                }
            },
            *this);
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
