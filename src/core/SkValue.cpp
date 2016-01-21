/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <unordered_map>
#include <vector>

#include "SkData.h"
#include "SkValue.h"

class SkValue::Obj {
public:
    void set(SkValue::Key k, SkValue&& v) { fMap[k] = std::move(v);  }
    const SkValue* get(SkValue::Key k) const {
        auto it = fMap.find(k);
        return it != fMap.end() ? &it->second : nullptr;
    }
    void foreach(std::function<void(Key, const SkValue&)> fn) const {
        for (const auto& pair : fMap) {
            fn(pair.first, pair.second);
        }
    }

private:
    std::unordered_map<SkValue::Key, SkValue> fMap;
};

class SkValue::Arr {
public:
    size_t length() const { return fVec.size(); }
    void append(SkValue&& val) { fVec.emplace_back(std::move(val)); }
    const SkValue& at(size_t index) const {
        SkASSERT(index < fVec.size());
        return fVec[index];
    }

private:
    std::vector<SkValue> fVec;
};

SkValue::SkValue() : fType(Null) {}

SkValue::SkValue(Type type) : fType(type) {}

SkValue::SkValue(const SkValue& o) {
    memcpy(this, &o, sizeof(o));
    if (this->isData()) {
        fBytes->ref();
    } else if (this->isObject()) {
        fObject = new Obj(*fObject);
    } else if (Array == fType) {
        fArray = new Arr(*fArray);
    }
}

SkValue::SkValue(SkValue&& o) {
    memcpy(this, &o, sizeof(o));
    new (&o) SkValue();
}

SkValue& SkValue::operator=(const SkValue& o) {
    if (this != &o) {
        this->~SkValue();
        new (this) SkValue(o);
    }
    return *this;
}

SkValue& SkValue::operator=(SkValue&& o) {
    if (this != &o) {
        this->~SkValue();
        new (this) SkValue(std::move(o));
    }
    return *this;
}

SkValue::~SkValue() {
    if (this->isData()) {
        fBytes->unref();
    } else if (this->isObject()) {
        delete fObject;
    } else if (Array == fType) {
        delete fArray;
    }
}

template <typename T>
SkValue SkValue::FromT(SkValue::Type type, T SkValue::*mp, T t) {
    SkValue v(type);
    v.*mp = t;
    return v;
}

SkValue SkValue::FromS32(int32_t  x) { return FromT(S32, &SkValue::fS32, x); }
SkValue SkValue::FromU32(uint32_t x) { return FromT(U32, &SkValue::fU32, x); }
SkValue SkValue::FromF32(float    x) { return FromT(F32, &SkValue::fF32, x); }

int32_t  SkValue::s32() const { SkASSERT(S32 == fType); return fS32; }
uint32_t SkValue::u32() const { SkASSERT(U32 == fType); return fU32; }
float    SkValue::f32() const { SkASSERT(F32 == fType); return fF32; }

SkValue SkValue::FromBytes(SkData* data) {
    if (!data) {
        return SkValue();
    }
    SkValue v(Bytes);
    v.fBytes = SkRef(data);
    return v;
}

SkValue SkValue::Object(SkValue::Type t) {
    SkValue v(t);
    SkASSERT(v.isObject());
    v.fObject = new Obj;
    return v;
}

SkValue SkValue::ValueArray() {
    SkValue v(Array);
    v.fArray = new Arr;
    return v;
}

SkData* SkValue::bytes() const {
    SkASSERT(this->isData());
    return fBytes;
}

void SkValue::set(SkValue::Key k, SkValue v) {
    SkASSERT(this->isObject());
    fObject->set(k, std::move(v));
}

const SkValue* SkValue::get(Key k) const {
    SkASSERT(this->isObject());
    return fObject->get(k);
}

void SkValue::foreach(std::function<void(Key, const SkValue&)> fn) const {
    SkASSERT(this->isObject());
    fObject->foreach(fn);
}

size_t SkValue::length() const {
    SkASSERT(Array == fType);
    return fArray->length();
}

const SkValue& SkValue::at(size_t index) const {
    SkASSERT(Array == fType);
    return fArray->at(index);
}

void SkValue::append(SkValue val) {
    SkASSERT(Array == fType);
    fArray->append(std::move(val));
}

template <typename T>
const T* SkValue::asTs(SkValue::Type t, int* count) const {
    SkASSERT(t == fType && this->isData());
    SkASSERT(count);
    *count = fBytes->size() / sizeof(T);
    return static_cast<const T*>(fBytes->data());
}

const uint16_t* SkValue::u16s(int* c) const { return this->asTs<uint16_t>(U16s, c); }
const uint32_t* SkValue::u32s(int* c) const { return this->asTs<uint32_t>(U32s, c); }
const float*    SkValue::f32s(int* c) const { return this->asTs<float   >(F32s, c); }

template <typename T>
SkValue SkValue::FromTs(SkValue::Type type, SkData* data) {
    SkValue val(type);
    val.fBytes = SkRef(data);
    SkASSERT(val.isData());
    SkASSERT(0 == (reinterpret_cast<uintptr_t>(data->bytes()) & (sizeof(T)-1)));
    return val;
}

SkValue SkValue::FromU16s(SkData* d) { return FromTs<uint16_t>(U16s, d); }
SkValue SkValue::FromU32s(SkData* d) { return FromTs<uint32_t>(U32s, d); }
SkValue SkValue::FromF32s(SkData* d) { return FromTs<   float>(F32s, d); }
