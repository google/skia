
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkPDFTypes.h"
#include "SkStream.h"

#ifdef SK_BUILD_FOR_WIN
    #define SNPRINTF    _snprintf
#else
    #define SNPRINTF    snprintf
#endif

////////////////////////////////////////////////////////////////////////////////

SkPDFObjRef::SkPDFObjRef(SkPDFObject* obj) : fObj(obj) {
    SkSafeRef(obj);
}

SkPDFObjRef::~SkPDFObjRef() {}

void SkPDFObjRef::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap& objNumMap,
                             const SkPDFSubstituteMap& substitutes) {
    SkPDFObject* obj = substitutes.getSubstitute(fObj);
    stream->writeDecAsText(objNumMap.getObjectNumber(obj));
    stream->writeText(" 0 R");  // Generation number is always 0.
}

void SkPDFObjRef::addResources(SkPDFObjNumMap* catalog,
                               const SkPDFSubstituteMap& substitutes) const {
    SkPDFObject* obj = substitutes.getSubstitute(fObj);
    SkASSERT(obj);
    if (catalog->addObject(obj)) {
        obj->addResources(catalog, substitutes);
    }
}

////////////////////////////////////////////////////////////////////////////////

SkPDFInt::SkPDFInt(int32_t value) : fValue(value) {}
SkPDFInt::~SkPDFInt() {}

void SkPDFInt::emitObject(SkWStream* stream,
                          const SkPDFObjNumMap&,
                          const SkPDFSubstituteMap&) {
    stream->writeDecAsText(fValue);
}

////////////////////////////////////////////////////////////////////////////////

SkPDFBool::SkPDFBool(bool value) : fValue(value) {}
SkPDFBool::~SkPDFBool() {}

void SkPDFBool::emitObject(SkWStream* stream,
                          const SkPDFObjNumMap&,
                          const SkPDFSubstituteMap&) {
    stream->writeText(fValue ? "true" : "false");
}

////////////////////////////////////////////////////////////////////////////////

SkPDFScalar::SkPDFScalar(SkScalar value) : fValue(value) {}
SkPDFScalar::~SkPDFScalar() {}

void SkPDFScalar::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap&,
                             const SkPDFSubstituteMap&) {
    SkPDFScalar::Append(fValue, stream);
}

// static
void SkPDFScalar::Append(SkScalar value, SkWStream* stream) {
    // The range of reals in PDF/A is the same as SkFixed: +/- 32,767 and
    // +/- 1/65,536 (though integers can range from 2^31 - 1 to -2^31).
    // When using floats that are outside the whole value range, we can use
    // integers instead.

#if !defined(SK_ALLOW_LARGE_PDF_SCALARS)
    if (value > 32767 || value < -32767) {
        stream->writeDecAsText(SkScalarRoundToInt(value));
        return;
    }

    char buffer[SkStrAppendScalar_MaxSize];
    char* end = SkStrAppendFixed(buffer, SkScalarToFixed(value));
    stream->write(buffer, end - buffer);
    return;
#endif  // !SK_ALLOW_LARGE_PDF_SCALARS

#if defined(SK_ALLOW_LARGE_PDF_SCALARS)
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
#endif  // SK_ALLOW_LARGE_PDF_SCALARS
}

////////////////////////////////////////////////////////////////////////////////

SkPDFString::SkPDFString(const char value[])
    : fValue(FormatString(value, strlen(value))) {
}

SkPDFString::SkPDFString(const SkString& value)
    : fValue(FormatString(value.c_str(), value.size())) {
}

SkPDFString::~SkPDFString() {}

void SkPDFString::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap&,
                             const SkPDFSubstituteMap&) {
    stream->write(fValue.c_str(), fValue.size());
}

