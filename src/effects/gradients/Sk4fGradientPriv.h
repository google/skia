/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Sk4fGradientPriv_DEFINED
#define Sk4fGradientPriv_DEFINED

#include "SkColor.h"
#include "SkHalf.h"
#include "SkImageInfo.h"
#include "SkNx.h"
#include "SkPM4f.h"
#include "SkPM4fPriv.h"

// Templates shared by various 4f gradient flavors.

namespace {

enum class ApplyPremul { True, False };

inline Sk4f premul_4f(const Sk4f& c) {
    const float alpha = c[SkPM4f::A];
    // FIXME: portable swizzle?
    return c * Sk4f(alpha, alpha, alpha, 1);
}

template <ApplyPremul premul>
inline SkPMColor trunc_from_4f_255(const Sk4f& c) {
    SkPMColor pmc;
    SkNx_cast<uint8_t>(c).store(&pmc);
    if (premul == ApplyPremul::True) {
        pmc = SkPreMultiplyARGB(SkGetPackedA32(pmc), SkGetPackedR32(pmc),
                                SkGetPackedG32(pmc), SkGetPackedB32(pmc));
    }
    return pmc;
}

template<typename DstType, SkColorProfileType, ApplyPremul premul>
void store(const Sk4f& color, DstType* dst);

template<>
inline void store<SkPM4f, kLinear_SkColorProfileType, ApplyPremul::False>
                 (const Sk4f& c, SkPM4f* dst) {
    c.store(dst);
}

template<>
inline void store<SkPM4f, kLinear_SkColorProfileType, ApplyPremul::True>
                 (const Sk4f& c, SkPM4f* dst) {
    premul_4f(c).store(dst);
}

template<>
inline void store<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::False>
                 (const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_4f_255<ApplyPremul::False>(c);
}

template<>
inline void store<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::True>
                 (const Sk4f& c, SkPMColor* dst) {
    *dst = trunc_from_4f_255<ApplyPremul::True>(c);
}

template<>
inline void store<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::False>
                 (const Sk4f& c, SkPMColor* dst) {
    // FIXME: this assumes opaque colors.  Handle unpremultiplication.
    *dst = Sk4f_toS32(c);
}

template<>
inline void store<SkPMColor, kSRGB_SkColorProfileType, ApplyPremul::True>
                 (const Sk4f& c, SkPMColor* dst) {
    *dst = Sk4f_toS32(premul_4f(c));
}

template<>
inline void store<uint64_t, kLinear_SkColorProfileType, ApplyPremul::False>
                 (const Sk4f& c, uint64_t* dst) {
    *dst = SkFloatToHalf_01(c);
}

template<>
inline void store<uint64_t, kLinear_SkColorProfileType, ApplyPremul::True>
                 (const Sk4f& c, uint64_t* dst) {
    *dst = SkFloatToHalf_01(premul_4f(c));
}

template<typename DstType, SkColorProfileType profile, ApplyPremul premul>
inline void store4x(const Sk4f& c0,
                    const Sk4f& c1,
                    const Sk4f& c2,
                    const Sk4f& c3,
                    DstType* dst) {
    store<DstType, profile, premul>(c0, dst++);
    store<DstType, profile, premul>(c1, dst++);
    store<DstType, profile, premul>(c2, dst++);
    store<DstType, profile, premul>(c3, dst++);
}

template<>
inline void store4x<SkPMColor, kLinear_SkColorProfileType, ApplyPremul::False>
                   (const Sk4f& c0, const Sk4f& c1,
                    const Sk4f& c2, const Sk4f& c3,
                    SkPMColor* dst) {
    Sk4f_ToBytes((uint8_t*)dst, c0, c1, c2, c3);
}

template<typename DstType, SkColorProfileType>
Sk4f scale_for_dest(const Sk4f& c)  {
    return c;
}

template<>
inline Sk4f scale_for_dest<SkPMColor, kLinear_SkColorProfileType>(const Sk4f& c)  {
    return c * 255;
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

template<>
inline Sk4f dst_swizzle<uint64_t>(const SkPM4f& c) {
    return c.to4f();
}

}

#endif // Sk4fGradientPriv_DEFINED
