/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientPriv_DEFINED
#define Sk4fGradientPriv_DEFINED

#include "SkColor.h"
#include "SkNx.h"
#include "SkPM4f.h"

// Templates shared by various 4f gradient flavors.

namespace {

inline Sk4f premul_4f(const Sk4f& c) {
    const float alpha = c[SkPM4f::A];
    // FIXME: portable swizzle?
    return c * Sk4f(alpha, alpha, alpha, 1);
}

template <bool do_premul>
inline SkPMColor trunc_from_255(const Sk4f& c) {
    SkPMColor pmc;
    SkNx_cast<uint8_t>(c).store(&pmc);
    if (do_premul) {
        pmc = SkPreMultiplyARGB(SkGetPackedA32(pmc), SkGetPackedR32(pmc),
                                SkGetPackedG32(pmc), SkGetPackedB32(pmc));
    }
    return pmc;
}

template<typename DstType, bool do_premul>
void store(const Sk4f& color, DstType* dst);

template<>
inline void store<SkPM4f, false>(const Sk4f& c, SkPM4f* dst) {
    c.store(dst);
}

template<>
inline void store<SkPM4f, true>(const Sk4f& c, SkPM4f* dst) {
    store<SkPM4f, false>(premul_4f(c), dst);
}

template<>
inline void store<SkPMColor, false>(const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_255<false>(c);
}

template<>
inline void store<SkPMColor, true>(const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_255<true>(c);
}

template<typename DstType, bool do_premul>
inline void store4x(const Sk4f& c0,
                    const Sk4f& c1,
                    const Sk4f& c2,
                    const Sk4f& c3,
                    DstType* dst) {
    store<DstType, do_premul>(c0, dst++);
    store<DstType, do_premul>(c1, dst++);
    store<DstType, do_premul>(c2, dst++);
    store<DstType, do_premul>(c3, dst++);
}

template<>
inline void store4x<SkPMColor, false>(const Sk4f& c0,
                                      const Sk4f& c1,
                                      const Sk4f& c2,
                                      const Sk4f& c3,
                                      SkPMColor* dst) {
    Sk4f_ToBytes((uint8_t*)dst, c0, c1, c2, c3);
}

template<typename DstType>
float dst_component_scale();

template<>
inline float dst_component_scale<SkPM4f>() {
    return 1;
}

template<>
inline float dst_component_scale<SkPMColor>() {
    return 255;
}

template<typename DstType>
Sk4f dst_swizzle(const SkPM4f&);

template<>
inline Sk4f dst_swizzle<SkPM4f>(const SkPM4f& c) {
    return c.to4f();
}

template<>
inline Sk4f dst_swizzle<SkPMColor>(const SkPM4f& c) {
    return c.to4f_pmorder();
}

}

#endif // Sk4fGradientPriv_DEFINED
