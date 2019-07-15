/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/utils/SkMetaData.h"

#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"

#include <cstring>

SkMetaData::~SkMetaData() = default;

void SkMetaData::D::operator()(Rec* r) { sk_free(r); }

void* SkMetaData::set(const SkString & name, const void* src,
                      size_t size, SkMetaData::Type t, size_t count) {
    Rec* rec = (Rec*)sk_malloc_throw(sizeof(Rec) + count * size);
    rec->fDataCount = SkToUInt(count);
    rec->fType = (unsigned)t;
    void* ptr = (void*)(rec + 1);
    ::memcpy(ptr, src, count * size);
    fMap.set(name, RecHolder(rec));
    return ptr;
} 

const void* SkMetaData::get(const SkString & name, SkMetaData::Type t, size_t* count) const {
    RecHolder* rh = fMap.find(name);
    if (!rh) { return nullptr; }
    Rec* rec = rh->get();
    if (rec->fType != (unsigned)t) { return nullptr; }
    if (count) { *count = SkToSizeT(rec->fDataCount); }
    return (const void*)(rec + 1);
}

bool SkMetaData::find(const SkString& name, size_t size, SkMetaData::Type type,
                      size_t* count, void* value) const {
    size_t n = 0;
    const void* ptr = this->get(name, type, &n);
    if (!ptr || n == 0) { return false; }
    if (count) { *count = n; }
    if (value) { ::memcpy(value, ptr, n * size); }
    return true;
}

bool SkMetaData::findOne(const SkString& name, size_t size, SkMetaData::Type type,
                         void* value) const {
    size_t n = 0;
    const void* ptr = this->get(name, type, &n);
    if (!ptr || n != 1) { return false; }
    if (value) { ::memcpy(value, ptr, size); }
    return true;
}

SkMetaData::Iter::Iter(const SkMetaData& metadata) {
    metadata.fMap.foreach([&](const SkString& n, const RecHolder& rh) {
        if (const Rec* rec = rh.get()) {  
            fEntries.push_back(Entry{n.c_str(), (Type)rec->fType, rec->fDataCount});
        }
    });
}

const char* SkMetaData::Iter::next(SkMetaData::Type* type, int* count) {
    if (fIndex >= fEntries.size()) { return nullptr; }
    if (type) { *type = fEntries[fIndex].type; }
    if (count) { *count = SkToInt(fEntries[fIndex].count); }
    return fEntries[fIndex++].name;
}
