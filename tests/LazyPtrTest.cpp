#include "Test.h"
#include "SkLazyPtr.h"
#include "SkRunnable.h"
#include "SkTaskGroup.h"

namespace {

struct CreateIntFromFloat {
    CreateIntFromFloat(float val) : fVal(val) {}
    int* operator()() const { return SkNEW_ARGS(int, ((int)fVal)); }
    float fVal;
};

// As a template argument this must have external linkage.
void custom_destroy(int* ptr) { *ptr = 99; }

} // namespace

DEF_TEST(LazyPtr, r) {
    // Basic usage: calls SkNEW(int).
    SkLazyPtr<int> lazy;
    int* ptr = lazy.get();
    REPORTER_ASSERT(r, ptr);
    REPORTER_ASSERT(r, lazy.get() == ptr);

    // Advanced usage: calls a functor.
    SkLazyPtr<int> lazyFunctor;
    int* six = lazyFunctor.get(CreateIntFromFloat(6.4f));
    REPORTER_ASSERT(r, six);
    REPORTER_ASSERT(r, 6 == *six);

    // Just makes sure this is safe.
    SkLazyPtr<double> neverRead;

    // SkLazyPtr supports custom destroy methods.
    {
        SkLazyPtr<int, custom_destroy> customDestroy;
        ptr = customDestroy.get();
        // custom_destroy called here.
    }
    REPORTER_ASSERT(r, ptr);
    REPORTER_ASSERT(r, 99 == *ptr);
    // Since custom_destroy didn't actually delete ptr, we do now.
    SkDELETE(ptr);
}

namespace {

struct Racer : public SkRunnable {
    Racer() : fLazy(NULL), fSeen(NULL) {}

    void run() override { fSeen = fLazy->get(); }

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
