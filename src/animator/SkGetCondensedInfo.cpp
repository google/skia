
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkMemberInfo.h"

#if SK_USE_CONDENSED_INFO == 1

// SkCondensed.cpp is auto-generated
// To generate it, execute SkDisplayType::BuildCondensedInfo()
#ifdef SK_DEBUG
#include "SkCondensedDebug.cpp"
#else
#include "SkCondensedRelease.cpp"
#endif

static int _searchByName(const unsigned char* lengths, int count, const char* strings, const char target[]) {
    int lo = 0;
    int hi = count - 1;
    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        if (strcmp(&strings[lengths[mid << 2]], target) < 0)
            lo = mid + 1;
        else 
            hi = mid;
    }
    if (strcmp(&strings[lengths[hi << 2]], target) != 0)
        return -1;
    return hi;
}

static int _searchByType(SkDisplayTypes type) {
    unsigned char match = (unsigned char) type;
    int lo = 0;
    int hi = kTypeIDs - 1;
    while (lo < hi) {
        int mid = (hi + lo) >> 1;
        if (gTypeIDs[mid] < match)
            lo = mid + 1;
        else
            hi = mid;
    }
    if (gTypeIDs[hi] != type)
        return -1;
    return hi;
}

const SkMemberInfo* SkDisplayType::GetMembers(SkAnimateMaker* , SkDisplayTypes type, int* infoCountPtr) {
    int lookup = _searchByType(type);
    if (lookup < 0)
        return NULL;
    if (infoCountPtr)
        *infoCountPtr = gInfoCounts[lookup];
    return gInfoTables[lookup];
}

// !!! replace with inline
const SkMemberInfo* SkDisplayType::GetMember(SkAnimateMaker* , SkDisplayTypes type, const char** matchPtr ) {
    const SkMemberInfo* info = SkMemberInfo::Find(type, matchPtr);
    SkASSERT(info);
    return info;
}

static const SkMemberInfo* _lookup(int lookup, const char** matchPtr) {
    int count = gInfoCounts[lookup];
    const SkMemberInfo* info = gInfoTables[lookup];
    if (info->fType == SkType_BaseClassInfo) {
        int baseTypeLookup = info->fOffset;
        const SkMemberInfo* result = _lookup(baseTypeLookup, matchPtr);
        if (result != NULL)
            return result;
        if (--count == 0)
            return NULL;
        info++;
    }
    SkASSERT(info->fType != SkType_BaseClassInfo);
    const char* match = *matchPtr;
    const char* strings = gInfoNames[lookup];
    int index = _searchByName(&info->fName, count, strings, match);
    if (index < 0)
        return NULL;
    return &info[index];
}

const SkMemberInfo* SkMemberInfo::Find(SkDisplayTypes type, int* index) {
    int count = gInfoCounts[lookup];
    const SkMemberInfo* info = gInfoTables[lookup];
    if (info->fType == SkType_BaseClassInfo) {
        int baseTypeLookup = info->fOffset;
        const SkMemberInfo* result = Find(baseTypeLookup, index);
        if (result != NULL)
            return result;
        if (--count == 0)
            return NULL;
        info++;
    }
    SkASSERT(info->fType != SkType_BaseClassInfo);
    if (*index >= count) {
        *index -= count;
        return NULL;
    }
    return &info[index];
}

const SkMemberInfo* SkMemberInfo::Find(SkDisplayTypes type, const char** matchPtr) {
    int lookup = _searchByType(type);
    SkASSERT(lookup >= 0);
    return _lookup(lookup, matchPtr);
}

const SkMemberInfo* SkMemberInfo::getInherited() const {
    int baseTypeLookup = fOffset;
    return gInfoTables[baseTypeLookup];
}

#endif

