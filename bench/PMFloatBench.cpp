#include "Benchmark.h"
#include "SkPMFloat.h"
#include "SkRandom.h"

struct PMFloatBench : public Benchmark {
    explicit PMFloatBench(bool clamp) : fClamp(clamp) {}

    const char* onGetName() SK_OVERRIDE { return fClamp ? "SkPMFloat_clamp" : "SkPMFloat_get"; }
    bool isSuitableFor(Backend backend) SK_OVERRIDE { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        SkRandom rand;
        for (int i = 0; i < loops; i++) {
            SkPMColor c = SkPreMultiplyColor(rand.nextU());
            SkPMFloat pmf;
            pmf.set(c);
            SkPMColor back = fClamp ? pmf.clamped() : pmf.get();
            if (c != back) { SkFAIL("no joy"); }  // This conditional makes this not compile away.
        }
    }

    bool fClamp;
};
DEF_BENCH(return new PMFloatBench( true);)
DEF_BENCH(return new PMFloatBench(false);)
