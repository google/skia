#ifndef SkPM_DEFINED
#define SkPM_DEFINED

#include "SkTypes.h"
#include "SkColor.h"

// As usual, there are two ways to increase alignment... the MSVC way and the everyone-else way.
#ifdef _MSC_VER
    #define ALIGN(N) __declspec(align(N))
#else
    #define ALIGN(N) __attribute__((aligned(N)))
#endif

// A pre-multiplied color in the same order as SkPMColor storing each component as a float.
struct ALIGN(16) SkPMFloat {
    float fColor[4];

    float a() const { return fColor[SK_A32_SHIFT / 8]; }
    float r() const { return fColor[SK_R32_SHIFT / 8]; }
    float g() const { return fColor[SK_G32_SHIFT / 8]; }
    float b() const { return fColor[SK_B32_SHIFT / 8]; }

    void setA(float val) { fColor[SK_A32_SHIFT / 8] = val; }
    void setR(float val) { fColor[SK_R32_SHIFT / 8] = val; }
    void setG(float val) { fColor[SK_G32_SHIFT / 8] = val; }
    void setB(float val) { fColor[SK_B32_SHIFT / 8] = val; }

    void set(SkPMColor);

    SkPMColor     get() const;  // May SkASSERT(this->isValid()).
    SkPMColor clamped() const;  // Will clamp all values to [0,1], then SkASSERT(this->isValid()).

    bool isValid() const {
        return this->a() >= 0 && this->a() <= 1
            && this->r() >= 0 && this->r() <= this->a()
            && this->g() >= 0 && this->g() <= this->a()
            && this->b() >= 0 && this->b() <= this->a();
    }
};
#undef ALIGN

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/SkPMFloat_SSE2.h"
#elif defined(__ARM_NEON__)
    #include "../opts/SkPMFloat_neon.h"
#else
    #include "../opts/SkPMFloat_none.h"
#endif

#endif//SkPM_DEFINED
