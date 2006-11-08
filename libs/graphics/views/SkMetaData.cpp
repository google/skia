/* libs/graphics/views/SkMetaData.cpp
**
** Copyright 2006, Google Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License"); 
** you may not use this file except in compliance with the License. 
** You may obtain a copy of the License at 
**
**     http://www.apache.org/licenses/LICENSE-2.0 
**
** Unless required by applicable law or agreed to in writing, software 
** distributed under the License is distributed on an "AS IS" BASIS, 
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
** See the License for the specific language governing permissions and 
** limitations under the License.
*/

#include "SkMetaData.h"

SkMetaData::SkMetaData() : fRec(NULL)
{
}

SkMetaData::SkMetaData(const SkMetaData& src) : fRec(NULL)
{
    *this = src;
}

SkMetaData::~SkMetaData()
{
    this->reset();
}

void SkMetaData::reset()
{
    Rec* rec = fRec;
    while (rec)
    {
        Rec* next = rec->fNext;
        Rec::Free(rec);
        rec = next;
    }
    fRec = NULL;
}

SkMetaData& SkMetaData::operator=(const SkMetaData& src)
{
    this->reset();

    const Rec* rec = src.fRec;
    while (rec)
    {
        this->set(rec->name(), rec->data(), rec->fDataLen, (Type)rec->fType, rec->fDataCount);
        rec = rec->fNext;
    }
    return *this;
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
    return NULL;
}

void SkMetaData::setString(const char name[], const char value[])
{
    (void)this->set(name, value, sizeof(char), kString_Type, strlen(value) + 1);
}

void SkMetaData::setPtr(const char name[], void* ptr)
{
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

    (void)this->remove(name, type);

    size_t  len = strlen(name);
    Rec*    rec = Rec::Alloc(sizeof(Rec) + dataSize * count + len + 1);

#ifndef SK_DEBUG
    rec->fType = SkToU8(type);
#else
    rec->fType = type;
#endif
    rec->fDataLen = SkToU8(dataSize);
    rec->fDataCount = SkToU16(count);
    if (data)
        memcpy(rec->data(), data, dataSize * count);
    memcpy(rec->name(), name, len + 1);

#ifdef SK_DEBUG
    rec->fName = rec->name();
    switch (type) {
    case kS32_Type:
        rec->fData.fS32 = *(const int32_t*)rec->data();
        break;
    case kScalar_Type:
        rec->fData.fScalar = *(const SkScalar*)rec->data();
        break;
    case kString_Type:
        rec->fData.fString = (const char*)rec->data();
        break;
    case kPtr_Type:
        rec->fData.fPtr = *(void**)rec->data();
        break;
    case kBool_Type:
        rec->fData.fBool = *(const bool*)rec->data();
        break;
    default:
        SkASSERT(!"bad type");
        break;
    }
#endif

    rec->fNext = fRec;
    fRec = rec;
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
    return NULL;
}

bool SkMetaData::findPtr(const char name[], void** value) const
{
    const Rec* rec = this->find(name, kPtr_Type);
    if (rec)
    {
        SkASSERT(rec->fDataCount == 1);
        if (value)
            *value = *(void**)rec->data();
        return true;
    }
    return false;
}

const char* SkMetaData::findString(const char name[]) const
{
    const Rec* rec = this->find(name, kString_Type);
    SkASSERT(rec == NULL || rec->fDataLen == sizeof(char));
    return rec ? (const char*)rec->data() : NULL;
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
    const Rec* rec = fRec;
    while (rec)
    {
        if (rec->fType == type && !strcmp(rec->name(), name))
            return rec;
        rec = rec->fNext;
    }
    return NULL;
}

bool SkMetaData::remove(const char name[], Type type)
{
    Rec* rec = fRec;
    Rec* prev = NULL;
    while (rec)
    {
        Rec* next = rec->fNext;
        if (rec->fType == type && !strcmp(rec->name(), name))
        {
            if (prev)
                prev->fNext = next;
            else
                fRec = next;
            Rec::Free(rec);
            return true;
        }
        prev = rec;
        rec = next;
    }
    return false;
}

bool SkMetaData::removeS32(const char name[])
{
    return this->remove(name, kS32_Type);
}

bool SkMetaData::removeScalar(const char name[])
{
    return this->remove(name, kScalar_Type);
}

