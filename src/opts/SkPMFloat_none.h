/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

inline SkPMFloat& SkPMFloat::operator=(const SkPMFloat& that) {
    for (int i = 0; i < 4; i++) { fColor[i] = that.fColor[i]; }
    return *this;
}

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

inline void SkPMFloat::From4PMColors(const SkPMColor colors[4],
                                     SkPMFloat* a, SkPMFloat* b, SkPMFloat* c, SkPMFloat* d) {
    *a = FromPMColor(colors[0]);
    *b = FromPMColor(colors[1]);
    *c = FromPMColor(colors[2]);
    *d = FromPMColor(colors[3]);
}

inline void SkPMFloat::To4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.get();
    colors[1] = b.get();
    colors[2] = c.get();
    colors[3] = d.get();
}

inline void SkPMFloat::ClampTo4PMColors(
        const SkPMFloat& a, const SkPMFloat& b, const SkPMFloat&c, const SkPMFloat& d,
        SkPMColor colors[4]) {
    colors[0] = a.clamped();
    colors[1] = b.clamped();
    colors[2] = c.clamped();
    colors[3] = d.clamped();
}
