#include "SkChecksum.h"
#include "SkString.h"
#include "SkTHash.h"
#include "Test.h"

namespace { uint32_t hash_int(int k) { return SkChecksum::Mix(k); } }

static void set_negative_key(int key, double* d) { *d = -key; }

DEF_TEST(HashMap, r) {
    SkTHashMap<int, double, hash_int> map;

    map.set(3, 4.0);
    REPORTER_ASSERT(r, map.count() == 1);

    double* found = map.find(3);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == 4.0);

    map.foreach(set_negative_key);
    found = map.find(3);
    REPORTER_ASSERT(r, found);
    REPORTER_ASSERT(r, *found == -3.0);

    REPORTER_ASSERT(r, !map.find(2));

    const int N = 20;

    for (int i = 0; i < N; i++) {
        map.set(i, 2.0*i);
    }
    for (int i = 0; i < N; i++) {
        double* found = map.find(i);;
        REPORTER_ASSERT(r, found);
        REPORTER_ASSERT(r, *found == i*2.0);
    }
    for (int i = N; i < 2*N; i++) {
        REPORTER_ASSERT(r, !map.find(i));
    }

    REPORTER_ASSERT(r, map.count() == N);
}

namespace { uint32_t hash_string(SkString s) { return (uint32_t)s.size(); } }

DEF_TEST(HashSet, r) {
    SkTHashSet<SkString, hash_string> set;

    set.add(SkString("Hello"));
    set.add(SkString("World"));

    REPORTER_ASSERT(r, set.count() == 2);

    REPORTER_ASSERT(r, set.contains(SkString("Hello")));
    REPORTER_ASSERT(r, set.contains(SkString("World")));
    REPORTER_ASSERT(r, !set.contains(SkString("Goodbye")));
}
