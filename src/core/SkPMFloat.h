#ifndef SkPM_DEFINED
#define SkPM_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "Sk4x.h"

// A pre-multiplied color storing each component in the same order as SkPMColor,
// but as a float in the range [0, 255].
class SK_STRUCT_ALIGN(16) SkPMFloat {
public:
    static SkPMFloat FromPMColor(SkPMColor c) { return SkPMFloat(c); }
    static SkPMFloat FromARGB(float a, float r, float g, float b) { return SkPMFloat(a,r,g,b); }

    // May be more efficient than one at a time.  No special alignment assumed for SkPMColors.
    static void From4PMColors(SkPMFloat[4], const SkPMColor[4]);

    explicit SkPMFloat(SkPMColor);
    SkPMFloat(float a, float r, float g, float b) {
        // TODO: faster when specialized?
        fColor[SK_A32_SHIFT / 8] = a;
        fColor[SK_R32_SHIFT / 8] = r;
        fColor[SK_G32_SHIFT / 8] = g;
        fColor[SK_B32_SHIFT / 8] = b;
    }

    // Uninitialized.
    SkPMFloat() {}

    // Copy and assign are fastest if we remind the compiler we work best as Sk4f.
    SkPMFloat(const SkPMFloat& that) { Sk4f(that).storeAligned(fColor); }
    SkPMFloat& operator=(const SkPMFloat& that) {
        Sk4f(that).storeAligned(fColor);
        return *this;
    }

    // Freely autoconvert between SkPMFloat and Sk4f.
    /*implicit*/ SkPMFloat(const Sk4f& fs) { fs.storeAligned(fColor); }
    /*implicit*/ operator Sk4f() const { return Sk4f::LoadAligned(fColor); }

    float a() const { return fColor[SK_A32_SHIFT / 8]; }
    float r() const { return fColor[SK_R32_SHIFT / 8]; }
    float g() const { return fColor[SK_G32_SHIFT / 8]; }
    float b() const { return fColor[SK_B32_SHIFT / 8]; }

    // get() and clamped() round component values to the nearest integer.
    SkPMColor     get() const;  // May SkASSERT(this->isValid()).  Some implementations may clamp.
    SkPMColor clamped() const;  // Will clamp all values to [0, 255].  Then may assert isValid().

    // 4-at-a-time versions of get() and clamped().  Like From4PMColors(), no alignment assumed.
    static void To4PMColors(SkPMColor[4], const SkPMFloat[4]);
    static void ClampTo4PMColors(SkPMColor[4], const SkPMFloat[4]);

    bool isValid() const {
        return this->a() >= 0 && this->a() <= 255
            && this->r() >= 0 && this->r() <= this->a()
            && this->g() >= 0 && this->g() <= this->a()
            && this->b() >= 0 && this->b() <= this->a();
    }

private:
    float fColor[4];
};

#if SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
    #include "../opts/SkPMFloat_SSSE3.h"
#elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
    #include "../opts/SkPMFloat_SSE2.h"
#elif defined(__ARM_NEON__)
    #include "../opts/SkPMFloat_neon.h"
#else
    #include "../opts/SkPMFloat_none.h"
#endif

#endif//SkPM_DEFINED
