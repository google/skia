/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    *this = SkPMFloat::FromARGB(SkGetPackedA32(c),
                                SkGetPackedR32(c),
                                SkGetPackedG32(c),
                                SkGetPackedB32(c));
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::trunc() const {
    return SkPackARGB32(this->a(), this->r(), this->g(), this->b());
}

inline SkPMColor SkPMFloat::round() const {
    SkPMColor c = SkPackARGB32(this->a()+0.5f, this->r()+0.5f, this->g()+0.5f, this->b()+0.5f);
    SkPMColorAssert(c);
    return c;
}

inline SkPMColor SkPMFloat::roundClamp() const {
    float a = this->a(),
          r = this->r(),
          g = this->g(),
          b = this->b();
    a = a < 0 ? 0 : (a > 255 ? 255 : a);
    r = r < 0 ? 0 : (r > 255 ? 255 : r);
    g = g < 0 ? 0 : (g > 255 ? 255 : g);
    b = b < 0 ? 0 : (b > 255 ? 255 : b);
    SkPMColor c = SkPackARGB32(a+0.5f, r+0.5f, g+0.5f, b+0.5f);
    SkPMColorAssert(c);
    return c;
}

inline void SkPMFloat::From4PMColors(const SkPMColor colors[4],
                                     SkPMFloat* a, SkPMFloat* b, SkPMFloat* c, SkPMFloat* d) {
    *a = FromPMColor(colors[0]);
    *b = FromPMColor(colors[1]);
    *c = FromPMColor(colors[2]);
    *d = FromPMColor(colors[3]);
}

inline void SkPMFloat::RoundTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.round();
    colors[1] = b.round();
    colors[2] = c.round();
    colors[3] = d.round();
}

inline void SkPMFloat::RoundClampTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.roundClamp();
    colors[1] = b.roundClamp();
    colors[2] = c.roundClamp();
    colors[3] = d.roundClamp();
}
