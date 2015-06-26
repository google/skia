/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

namespace {  // See SkPMFloat.h

inline SkPMFloat::SkPMFloat(SkPMColor c) {
    float inv255 = 1.0f/255;
    *this = SkPMFloat::FromARGB(SkGetPackedA32(c) * inv255,
                                SkGetPackedR32(c) * inv255,
                                SkGetPackedG32(c) * inv255,
                                SkGetPackedB32(c) * inv255);
    SkASSERT(this->isValid());
}

inline SkPMColor SkPMFloat::round() const {
    float a = this->a(),
          r = this->r(),
          g = this->g(),
          b = this->b();
    a = a < 0 ? 0 : (a > 1 ? 1 : a);
    r = r < 0 ? 0 : (r > 1 ? 1 : r);
    g = g < 0 ? 0 : (g > 1 ? 1 : g);
    b = b < 0 ? 0 : (b > 1 ? 1 : b);
    SkPMColor c = SkPackARGB32(255*a+0.5f, 255*r+0.5f, 255*g+0.5f, 255*b+0.5f);
    SkPMColorAssert(c);
    return c;
}

inline Sk4f SkPMFloat::alphas() const {
    return Sk4f(this->a());
}

}  // namespace
