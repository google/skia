/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieJson.h"

#include "SkData.h"
#include "SkScalar.h"
#include "SkPath.h"
#include "SkPoint.h"
#include "SkStream.h"
#include "SkString.h"
#include "SkottieValue.h"

#include "rapidjson/error/en.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <vector>

namespace skottie {

namespace json {

template <>
bool ValueRef::to<SkScalar>(SkScalar* v) const {
    if (!fValue) return false;

    // Some versions wrap values as single-element arrays.
    if (fValue->IsArray() && fValue->Size() == 1) {
        return ValueRef(fValue->operator[](0)).to(v);
    }

    if (!fValue->IsNumber())
        return false;

    *v = static_cast<SkScalar>(fValue->GetDouble());

    return true;
}

template <>
bool ValueRef::to<bool>(bool* v) const {
    if (!fValue) return false;

    switch(fValue->GetType()) {
    case rapidjson::kNumberType:
        *v = SkToBool(fValue->GetDouble());
        return true;
    case rapidjson::kFalseType:
    case rapidjson::kTrueType:
        *v = fValue->GetBool();
        return true;
    default:
        break;
    }

    return false;
}

template <>
bool ValueRef::to<int>(int* v) const {
    if (!fValue || !fValue->IsInt())
        return false;

    *v = fValue->GetInt();

    return true;
}

template <>
bool ValueRef::to<SkString>(SkString* v) const {
    if (!fValue || !fValue->IsString())
        return false;

    v->set(fValue->GetString());

    return true;
}

template <>
bool ValueRef::to<SkPoint>(SkPoint* v) const {
    if (!fValue || !fValue->IsObject())
        return false;

    const auto jvx = ValueRef(this->operator[]("x")),
               jvy = ValueRef(this->operator[]("y"));

    // Some BM versions seem to store x/y as single-element arrays.
    return ValueRef(jvx.isArray() ? jvx.operator[](size_t(0)) : jvx).to(&v->fX)
        && ValueRef(jvy.isArray() ? jvy.operator[](size_t(0)) : jvy).to(&v->fY);
}

template <>
bool ValueRef::to<std::vector<float>>(std::vector<float>* v) const {
    if (!fValue || !fValue->IsArray())
        return false;

    v->resize(fValue->Size());
    for (size_t i = 0; i < fValue->Size(); ++i) {
        if (!ValueRef(fValue->operator[](i)).to(v->data() + i)) {
            return false;
        }
    }

    return true;
}

namespace {

bool ParsePointVec(const ValueRef& jv, std::vector<SkPoint>* pts) {
    if (!jv.isArray())
        return false;

    pts->clear();
    pts->reserve(jv.size());

    std::vector<float> vec;
    for (size_t i = 0; i < jv.size(); ++i) {
        if (!jv[i].to(&vec) || vec.size() != 2)
            return false;
        pts->push_back(SkPoint::Make(vec[0], vec[1]));
    }

    return true;
}

} // namespace

template <>
bool ValueRef::to<ShapeValue>(ShapeValue* v) const {
    SkASSERT(v->fVertices.empty());

    if (!fValue)
        return false;

    // Some versions wrap values as single-element arrays.
    if (fValue->IsArray() && fValue->Size() == 1) {
        return ValueRef(fValue->operator[](0)).to(v);
    }

    std::vector<SkPoint> inPts,  // Cubic Bezier "in" control points, relative to vertices.
                         outPts, // Cubic Bezier "out" control points, relative to vertices.
                         verts;  // Cubic Bezier vertices.

    if (!fValue->IsObject() ||
        !ParsePointVec(this->operator[]("i"), &inPts) ||
        !ParsePointVec(this->operator[]("o"), &outPts) ||
        !ParsePointVec(this->operator[]("v"), &verts) ||
        inPts.size() != outPts.size() ||
        inPts.size() != verts.size()) {

        return false;
    }

    v->fVertices.reserve(inPts.size());
    for (size_t i = 0; i < inPts.size(); ++i) {
        v->fVertices.push_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }
    v->fClosed = this->operator[]("c").toDefault<bool>(false);

    return true;
}

size_t ValueRef::size() const {
    return this->isArray() ? fValue->Size() : 0;
}

ValueRef ValueRef::operator[](size_t i) const {
    return i < this->size() ? ValueRef(fValue->operator[](i)) : ValueRef();
}

ValueRef ValueRef::operator[](const char* key) const {
    if (!this->isObject())
        return ValueRef();

    const auto m = fValue->FindMember(key);
    return m == fValue->MemberEnd() ? ValueRef() : ValueRef(m->value);
}

const rapidjson::Value* ValueRef::begin() const {
    return this->isArray() ? fValue->Begin() : nullptr;
}

const rapidjson::Value* ValueRef::end() const {
    return this->isArray() ? fValue->End() : nullptr;
}

SkString ValueRef::toString() const {
#ifdef SK_DEBUG
    rapidjson::StringBuffer buf;
    if (fValue) {
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buf);
        fValue->Accept(writer);
    }

    return SkString(buf.GetString());
#else
    return SkString();
#endif // SK_DEBUG
}

Document::Document(SkStream* stream) {
    if (!stream->hasLength()) {
        SkDebugf("!! unsupported unseekable json stream\n");
        return;
    }

    // RapidJSON provides three DOM-builder approaches:
    //
    //   1) in-place   : all data buffered, constructs the DOM in-place -- this is the fastest
    //   2) from buffer: all data buffered, copies to DOM -- this is slightly slower
    //   3) from stream: streamed data, reads/copies to DOM -- this is *significantly* slower
    //
    // We like fast, so #1 it is.

    // The buffer needs to be C-string.
    const auto size = stream->getLength();
    fData = SkData::MakeUninitialized(size + 1);
    if (stream->read(fData->writable_data(), size) < size) {
        SkDebugf("!! could not read JSON stream\n");
        return;
    }

    auto data = static_cast<char*>(fData->writable_data());
    data[size] = '\0';

    fDocument.ParseInsitu(data);

#ifdef SK_DEBUG
        if (fDocument.HasParseError()) {
            SkDebugf("!! failed to parse json: %s\n",
                     rapidjson::GetParseError_En(fDocument.GetParseError()));
        }
#endif
}

} // namespace json

} // namespace skottie
