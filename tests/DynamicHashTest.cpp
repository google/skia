/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkTDynamicHash.h"

namespace {

struct Entry {
    int key;
    double value;
};

const int& GetKey(const Entry& entry) { return entry.key; }
uint32_t GetHash(const int& key) { return key; }
bool AreEqual(const Entry& entry, const int& key) { return entry.key == key; }

class Hash : public SkTDynamicHash<Entry, int, GetKey, GetHash, AreEqual> {
public:
    Hash() : INHERITED() {}
    Hash(int capacity) : INHERITED(capacity) {}

    // Promote protected methods to public for this test.
    int capacity() const { return this->INHERITED::capacity(); }
    int countCollisions(const int& key) const { return this->INHERITED::countCollisions(key); }

private:
    typedef SkTDynamicHash<Entry, int, GetKey, GetHash, AreEqual> INHERITED;
};

}  // namespace

#define ASSERT(x) REPORTER_ASSERT(reporter, x)

static void test_growth(skiatest::Reporter* reporter) {
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };
    Entry c = { 3, 4.0 };
    Entry d = { 4, 5.0 };
    Entry e = { 5, 6.0 };

    Hash hash(4);
    ASSERT(hash.capacity() == 4);

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

static void test_add(skiatest::Reporter* reporter) {
    Hash hash;
    Entry a = { 1, 2.0 };
    Entry b = { 2, 3.0 };

    ASSERT(hash.count() == 0);
    hash.add(&a);
    ASSERT(hash.count() == 1);
    hash.add(&b);
    ASSERT(hash.count() == 2);
}

static void test_lookup(skiatest::Reporter* reporter) {
    Hash hash(4);
    ASSERT(hash.capacity() == 4);

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
    ASSERT(hash.find(1) != NULL);
    ASSERT(hash.find(1)->value == 2.0);
    ASSERT(hash.find(5) != NULL);
    ASSERT(hash.find(5)->value == 3.0);

    // These aren't in the hash.
    ASSERT(hash.find(2) == NULL);
    ASSERT(hash.find(9) == NULL);
}

static void test_remove(skiatest::Reporter* reporter) {
    Hash hash(4);
    ASSERT(hash.capacity() == 4);

    // These collide.
    Entry a = { 1, 2.0 };
    Entry b = { 5, 3.0 };
    Entry c = { 9, 4.0 };

    hash.add(&a);
    hash.add(&b);
    hash.remove(1);
    // a should be marked deleted, and b should still be findable.

    ASSERT(hash.find(1) == NULL);
    ASSERT(hash.find(5) != NULL);
    ASSERT(hash.find(5)->value == 3.0);

    // This will go in the same slot as 'a' did before.
    ASSERT(hash.countCollisions(9) == 0);
    hash.add(&c);
    ASSERT(hash.find(9) != NULL);
    ASSERT(hash.find(9)->value == 4.0);
    ASSERT(hash.find(5) != NULL);
    ASSERT(hash.find(5)->value == 3.0);
}

DEF_TEST(DynamicHash, reporter) {
    test_growth(reporter);
    test_add(reporter);
    test_lookup(reporter);
    test_remove(reporter);
}
