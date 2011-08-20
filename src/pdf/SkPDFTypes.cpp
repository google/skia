
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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

void SkPDFObject::emit(SkWStream* stream, SkPDFCatalog* catalog,
                       bool indirect) {
    SkPDFObject* realObject = catalog->getSubstituteObject(this);
    return realObject->emitObject(stream, catalog, indirect);
}

size_t SkPDFObject::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkDynamicMemoryWStream buffer;
    emit(&buffer, catalog, indirect);
    return buffer.getOffset();
}

void SkPDFObject::getResources(SkTDArray<SkPDFObject*>* resourceList) {}

void SkPDFObject::emitIndirectObject(SkWStream* stream, SkPDFCatalog* catalog) {
    catalog->emitObjectNumber(stream, this);
    stream->writeText(" obj\n");
    emit(stream, catalog, false);
    stream->writeText("\nendobj\n");
}

size_t SkPDFObject::getIndirectOutputSize(SkPDFCatalog* catalog) {
    return catalog->getObjectNumberSize(this) + strlen(" obj\n") +
        this->getOutputSize(catalog, false) + strlen("\nendobj\n");
}

void SkPDFObject::AddResourceHelper(SkPDFObject* resource,
                                    SkTDArray<SkPDFObject*>* list) {
    list->push(resource);
    resource->ref();
}

void SkPDFObject::GetResourcesHelper(SkTDArray<SkPDFObject*>* resources,
                                     SkTDArray<SkPDFObject*>* result) {
    if (resources->count()) {
        result->setReserve(result->count() + resources->count());
        for (int i = 0; i < resources->count(); i++) {
            result->push((*resources)[i]);
            (*resources)[i]->ref();
            (*resources)[i]->getResources(result);
        }
    }
}

SkPDFObjRef::SkPDFObjRef(SkPDFObject* obj) : fObj(obj) {}
SkPDFObjRef::~SkPDFObjRef() {}

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
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }
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
    if (fValue) {
        return strlen("true");
    }
    return strlen("false");
}

SkPDFScalar::SkPDFScalar(SkScalar value) : fValue(value) {}
SkPDFScalar::~SkPDFScalar() {}

void SkPDFScalar::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }

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
    : fValue(FormatString(value, strlen(value))) {
}

SkPDFString::SkPDFString(const SkString& value)
    : fValue(FormatString(value.c_str(), value.size())) {
}

SkPDFString::SkPDFString(const uint16_t* value, size_t len, bool wideChars)
    : fValue(FormatString(value, len, wideChars)) {
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
SkString SkPDFString::FormatString(const char* input, size_t len) {
    return DoFormatString(input, len, false, false);
}

SkString SkPDFString::FormatString(const uint16_t* input, size_t len,
                                   bool wideChars) {
    return DoFormatString(input, len, true, wideChars);
}

// static
SkString SkPDFString::DoFormatString(const void* input, size_t len,
                                     bool wideInput, bool wideOutput) {
    SkASSERT(len <= kMaxLen);
    const uint16_t* win = (const uint16_t*) input;
    const char* cin = (const char*) input;

    if (wideOutput) {
        SkASSERT(wideInput);
        SkString result;
        result.append("<");
        for (size_t i = 0; i < len; i++) {
            result.appendHex(win[i], 4);
        }
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
            if (val == '\\' || val == '(' || val == ')') {
                result.append("\\");
            }
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

SkPDFName::SkPDFName(const char name[]) : fValue(FormatName(SkString(name))) {}
SkPDFName::SkPDFName(const SkString& name) : fValue(FormatName(name)) {}
SkPDFName::~SkPDFName() {}

bool SkPDFName::operator==(const SkPDFName& b) const {
    return fValue == b.fValue;
}

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
SkString SkPDFName::FormatName(const SkString& input) {
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
    fValue.unrefAll();
}

void SkPDFArray::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                            bool indirect) {
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }

    stream->writeText("[");
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i]->emit(stream, catalog, false);
        if (i + 1 < fValue.count()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

size_t SkPDFArray::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect) {
        return getIndirectOutputSize(catalog);
    }

    size_t result = strlen("[]");
    if (fValue.count()) {
        result += fValue.count() - 1;
    }
    for (int i = 0; i < fValue.count(); i++) {
        result += fValue[i]->getOutputSize(catalog, false);
    }
    return result;
}

