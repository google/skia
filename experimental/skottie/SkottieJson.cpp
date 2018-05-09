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
bool json::ValueRef::to<SkScalar>(SkScalar* v) const {
    if (!fValue) return false;

    // Some versions wrap values as single-element arrays.
    if (fValue->IsArray() && fValue->Size() == 1) {
        return json::ValueRef(fValue->operator[](0)).to(v);
    }

    if (!fValue->IsNumber())
        return false;

    *v = static_cast<SkScalar>(fValue->GetDouble());

    return true;
}

template <>
bool json::ValueRef::to<bool>(bool* v) const {
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
bool json::ValueRef::to<int>(int* v) const {
    if (!fValue || !fValue->IsInt())
        return false;

    *v = fValue->GetInt();

    return true;
}

template <>
bool json::ValueRef::to<SkString>(SkString* v) const {
    if (!fValue || !fValue->IsString())
        return false;

    v->set(fValue->GetString());

    return true;
}

template <>
bool json::ValueRef::to<SkPoint>(SkPoint* v) const {
    if (!fValue || !fValue->IsObject())
        return false;

    const auto jvx = ValueRef(fValue->operator[]("x")),
               jvy = ValueRef(fValue->operator[]("y"));

            // Some BM versions seem to store x/y as single-element arrays.
    return json::ValueRef(jvx.isArray() ? jvx.operator[](size_t(0)) : jvx).to(&v->fX)
        && json::ValueRef(jvy.isArray() ? jvy.operator[](size_t(0)) : jvy).to(&v->fY);
}

template <>
bool json::ValueRef::to<std::vector<float>>(std::vector<float>* v) const {
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

bool ParsePointVec(const rapidjson::Value& jv, std::vector<SkPoint>* pts) {
    if (!jv.IsArray())
        return false;

    pts->clear();
    pts->reserve(jv.Size());

    std::vector<float> vec;
    for (size_t i = 0; i < jv.Size(); ++i) {
        if (!ValueRef(jv[i]).to(&vec) || vec.size() != 2)
            return false;
        pts->push_back(SkPoint::Make(vec[0], vec[1]));
    }

    return true;
}

} // namespace

template <>
bool json::ValueRef::to<ShapeValue>(ShapeValue* v) const {
    SkASSERT(v->fVertices.empty());

    if (!fValue)
        return false;

    // Some versions wrap values as single-element arrays.
    if (fValue->IsArray() && fValue->Size() == 1) {
        return json::ValueRef(fValue->operator[](0)).to(v);
    }

    std::vector<SkPoint> inPts,  // Cubic Bezier "in" control points, relative to vertices.
                         outPts, // Cubic Bezier "out" control points, relative to vertices.
                         verts;  // Cubic Bezier vertices.

    if (!fValue->IsObject() ||
        !ParsePointVec(fValue->operator[]("i"), &inPts) ||
        !ParsePointVec(fValue->operator[]("o"), &outPts) ||
        !ParsePointVec(fValue->operator[]("v"), &verts) ||
        inPts.size() != outPts.size() ||
        inPts.size() != verts.size()) {

        return false;
    }

    v->fVertices.reserve(inPts.size());
    for (size_t i = 0; i < inPts.size(); ++i) {
        v->fVertices.push_back(BezierVertex({inPts[i], outPts[i], verts[i]}));
    }
    v->fClosed = json::ValueRef(fValue->operator[]("c")).toDefault<bool>(false);

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
    if (this->isArray()) {
        return fValue->Begin();
    }
    if (this->isObject()) {
        return &fValue->MemberBegin()->value;
    }

    return nullptr;
}

const rapidjson::Value* ValueRef::end() const {
    if (this->isArray()) {
        return fValue->End();
    }
    if (this->isObject()) {
        return &fValue->MemberEnd()->value;
    }

    return nullptr;
}

SkString json::ValueRef::toString() const {
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
