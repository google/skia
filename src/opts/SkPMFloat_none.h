#include "SkColorPriv.h"

inline void SkPMFloat::set(SkPMColor c) {
    float scale = 1.0f / 255.0f;
    this->setA(SkGetPackedA32(c) * scale);
    this->setR(SkGetPackedR32(c) * scale);
    this->setG(SkGetPackedG32(c) * scale);
    this->setB(SkGetPackedB32(c) * scale);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::get() const {
    SkASSERT(this->isValid());
    return SkPackARGB32(this->a() * 255, this->r() * 255, this->g() * 255, this->b() * 255);
}

inline SkPMColor SkPMFloat::clamped() const {
    float a = this->a(),
          r = this->r(),
          g = this->g(),
          b = this->b();
    a = a < 0 ? 0 : (a > 1 ? 1 : a);
    r = r < 0 ? 0 : (r > 1 ? 1 : r);
    g = g < 0 ? 0 : (g > 1 ? 1 : g);
    b = b < 0 ? 0 : (b > 1 ? 1 : b);
    return SkPackARGB32(a * 255, r * 255, g * 255, b * 255);
}
