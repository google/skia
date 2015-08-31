/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPM_DEFINED
#define SkPM_DEFINED

#include "SkTypes.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkNx.h"

// This file may be included multiple times by .cpp files with different flags, leading
// to different definitions.  Usually that doesn't matter because it's all inlined, but
// in Debug modes the compilers may not inline everything.  So wrap everything in an
// anonymous namespace to give each includer their own silo of this code (or the linker
// will probably pick one randomly for us, which is rarely correct).
namespace {

// A pre-multiplied color storing each component in the same order as SkPMColor,
// but as a float in the range [0, 1].
class SkPMFloat : public Sk4f {
public:
    static SkPMFloat FromPMColor(SkPMColor c) { return SkPMFloat(c); }
    static SkPMFloat FromARGB(float a, float r, float g, float b) { return SkPMFloat(a,r,g,b); }
    static SkPMFloat FromOpaqueColor(SkColor c);  // Requires c's alpha == 0xFF.

    Sk4f alphas() const { return Sk4f(this->a()); }

    // Uninitialized.
    SkPMFloat() {}
    explicit SkPMFloat(SkPMColor c) { *this = Sk4f::FromBytes((uint8_t*)&c) * Sk4f(1.0f/255); }
    SkPMFloat(float a, float r, float g, float b)
    #ifdef SK_PMCOLOR_IS_RGBA
        : INHERITED(r,g,b,a) {}
    #else
        : INHERITED(b,g,r,a) {}
    #endif

    SkPMFloat(const Sk4f& fs) : INHERITED(fs) {}

    float a() const { return this->kth<SK_A32_SHIFT / 8>(); }
    float r() const { return this->kth<SK_R32_SHIFT / 8>(); }
    float g() const { return this->kth<SK_G32_SHIFT / 8>(); }
    float b() const { return this->kth<SK_B32_SHIFT / 8>(); }

    SkPMColor round() const {
        SkPMColor c;
        (*this * Sk4f(255) + Sk4f(0.5f)).toBytes((uint8_t*)&c);
        return c;
    }

    bool isValid() const {
        return this->a() >= 0 && this->a() <= 1
            && this->r() >= 0 && this->r() <= this->a()
            && this->g() >= 0 && this->g() <= this->a()
            && this->b() >= 0 && this->b() <= this->a();
    }

private:
    typedef Sk4f INHERITED;
};

}  // namespace

#endif//SkPM_DEFINED
