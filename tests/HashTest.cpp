/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/private/SkChecksum.h"
#include "include/private/SkTHash.h"
#include "src/core/SkOpts.h"
#include "tests/Test.h"

#include <tuple>

// Tests use of const foreach().  map.count() is of course the better way to do this.
static int count(const SkTHashMap<int, double>& map) {
    int n = 0;
    map.foreach([&n](int, double) { n++; });
    return n;
}

DEF_TEST(HashMap, r) {
    SkTHashMap<int, double> map;

    map.set(3, 4.0);
    REPORTER_ASSERT(r, map.count() == 1);

    REPORTER_ASSERT(r, map.approxBytesUsed() > 0);

    double* found = map.find(3);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == 4.0);

    map.foreach([](int key, double* d){ *d = -key; });
    REPORTER_ASSERT(r, count(map) == 1);

    found = map.find(3);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == -3.0);

    REPORTER_ASSERT(r, !map.find(2));

    const int N = 20;

    for (int i = 0; i < N; i++) {
        map.set(i, 2.0*i);
    }

    // Test walking the map with iterators, using preincrement (++iter).
    for (SkTHashMap<int, double>::Iter iter = map.begin(); iter != map.end(); ++iter) {
        REPORTER_ASSERT(r, iter->first * 2 == (*iter).second);
    }

    // Test walking the map with range-based for.
    for (auto& entry : map) {
        REPORTER_ASSERT(r, entry.first * 2 == entry.second);
    }

    // Ensure that iteration works equally well on a const map, using postincrement (iter++).
    const auto& cmap = map;
    for (SkTHashMap<int, double>::Iter iter = cmap.begin(); iter != cmap.end(); iter++) {
        REPORTER_ASSERT(r, iter->first * 2 == (*iter).second);
    }

    // Ensure that range-based for works equally well on a const map.
    for (const auto& entry : cmap) {
        REPORTER_ASSERT(r, entry.first * 2 == entry.second);
    }

    // Ensure that structured bindings work.
    for (const auto& [number, timesTwo] : cmap) {
        REPORTER_ASSERT(r, number * 2 == timesTwo);
    }

    SkTHashMap<int, double> clone = map;

    for (int i = 0; i < N; i++) {
        found = map.find(i);
        REPORTER_ASSERT(r, found);
        REPORTER_ASSERT(r, *found == i*2.0);

        found = clone.find(i);
        REPORTER_ASSERT(r, found);
        REPORTER_ASSERT(r, *found == i*2.0);
    }
    for (int i = N; i < 2*N; i++) {
        REPORTER_ASSERT(r, !map.find(i));
        REPORTER_ASSERT(r, !clone.find(i));
    }

    REPORTER_ASSERT(r, map.count() == N);
    REPORTER_ASSERT(r, clone.count() == N);

    for (int i = 0; i < N/2; i++) {
        map.remove(i);
    }
    for (int i = 0; i < N; i++) {
        found = map.find(i);
        REPORTER_ASSERT(r, (found == nullptr) == (i < N/2));

        found = clone.find(i);
        REPORTER_ASSERT(r, *found == i*2.0);
    }
    REPORTER_ASSERT(r, map.count() == N/2);
    REPORTER_ASSERT(r, clone.count() == N);

    map.reset();
    REPORTER_ASSERT(r, map.count() == 0);
    REPORTER_ASSERT(r, clone.count() == N);

    clone = map;
    REPORTER_ASSERT(r, clone.count() == 0);

    {
        // Test that we don't leave dangling values in empty slots.
        SkTHashMap<int, sk_sp<SkRefCnt>> refMap;
        auto ref = sk_make_sp<SkRefCnt>();
        REPORTER_ASSERT(r, ref->unique());

        refMap.set(0, ref);
        REPORTER_ASSERT(r, refMap.count() == 1);
        REPORTER_ASSERT(r, !ref->unique());

        refMap.remove(0);
        REPORTER_ASSERT(r, refMap.count() == 0);
        REPORTER_ASSERT(r, ref->unique());
    }
}

DEF_TEST(HashMapCtor, r) {
    SkTHashMap<int, std::string_view> map{{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};
    REPORTER_ASSERT(r, map.count() == 4);
    REPORTER_ASSERT(r, map.approxBytesUsed() >= 4 * (sizeof(int) + sizeof(std::string_view)));

    std::string_view* found = map.find(1);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "one");

    found = map.find(2);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "two");

    found = map.find(3);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "three");

    found = map.find(4);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "four");

    found = map.find(5);
    REPORTER_ASSERT(r, !found);

    found = map.find(6);
    REPORTER_ASSERT(r, !found);
}

DEF_TEST(HashSetCtor, r) {
    SkTHashSet<std::string_view> set{"one", "two", "three", "four"};
    REPORTER_ASSERT(r, set.count() == 4);
    REPORTER_ASSERT(r, set.approxBytesUsed() >= 4 * sizeof(std::string_view));

    REPORTER_ASSERT(r, set.contains("one"));
    REPORTER_ASSERT(r, set.contains("two"));
    REPORTER_ASSERT(r, set.contains("three"));
    REPORTER_ASSERT(r, set.contains("four"));
    REPORTER_ASSERT(r, !set.contains("five"));
    REPORTER_ASSERT(r, !set.contains("six"));
}