bool SkMetaData::removeString(const char name[])
{
    return this->remove(name, kString_Type);
}

bool SkMetaData::removePtr(const char name[])
{
    return this->remove(name, kPtr_Type);
}

bool SkMetaData::removeBool(const char name[])
{
    return this->remove(name, kBool_Type);
}

///////////////////////////////////////////////////////////////////////////////////

SkMetaData::Iter::Iter(const SkMetaData& metadata)
{
    fRec = metadata.fRec;
}

void SkMetaData::Iter::reset(const SkMetaData& metadata)
{
    fRec = metadata.fRec;
}

const char* SkMetaData::Iter::next(SkMetaData::Type* t, int* count)
{
    const char* name = NULL;

    if (fRec)
    {
        if (t)
            *t = (SkMetaData::Type)fRec->fType;
        if (count)
            *count = fRec->fDataCount;
        name = fRec->name();

        fRec = fRec->fNext;
    }
    return name;
}

///////////////////////////////////////////////////////////////////////////////////

SkMetaData::Rec* SkMetaData::Rec::Alloc(size_t size)
{
    return (Rec*)sk_malloc_throw(size);
}

void SkMetaData::Rec::Free(Rec* rec)
{
    sk_free(rec);
}

///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkMetaData::UnitTest()
{
#ifdef SK_SUPPORT_UNITTEST
    SkMetaData  m1;

    SkASSERT(!m1.findS32("int"));
    SkASSERT(!m1.findScalar("scalar"));
    SkASSERT(!m1.findString("hello"));
    SkASSERT(!m1.removeS32("int"));
    SkASSERT(!m1.removeScalar("scalar"));
    SkASSERT(!m1.removeString("hello"));
    SkASSERT(!m1.removeString("true"));
    SkASSERT(!m1.removeString("false"));

    m1.setS32("int", 12345);
    m1.setScalar("scalar", SK_Scalar1 * 42);
    m1.setString("hello", "world");
    m1.setPtr("ptr", &m1);
    m1.setBool("true", true);
    m1.setBool("false", false);

    int32_t     n;
    SkScalar    s;

    m1.setScalar("scalar", SK_Scalar1/2);

    SkASSERT(m1.findS32("int", &n) && n == 12345);
    SkASSERT(m1.findScalar("scalar", &s) && s == SK_Scalar1/2);
    SkASSERT(!strcmp(m1.findString("hello"), "world"));
    SkASSERT(m1.hasBool("true", true));
    SkASSERT(m1.hasBool("false", false));

    Iter    iter(m1);
    const char* name;

    static const struct {
        const char*         fName;
        SkMetaData::Type    fType;
        int                 fCount;
    } gElems[] = {
        { "int",    SkMetaData::kS32_Type,      1 },
        { "scalar", SkMetaData::kScalar_Type,   1 },
        { "ptr",    SkMetaData::kPtr_Type,      1 },
        { "hello",  SkMetaData::kString_Type,   sizeof("world") },
        { "true",   SkMetaData::kBool_Type,     1 },
        { "false",  SkMetaData::kBool_Type,     1 }
    };

    int                 loop = 0;
    int count;
    SkMetaData::Type    t;
    while ((name = iter.next(&t, &count)) != NULL)
    {
        int match = 0;
        for (unsigned i = 0; i < SK_ARRAY_COUNT(gElems); i++)
        {
            if (!strcmp(name, gElems[i].fName))
            {
                match += 1;
                SkASSERT(gElems[i].fType == t);
                SkASSERT(gElems[i].fCount == count);
            }
        }
        SkASSERT(match == 1);
        loop += 1;
    }
    SkASSERT(loop == SK_ARRAY_COUNT(gElems));

    SkASSERT(m1.removeS32("int"));
    SkASSERT(m1.removeScalar("scalar"));
    SkASSERT(m1.removeString("hello"));
    SkASSERT(m1.removeBool("true"));
    SkASSERT(m1.removeBool("false"));

    SkASSERT(!m1.findS32("int"));
    SkASSERT(!m1.findScalar("scalar"));
    SkASSERT(!m1.findString("hello"));
    SkASSERT(!m1.findBool("true"));
    SkASSERT(!m1.findBool("false"));
#endif
}

#endif


