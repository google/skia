/*
 * Copyright (C) 2011 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "SkPDFCatalog.h"
#include "SkPDFTypes.h"
#include "SkStream.h"

#ifdef SK_BUILD_FOR_WIN
    #define SNPRINTF    _snprintf
#else
    #define SNPRINTF    snprintf
#endif

SkPDFObject::SkPDFObject() {}
SkPDFObject::~SkPDFObject() {}

size_t SkPDFObject::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkDynamicMemoryWStream buffer;
    emitObject(&buffer, catalog, indirect);
    return buffer.getOffset();
}

void SkPDFObject::getResources(SkTDArray<SkPDFObject*>* resourceList) {}

void SkPDFObject::emitIndirectObject(SkWStream* stream, SkPDFCatalog* catalog) {
    catalog->emitObjectNumber(stream, this);
    stream->writeText(" obj\n");
    emitObject(stream, catalog, false);
    stream->writeText("\nendobj\n");
}

SkPDFObjRef::SkPDFObjRef(SkPDFObject* obj) : fObj(obj) {}
SkPDFObjRef::~SkPDFObjRef() {}

size_t SkPDFObject::getIndirectOutputSize(SkPDFCatalog* catalog) {
    return catalog->getObjectNumberSize(this) + strlen(" obj\n") +
        this->getOutputSize(catalog, false) + strlen("\nendobj\n");
}

void SkPDFObjRef::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    SkASSERT(!indirect);
    catalog->emitObjectNumber(stream, fObj.get());
    stream->writeText(" R");
}

size_t SkPDFObjRef::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkASSERT(!indirect);
    return catalog->getObjectNumberSize(fObj.get()) + strlen(" R");
}

SkPDFInt::SkPDFInt(int32_t value) : fValue(value) {}
SkPDFInt::~SkPDFInt() {}

void SkPDFInt::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                          bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);
    stream->writeDecAsText(fValue);
}

SkPDFBool::SkPDFBool(bool value) : fValue(value) {}
SkPDFBool::~SkPDFBool() {}

void SkPDFBool::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                          bool indirect) {
    SkASSERT(!indirect);
    if (fValue) {
        stream->writeText("true");
    } else {
        stream->writeText("false");
    }
}

size_t SkPDFBool::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkASSERT(!indirect);
    if (fValue)
        return strlen("true");
    return strlen("false");
}

SkPDFScalar::SkPDFScalar(SkScalar value) : fValue(value) {}
SkPDFScalar::~SkPDFScalar() {}

void SkPDFScalar::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    Append(fValue, stream);
}

// static
void SkPDFScalar::Append(SkScalar value, SkWStream* stream) {
    // The range of reals in PDF/A is the same as SkFixed: +/- 32,767 and
    // +/- 1/65,536 (though integers can range from 2^31 - 1 to -2^31).
    // When using floats that are outside the whole value range, we can use
    // integers instead.


#if defined(SK_SCALAR_IS_FIXED)
    stream->writeScalarAsText(value);
    return;
#endif  // SK_SCALAR_IS_FIXED

#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    if (value > 32767 || value < -32767) {
        stream->writeDecAsText(SkScalarRound(value));
        return;
    }

    char buffer[SkStrAppendScalar_MaxSize];
    char* end = SkStrAppendFixed(buffer, SkScalarToFixed(value));
    stream->write(buffer, end - buffer);
    return;
#endif  // !SK_ALLOW_LARGE_PDF_SCALARS

#if defined(SK_SCALAR_IS_FLOAT) && defined(SK_ALLOW_LARGE_PDF_SCALARS)
    // Floats have 24bits of significance, so anything outside that range is
    // no more precise than an int. (Plus PDF doesn't support scientific
    // notation, so this clamps to SK_Max/MinS32).
    if (value > (1 << 24) || value < -(1 << 24)) {
        stream->writeDecAsText(value);
        return;
    }
    // Continue to enforce the PDF limits for small floats.
    if (value < 1.0f/65536 && value > -1.0f/65536) {
        stream->writeDecAsText(0);
        return;
    }
    // SkStrAppendFloat might still use scientific notation, so use snprintf
    // directly..
    static const int kFloat_MaxSize = 19;
    char buffer[kFloat_MaxSize];
    int len = SNPRINTF(buffer, kFloat_MaxSize, "%#.8f", value);
    // %f always prints trailing 0s, so strip them.
    for (; buffer[len - 1] == '0' && len > 0; len--) {
        buffer[len - 1] = '\0';
    }
    if (buffer[len - 1] == '.') {
        buffer[len - 1] = '\0';
    }
    stream->writeText(buffer);
    return;
#endif  // SK_SCALAR_IS_FLOAT && SK_ALLOW_LARGE_PDF_SCALARS
}

SkPDFString::SkPDFString(const char value[])
    : fValue(formatString(value, strlen(value))) {
}

SkPDFString::SkPDFString(const SkString& value)
    : fValue(formatString(value.c_str(), value.size())) {
}

SkPDFString::SkPDFString(const uint16_t* value, size_t len, bool wideChars)
    : fValue(formatString(value, len, wideChars)) {
}

SkPDFString::~SkPDFString() {}

void SkPDFString::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);
    stream->write(fValue.c_str(), fValue.size());
}

size_t SkPDFString::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);
    return fValue.size();
}

// static
SkString SkPDFString::formatString(const char* input, size_t len) {
    return doFormatString(input, len, false, false);
}

SkString SkPDFString::formatString(const uint16_t* input, size_t len,
                                   bool wideChars) {
    return doFormatString(input, len, true, wideChars);
}

// static
SkString SkPDFString::doFormatString(const void* input, size_t len,
                                     bool wideInput, bool wideOutput) {
    SkASSERT(len <= kMaxLen);
    const uint16_t* win = (const uint16_t*) input;
    const char* cin = (const char*) input;

    if (wideOutput) {
        SkASSERT(wideInput);
        SkString result;
        result.append("<");
        for (size_t i = 0; i < len; i++)
            result.appendHex(win[i], 4);
        result.append(">");
        return result;
    }

    // 7-bit clean is a heuristic to decide what string format to use;
    // a 7-bit clean string should require little escaping.
    bool sevenBitClean = true;
    for (size_t i = 0; i < len; i++) {
        SkASSERT(!wideInput || !(win[i] & ~0xFF));
        char val = wideInput ? win[i] : cin[i];
        if (val > '~' || val < ' ') {
            sevenBitClean = false;
            break;
        }
    }

    SkString result;
    if (sevenBitClean) {
        result.append("(");
        for (size_t i = 0; i < len; i++) {
            SkASSERT(!wideInput || !(win[i] & ~0xFF));
            char val = wideInput ? win[i] : cin[i];
            if (val == '\\' || val == '(' || val == ')')
                result.append("\\");
            result.append(&val, 1);
        }
        result.append(")");
    } else {
        result.append("<");
        for (size_t i = 0; i < len; i++) {
            SkASSERT(!wideInput || !(win[i] & ~0xFF));
            unsigned char val = wideInput ? win[i] : cin[i];
            result.appendHex(val, 2);
        }
        result.append(">");
    }

    return result;
}

SkPDFName::SkPDFName(const char name[]) : fValue(formatName(SkString(name))) {}
SkPDFName::SkPDFName(const SkString& name) : fValue(formatName(name)) {}
SkPDFName::~SkPDFName() {}

void SkPDFName::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                           bool indirect) {
    SkASSERT(!indirect);
    stream->write(fValue.c_str(), fValue.size());
}

size_t SkPDFName::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkASSERT(!indirect);
    return fValue.size();
}

// static
SkString SkPDFName::formatName(const SkString& input) {
    SkASSERT(input.size() <= kMaxLen);

    SkString result("/");
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] & 0x80 || input[i] < '!' || input[i] == '#') {
            result.append("#");
            result.appendHex(input[i], 2);
        } else {
            result.append(input.c_str() + i, 1);
        }
    }

    return result;
}

SkPDFArray::SkPDFArray() {}
SkPDFArray::~SkPDFArray() {
    fValue.safeUnrefAll();
}

void SkPDFArray::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                            bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    stream->writeText("[");
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i]->emitObject(stream, catalog, false);
        if (i + 1 < fValue.count())
            stream->writeText(" ");
    }
    stream->writeText("]");
}

size_t SkPDFArray::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    size_t result = strlen("[]");
    if (fValue.count())
        result += fValue.count() - 1;
    for (int i = 0; i < fValue.count(); i++)
        result += fValue[i]->getOutputSize(catalog, false);
    return result;
}

void SkPDFArray::reserve(int length) {
    SkASSERT(length <= kMaxLen);
    fValue.setReserve(length);
}

SkPDFObject* SkPDFArray::setAt(int offset, SkPDFObject* value) {
    SkASSERT(offset < fValue.count());
    SkSafeUnref(fValue[offset]);
    fValue[offset] = value;
    SkSafeRef(fValue[offset]);
    return value;
}

SkPDFObject* SkPDFArray::append(SkPDFObject* value) {
    SkASSERT(fValue.count() < kMaxLen);
    SkSafeRef(value);
    fValue.push(value);
    return value;
}

SkPDFDict::SkPDFDict() {}

SkPDFDict::SkPDFDict(const char type[]) {
    insert("Type", new SkPDFName(type))->unref();
}

SkPDFDict::~SkPDFDict() {
    clear();
}

void SkPDFDict::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                           bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);

    stream->writeText("<<");
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i].key->emitObject(stream, catalog, false);
        stream->writeText(" ");
        fValue[i].value->emitObject(stream, catalog, false);
        stream->writeText("\n");
    }
    stream->writeText(">>");
}

size_t SkPDFDict::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect)
        return getIndirectOutputSize(catalog);

    size_t result = strlen("<<>>") + (fValue.count() * 2);
    for (int i = 0; i < fValue.count(); i++) {
        result += fValue[i].key->getOutputSize(catalog, false);
        result += fValue[i].value->getOutputSize(catalog, false);
    }
    return result;
}

SkPDFObject* SkPDFDict::insert(SkPDFName* key, SkPDFObject* value) {
    struct Rec* newEntry = fValue.append();
    newEntry->key = key;
    SkSafeRef(newEntry->key);
    newEntry->value = value;
    SkSafeRef(newEntry->value);
    return value;
}

SkPDFObject* SkPDFDict::insert(const char key[], SkPDFObject* value) {
    SkRefPtr<SkPDFName> keyName = new SkPDFName(key);
    keyName->unref();  // SkRefPtr and new both took a reference.
    return insert(keyName.get(), value);
}

void SkPDFDict::clear() {
    for (int i = 0; i < fValue.count(); i++) {
        SkSafeUnref(fValue[i].key);
        SkSafeUnref(fValue[i].value);
    }
    fValue.reset();
}
