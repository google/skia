/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "tests/Test.h"

#include <cstdint>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

using namespace skia_private;

// Tests use of const foreach().  map.count() is of course the better way to do this.
static int count(const THashMap<int, double>& map) {
    int n = 0;
    map.foreach([&n](int, double) { n++; });
    return n;
}

DEF_TEST(HashMap, r) {
    THashMap<int, double> map;

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

    // Test walking the map with foreach(const K&, V)
    map.foreach([&](const int& key, double value) {
        REPORTER_ASSERT(r, key * 2 == value);
    });

    // Test walking the map with foreach(const K&, V*)
    map.foreach([&](const int& key, double* value) {
        REPORTER_ASSERT(r, key * 2 == *value);
    });

    // Test walking the map with foreach(const Pair&)
    map.foreach([&](const THashMap<int, double>::Pair& pair) {
        REPORTER_ASSERT(r, pair.first * 2 == pair.second);
    });

    // Test walking the map with iterators, using preincrement (++iter).
    for (THashMap<int, double>::Iter iter = map.begin(); iter != map.end(); ++iter) {
        REPORTER_ASSERT(r, iter->first * 2 == (*iter).second);
    }

    // Test walking the map with range-based for.
    for (auto& entry : map) {
        REPORTER_ASSERT(r, entry.first * 2 == entry.second);
    }

    // Ensure that iteration works equally well on a const map, using postincrement (iter++).
    const auto& cmap = map;
    for (THashMap<int, double>::Iter iter = cmap.begin(); iter != cmap.end(); iter++) {
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

    THashMap<int, double> clone = map;

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
        THashMap<int, sk_sp<SkRefCnt>> refMap;
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
    THashMap<int, std::string_view> map{{1, "one"}, {2, "two"}, {3, "three"}, {4, "four"}};
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

DEF_TEST(HashMapCtorOneElem, r) {
    // Start out with a single element. The initializer list constructor sets the capacity
    // conservatively. Searching for elements beyond the capacity should succeed.
    THashMap<int, std::string_view> map{{1, "one"}};
    REPORTER_ASSERT(r, map.count() == 1);
    REPORTER_ASSERT(r, map.approxBytesUsed() >= (sizeof(int) + sizeof(std::string_view)));

    std::string_view* found = map.find(1);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "one");

    found = map.find(2);
    REPORTER_ASSERT(r, !found);

    // Grow the collection by one element. Searching for non-existing elements should still succeed.
    map.set(2, "two");
    found = map.find(2);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == "two");

    found = map.find(3);
    REPORTER_ASSERT(r, !found);
}

DEF_TEST(HashSetCtor, r) {
    THashSet<std::string_view> set{"one", "two", "three", "four"};
    REPORTER_ASSERT(r, set.count() == 4);
    REPORTER_ASSERT(r, set.approxBytesUsed() >= 4 * sizeof(std::string_view));

    REPORTER_ASSERT(r, set.contains("one"));
    REPORTER_ASSERT(r, set.contains("two"));
    REPORTER_ASSERT(r, set.contains("three"));
    REPORTER_ASSERT(r, set.contains("four"));
    REPORTER_ASSERT(r, !set.contains("five"));
    REPORTER_ASSERT(r, !set.contains("six"));
}

DEF_TEST(HashSetCtorOneElem, r) {
    // Start out with a single element. The initializer list constructor sets the capacity
    // conservatively. Searching for elements beyond the capacity should succeed.
    THashSet<std::string_view> set{"one"};
    REPORTER_ASSERT(r, set.count() == 1);
    REPORTER_ASSERT(r, set.approxBytesUsed() >= sizeof(std::string_view));

    REPORTER_ASSERT(r, set.contains("one"));
    REPORTER_ASSERT(r, !set.contains("two"));

    // Grow the collection by one element. Searching for non-existing elements should still succeed.
    set.add("two");
    REPORTER_ASSERT(r, set.contains("one"));
    REPORTER_ASSERT(r, set.contains("two"));
    REPORTER_ASSERT(r, !set.contains("three"));
}

template <typename T>
static void test_hash_set(skiatest::Reporter* r) {
    THashSet<T> set;

    set.add(T("Hello"));
    set.add(T("World"));
    REPORTER_ASSERT(r, set.count() == 2);
    REPORTER_ASSERT(r, set.contains(T("Hello")));
    REPORTER_ASSERT(r, set.contains(T("World")));
    REPORTER_ASSERT(r, !set.contains(T("Goodbye")));
    REPORTER_ASSERT(r, set.find(T("Hello")));
    REPORTER_ASSERT(r, *set.find(T("Hello")) == T("Hello"));

    // Test walking the set with iterators, using preincrement (++iter).
    for (typename THashSet<T>::Iter iter = set.begin(); iter != set.end(); ++iter) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Test walking the set with iterators, using postincrement (iter++).
    for (typename THashSet<T>::Iter iter = set.begin(); iter != set.end(); iter++) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Test walking the set with range-based for.
    for (auto& entry : set) {
        REPORTER_ASSERT(r, entry == T("Hello") || entry == T("World"));
    }

    // Ensure that iteration works equally well on a const set.
    const auto& cset = set;
    for (typename THashSet<T>::Iter iter = cset.begin(); iter != cset.end(); iter++) {
        REPORTER_ASSERT(r, *iter == T("Hello") || *iter == T("World"));
    }

    // Ensure that range-based for works equally well on a const set.
    for (auto& entry : cset) {
        REPORTER_ASSERT(r, entry == T("Hello") || entry == T("World"));
    }

    THashSet<T> clone = set;
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
    THashSet<CopyCounter, HashCopyCounter> set;

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

    THashTable<Entry*, int, HashTraits> table;

    REPORTER_ASSERT(r, nullptr == table.findOrNull(7));

    Entry seven = { 7, 24 };
    table.set(&seven);

    REPORTER_ASSERT(r, &seven == table.findOrNull(7));
}

DEF_TEST(HashTableGrowsAndShrinks, r) {
    THashSet<int> s;
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