// static
SkString SkPDFString::FormatString(const char* cin, size_t len) {
    SkASSERT(len <= kMaxLen);

    // 7-bit clean is a heuristic to decide what string format to use;
    // a 7-bit clean string should require little escaping.
    bool sevenBitClean = true;
    size_t characterCount = 2 + len;
    for (size_t i = 0; i < len; i++) {
        if (cin[i] > '~' || cin[i] < ' ') {
            sevenBitClean = false;
            break;
        }
        if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
            ++characterCount;
        }
    }
    SkString result;
    if (sevenBitClean) {
        result.resize(characterCount);
        char* str = result.writable_str();
        *str++ = '(';
        for (size_t i = 0; i < len; i++) {
            if (cin[i] == '\\' || cin[i] == '(' || cin[i] == ')') {
                *str++ = '\\';
            }
            *str++ = cin[i];
        }
        *str++ = ')';
    } else {
        result.resize(2 * len + 2);
        char* str = result.writable_str();
        *str++ = '<';
        for (size_t i = 0; i < len; i++) {
            uint8_t c = static_cast<uint8_t>(cin[i]);
            static const char gHex[] = "0123456789ABCDEF";
            *str++ = gHex[(c >> 4) & 0xF];
            *str++ = gHex[(c     ) & 0xF];
        }
        *str++ = '>';
    }
    return result;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFName::SkPDFName(const char name[]) : fValue(FormatName(SkString(name))) {}
SkPDFName::SkPDFName(const SkString& name) : fValue(FormatName(name)) {}
SkPDFName::~SkPDFName() {}

bool SkPDFName::operator==(const SkPDFName& b) const {
    return fValue == b.fValue;
}

void SkPDFName::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap&,
                             const SkPDFSubstituteMap&) {
    stream->write(fValue.c_str(), fValue.size());
}

// static
SkString SkPDFName::FormatName(const SkString& input) {
    SkASSERT(input.size() <= kMaxLen);
    // TODO(vandebo) If more escaping is needed, improve the linear scan.
    static const char escaped[] = "#/%()<>[]{}";

    SkString result("/");
    for (size_t i = 0; i < input.size(); i++) {
        if (input[i] & 0x80 || input[i] < '!' || strchr(escaped, input[i])) {
            result.append("#");
            // Mask with 0xFF to avoid sign extension. i.e. #FFFFFF81
            result.appendHex(input[i] & 0xFF, 2);
        } else {
            result.append(input.c_str() + i, 1);
        }
    }

    return result;
}

////////////////////////////////////////////////////////////////////////////////

SkPDFArray::SkPDFArray() {}
SkPDFArray::~SkPDFArray() {
    fValue.unrefAll();
}

void SkPDFArray::emitObject(SkWStream* stream,
                             const SkPDFObjNumMap& objNumMap,
                             const SkPDFSubstituteMap& substitutes) {
    stream->writeText("[");
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(substitutes.getSubstitute(fValue[i]) == fValue[i]);
        fValue[i]->emitObject(stream, objNumMap, substitutes);
        if (i + 1 < fValue.count()) {
            stream->writeText(" ");
        }
    }
    stream->writeText("]");
}

void SkPDFArray::addResources(SkPDFObjNumMap* catalog,
                              const SkPDFSubstituteMap& substitutes) const {
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(substitutes.getSubstitute(fValue[i]) == fValue[i]);
        fValue[i]->addResources(catalog, substitutes);
    }
}

void SkPDFArray::reserve(int length) {
    SkASSERT(length <= kMaxLen);
    fValue.setReserve(length);
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

int SkPDFDict::size() const {
    return fValue.count();
}

void SkPDFDict::emitObject(SkWStream* stream,
                           const SkPDFObjNumMap& objNumMap,
                           const SkPDFSubstituteMap& substitutes) {
    stream->writeText("<<");
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(fValue[i].key);
        SkASSERT(fValue[i].value);
        SkASSERT(substitutes.getSubstitute(fValue[i].key) == fValue[i].key);
        SkASSERT(substitutes.getSubstitute(fValue[i].value) == fValue[i].value);
        fValue[i].key->emitObject(stream, objNumMap, substitutes);
        stream->writeText(" ");
        fValue[i].value->emitObject(stream, objNumMap, substitutes);
        if (i + 1 < fValue.count()) {
            stream->writeText("\n");
        }
    }
    stream->writeText(">>");
}

