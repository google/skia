/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkMetaData.h"

#include "include/core/SkRefCnt.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"

SkMetaData::~SkMetaData() {
    std::unique_ptr<Rec, D> rec = std::move(fRec);
    while (rec) {
        rec = std::move(rec->fNext);
    }
}

void SkMetaData::D::operator()(SkMetaData::Rec* r) { sk_free(r); }

void* SkMetaData::set(const char name[], const void* data, size_t dataSize, Type type, int count)
{
    if (count <= 0) {
        return nullptr;
    }
    SkASSERT(name);
    SkASSERT(dataSize);

    (void)this->remove(name, type);

    size_t len = strlen(name) + 1;
    std::unique_ptr<Rec, D> rec(new (sk_malloc_throw(sizeof(Rec) + dataSize * count + len)) Rec);

    rec->fType = SkToU8(type);
    rec->fDataLen = SkToU8(dataSize);
    rec->fDataCount = SkToU16(count);
    if (data) {
        memcpy(rec->data(), data, dataSize * count);
    }
    memcpy(rec->name(), name, len);


    rec->fNext = std::move(fRec);
    fRec = std::move(rec);
    return fRec->data();
}

const void* SkMetaData::find(const char name[], Type type, int* count) const {
    const Rec* rec = fRec.get();
    while (rec) {
        if (rec->fType == type && !strcmp(rec->name(), name)) {
            break;
        }
        rec = rec->fNext.get();
    }
    if (!rec) {
        return nullptr;
    }
    SkASSERT(rec->fDataCount >= 1);
    if (count) {
        *count = rec->fDataCount;
    }
    return rec->data();
}


bool SkMetaData::remove(const char name[], Type type) {
    Rec* rec = fRec.get();
    Rec* prev = nullptr;
    while (rec) {
        if (rec->fType == type && !strcmp(rec->name(), name)) {
            if (prev) {
                prev->fNext = std::move(rec->fNext);
            } else {
                fRec = std::move(rec->fNext);
            }
            return true;
        }
        prev = rec;
        rec = rec->fNext.get();
    }
    return false;
}


///////////////////////////////////////////////////////////////////////////////

SkMetaData::Iter::Iter(const SkMetaData& metadata) {
    fRec = metadata.fRec.get();
}

const char* SkMetaData::Iter::next(SkMetaData::Type* t, int* count) {
    const char* name = nullptr;

    if (fRec) {
        if (t) {
            *t = (SkMetaData::Type)fRec->fType;
        }
        if (count) {
            *count = fRec->fDataCount;
        }
        name = fRec->name();

        fRec = fRec->fNext.get();
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////

