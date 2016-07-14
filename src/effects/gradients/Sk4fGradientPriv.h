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
#include "SkUtils.h"

// Templates shared by various 4f gradient flavors.

namespace {

enum class ApplyPremul { True, False };

enum class DstType {
    L32,  // Linear 32bit.  Used for both shader/blitter paths.
    S32,  // SRGB 32bit.  Used for the blitter path only.
    F16,  // Linear half-float.  Used for blitters only.
    F32,  // Linear float.  Used for shaders only.
};

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

template <ApplyPremul>
struct PremulTraits;

template <>
struct PremulTraits<ApplyPremul::False> {
    static Sk4f apply(const Sk4f& c) { return c; }
};

template <>
struct PremulTraits<ApplyPremul::True> {
    static Sk4f apply(const Sk4f& c) {
        const float alpha = c[SkPM4f::A];
        // FIXME: portable swizzle?
        return c * Sk4f(alpha, alpha, alpha, 1);
    }
};

// Struct encapsulating various dest-dependent ops:
//
//   - load()       Load a SkPM4f value into Sk4f.  Normally called once per interval
//                  advance.  Also applies a scale and swizzle suitable for DstType.
//
//   - store()      Store one Sk4f to dest.  Optionally handles premul, color space
//                  conversion, etc.
//
//   - store(count) Store the Sk4f value repeatedly to dest, count times.
//
//   - store4x()    Store 4 Sk4f values to dest (opportunistic optimization).
//
template <DstType, ApplyPremul premul = ApplyPremul::False>
struct DstTraits;

template <ApplyPremul premul>
struct DstTraits<DstType::L32, premul> {
    using Type = SkPMColor;

    // For L32, we prescale the values by 255 to save a per-pixel multiplication.
    static Sk4f load(const SkPM4f& c) {
        return c.to4f_pmorder() * Sk4f(255);
    }

    static void store(const Sk4f& c, Type* dst) {
        *dst = trunc_from_4f_255<premul>(c);
    }

    static void store(const Sk4f& c, Type* dst, int n) {
        sk_memset32(dst, trunc_from_4f_255<premul>(c), n);
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        Type* dst) {
        if (premul == ApplyPremul::False) {
            Sk4f_ToBytes((uint8_t*)dst, c0, c1, c2, c3);
        } else {
            store(c0, dst + 0);
            store(c1, dst + 1);
            store(c2, dst + 2);
            store(c3, dst + 3);
        }
    }
};

template <ApplyPremul premul>
struct DstTraits<DstType::S32, premul> {
    using PM   = PremulTraits<premul>;
    using Type = SkPMColor;

    static Sk4f load(const SkPM4f& c) {
        // Prescaling by (255^2, 255^2, 255^2, 255) on load, to avoid a 255 multiply on
        // each store (S32 conversion yields a uniform 255 factor).
        return c.to4f_pmorder() * Sk4f(255 * 255, 255 * 255, 255 * 255, 255);
    }

    static void store(const Sk4f& c, Type* dst) {
        // FIXME: this assumes opaque colors.  Handle unpremultiplication.
        *dst = to_4b(linear_to_srgb(PM::apply(c)));
    }

    static void store(const Sk4f& c, Type* dst, int n) {
        sk_memset32(dst, to_4b(linear_to_srgb(PM::apply(c))), n);
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        Type* dst) {
        store(c0, dst + 0);
        store(c1, dst + 1);
        store(c2, dst + 2);
        store(c3, dst + 3);
    }
};

template <ApplyPremul premul>
struct DstTraits<DstType::F16, premul> {
    using PM   = PremulTraits<premul>;
    using Type = uint64_t;

    static Sk4f load(const SkPM4f& c) {
        return c.to4f();
    }

    static void store(const Sk4f& c, Type* dst) {
        *dst = SkFloatToHalf_01(PM::apply(c));
    }

    static void store(const Sk4f& c, Type* dst, int n) {
        sk_memset64(dst, SkFloatToHalf_01(PM::apply(c)), n);
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        Type* dst) {
        store(c0, dst + 0);
        store(c1, dst + 1);
        store(c2, dst + 2);
        store(c3, dst + 3);
    }
};

template <ApplyPremul premul>
struct DstTraits<DstType::F32, premul> {
    using PM   = PremulTraits<premul>;
    using Type = SkPM4f;

    static Sk4f load(const SkPM4f& c) {
        return c.to4f();
    }

    static void store(const Sk4f& c, Type* dst) {
        PM::apply(c).store(dst->fVec);
    }

    static void store(const Sk4f& c, Type* dst, int n) {
        const Sk4f pmc = PM::apply(c);
        for (int i = 0; i < n; ++i) {
            pmc.store(dst[i].fVec);
        }
    }

    static void store4x(const Sk4f& c0, const Sk4f& c1,
                        const Sk4f& c2, const Sk4f& c3,
                        Type* dst) {
        store(c0, dst + 0);
        store(c1, dst + 1);
        store(c2, dst + 2);
        store(c3, dst + 3);
    }
};

} // anonymous namespace

#endif // Sk4fGradientPriv_DEFINED
