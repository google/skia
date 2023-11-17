/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tools/SkMetaData.h"

#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"

void SkMetaData::reset()
{
    Rec* rec = fRec;
    while (rec) {
        Rec* next = rec->fNext;
        Rec::Free(rec);
        rec = next;
    }
    fRec = nullptr;
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

void* SkMetaData::set(const char name[], const void* data, size_t dataSize, Type type, int count)
{
    SkASSERT(name);
    SkASSERT(dataSize);
    SkASSERT(count > 0);

    FindResult result = this->findWithPrev(name, type);

    Rec* rec;
    bool reuseRec = result.rec &&
                    result.rec->fDataLen == dataSize &&
                    result.rec->fDataCount == count;
    if (reuseRec) {
        rec = result.rec;
    } else {
        size_t len = strlen(name);
        rec = Rec::Alloc(sizeof(Rec) + dataSize * count + len + 1);
        rec->fType = SkToU8(type);
        rec->fDataLen = SkToU8(dataSize);
        rec->fDataCount = SkToU16(count);

        memcpy(rec->name(), name, len + 1);
    }
    if (data) {
        memcpy(rec->data(), data, dataSize * count);
    }

    if (reuseRec) {
        // Do nothing, reused
    } else if (result.rec) {
        // Had one, but had to create a new one. Invalidates iterators.
        // Delayed removal since name or data may have been in the result.rec.
        this->remove(result);
        if (result.prev) {
            rec->fNext = result.prev->fNext;
            result.prev->fNext = rec;
        }
    } else {
        // Adding a new one, stick it at head.
        rec->fNext = fRec;
        fRec = rec;
    }
    return rec->data();
}

bool SkMetaData::findS32(const char name[], int32_t* value) const
{
    const Rec* rec = this->find(name, kS32_Type);
    if (rec)
    {
        SkASSERT(rec->fDataCount == 1);
        if (value)
            *value = *(const int32_t*)rec->data();
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
        if (value)
            *value = *(const SkScalar*)rec->data();
        return true;
    }
    return false;
}

const SkScalar* SkMetaData::findScalars(const char name[], int* count, SkScalar values[]) const
{
    const Rec* rec = this->find(name, kScalar_Type);
    if (rec)
    {
        if (count)
            *count = rec->fDataCount;
        if (values)
            memcpy(values, rec->data(), rec->fDataCount * rec->fDataLen);
        return (const SkScalar*)rec->data();
    }
    return nullptr;
}

bool SkMetaData::findPtr(const char name[], void** ptr) const {
    const Rec* rec = this->find(name, kPtr_Type);
    if (rec) {
        SkASSERT(rec->fDataCount == 1);
        void* const* found = (void* const*)rec->data();
        if (ptr) {
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

SkMetaData::FindResult SkMetaData::findWithPrev(const char name[], Type type) const {
    FindResult current { fRec, nullptr };
    while (current.rec) {
        if (current.rec->fType == type && !strcmp(current.rec->name(), name))
            return current;
        current.prev = current.rec;
        current.rec = current.rec->fNext;
    }
    return current;
}


const SkMetaData::Rec* SkMetaData::find(const char name[], Type type) const {
    return this->findWithPrev(name, type).rec;
}

void SkMetaData::remove(FindResult result) {
    SkASSERT(result.rec);
    if (result.prev) {
        result.prev->fNext = result.rec->fNext;
    } else {
        fRec = result.rec->fNext;
    }
    Rec::Free(result.rec);
}

bool SkMetaData::remove(const char name[], Type type) {
    FindResult result = this->findWithPrev(name, type);
    if (!result.rec) {
        return false;
    }
    this->remove(result);
    return true;
}

bool SkMetaData::removeS32(const char name[])
{
    return this->remove(name, kS32_Type);
}

bool SkMetaData::removeScalar(const char name[])
{
    return this->remove(name, kScalar_Type);
}

bool SkMetaData::removePtr(const char name[])
{
    return this->remove(name, kPtr_Type);
}

bool SkMetaData::removeBool(const char name[])
{
    return this->remove(name, kBool_Type);
}

///////////////////////////////////////////////////////////////////////////////

SkMetaData::Iter::Iter(const SkMetaData& metadata) {
    fRec = metadata.fRec;
}

void SkMetaData::Iter::reset(const SkMetaData& metadata) {
    fRec = metadata.fRec;
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

        fRec = fRec->fNext;
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////

SkMetaData::Rec* SkMetaData::Rec::Alloc(size_t size) {
    return (Rec*)sk_malloc_throw(size);
}

void SkMetaData::Rec::Free(Rec* rec) {
    sk_free(rec);
}
