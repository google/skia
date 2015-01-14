/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "SkTHashCache.h"


// Tests the SkTHashCache<T> class template.

struct Tint {
    uint32_t value;

    bool operator==(const Tint& rhs) const {
        return value == rhs.value;
    }
};

class Element {
public:

    bool operator==(const Element& rhs) const {
        return value == rhs.value && key == rhs.key;
    }

    static const Tint& GetKey(const Element& element) {
        return element.key;
    }

    static uint32_t Hash(const Tint& key) {
        return key.value;
    }

    Element(Tint key, int value) : key(key), value(value) {
    }

    Tint key;
    int value;
};

typedef SkTHashCache<Element, Tint> CacheType;

DEF_TEST(THashCache, reporter) {
    Tint k11 = {11};
    Element e11(k11, 22);

    Element e11Collision(k11, 0);
    //    Element e42(4, 2);

    //Some tests for the class Element
    REPORTER_ASSERT(reporter, Element::GetKey(e11) == k11);
    REPORTER_ASSERT(reporter, Element::Hash(k11) == 11);

    CacheType cache;

    // Is the cahce correctly initialized ?
    REPORTER_ASSERT(reporter, 0 == cache.size());
    REPORTER_ASSERT(reporter, NULL == cache.find(k11));

    Element& e11_c = cache.add(e11);

    // Tests for simple insertion, verifying that the returned element
    // has the same values as the original one
    REPORTER_ASSERT(reporter, 1 == cache.size());
    REPORTER_ASSERT(reporter, NULL != cache.find(k11));
    REPORTER_ASSERT(reporter, e11_c == e11);

    Element& e11Collision_c = cache.add(e11Collision);
    // Verifying that, in case of collision, the element alerady in the cache is not removed
    REPORTER_ASSERT(reporter, e11Collision_c == e11);
    REPORTER_ASSERT(reporter, 1 == cache.size());

    Tint k42 = {42};
    Element e42(k42, 2);
    cache.add(e42);
    // Can we add more than one element?
    REPORTER_ASSERT(reporter, NULL != cache.find(k11));
    REPORTER_ASSERT(reporter, NULL != cache.find(k42));
    REPORTER_ASSERT(reporter, 2 == cache.size());

    cache.reset();
    // Does clear do its job?
    REPORTER_ASSERT(reporter, 0 == cache.size());
    REPORTER_ASSERT(reporter, NULL == cache.find(k11));
    REPORTER_ASSERT(reporter, NULL == cache.find(k42));
}
