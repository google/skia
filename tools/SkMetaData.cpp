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

void SkMetaData::setS32(const char name[], int32_t value)
{
    (void)this->set(name, &value, sizeof(int32_t), kS32_Type, 1);
}

void SkMetaData::setScalar(const char name[], SkScalar value)
{
    (void)this->set(name, &value, sizeof(SkScalar), kScalar_Type, 1);
}

SkScalar* SkMetaData::setScalars(const char name[], int count, const SkScalar values[])
{
    SkASSERT(count > 0);
    if (count > 0)
        return (SkScalar*)this->set(name, values, sizeof(SkScalar), kScalar_Type, count);
    return nullptr;
}

void SkMetaData::setPtr(const char name[], void* ptr) {
    (void)this->set(name, &ptr, sizeof(void*), kPtr_Type, 1);
}

void SkMetaData::setBool(const char name[], bool value)
{
    (void)this->set(name, &value, sizeof(bool), kBool_Type, 1);
}

void SkMetaData::D::operator()(SkMetaData::Rec* r) { sk_free(r); }

void* SkMetaData::set(const char name[], const void* data, size_t dataSize, Type type, int count)
{
    SkASSERT(name);
    SkASSERT(dataSize);
    SkASSERT(count > 0);

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

bool SkMetaData::findS32(const char name[], int32_t* value) const
{
    const Rec* rec = this->find(name, kS32_Type);
    if (rec)
    {
        SkASSERT(rec->fDataCount == 1);
        if (value) {
            *value = *(const int32_t*)rec->data();
        }
        return true;
    }
    return false;
}

bool SkMetaData::findScalar(const char name[], SkScalar* value) const
{
    const Rec* rec = this->find(name, kScalar_Type);
    if (rec)
    {
        SkASSERT(rec->fDataCount == 1);
        if (value) {
            *value = *(const SkScalar*)rec->data();
        }
        return true;
    }
    return false;
}

const SkScalar* SkMetaData::findScalars(const char name[], int* count, SkScalar values[]) const
{
    const Rec* rec = this->find(name, kScalar_Type);
    if (rec)
    {
        if (count) {
            *count = rec->fDataCount;
        }
        if (values) {
            memcpy(values, rec->data(), rec->fDataCount * rec->fDataLen);
        }
        return (const SkScalar*)rec->data();
    }
    return nullptr;
}

bool SkMetaData::findPtr(const char name[], void** ptr) const {
    const Rec* rec = this->find(name, kPtr_Type);
    if (rec) {
        SkASSERT(rec->fDataCount == 1);
        void** found = (void**)rec->data();
        if (found && ptr) {
            *ptr = *found;
        }
        return true;
    }
    return false;
}


bool SkMetaData::findBool(const char name[], bool* value) const
{
    const Rec* rec = this->find(name, kBool_Type);
    if (rec)
    {
        SkASSERT(rec->fDataCount == 1);
        if (value)
            *value = *(const bool*)rec->data();
        return true;
    }
    return false;
}

const SkMetaData::Rec* SkMetaData::find(const char name[], Type type) const
{
    const Rec* rec = fRec.get();
    while (rec)
    {
        if (rec->fType == type && !strcmp(rec->name(), name))
            return rec;
        rec = rec->fNext.get();
    }
    return nullptr;
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

