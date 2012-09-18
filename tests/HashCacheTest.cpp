/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "Test.h"

// This is a GR test
#if SK_SUPPORT_GPU
#include "GrTHashCache.h"

struct HashElement {
    int     fKey;
    int     fValue;
};

class GrFindPositivesFunctor {
public:
    // only return elements with positive values
    bool operator()(const HashElement* elem) const { 
        return elem->fValue > 0;
    }
};

class GrFindNegativesFunctor {
public:
    // only return elements with negative values
    bool operator()(const HashElement* elem) const { 
        return elem->fValue < 0;
    }
};

class HashKey {
public:
    HashKey(int key) : fKey(key) {}

    uint32_t getHash() const { return fKey; }

    static bool LT(const HashElement& entry, const HashKey& key) {
        return entry.fKey < key.fKey;
    }
    static bool EQ(const HashElement& entry, const HashKey& key) {
        return entry.fKey == key.fKey;
    }

#if GR_DEBUG
    static uint32_t GetHash(const HashElement& entry) {
        return entry.fKey;
    }
    static bool LT(const HashElement& a, const HashElement& b) {
        return a.fKey < b.fKey;
    }
    static bool EQ(const HashElement& a, const HashElement& b) {
        return a.fKey == b.fKey;
    }
#endif

protected:
    int fKey;
};

////////////////////////////////////////////////////////////////////////////////
static void TestHashCache(skiatest::Reporter* reporter, GrContext* context) {

    GrTHashTable<HashElement, HashKey, 4> cache;

    HashElement negHashElements[10] = { 
        { 0,  0 }, 
        { 1, -1 },
        { 2, -2 }, 
        { 3, -3 }, 
        { 4, -4 }, 
        { 5, -5 }, 
        { 6, -6 }, 
        { 7, -7 }, 
        { 8, -8 }, 
        { 9, -9 }
    };
    HashElement posHashElements[10] = { 
        { 0, 0 }, 
        { 1, 1 },
        { 2, 2 }, 
        { 3, 3 }, 
        { 4, 4 }, 
        { 5, 5 }, 
        { 6, 6 }, 
        { 7, 7 }, 
        { 8, 8 }, 
        { 9, 9 }
    };

    // add i: -i pairs
    for (int i = 0; i < 10; ++i) {
        cache.insert(HashKey(i), &negHashElements[i]);
    }

    REPORTER_ASSERT(reporter, 10 == cache.count());

    // look for all i's and assert we found the -i's
    for (int i = 0; i < 10; ++i) {
        HashElement* found = cache.find(i);
        REPORTER_ASSERT(reporter, NULL != found && -i == found->fValue);
    }

    // look for something not in the cache
    {
        HashElement* found = cache.find(10);
        REPORTER_ASSERT(reporter, NULL == found);
    }

    // add i:i duplicates (so each i will have a positive & negative entry)
    for (int i = 0; i < 10; ++i) {
        cache.insert(i, &posHashElements[i]);
    }

    REPORTER_ASSERT(reporter, 20 == cache.count());
    
    // test out the find functor to find all the positive values
    {
        GrFindPositivesFunctor findPos;

        HashElement* found = cache.find(0, findPos);
        REPORTER_ASSERT(reporter, NULL == found);

        for (int i = 1; i < 10; ++i) {
            found = cache.find(i, findPos);

            REPORTER_ASSERT(reporter, NULL != found && found->fValue > 0);
        }
    }

    // make sure finding the positives wasn't a fluke - find the negatives
    {
        GrFindNegativesFunctor findNeg;

        HashElement* found = cache.find(0, findNeg);
        REPORTER_ASSERT(reporter, NULL == found);

        for (int i = 1; i < 10; ++i) {
            found = cache.find(i, findNeg);

            REPORTER_ASSERT(reporter, NULL != found && found->fValue < 0);
        }
    }

    // remove the 0:0 entries
    {
        cache.remove(0, &negHashElements[0]);
        cache.remove(0, &posHashElements[0]);
        REPORTER_ASSERT(reporter, 18 == cache.count());

        HashElement* found = cache.find(0);
        REPORTER_ASSERT(reporter, NULL == found);
    }

    // remove all
    {
        cache.removeAll();
        REPORTER_ASSERT(reporter, 0 == cache.count());
    }
}

////////////////////////////////////////////////////////////////////////////////
#include "TestClassDef.h"
DEFINE_GPUTESTCLASS("HashCache", HashCacheTestClass, TestHashCache)

#endif