template <typename T>
static void test_hash_set(skiatest::Reporter* r) {
    SkTHashSet<T> set;

    set.add(T("Hello"));
    set.add(T("World"));
    REPORTER_ASSERT(r, set.count() == 2);
    REPORTER_ASSERT(r, set.contains(T("Hello")));
    REPORTER_ASSERT(r, set.contains(T("World")));
    REPORTER_ASSERT(r, !set.contains(T("Goodbye")));
    REPORTER_ASSERT(r, set.find(T("Hello")));
    REPORTER_ASSERT(r, *set.find(T("Hello")) == T("Hello"));

    // Test walking the set with iterators, using preincrement (++iter).
    for (typename SkTHashSet<T>::Iter iter = set.begin(); iter != set.end(); ++iter) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Test walking the set with iterators, using postincrement (iter++).
    for (typename SkTHashSet<T>::Iter iter = set.begin(); iter != set.end(); iter++) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Test walking the set with range-based for.
    for (auto& entry : set) {
        REPORTER_ASSERT(r, entry == T("Hello") || entry == T("World"));
    }

    // Ensure that iteration works equally well on a const set.
    const auto& cset = set;
    for (typename SkTHashSet<T>::Iter iter = cset.begin(); iter != cset.end(); iter++) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Ensure that range-based for works equally well on a const set.
    for (auto& entry : cset) {
        REPORTER_ASSERT(r, entry == T("Hello") || entry == T("World"));
    }

    SkTHashSet<T> clone = set;
    REPORTER_ASSERT(r, clone.count() == 2);
    REPORTER_ASSERT(r, clone.contains(T("Hello")));
    REPORTER_ASSERT(r, clone.contains(T("World")));
    REPORTER_ASSERT(r, !clone.contains(T("Goodbye")));
    REPORTER_ASSERT(r, clone.find(T("Hello")));
    REPORTER_ASSERT(r, *clone.find(T("Hello")) == T("Hello"));

    set.remove(T("Hello"));
    REPORTER_ASSERT(r, !set.contains(T("Hello")));
    REPORTER_ASSERT(r, set.count() == 1);
    REPORTER_ASSERT(r, clone.contains(T("Hello")));
    REPORTER_ASSERT(r, clone.count() == 2);

    set.reset();
    REPORTER_ASSERT(r, set.count() == 0);

    clone = set;
    REPORTER_ASSERT(r, clone.count() == 0);
}

DEF_TEST(HashSetWithSkString, r) {
    test_hash_set<SkString>(r);
}

DEF_TEST(HashSetWithStdString, r) {
    test_hash_set<std::string>(r);
}

DEF_TEST(HashSetWithStdStringView, r) {
    test_hash_set<std::string_view>(r);
}

namespace {

class CopyCounter {
public:
    CopyCounter() : fID(0), fCounter(nullptr) {}

    CopyCounter(uint32_t id, uint32_t* counter) : fID(id), fCounter(counter) {}

    CopyCounter(const CopyCounter& other)
        : fID(other.fID)
        , fCounter(other.fCounter) {
        SkASSERT(fCounter);
        *fCounter += 1;
    }

    void operator=(const CopyCounter& other) {
        fID = other.fID;
        fCounter = other.fCounter;
        *fCounter += 1;
    }

    CopyCounter(CopyCounter&& other) { *this = std::move(other); }
    void operator=(CopyCounter&& other) {
        fID = other.fID;
        fCounter = other.fCounter;
    }


    bool operator==(const CopyCounter& other) const {
        return fID == other.fID;
    }

private:
    uint32_t  fID;
    uint32_t* fCounter;
};

struct HashCopyCounter {
    uint32_t operator()(const CopyCounter&) const {
        return 0; // let them collide, what do we care?
    }
};

}  // namespace

DEF_TEST(HashSetCopyCounter, r) {
    SkTHashSet<CopyCounter, HashCopyCounter> set;

    uint32_t globalCounter = 0;
    CopyCounter copyCounter1(1, &globalCounter);
    CopyCounter copyCounter2(2, &globalCounter);
    REPORTER_ASSERT(r, globalCounter == 0);

    set.add(copyCounter1);
    REPORTER_ASSERT(r, globalCounter == 1);
    REPORTER_ASSERT(r, set.contains(copyCounter1));
    REPORTER_ASSERT(r, globalCounter == 1);
    set.add(copyCounter1);
    // We allow copies for same-value adds for now.
    REPORTER_ASSERT(r, globalCounter == 2);

    set.add(copyCounter2);
    REPORTER_ASSERT(r, globalCounter == 3);
    REPORTER_ASSERT(r, set.contains(copyCounter1));
    REPORTER_ASSERT(r, set.contains(copyCounter2));
    REPORTER_ASSERT(r, globalCounter == 3);
    set.add(copyCounter1);
    set.add(copyCounter2);
    // We allow copies for same-value adds for now.
    REPORTER_ASSERT(r, globalCounter == 5);
}


