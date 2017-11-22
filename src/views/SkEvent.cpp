/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkDOM.h"
#include "SkEvent.h"

void SkEvent::initialize(const char* type, size_t typeLen) {
    fType = nullptr;
    setType(type, typeLen);
    f32 = 0;
#ifdef SK_DEBUG
    fTime = 0;
    fNextEvent = nullptr;
#endif
}

SkEvent::SkEvent()
{
    initialize("", 0);
}

SkEvent::SkEvent(const SkEvent& src)
{
    *this = src;
    if (((size_t) fType & 1) == 0)
        setType(src.fType);
}

SkEvent::SkEvent(const SkString& type)
{
    initialize(type.c_str(), type.size());
}

SkEvent::SkEvent(const char type[])
{
    SkASSERT(type);
    initialize(type, strlen(type));
}

SkEvent::~SkEvent()
{
    if (((size_t) fType & 1) == 0)
        sk_free((void*) fType);
}

static size_t makeCharArray(char* buffer, size_t compact)
{
    size_t bits = (size_t) compact >> 1;
    memcpy(buffer, &bits, sizeof(compact));
    buffer[sizeof(compact)] = 0;
    return strlen(buffer);
}

void SkEvent::getType(SkString* str) const
{
    if (str)
    {
        if ((size_t) fType & 1) // not a pointer
        {
            char chars[sizeof(size_t) + 1];
            size_t len = makeCharArray(chars, (size_t) fType);
            str->set(chars, len);
        }
        else
            str->set(fType);
    }
}

bool SkEvent::isType(const SkString& str) const
{
    return this->isType(str.c_str(), str.size());
}

bool SkEvent::isType(const char type[], size_t typeLen) const
{
    if (typeLen == 0)
        typeLen = strlen(type);
    if ((size_t) fType & 1) {   // not a pointer
        char chars[sizeof(size_t) + 1];
        size_t len = makeCharArray(chars, (size_t) fType);
        return len == typeLen && strncmp(chars, type, typeLen) == 0;
    }
    return strncmp(fType, type, typeLen) == 0 && fType[typeLen] == 0;
}

void SkEvent::setType(const char type[], size_t typeLen)
{
    if (typeLen == 0)
        typeLen = strlen(type);
    if (typeLen <= sizeof(fType)) {
        size_t slot = 0;
        memcpy(&slot, type, typeLen);
        if (slot << 1 >> 1 != slot)
            goto useCharStar;
        slot <<= 1;
        slot |= 1;
        fType = (char*) slot;
    } else {
useCharStar:
        fType = (char*) sk_malloc_throw(typeLen + 1);
        SkASSERT(((size_t) fType & 1) == 0);
        memcpy(fType, type, typeLen);
        fType[typeLen] = 0;
    }
}

void SkEvent::setType(const SkString& type)
{
    setType(type.c_str());
}

////////////////////////////////////////////////////////////////////////////

#include "SkParse.h"

void SkEvent::inflate(const SkDOM& dom, const SkDOM::Node* node)
{
    const char* name = dom.findAttr(node, "type");
    if (name)
        this->setType(name);

    const char* value;
    if ((value = dom.findAttr(node, "fast32")) != nullptr)
    {
        int32_t n;
        if (SkParse::FindS32(value, &n))
            this->setFast32(n);
    }

    for (node = dom.getFirstChild(node); node; node = dom.getNextSibling(node))
    {
        if (strcmp(dom.getName(node), "data"))
        {
            SkDEBUGCODE(SkDebugf("SkEvent::inflate unrecognized subelement <%s>\n", dom.getName(node));)
            continue;
        }

        name = dom.findAttr(node, "name");
        if (name == nullptr)
        {
            SkDEBUGCODE(SkDebugf("SkEvent::inflate missing required \"name\" attribute in <data> subelement\n");)
            continue;
        }

        if ((value = dom.findAttr(node, "s32")) != nullptr)
        {
            int32_t n;
            if (SkParse::FindS32(value, &n))
                this->setS32(name, n);
        }
        else if ((value = dom.findAttr(node, "scalar")) != nullptr)
        {
            SkScalar x;
            if (SkParse::FindScalar(value, &x))
                this->setScalar(name, x);
        }
        else if ((value = dom.findAttr(node, "string")) != nullptr)
            this->setString(name, value);
#ifdef SK_DEBUG
        else
        {
            SkDebugf("SkEvent::inflate <data name=\"%s\"> subelement missing required type attribute [S32 | scalar | string]\n", name);
        }
#endif
    }
}

#ifdef SK_DEBUG

    #ifndef SkScalarToFloat
        #define SkScalarToFloat(x)  ((x) / 65536.f)
    #endif

    void SkEvent::dump(const char title[])
    {
        if (title)
            SkDebugf("%s ", title);

        SkString    etype;
        this->getType(&etype);
        SkDebugf("event<%s> fast32=%d", etype.c_str(), this->getFast32());

        const SkMetaData&   md = this->getMetaData();
        SkMetaData::Iter    iter(md);
        SkMetaData::Type    mtype;
        int                 count;
        const char*         name;

        while ((name = iter.next(&mtype, &count)) != nullptr)
        {
            SkASSERT(count > 0);

            SkDebugf(" <%s>=", name);
            switch (mtype) {
            case SkMetaData::kS32_Type:     // vector version???
                {
                    int32_t value;
                    md.findS32(name, &value);
                    SkDebugf("%d ", value);
                }
                break;
            case SkMetaData::kScalar_Type:
                {
                    const SkScalar* values = md.findScalars(name, &count, nullptr);
                    SkDebugf("%f", SkScalarToFloat(values[0]));
                    for (int i = 1; i < count; i++)
                        SkDebugf(", %f", SkScalarToFloat(values[i]));
                    SkDebugf(" ");
                }
                break;
            case SkMetaData::kString_Type:
                {
                    const char* value = md.findString(name);
                    SkASSERT(value);
                    SkDebugf("<%s> ", value);
                }
                break;
            case SkMetaData::kPtr_Type:     // vector version???
                {
                    void*   value;
                    md.findPtr(name, &value);
                    SkDebugf("%p ", value);
                }
                break;
            case SkMetaData::kBool_Type:    // vector version???
                {
                    bool    value;
                    md.findBool(name, &value);
                    SkDebugf("%s ", value ? "true" : "false");
                }
                break;
            default:
                SkDEBUGFAIL("unknown metadata type returned from iterator");
                break;
            }
        }
        SkDebugf("\n");
    }
#endif
