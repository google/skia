#include "Benchmark.h"
#include "SkPMFloat.h"

// Used to prevent the compiler from optimizing away the whole loop.
volatile uint32_t blackhole = 0;

// Not a great random number generator, but it's very fast.
// The code we're measuring is quite fast, so low overhead is essential.
static uint32_t lcg_rand(uint32_t* seed) {
    *seed *= 1664525;
    *seed += 1013904223;
    return *seed;
}

struct PMFloatBench : public Benchmark {
    explicit PMFloatBench(bool clamp) : fClamp(clamp) {}

    const char* onGetName() SK_OVERRIDE { return fClamp ? "SkPMFloat_clamp" : "SkPMFloat_get"; }
    bool isSuitableFor(Backend backend) SK_OVERRIDE { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        // Unlike blackhole, junk can and probably will be a register.
        uint32_t junk = 0;
        uint32_t seed = 0;
        for (int i = 0; i < loops; i++) {
        #ifdef SK_DEBUG
            // Our SkASSERTs will remind us that it's technically required that we premultiply.
            SkPMColor c = SkPreMultiplyColor(lcg_rand(&seed));
        #else
            // But it's a lot faster not to, and this code won't really mind the non-PM colors.
            SkPMColor c = lcg_rand(&seed);
        #endif
            SkPMFloat pmf = SkPMFloat::FromPMColor(c);
            SkPMColor back = fClamp ? pmf.clamped() : pmf.get();
            junk ^= back;
        }
        blackhole ^= junk;
    }

    bool fClamp;
};
DEF_BENCH(return new PMFloatBench( true);)
DEF_BENCH(return new PMFloatBench(false);)
