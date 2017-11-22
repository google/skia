/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkEvent.h"

void SkEvent::initialize(const char* type, size_t typeLen) {
    fType = nullptr;
    setType(type, typeLen);
    f32 = 0;
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