DEF_TEST(HashFindOrNull, r) {
    struct Entry {
        int key = 0;
        int val = 0;
    };

    struct HashTraits {
        static int GetKey(const Entry* e) { return e->key; }
        static uint32_t Hash(int key) { return key; }
    };

    SkTHashTable<Entry*, int, HashTraits> table;

    REPORTER_ASSERT(r, nullptr == table.findOrNull(7));

    Entry seven = { 7, 24 };
    table.set(&seven);

    REPORTER_ASSERT(r, &seven == table.findOrNull(7));
}

DEF_TEST(HashTableGrowsAndShrinks, r) {
    SkTHashSet<int> s;
    auto check_count_cap = [&](int count, int cap) {
        REPORTER_ASSERT(r, s.count() == count);
        REPORTER_ASSERT(r, s.approxBytesUsed() == (sizeof(int) + sizeof(uint32_t)) * cap);
    };

    // Add and remove some elements to test basic growth and shrink patterns.
                 check_count_cap(0,0);
    s.add(1);    check_count_cap(1,4);
    s.add(2);    check_count_cap(2,4);
    s.add(3);    check_count_cap(3,4);
    s.add(4);    check_count_cap(4,8);

    s.remove(4); check_count_cap(3,8);
    s.remove(3); check_count_cap(2,4);
    s.remove(2); check_count_cap(1,4);
    s.remove(1); check_count_cap(0,4);

    s.add(1);    check_count_cap(1,4);
    s.add(2);    check_count_cap(2,4);
    s.add(3);    check_count_cap(3,4);
    s.add(4);    check_count_cap(4,8);

    // Add and remove single elements repeatedly to test hysteresis
    // avoids reallocating these small tables all the time.
    for (int i = 0; i < 10; i++) {
        s.   add(5); check_count_cap(5,8);
        s.remove(5); check_count_cap(4,8);
    }

    s.remove(4);     check_count_cap(3,8);
    for (int i = 0; i < 10; i++) {
        s.   add(4); check_count_cap(4,8);
        s.remove(4); check_count_cap(3,8);
    }

    s.remove(3);     check_count_cap(2,4);
    for (int i = 0; i < 10; i++) {
        s.   add(4); check_count_cap(3,4);
        s.remove(4); check_count_cap(2,4);
    }

    s.remove(2);     check_count_cap(1,4);
    for (int i = 0; i < 10; i++) {
        s.   add(2); check_count_cap(2,4);
        s.remove(2); check_count_cap(1,4);
    }
}

DEF_TEST(HashCollision, r) {

    // Two different sets of data. Same hash.
    uint8_t data1[] = {
        15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 63, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 3, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 28, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 6, 0, 0, 0, 13, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 6, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 8, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 8, 0, 0, 0, 17, 0, 0, 0, 18, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 128, 63, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 27, 0, 0, 0, 16, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 16, 0, 0, 0, 21, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 28, 0, 0, 0, 21, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 16, 0, 0, 0, 21, 0, 0, 0, 22, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 28, 0, 0, 0, 21, 0, 0, 0, 22, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 31, 0, 0, 0, 27, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 0, 0, 32, 0, 0, 0, 27, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 33, 0, 0, 0, 23, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 33, 0, 0, 0, 23, 0, 0, 0, 24, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    uint8_t data2[] = {
        15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 11, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 21, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 2, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 63, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 4, 0, 0, 0, 5, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 40, 0, 0, 0, 3, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 18, 0, 0, 0, 7, 0, 0, 0, 5, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 16, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 20, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 24, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 28, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 6, 0, 0, 0, 13, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 6, 0, 0, 0, 13, 0, 0, 0, 14, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 32, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 36, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 8, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 8, 0, 0, 0, 17, 0, 0, 0, 18, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 40, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 44, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 48, 0, 0, 0, 0, 0, 0, 0, 15, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 52, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 17, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 128, 63, 0, 0, 0, 0, 0, 0, 0, 0, 20, 0, 0, 0, 27, 0, 0, 0, 16, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 16, 0, 0, 0, 21, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 28, 0, 0, 0, 21, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 16, 0, 0, 0, 21, 0, 0, 0, 22, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 28, 0, 0, 0, 21, 0, 0, 0, 22, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 31, 0, 0, 0, 27, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 26, 0, 0, 0, 32, 0, 0, 0, 27, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 22, 0, 0, 0, 33, 0, 0, 0, 23, 0, 0, 0, 255, 255, 255, 255, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 0, 0, 0, 33, 0, 0, 0, 23, 0, 0, 0, 24, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    REPORTER_ASSERT(r, sizeof(data1) == sizeof(data2));
    REPORTER_ASSERT(r, memcmp(data1, data2, sizeof(data1)) != 0);

    uint32_t hash1 = SkOpts::hash(data1, sizeof(data1), 0);
    uint32_t hash2 = SkOpts::hash(data2, sizeof(data2), 0);
    REPORTER_ASSERT(r, hash1 != hash2);
}