void SkPDFDict::addResources(SkPDFObjNumMap* catalog,
                             const SkPDFSubstituteMap& substitutes) const {
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(fValue[i].key);
        SkASSERT(fValue[i].value);
        fValue[i].key->addResources(catalog, substitutes);
        SkASSERT(substitutes.getSubstitute(fValue[i].value) == fValue[i].value);
        fValue[i].value->addResources(catalog, substitutes);
    }
}

SkPDFObject*  SkPDFDict::append(SkPDFName* key, SkPDFObject* value) {
    SkASSERT(key);
    SkASSERT(value);
    *(fValue.append()) = Rec(key, value);
    return value;
}

SkPDFObject* SkPDFDict::insert(SkPDFName* key, SkPDFObject* value) {
    return this->append(SkRef(key), SkRef(value));
}

SkPDFObject* SkPDFDict::insert(const char key[], SkPDFObject* value) {
    return this->append(new SkPDFName(key), SkRef(value));
}

void SkPDFDict::insertInt(const char key[], int32_t value) {
    (void)this->append(new SkPDFName(key), new SkPDFInt(value));
}

void SkPDFDict::insertScalar(const char key[], SkScalar value) {
    (void)this->append(new SkPDFName(key), new SkPDFScalar(value));
}

void SkPDFDict::insertName(const char key[], const char name[]) {
    (void)this->append(new SkPDFName(key), new SkPDFName(name));
}

void SkPDFDict::clear() {
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(fValue[i].key);
        SkASSERT(fValue[i].value);
        fValue[i].key->unref();
        fValue[i].value->unref();
    }
    fValue.reset();
}

void SkPDFDict::remove(const char key[]) {
    SkASSERT(key);
    SkPDFName name(key);
    for (int i = 0; i < fValue.count(); i++) {
        SkASSERT(fValue[i].key);
        if (*(fValue[i].key) == name) {
            fValue[i].key->unref();
            SkASSERT(fValue[i].value);
            fValue[i].value->unref();
            fValue.removeShuffle(i);
            return;
        }
    }
}

void SkPDFDict::mergeFrom(const SkPDFDict& other) {
    for (int i = 0; i < other.fValue.count(); i++) {
        *(fValue.append()) =
                Rec(SkRef(other.fValue[i].key), SkRef(other.fValue[i].value));
    }
}

////////////////////////////////////////////////////////////////////////////////

SkPDFSubstituteMap::~SkPDFSubstituteMap() {
    fSubstituteMap.foreach(
            [](SkPDFObject*, SkPDFObject** v) { (*v)->unref(); });
}

void SkPDFSubstituteMap::setSubstitute(SkPDFObject* original,
                                       SkPDFObject* substitute) {
    SkASSERT(original != substitute);
    SkASSERT(!fSubstituteMap.find(original));
    fSubstituteMap.set(original, SkRef(substitute));
}

SkPDFObject* SkPDFSubstituteMap::getSubstitute(SkPDFObject* object) const {
    SkPDFObject** found = fSubstituteMap.find(object);
    return found ? *found : object;
}

////////////////////////////////////////////////////////////////////////////////

bool SkPDFObjNumMap::addObject(SkPDFObject* obj) {
    if (fObjectNumbers.find(obj)) {
        return false;
    }
    fObjectNumbers.set(obj, fObjectNumbers.count() + 1);
    fObjects.push(obj);
    return true;
}

int32_t SkPDFObjNumMap::getObjectNumber(SkPDFObject* obj) const {
    int32_t* objectNumberFound = fObjectNumbers.find(obj);
    SkASSERT(objectNumberFound);
    return *objectNumberFound;
}

