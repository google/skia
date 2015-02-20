#include "SkChecksum.h"
#include "SkString.h"
#include "SkTHash.h"
#include "Test.h"

namespace { uint32_t hash_int(const int& k) { return SkChecksum::Mix(k); } }

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

    map.reset();
    REPORTER_ASSERT(r, map.count() == 0);
}

namespace { uint32_t hash_string(const SkString& s) { return SkToInt(s.size()); } }

DEF_TEST(HashSet, r) {
    SkTHashSet<SkString, hash_string> set;

    set.add(SkString("Hello"));
    set.add(SkString("World"));

    REPORTER_ASSERT(r, set.count() == 2);

    REPORTER_ASSERT(r, set.contains(SkString("Hello")));
    REPORTER_ASSERT(r, set.contains(SkString("World")));
    REPORTER_ASSERT(r, !set.contains(SkString("Goodbye")));

    set.reset();
    REPORTER_ASSERT(r, set.count() == 0);
}

namespace {

class CopyCounter {
public:
    CopyCounter() : fID(0), fCounter(NULL) {}

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

    bool operator==(const CopyCounter& other) const {
        return fID == other.fID;
    }

private:
    uint32_t  fID;
    uint32_t* fCounter;
};

uint32_t hash_copy_counter(const CopyCounter&) {
    return 0; // let them collide, what do we care?
}

}

DEF_TEST(HashSetCopyCounter, r) {
    SkTHashSet<CopyCounter, hash_copy_counter> set;

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
