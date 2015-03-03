#ifndef SkPM_DEFINED
#define SkPM_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "Sk4x.h"

// A pre-multiplied color storing each component as a float in the range [0, 255].
class SK_STRUCT_ALIGN(16) SkPMFloat {
public:
    // Normal POD copies and do-nothing initialization.
    SkPMFloat()                            = default;
    SkPMFloat(const SkPMFloat&)            = default;
    SkPMFloat& operator=(const SkPMFloat&) = default;

    // Freely autoconvert between SkPMFloat and Sk4f.
    /*implicit*/ SkPMFloat(const Sk4f& fs) { fs.storeAligned(fColor); }
    /*implicit*/ operator Sk4f() const { return Sk4f::LoadAligned(fColor); }

    float a() const { return fColor[SK_A32_SHIFT / 8]; }
    float r() const { return fColor[SK_R32_SHIFT / 8]; }
    float g() const { return fColor[SK_G32_SHIFT / 8]; }
    float b() const { return fColor[SK_B32_SHIFT / 8]; }

    void setA(float val) { fColor[SK_A32_SHIFT / 8] = val; }
    void setR(float val) { fColor[SK_R32_SHIFT / 8] = val; }
    void setG(float val) { fColor[SK_G32_SHIFT / 8] = val; }
    void setB(float val) { fColor[SK_B32_SHIFT / 8] = val; }

    void set(SkPMColor);

    // get() and clamped() round component values to the nearest integer.
    SkPMColor     get() const;  // May SkASSERT(this->isValid()).  Some implementations may clamp.
    SkPMColor clamped() const;  // Will clamp all values to [0, 255].  Then may assert isValid().

    bool isValid() const {
        return this->a() >= 0 && this->a() <= 255
            && this->r() >= 0 && this->r() <= this->a()
            && this->g() >= 0 && this->g() <= this->a()
            && this->b() >= 0 && this->b() <= this->a();
    }

private:
    // We mirror SkPMColor order only to make set()/get()/clamped() as fast as possible.
    float fColor[4];
};

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/SkPMFloat_SSE2.h"
#elif defined(__ARM_NEON__)
    #include "../opts/SkPMFloat_neon.h"
#else
    #include "../opts/SkPMFloat_none.h"
#endif

#endif//SkPM_DEFINED
