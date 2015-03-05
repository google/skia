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

// I'm having better luck getting these to constant-propagate away as template parameters.
template <bool kClamp, bool kWide>
struct PMFloatBench : public Benchmark {
    PMFloatBench() {}

    const char* onGetName() SK_OVERRIDE {
        switch (kClamp << 1 | kWide) {
            case 0: return "SkPMFloat_get_1x";
            case 1: return "SkPMFloat_get_4x";
            case 2: return "SkPMFloat_clamp_1x";
            case 3: return "SkPMFloat_clamp_4x";
        }
        SkFAIL("unreachable");
        return "oh bother";
    }
    bool isSuitableFor(Backend backend) SK_OVERRIDE { return backend == kNonRendering_Backend; }

    void onDraw(const int loops, SkCanvas* canvas) SK_OVERRIDE {
        // Unlike blackhole, junk can and probably will be a register.
        uint32_t junk = 0;
        uint32_t seed = 0;
        for (int i = 0; i < loops; i++) {
            SkPMColor colors[4];
        #ifdef SK_DEBUG
            for (int i = 0; i < 4; i++) {
                // Our SkASSERTs will remind us that it's technically required that we premultiply.
                colors[i] = SkPreMultiplyColor(lcg_rand(&seed));
            }
        #else
            // But it's a lot faster not to, and this code won't really mind the non-PM colors.
            (void)lcg_rand(&seed);
            colors[0] = seed + 0;
            colors[1] = seed + 1;
            colors[2] = seed + 2;
            colors[3] = seed + 3;
        #endif

            SkPMFloat floats[4];
            if (kWide) {
                SkPMFloat::From4PMColors(floats, colors);
            } else {
                for (int i = 0; i < 4; i++) {
                    floats[i] = SkPMFloat::FromPMColor(colors[i]);
                }
            }

            SkPMColor back[4];
            switch (kClamp << 1 | kWide) {
                case 0: for (int i = 0; i < 4; i++) { back[i] = floats[i].get(); }     break;
                case 1: SkPMFloat::To4PMColors(back, floats);                          break;
                case 2: for (int i = 0; i < 4; i++) { back[i] = floats[i].clamped(); } break;
                case 3: SkPMFloat::ClampTo4PMColors(back, floats);                     break;
            }
            for (int i = 0; i < 4; i++) {
                junk ^= back[i];
            }
        }
        blackhole ^= junk;
    }
};

// Extra () help DEF_BENCH not get confused by the comma inside the <>.
DEF_BENCH(return (new PMFloatBench< true,  true>);)
DEF_BENCH(return (new PMFloatBench<false,  true>);)
DEF_BENCH(return (new PMFloatBench< true, false>);)
DEF_BENCH(return (new PMFloatBench<false, false>);)
