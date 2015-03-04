#include "SkColorPriv.h"

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    *this = SkPMFloat::FromARGB(SkGetPackedA32(c),
                                SkGetPackedR32(c),
                                SkGetPackedG32(c),
                                SkGetPackedB32(c));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    return SkPackARGB32(this->a()+0.5f, this->r()+0.5f, this->g()+0.5f, this->b()+0.5f);
}

inline SkPMColor SkPMFloat::clamped() const {
    float a = this->a(),
          r = this->r(),
          g = this->g(),
          b = this->b();
    a = a < 0 ? 0 : (a > 255 ? 255 : a);
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    return SkPackARGB32(a+0.5f, r+0.5f, g+0.5f, b+0.5f);
}
