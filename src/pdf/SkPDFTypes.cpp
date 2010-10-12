/*
 * Copyright (C) 2010 The Android Open Source Project
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

SkPDFObject::SkPDFObject() {}
SkPDFObject::~SkPDFObject() {}

size_t SkPDFObject::getOutputSize(SkPDFCatalog* catalog, bool indirect) {
    SkDynamicMemoryWStream buffer;
    emitObject(&buffer, catalog, indirect);
    return buffer.getOffset();
}

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

SkPDFScalar::SkPDFScalar(SkScalar value) : fValue(value) {}
SkPDFScalar::~SkPDFScalar() {}

void SkPDFScalar::emitObject(SkWStream* stream, SkPDFCatalog* catalog,
                             bool indirect) {
    if (indirect)
        return emitIndirectObject(stream, catalog);
    stream->writeScalarAsText(fValue);
}

SkPDFString::SkPDFString(const char value[])
    : fValue(formatString(SkString(value))) {
}

SkPDFString::SkPDFString(const SkString& value)
    : fValue(formatString(value)) {
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

SkString SkPDFString::formatString(const SkString& input) {
    SkASSERT(input.size() <= kMaxLen);

    // 7-bit clean is a heuristic to decide what string format to use;
    // a 7-bit clean string should require little escaping.
    bool sevenBitClean = true;
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] & 0x80 || input[i] < ' ') {
            sevenBitClean = false;
            break;
        }
    }

    SkString result;
    if (sevenBitClean) {
        result.append("(");
        for (size_t i = 0; i < input.size(); i++) {
            if (input[i] == '\\' || input[i] == '(' || input[i] == ')')
                result.append("\\");
            result.append(input.c_str() + i, 1);
        }
        result.append(")");
    } else {
        result.append("<");
        for (size_t i = 0; i < input.size(); i++)
            result.appendHex(input[i], 2);
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

void SkPDFArray::setAt(int offset, SkPDFObject* value) {
    SkASSERT(offset < fValue.count());
    SkSafeUnref(fValue[offset]);
    fValue[offset] = value;
    SkSafeRef(fValue[offset]);
}

void SkPDFArray::append(SkPDFObject* value) {
    SkASSERT(fValue.count() < kMaxLen);
    SkSafeRef(value);
    fValue.push(value);
}

SkPDFDict::SkPDFDict() {}

SkPDFDict::SkPDFDict(const char type[]) {
    SkRefPtr<SkPDFName> typeName = new SkPDFName(type);
    typeName->unref();  // SkRefPtr and new both took a reference.
    insert("Type", typeName.get());
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

void SkPDFDict::insert(SkPDFName* key, SkPDFObject* value) {
    struct Rec* newEntry = fValue.append();
    newEntry->key = key;
    SkSafeRef(newEntry->key);
    newEntry->value = value;
    SkSafeRef(newEntry->value);
}

void SkPDFDict::insert(const char key[], SkPDFObject* value) {
    SkRefPtr<SkPDFName> keyName = new SkPDFName(key);
    keyName->unref();  // SkRefPtr and new both took a reference.
    insert(keyName.get(), value);
}

void SkPDFDict::clear() {
    for (int i = 0; i < fValue.count(); i++) {
        fValue[i].key->safeUnref();
        fValue[i].value->safeUnref();
    }
    fValue.reset();
}
