/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRandom.h"
#include "Test.h"
// This is a GPU-backend specific test
#if SK_SUPPORT_GPU
#include "GrOrderedSet.h"

typedef GrOrderedSet<int> Set;
typedef GrOrderedSet<const char*, GrStrLess> Set2;

DEF_TEST(GrOrderedSet, reporter) {
    Set set;

    REPORTER_ASSERT(reporter, set.empty());

    SkRandom r;

    int count[1000] = {0};
    // add 10K ints
    for (int i = 0; i < 10000; ++i) {
        int x = r.nextU() % 1000;
        Set::Iter xi = set.insert(x);
        REPORTER_ASSERT(reporter, *xi == x);
        REPORTER_ASSERT(reporter, !set.empty());
        count[x] = 1;
    }
    set.insert(0);
    count[0] = 1;
    set.insert(999);
    count[999] = 1;
    int totalCount = 0;
    for (int i = 0; i < 1000; ++i) {
        totalCount += count[i];
    }
    REPORTER_ASSERT(reporter, *set.begin() == 0);
    REPORTER_ASSERT(reporter, *set.last() == 999);
    REPORTER_ASSERT(reporter, --(++set.begin()) == set.begin());
    REPORTER_ASSERT(reporter, --set.end() == set.last());
    REPORTER_ASSERT(reporter, set.count() == totalCount);

    int c = 0;
    // check that we iterate through the correct number of
    // elements and they are properly sorted.
    for (Set::Iter a = set.begin(); set.end() != a; ++a) {
        Set::Iter b = a;
        ++b;
        ++c;
        REPORTER_ASSERT(reporter, b == set.end() || *a <= *b);
    }
    REPORTER_ASSERT(reporter, c == set.count());

    // check that the set finds all ints and only ints added to set
    for (int i = 0; i < 1000; ++i) {
        bool existsFind = set.find(i) != set.end();
        bool existsCount = 0 != count[i];
        REPORTER_ASSERT(reporter, existsFind == existsCount);
    }
    // remove all the ints between 100 and 200.
    for (int i = 100; i < 200; ++i) {
        set.remove(set.find(i));
        if (1 == count[i]) {
            count[i] = 0;
            --totalCount;
        }
        REPORTER_ASSERT(reporter, set.count() == totalCount);
        REPORTER_ASSERT(reporter, set.find(i) == set.end());
    }
    // remove the 0 entry. (tests removing begin())
    REPORTER_ASSERT(reporter, *set.begin() == 0);
    REPORTER_ASSERT(reporter, *(--set.end()) == 999);
    set.remove(set.find(0));
    count[0] = 0;
    --totalCount;
    REPORTER_ASSERT(reporter, set.count() == totalCount);
    REPORTER_ASSERT(reporter, set.find(0) == set.end());
    REPORTER_ASSERT(reporter, 0 < *set.begin());

    // remove all the 999 entries (tests removing last()).
    set.remove(set.find(999));
    count[999] = 0;
    --totalCount;
    REPORTER_ASSERT(reporter, set.count() == totalCount);
    REPORTER_ASSERT(reporter, set.find(999) == set.end());
    REPORTER_ASSERT(reporter, 999 > *(--set.end()));
    REPORTER_ASSERT(reporter, set.last() == --set.end());

    // Make sure iteration still goes through correct number of entries
    // and is still sorted correctly.
    c = 0;
    for (Set::Iter a = set.begin(); set.end() != a; ++a) {
        Set::Iter b = a;
        ++b;
        ++c;
        REPORTER_ASSERT(reporter, b == set.end() || *a <= *b);
    }
    REPORTER_ASSERT(reporter, c == set.count());

    // repeat check that the set finds all ints and only ints added to set
    for (int i = 0; i < 1000; ++i) {
        bool existsFind = set.find(i) != set.end();
        bool existsCount = 0 != count[i];
        REPORTER_ASSERT(reporter, existsFind == existsCount);
    }

    // remove all entries
    while (!set.empty()) {
        set.remove(set.begin());
    }

    // test reset on empty set.
    set.reset();
    REPORTER_ASSERT(reporter, set.empty());


    // test using c strings
    const char* char1 = "dog";
    const char* char2 = "cat";
    const char* char3 = "dog";

    Set2 set2;

    set2.insert("ape");
    set2.insert(char1);
    set2.insert(char2);
    set2.insert(char3);
    set2.insert("ant");
    set2.insert("cat");

    REPORTER_ASSERT(reporter, set2.count() == 4);
    REPORTER_ASSERT(reporter, set2.find("dog") == set2.last());
    REPORTER_ASSERT(reporter, set2.find("cat") != set2.end());
    REPORTER_ASSERT(reporter, set2.find("ant") == set2.begin());
    REPORTER_ASSERT(reporter, set2.find("bug") == set2.end());

    set2.remove(set2.find("ant"));
    REPORTER_ASSERT(reporter, set2.find("ant") == set2.end());
    REPORTER_ASSERT(reporter, set2.count() == 3);

    set2.reset();
    REPORTER_ASSERT(reporter, set2.empty());
}

#endif
