/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkTDynamicHash.h"
#include "tests/Test.h"

#include <cstring>

namespace {

struct Entry {
    int key;
    double value;

    static const int& GetKey(const Entry& entry) { return entry.key; }
    static uint32_t Hash(const int& key) { return key; }
};


class Hash : public SkTDynamicHash<Entry, int> {
public:
    Hash() : INHERITED() {}

    // Promote protected methods to public for this test.
    int capacity() const { return this->INHERITED::capacity(); }
    int countCollisions(const int& key) const { return this->INHERITED::countCollisions(key); }

private:
    typedef SkTDynamicHash<Entry, int> INHERITED;
};

}  // namespace

#define ASSERT(x) REPORTER_ASSERT(reporter, x)

DEF_TEST(DynamicHash_growth, reporter) {
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };
    Entry c = { 3, 4.0 };
    Entry d = { 4, 5.0 };
    Entry e = { 5, 6.0 };

    Hash hash;
    ASSERT(hash.capacity() == 0);

    hash.add(&a);
    ASSERT(hash.capacity() == 4);

    hash.add(&b);
    ASSERT(hash.capacity() == 4);

    hash.add(&c);
    ASSERT(hash.capacity() == 4);

    hash.add(&d);
    ASSERT(hash.capacity() == 8);

    hash.add(&e);
    ASSERT(hash.capacity() == 8);

    ASSERT(hash.count() == 5);
}

DEF_TEST(DynamicHash_growth_bounded, reporter) {
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };
    Entry c = { 3, 4.0 };
    Entry d = { 4, 5.0 };
    Entry e = { 5, 6.0 };

    Hash hash;
    ASSERT(hash.capacity() == 0);

    hash.add(&a);
    ASSERT(hash.capacity() == 4);

    hash.remove(a.key);
    hash.add(&b);
    ASSERT(hash.capacity() == 4);

    hash.remove(b.key);
    hash.add(&c);
    ASSERT(hash.capacity() == 4);

    hash.remove(c.key);
    hash.add(&d);
    ASSERT(hash.capacity() == 4);

    hash.remove(d.key);
    hash.add(&e);
    ASSERT(hash.capacity() == 4);

    ASSERT(hash.count() == 1);
}

DEF_TEST(DynamicHash_add, reporter) {
    Hash hash;
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };

    ASSERT(hash.count() == 0);
    hash.add(&a);
    ASSERT(hash.count() == 1);
    hash.add(&b);
    ASSERT(hash.count() == 2);
}

DEF_TEST(DynamicHash_lookup, reporter) {
    Hash hash;

    // These collide.
    Entry a = { 1, 2.0 };
    Entry b = { 5, 3.0 };

    // Before we insert anything, nothing can collide.
    ASSERT(hash.countCollisions(1) == 0);
    ASSERT(hash.countCollisions(5) == 0);
    ASSERT(hash.countCollisions(9) == 0);

    // First is easy.
    hash.add(&a);
    ASSERT(hash.countCollisions(1) == 0);
    ASSERT(hash.countCollisions(5) == 1);
    ASSERT(hash.countCollisions(9) == 1);

    // Second is one step away.
    hash.add(&b);
    ASSERT(hash.countCollisions(1) == 0);
    ASSERT(hash.countCollisions(5) == 1);
    ASSERT(hash.countCollisions(9) == 2);

    // We can find our data right?
    ASSERT(hash.find(1) != nullptr);
    ASSERT(hash.find(1)->value == 2.0);
    ASSERT(hash.find(5) != nullptr);
    ASSERT(hash.find(5)->value == 3.0);

    // These aren't in the hash.
    ASSERT(hash.find(2) == nullptr);
    ASSERT(hash.find(9) == nullptr);
}

DEF_TEST(DynamicHash_remove, reporter) {
    Hash hash;

    // These collide.
    Entry a = { 1, 2.0 };
    Entry b = { 5, 3.0 };
    Entry c = { 9, 4.0 };

    hash.add(&a);
    hash.add(&b);
    hash.remove(1);
    // a should be marked deleted, and b should still be findable.

    ASSERT(hash.find(1) == nullptr);
    ASSERT(hash.find(5) != nullptr);
    ASSERT(hash.find(5)->value == 3.0);

    // This will go in the same slot as 'a' did before.
    ASSERT(hash.countCollisions(9) == 0);
    hash.add(&c);
    ASSERT(hash.find(9) != nullptr);
    ASSERT(hash.find(9)->value == 4.0);
    ASSERT(hash.find(5) != nullptr);
    ASSERT(hash.find(5)->value == 3.0);
}

template<typename T> static void TestIter(skiatest::Reporter* reporter) {
    Hash hash;

    int count = 0;
    // this should fall out of loop immediately
    for (T iter(&hash); !iter.done(); ++iter) {
        ++count;
    }
    ASSERT(0 == count);

    // These collide.
    Entry a = { 1, 2.0 };
    Entry b = { 5, 3.0 };
    Entry c = { 9, 4.0 };

    hash.add(&a);
    hash.add(&b);
    hash.add(&c);

    // should see all 3 unique keys when iterating over hash
    count = 0;
    int keys[3] = {0, 0, 0};
    for (T iter(&hash); !iter.done(); ++iter) {
        int key = (*iter).key;
        keys[count] = key;
        ASSERT(hash.find(key) != nullptr);
        ++count;
    }
    ASSERT(3 == count);
    ASSERT(keys[0] != keys[1]);
    ASSERT(keys[0] != keys[2]);
    ASSERT(keys[1] != keys[2]);

    // should see 2 unique keys when iterating over hash that aren't 1
    hash.remove(1);
    count = 0;
    memset(keys, 0, sizeof(keys));
    for (T iter(&hash); !iter.done(); ++iter) {
        int key = (*iter).key;
        keys[count] = key;
        ASSERT(key != 1);
        ASSERT(hash.find(key) != nullptr);
        ++count;
    }
    ASSERT(2 == count);
    ASSERT(keys[0] != keys[1]);
}

DEF_TEST(DynamicHash_iterator, reporter) {
    TestIter<Hash::Iter>(reporter);
    TestIter<Hash::ConstIter>(reporter);
}

static void TestResetOrRewind(skiatest::Reporter* reporter, bool testReset) {
    Hash hash;
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };

    ASSERT(hash.capacity() == 0);
    hash.add(&a);
    hash.add(&b);
    ASSERT(hash.count() == 2);
    ASSERT(hash.capacity() == 4);

    if (testReset) {
        hash.reset();
        ASSERT(hash.capacity() == 0);
    } else {
        hash.rewind();
        ASSERT(hash.capacity() == 4);
    }
    ASSERT(hash.count() == 0);

    // make sure things still work
    hash.add(&a);
    hash.add(&b);
    ASSERT(hash.count() == 2);
    ASSERT(hash.capacity() == 4);

    ASSERT(hash.find(1) != nullptr);
    ASSERT(hash.find(2) != nullptr);
}

DEF_TEST(DynamicHash_reset, reporter) {
    TestResetOrRewind(reporter, true);
}

DEF_TEST(DynamicHash_rewind, reporter) {
    TestResetOrRewind(reporter, false);
}
