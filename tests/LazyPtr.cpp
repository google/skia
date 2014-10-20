#include "Test.h"
#include "SkLazyPtr.h"
#include "SkTaskGroup.h"

DEF_TEST(LazyPtr, r) {
    SkLazyPtr<int> lazy;
    int* ptr = lazy.get();

    REPORTER_ASSERT(r, ptr);
    REPORTER_ASSERT(r, lazy.get() == ptr);

    SkLazyPtr<double> neverRead;
}

namespace {

struct Racer : public SkRunnable {
    Racer() : fLazy(NULL), fSeen(NULL) {}

    virtual void run() SK_OVERRIDE { fSeen = fLazy->get(); }

    SkLazyPtr<int>* fLazy;
    int* fSeen;
};

} // namespace

DEF_TEST(LazyPtr_Threaded, r) {
    static const int kRacers = 321;

    SkLazyPtr<int> lazy;

    Racer racers[kRacers];
    for (int i = 0; i < kRacers; i++) {
        racers[i].fLazy = &lazy;
    }

    SkTaskGroup tg;
    for (int i = 0; i < kRacers; i++) {
        tg.add(racers + i);
    }
    tg.wait();

    for (int i = 1; i < kRacers; i++) {
        REPORTER_ASSERT(r, racers[i].fSeen);
        REPORTER_ASSERT(r, racers[i].fSeen == racers[0].fSeen);
    }
}
