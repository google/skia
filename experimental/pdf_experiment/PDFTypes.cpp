// Copyright 2018 Google LLC.
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#include "PDFTypes.h"

#include "SkArenaAlloc.h"
#include "SkPDFUtils.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkUtils.h"

#include <type_traits>

static_assert(std::is_trivially_destructible<PDFInt>::value, "");
static_assert(std::is_trivially_destructible<PDFColorComponent>::value, "");
static_assert(std::is_trivially_destructible<PDFFloat>::value, "");
static_assert(std::is_trivially_destructible<PDFBool>::value, "");
static_assert(std::is_trivially_destructible<PDFString>::value, "");
static_assert(std::is_trivially_destructible<PDFName>::value, "");
static_assert(std::is_trivially_destructible<PDFIndirectReference>::value, "");
static_assert(std::is_trivially_destructible<PDFList>::value, "");
static_assert(std::is_trivially_destructible<PDFDict>::value, "");

////////////////////////////////////////////////////////////////////////////////

// Given an arbitrary string, write it as a valid name
static void write_name_escaped(SkWStream* stream, const char* name, size_t len) {
    stream->writeText("/");
    static constexpr char kToEscape[] = "#/%()<>[]{}";
    const char* stop = name + len;
    const char* span = name;
    while (name != stop) {
        uint8_t v = (uint8_t)(*name);
        if (v < '!' || v > '~' || strchr(kToEscape, v)) {
            char buffer[3] = {'#',
                              SkHexadecimalDigits::gUpper[v >> 4],
                              SkHexadecimalDigits::gUpper[v & 0xF]};
            if (span < name) {
                stream->write(span, name - span);
                span = name + 1;
            }
            stream->write(buffer, sizeof(buffer));
        }
        ++name;
    }
    if (span < stop) {
        stream->write(span, stop - span);
        span = name;
    }
}

static void write_string(SkWStream* stream, const char* cin, size_t len) {
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
        stream->writeText("(");
        for (size_t i = 0; i < len; i++) {
            if (cin[i] > '~' || cin[i] < ' ') {
                uint8_t c = static_cast<uint8_t>(cin[i]);
                uint8_t octal[4] = { '\\',
                                     (uint8_t)('0' | ( c >> 6        )),
                                     (uint8_t)('0' | ((c >> 3) & 0x07)),
                                     (uint8_t)('0' | ( c       & 0x07)) };
                stream->write(octal, 4);
            } else {
                if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
                    stream->writeText("\\");
                }
                stream->write(&cin[i], 1);
            }
        }
        stream->writeText(")");
    } else {
        stream->writeText("<");
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            char hexValue[2] = { SkHexadecimalDigits::gUpper[c >> 4],
                                 SkHexadecimalDigits::gUpper[c & 0xF] };
            stream->write(hexValue, 2);
        }
        stream->writeText(">");
    }
}

void PDFInt::emit(SkWStream* stream) const {
    char buffer[SkStrAppendS32_MaxSize];
    stream->write(buffer, SkStrAppendS32(buffer, fValue) - buffer);
}

void PDFColorComponent::emit(SkWStream* stream) const {
    char buffer[SkPDFUtils::kFloatColorDecimalCount + 2];
    stream->write(buffer, SkPDFUtils::ColorToDecimalF(fValue, buffer));
}

void PDFFloat::emit(SkWStream* stream) const {
    char buffer[kMaximumSkFloatToDecimalLength];
    stream->write(buffer, SkFloatToDecimal(fValue, buffer));
}

void PDFBool::emit(SkWStream* stream) const {
    stream->writeText(fValue ? "true" : "false");
}

void PDFString::emit(SkWStream* stream) const {
    write_string(stream, fValue.fStart, fValue.fLength);
}

void PDFName::emit(SkWStream* stream) const {
    write_name_escaped(stream, fValue.fStart, fValue.fLength);
}

void PDFIndirectReference::emit(SkWStream* stream) const {
    stream->writeDecAsText(fValue.fValue);
    stream->writeText(" 0 R");
}

PDFList::PDFList(SkArenaAlloc* arena, size_t maximumSize)
    : PDFListImpl(arena->makeArrayDefault<PDFObject*>(maximumSize), maximumSize) {}

void PDFListImpl::emit(SkWStream* stream) const {
    stream->writeText("[");
    for (size_t i = 0; i < fSize; ++i) {
        fValues[i]->emit(stream);
        if (i + 1 != fSize) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

PDFList* PDFList::Make(SkArenaAlloc* arena, size_t maximumSize) {
    return arena->make<PDFList>(arena, maximumSize);
}

PDFDict::PDFDict(SkArenaAlloc* arena, size_t maximumSize)
   : PDFDictImpl(arena->makeArrayDefault<Record>(maximumSize), maximumSize) {}

void PDFDictImpl::emit(SkWStream* stream) const {
    stream->writeText("<<");
    this->innerEmit(stream);
    stream->writeText(">>");
}
void PDFDictImpl::innerEmit(SkWStream* stream) const {
    for (size_t i = 0; i < fSize; ++i) {
        PDFName{fRecords[i].fKey}.emit(stream);
        stream->writeText(" ");
        fRecords[i].fValue->emit(stream);
        if (i + 1 != fSize) {
            stream->writeText("\n");
        }
    }
}

PDFDict* PDFDict::Make(SkArenaAlloc* arena, size_t maximumSize) {
    return arena->make<PDFDict>(arena, maximumSize);
}