void SkPDFArray::reserve(int length) {
    SkASSERT(length <= kMaxLen);
    fValue.setReserve(length);
}

SkPDFObject* SkPDFArray::setAt(int offset, SkPDFObject* value) {
    SkASSERT(offset < fValue.count());
    value->ref();
    fValue[offset]->unref();
    fValue[offset] = value;
    return value;
}

SkPDFObject* SkPDFArray::append(SkPDFObject* value) {
    SkASSERT(fValue.count() < kMaxLen);
    value->ref();
    fValue.push(value);
    return value;
}

void SkPDFArray::appendInt(int32_t value) {
    SkASSERT(fValue.count() < kMaxLen);
    fValue.push(new SkPDFInt(value));
}

void SkPDFArray::appendScalar(SkScalar value) {
    SkASSERT(fValue.count() < kMaxLen);
    fValue.push(new SkPDFScalar(value));
}

void SkPDFArray::appendName(const char name[]) {
    SkASSERT(fValue.count() < kMaxLen);
    fValue.push(new SkPDFName(name));
}

///////////////////////////////////////////////////////////////////////////////

SkPDFDict::SkPDFDict() {}

SkPDFDict::SkPDFDict(const char type[]) {
    insertName("Type", type);
}

SkPDFDict::~SkPDFDict() {
    clear();
}

void SkPDFDict::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                           bool indirect) {
    if (indirect) {
        return emitIndirectObject(stream, catalog);
    }

    stream->writeText("<<");
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i].key->emitObject(stream, catalog, false);
        stream->writeText(" ");
        fValue[i].value->emit(stream, catalog, false);
        stream->writeText("\n");
    }
    stream->writeText(">>");
}

size_t SkPDFDict::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    if (indirect) {
        return getIndirectOutputSize(catalog);
    }

    size_t result = strlen("<<>>") + (fValue.count() * 2);
    for (int i = 0; i < fValue.count(); i++) {
        result += fValue[i].key->getOutputSize(catalog, false);
        result += fValue[i].value->getOutputSize(catalog, false);
    }
    return result;
}

SkPDFObject* SkPDFDict::insert(SkPDFName* key, SkPDFObject* value) {
    key->ref();
    value->ref();
    struct Rec* newEntry = fValue.append();
    newEntry->key = key;
    newEntry->value = value;
    return value;
}

SkPDFObject* SkPDFDict::insert(const char key[], SkPDFObject* value) {
    value->ref();
    struct Rec* newEntry = fValue.append();
    newEntry->key = new SkPDFName(key);
    newEntry->value = value;
    return value;
}

void SkPDFDict::insertInt(const char key[], int32_t value) {
    struct Rec* newEntry = fValue.append();
    newEntry->key = new SkPDFName(key);
    newEntry->value = new SkPDFInt(value);
}

void SkPDFDict::insertScalar(const char key[], SkScalar value) {
    struct Rec* newEntry = fValue.append();
    newEntry->key = new SkPDFName(key);
    newEntry->value = new SkPDFScalar(value);
}

void SkPDFDict::insertName(const char key[], const char name[]) {
    struct Rec* newEntry = fValue.append();
    newEntry->key = new SkPDFName(key);
    newEntry->value = new SkPDFName(name);
}

void SkPDFDict::clear() {
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i].key->unref();
        fValue[i].value->unref();
    }
    fValue.reset();
}

SkPDFDict::Iter::Iter(const SkPDFDict& dict)
    : fIter(dict.fValue.begin()),
      fStop(dict.fValue.end()) {
}

SkPDFName* SkPDFDict::Iter::next(SkPDFObject** value) {
    if (fIter != fStop) {
        Rec* cur = fIter;
        fIter++;
        *value = cur->value;
        return cur->key;
    }
    *value = NULL;
    return NULL;
}
